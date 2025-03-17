#include "tiny_ptr_fixed.h"
#include "tiny_ptr_simple.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

struct FixedTable {
    SimpleTable *primary;
    SimpleTable *secondary;
    size_t primary_capacity;
    size_t secondary_capacity;
    pthread_mutex_t mutex;
};

FixedTable* fixed_create(size_t total_capacity, double load_factor) {
    FixedTable *ft = malloc(sizeof(FixedTable));
    if (!ft) return NULL;
    ft->primary_capacity = (size_t)(total_capacity * 0.90);
    ft->secondary_capacity = total_capacity - ft->primary_capacity;
    ft->primary = simple_create(ft->primary_capacity);
    ft->secondary = simple_create(ft->secondary_capacity);
    if (!ft->primary || !ft->secondary) {
        simple_destroy(ft->primary);
        simple_destroy(ft->secondary);
        free(ft);
        return NULL;
    }
    pthread_mutex_init(&ft->mutex, NULL);
    return ft;
}

void fixed_destroy(FixedTable *ft) {
    if (!ft) return;
    pthread_mutex_destroy(&ft->mutex);
    simple_destroy(ft->primary);
    simple_destroy(ft->secondary);
    free(ft);
}

int fixed_allocate(FixedTable *ft, int key, int value) {
    if (!ft) return -1;
    pthread_mutex_lock(&ft->mutex);
    int tp = simple_allocate(ft->primary, key, value);
    int encoded;
    if (tp != -1) {
        encoded = (tp << 1) | 0;  /* flag 0 indicates primary table */
        pthread_mutex_unlock(&ft->mutex);
        return encoded;
    }
    tp = simple_allocate(ft->secondary, key, value);
    if (tp != -1) {
        encoded = (tp << 1) | 1;  /* flag 1 indicates secondary table */
        pthread_mutex_unlock(&ft->mutex);
        return encoded;
    }
    pthread_mutex_unlock(&ft->mutex);
    return -1;
}

int fixed_dereference(FixedTable *ft, int key, int tiny_ptr) {
    if (!ft) return -1;
    int flag = tiny_ptr & 1;
    int offset = tiny_ptr >> 1;
    int ret;
    pthread_mutex_lock(&ft->mutex);
    if (flag == 0)
        ret = simple_dereference(ft->primary, key, offset);
    else
        ret = simple_dereference(ft->secondary, key, offset);
    pthread_mutex_unlock(&ft->mutex);
    return ret;
}

void fixed_free(FixedTable *ft, int key, int tiny_ptr) {
    if (!ft) return;
    int flag = tiny_ptr & 1;
    int offset = tiny_ptr >> 1;
    pthread_mutex_lock(&ft->mutex);
    if (flag == 0)
        simple_free(ft->primary, key, offset);
    else
        simple_free(ft->secondary, key, offset);
    pthread_mutex_unlock(&ft->mutex);
}
