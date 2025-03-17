#include "tiny_ptr_simple.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <math.h>

#define TINY_PTR_MAX_BUCKET_SIZE 32

/* 
 * Hash function with seed (mixing similar to MurmurHash3 finalizer).
 */
static inline uint32_t hash_int_with_seed(int key, uint32_t seed) {
    uint32_t h = (uint32_t) key;
    h ^= seed;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

// Returns the index of the least–significant set bit.
static inline int find_first_free(uint32_t free_mask) {
    return free_mask ? __builtin_ctz(free_mask) : -1;
}

// Computes the next power of two greater than or equal to x.
static size_t next_power_of_two(size_t x) {
    size_t power = 1;
    while (power < x)
        power *= 2;
    return power;
}

// Computes floor(log2(x)) for x > 0.
static int int_log2(int x) {
    int log = 0;
    while (x >>= 1)
        log++;
    return log;
}

/* The SimpleTable structure now stores a target load factor.
   The table is allocated so that total_slots >= ceil(capacity/load_factor). */
struct SimpleTable {
    size_t requested_capacity;  /* The maximum number of items intended */
    size_t total_slots;         /* Total slots available (bucket_count * bucket_size) */
    size_t bucket_count;        /* Number of buckets (power of 2) */
    size_t bucket_size;         /* Number of slots per bucket */
    int *store;                 /* Array storing the values */
    int *keys;                  /* Array storing the keys (-1 indicates free) */
    uint32_t *bucket_free;      /* Bitmask per bucket: bit set means free */
    uint32_t hash_seed;         /* Seed used in the hash function */
    double load_factor;         /* Target load factor (e.g., 0.9) */
    pthread_mutex_t mutex;      /* Mutex for thread safety */
};

SimpleTable* simple_create_ex(size_t capacity, double load_factor) {
    if (capacity == 0 || load_factor <= 0 || load_factor > 1.0) return NULL;
    SimpleTable *st = malloc(sizeof(SimpleTable));
    if (!st) return NULL;
    st->requested_capacity = capacity;
    st->load_factor = load_factor;
    /* Choose bucket size based on capacity; enforce a minimum of 8 slots per bucket */
    int bs = int_log2(capacity);
    bs = bs / 2;
    if (bs < 8) bs = 8;
    if (bs > TINY_PTR_MAX_BUCKET_SIZE)
        bs = TINY_PTR_MAX_BUCKET_SIZE;
    st->bucket_size = (size_t)bs;
    /* Compute minimum slots so that capacity/slots <= load_factor */
    size_t min_slots = (size_t) ceil((double) capacity / load_factor);
    size_t desired_buckets = (min_slots + st->bucket_size - 1) / st->bucket_size;
    st->bucket_count = next_power_of_two(desired_buckets);
    st->total_slots = st->bucket_count * st->bucket_size;
    st->store = calloc(st->total_slots, sizeof(int));
    st->keys = malloc(st->total_slots * sizeof(int));
    st->bucket_free = malloc(st->bucket_count * sizeof(uint32_t));
    if (!st->store || !st->keys || !st->bucket_free) {
        free(st->store); free(st->keys); free(st->bucket_free);
        free(st);
        return NULL;
    }
    for (size_t i = 0; i < st->total_slots; i++) {
        st->keys[i] = -1;  // Mark slot as free.
    }
    for (size_t i = 0; i < st->bucket_count; i++) {
        st->bucket_free[i] = ((1U << st->bucket_size) - 1);
    }
    pthread_mutex_init(&st->mutex, NULL);
    /* Set the hash seed to depend on the requested capacity */
    st->hash_seed = ((uint32_t) capacity) ^ 0x9e3779b9;
    return st;
}

void simple_destroy(SimpleTable *st) {
    if (!st) return;
    pthread_mutex_destroy(&st->mutex);
    free(st->store);
    free(st->keys);
    free(st->bucket_free);
    free(st);
}

int simple_allocate(SimpleTable *st, int key, int value) {
    if (!st) return -1;
    pthread_mutex_lock(&st->mutex);
    uint32_t h = hash_int_with_seed(key, st->hash_seed);
    int bucket = h & (st->bucket_count - 1);
    uint32_t free_mask = st->bucket_free[bucket];
    if (free_mask == 0) {
        pthread_mutex_unlock(&st->mutex);
        return -1;
    }
    int slot_offset = find_first_free(free_mask);
    if (slot_offset < 0) {
        pthread_mutex_unlock(&st->mutex);
        return -1;
    }
    st->bucket_free[bucket] &= ~(1U << slot_offset);
    size_t index = bucket * st->bucket_size + slot_offset;
    st->store[index] = value;
    st->keys[index] = key;
    pthread_mutex_unlock(&st->mutex);
    return slot_offset;
}

int simple_dereference(SimpleTable *st, int key, int tiny_ptr) {
    if (!st) return -1;
    pthread_mutex_lock(&st->mutex);
    uint32_t h = hash_int_with_seed(key, st->hash_seed);
    int bucket = h & (st->bucket_count - 1);
    size_t index = bucket * st->bucket_size + tiny_ptr;
    int ret = st->store[index];
    pthread_mutex_unlock(&st->mutex);
    return ret;
}

void simple_free(SimpleTable *st, int key, int tiny_ptr) {
    if (!st) return;
    pthread_mutex_lock(&st->mutex);
    uint32_t h = hash_int_with_seed(key, st->hash_seed);
    int bucket = h & (st->bucket_count - 1);
    size_t index = bucket * st->bucket_size + tiny_ptr;
    st->keys[index] = -1;
    st->store[index] = 0;  // Optionally clear the value.
    st->bucket_free[bucket] |= (1U << tiny_ptr);
    pthread_mutex_unlock(&st->mutex);
}

/*
 * simple_resize creates a new SimpleTable with new_capacity and the same load factor,
 * rehashes all allocated entries from the old table into the new table,
 * and destroys the old table.
 */
SimpleTable* simple_resize(SimpleTable *old_st, size_t new_capacity) {
    if (!old_st) return NULL;
    SimpleTable *new_st = simple_create_ex(new_capacity, old_st->load_factor);
    if (!new_st) return NULL;

    pthread_mutex_lock(&old_st->mutex);
    pthread_mutex_lock(&new_st->mutex);
    for (size_t i = 0; i < old_st->total_slots; i++) {
        if (old_st->keys[i] != -1) {
            int key = old_st->keys[i];
            int value = old_st->store[i];
            uint32_t h = hash_int_with_seed(key, new_st->hash_seed);
            int bucket = h & (new_st->bucket_count - 1);
            uint32_t free_mask = new_st->bucket_free[bucket];
            if (free_mask == 0) {
                pthread_mutex_unlock(&new_st->mutex);
                pthread_mutex_unlock(&old_st->mutex);
                simple_destroy(new_st);
                return NULL;
            }
            int slot_offset = find_first_free(free_mask);
            if (slot_offset < 0) {
                pthread_mutex_unlock(&new_st->mutex);
                pthread_mutex_unlock(&old_st->mutex);
                simple_destroy(new_st);
                return NULL;
            }
            new_st->bucket_free[bucket] &= ~(1U << slot_offset);
            size_t new_index = bucket * new_st->bucket_size + slot_offset;
            new_st->store[new_index] = value;
            new_st->keys[new_index] = key;
        }
    }
    pthread_mutex_unlock(&new_st->mutex);
    pthread_mutex_unlock(&old_st->mutex);
    simple_destroy(old_st);
    return new_st;
}

/* 
 * Alias for legacy code: simple_create calls simple_create_ex with default load factor 0.9.
 */
SimpleTable* simple_create(size_t capacity) {
    return simple_create_ex(capacity, 0.9);
}
