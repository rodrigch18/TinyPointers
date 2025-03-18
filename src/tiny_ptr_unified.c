#include "tiny_ptr_unified.h"
#include "tiny_ptr_simple.h"
#include "tiny_ptr_fixed.h"
#include "tiny_ptr_variable.h"
#include <stdlib.h>

tiny_ptr_table_t* tiny_ptr_create(size_t capacity, TinyPtrVariant variant, double load_factor) {
    tiny_ptr_table_t* ut = malloc(sizeof(tiny_ptr_table_t));
    if (!ut) return NULL;
    ut->variant = variant;
    switch (variant) {
        case TINY_PTR_SIMPLE:
            /* For the simple variant, use our extended create function */
            ut->table = simple_create_ex(capacity, load_factor);
            break;
        case TINY_PTR_FIXED:
            ut->table = fixed_create(capacity, load_factor);
            break;
        case TINY_PTR_VARIABLE: {
            size_t container_capacity = capacity / 4;
            if (container_capacity == 0) container_capacity = 1;
            size_t level_count = 4;  // default level count
            ut->table = variable_create(capacity, container_capacity, level_count);
            break;
        }
        default:
            free(ut);
            return NULL;
    }
    if (!ut->table) {
        free(ut);
        return NULL;
    }
    return ut;
}

int tiny_ptr_allocate(tiny_ptr_table_t* ut, int key, int value) {
    if (!ut) return -1;
    switch (ut->variant) {
        case TINY_PTR_SIMPLE:
            return simple_allocate((SimpleTable*) ut->table, key, value);
        case TINY_PTR_FIXED:
            return fixed_allocate((struct FixedTable*) ut->table, key, value);
        case TINY_PTR_VARIABLE:
            return variable_allocate((struct VariableTable*) ut->table, key, value);
        default:
            return -1;
    }
}

int tiny_ptr_dereference(tiny_ptr_table_t* ut, int key, int tiny_ptr) {
    if (!ut) return -1;
    switch (ut->variant) {
        case TINY_PTR_SIMPLE:
            return simple_dereference((SimpleTable*) ut->table, key, tiny_ptr);
        case TINY_PTR_FIXED:
            return fixed_dereference((struct FixedTable*) ut->table, key, tiny_ptr);
        case TINY_PTR_VARIABLE:
            return variable_dereference((struct VariableTable*) ut->table, key, tiny_ptr);
        default:
            return -1;
    }
}

void tiny_ptr_free(tiny_ptr_table_t* ut, int key, int tiny_ptr) {
    if (!ut) return;
    switch (ut->variant) {
        case TINY_PTR_SIMPLE:
            simple_free((SimpleTable*) ut->table, key, tiny_ptr);
            break;
        case TINY_PTR_FIXED:
            fixed_free((struct FixedTable*) ut->table, key, tiny_ptr);
            break;
        case TINY_PTR_VARIABLE:
            variable_free((struct VariableTable*) ut->table, key, tiny_ptr);
            break;
    }
}

/* Only the simple variant supports resizing. */
int tiny_ptr_resize(tiny_ptr_table_t** ut_ptr, size_t new_capacity) {
    if (!ut_ptr || !(*ut_ptr)) return -1;
    tiny_ptr_table_t* ut = *ut_ptr;
    if (ut->variant != TINY_PTR_SIMPLE)
        return -1; // Resizing is not supported for fixed or variable variants.
    SimpleTable* st = (SimpleTable*) ut->table;
    SimpleTable* new_st = simple_resize(st, new_capacity);
    if (!new_st)
        return -1;
    ut->table = new_st;
    return 0;
}

void tiny_ptr_destroy(tiny_ptr_table_t* ut) {
    if (!ut) return;
    switch (ut->variant) {
        case TINY_PTR_SIMPLE:
            simple_destroy((SimpleTable*) ut->table);
            break;
        case TINY_PTR_FIXED:
            fixed_destroy((struct FixedTable*) ut->table);
            break;
        case TINY_PTR_VARIABLE:
            variable_destroy((struct VariableTable*) ut->table);
            break;
    }
    free(ut);
}
