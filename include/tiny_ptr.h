#ifndef TINY_PTR_H
#define TINY_PTR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TINY_PTR_SIMPLE,
    TINY_PTR_FIXED,
    TINY_PTR_VARIABLE
} TinyPtrVariant;

typedef struct tiny_ptr_table_t {
    TinyPtrVariant variant;
    void* table;  // For the simple variant, this points to a SimpleTable.
} tiny_ptr_table_t;

/* Unified interface */
tiny_ptr_table_t* tiny_ptr_create(size_t capacity, TinyPtrVariant variant, double load_factor);
int tiny_ptr_allocate(tiny_ptr_table_t* table, int key, int value);
int tiny_ptr_dereference(tiny_ptr_table_t* table, int key, int tiny_ptr);
void tiny_ptr_free(tiny_ptr_table_t* table, int key, int tiny_ptr);
int tiny_ptr_resize(tiny_ptr_table_t** table, size_t new_capacity);
void tiny_ptr_destroy(tiny_ptr_table_t* table);

#ifdef __cplusplus
}
#endif

#endif /* TINY_PTR_H */
