#ifndef TINY_PTR_SIMPLE_H
#define TINY_PTR_SIMPLE_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SimpleTable SimpleTable;

/* Extended creation: accepts a load factor */
SimpleTable* simple_create_ex(size_t capacity, double load_factor);
void simple_destroy(SimpleTable* st);
int simple_allocate(SimpleTable* st, int key, int value);
int simple_dereference(SimpleTable* st, int key, int tiny_ptr);
void simple_free(SimpleTable* st, int key, int tiny_ptr);
SimpleTable* simple_resize(SimpleTable* st, size_t new_capacity);

/* Alias for legacy code: simple_create calls simple_create_ex with a default load factor */
SimpleTable* simple_create(size_t capacity);

#ifdef __cplusplus
}
#endif

#endif /* TINY_PTR_SIMPLE_H */
