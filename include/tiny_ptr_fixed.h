#ifndef TINY_PTR_FIXED_H
#define TINY_PTR_FIXED_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque type for FixedTable */
typedef struct FixedTable FixedTable;

/* Create a FixedTable with given total capacity and load factor */
FixedTable* fixed_create(size_t total_capacity, double load_factor);

/* Destroy a FixedTable */
void fixed_destroy(FixedTable *ft);

/* Allocate an entry in a FixedTable */
int fixed_allocate(FixedTable *ft, int key, int value);

/* Dereference an entry in a FixedTable */
int fixed_dereference(FixedTable *ft, int key, int tiny_ptr);

/* Free an entry in a FixedTable */
void fixed_free(FixedTable *ft, int key, int tiny_ptr);

#ifdef __cplusplus
}
#endif

#endif /* TINY_PTR_FIXED_H */
