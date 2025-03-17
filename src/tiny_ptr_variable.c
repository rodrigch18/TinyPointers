#include "tiny_ptr_variable.h"
#include "tiny_ptr_simple.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

typedef struct {
    size_t level_count;
    SimpleTable **levels;
} Container;

static Container* container_create(size_t container_capacity, size_t level_count) {
    Container *c = malloc(sizeof(Container));
    if (!c) return NULL;
    c->level_count = level_count;
    c->levels = malloc(level_count * sizeof(SimpleTable*));
    if (!c->levels) { free(c); return NULL; }
    size_t level_capacity = container_capacity / level_count;
    if (level_capacity == 0) level_capacity = 1;
    for (size_t i = 0; i < level_count; i++) {
        c->levels[i] = simple_create(level_capacity);
        if (!c->levels[i]) {
            for (size_t j = 0; j < i; j++)
                simple_destroy(c->levels[j]);
            free(c->levels);
            free(c);
            return NULL;
        }
    }
    return c;
}

/* Revised container_destroy: do not free the container itself if it was copied into an array */
static void container_destroy(Container *c) {
    if (!c) return;
    for (size_t i = 0; i < c->level_count; i++) {
        simple_destroy(c->levels[i]);
    }
    free(c->levels);
    /* Do not free(c) since the container struct is stored in an external array */
}

struct VariableTable {
    size_t container_count;
    Container *containers;   /* Array of Container structs */
    pthread_mutex_t mutex;
};

VariableTable* variable_create(size_t total_capacity, size_t container_capacity, size_t level_count) {
    VariableTable *vt = malloc(sizeof(VariableTable));
    if (!vt) return NULL;
    vt->container_count = (total_capacity + container_capacity - 1) / container_capacity;
    vt->containers = malloc(vt->container_count * sizeof(Container));
    if (!vt->containers) { free(vt); return NULL; }
    for (size_t i = 0; i < vt->container_count; i++) {
        Container *c = container_create(container_capacity, level_count);
        if (!c) {
            for (size_t j = 0; j < i; j++)
                container_destroy(&vt->containers[j]);
            free(vt->containers);
            free(vt);
            return NULL;
        }
        vt->containers[i] = *c;  // Copy container structure into array.
        free(c);  // Free the temporary allocation.
    }
    pthread_mutex_init(&vt->mutex, NULL);
    return vt;
}

void variable_destroy(VariableTable *vt) {
    if (!vt) return;
    pthread_mutex_destroy(&vt->mutex);
    for (size_t i = 0; i < vt->container_count; i++) {
        container_destroy(&vt->containers[i]);
    }
    free(vt->containers);
    free(vt);
}

int variable_allocate(VariableTable *vt, int key, int value) {
    if (!vt) return -1;
    pthread_mutex_lock(&vt->mutex);
    uint32_t h = 0;
    {   /* Simple hash for int keys */
        uint32_t k = (uint32_t) key;
        k ^= k >> 16;
        k *= 0x85ebca6b;
        k ^= k >> 13;
        k *= 0xc2b2ae35;
        k ^= k >> 16;
        h = k;
    }
    int container_index = h % vt->container_count;
    Container *c = &vt->containers[container_index];
    int tp = -1, level_found = -1;
    for (size_t level = 0; level < c->level_count; level++) {
        tp = simple_allocate(c->levels[level], key, value);
        if (tp != -1) {
            level_found = (int)level;
            break;
        }
    }
    int tiny_ptr = -1;
    if (tp != -1 && level_found != -1) {
        tiny_ptr = ((container_index & 0xFF) << 8) | ((level_found & 0xF) << 4) | (tp & 0xF);
    }
    pthread_mutex_unlock(&vt->mutex);
    return tiny_ptr;
}

int variable_dereference(VariableTable *vt, int key, int tiny_ptr) {
    if (!vt) return -1;
    uint32_t container_index = (tiny_ptr >> 8) & 0xFF;
    uint32_t level = (tiny_ptr >> 4) & 0xF;
    uint32_t tp = tiny_ptr & 0xF;
    pthread_mutex_lock(&vt->mutex);
    Container *c = &vt->containers[container_index];
    int ret = simple_dereference(c->levels[level], key, tp);
    pthread_mutex_unlock(&vt->mutex);
    return ret;
}

void variable_free(VariableTable *vt, int key, int tiny_ptr) {
    if (!vt) return;
    uint32_t container_index = (tiny_ptr >> 8) & 0xFF;
    uint32_t level = (tiny_ptr >> 4) & 0xF;
    uint32_t tp = tiny_ptr & 0xF;
    pthread_mutex_lock(&vt->mutex);
    Container *c = &vt->containers[container_index];
    simple_free(c->levels[level], key, tp);
    pthread_mutex_unlock(&vt->mutex);
}
