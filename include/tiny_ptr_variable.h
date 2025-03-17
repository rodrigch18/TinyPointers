#ifndef TINY_PTR_VARIABLE_H
#define TINY_PTR_VARIABLE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque type for VariableTable */
typedef struct VariableTable VariableTable;

/* Create a VariableTable with total_capacity, container_capacity, and level_count */
VariableTable* variable_create(size_t total_capacity, size_t container_capacity, size_t level_count);

/* Destroy a VariableTable */
void variable_destroy(VariableTable *vt);

/* Allocate an entry in a VariableTable */
int variable_allocate(VariableTable *vt, int key, int value);

/* Dereference an entry in a VariableTable */
int variable_dereference(VariableTable *vt, int key, int tiny_ptr);

/* Free an entry in a VariableTable */
void variable_free(VariableTable *vt, int key, int tiny_ptr);

#ifdef __cplusplus
}
#endif

#endif /* TINY_PTR_VARIABLE_H */
