#ifndef __DARRAY_H__
#define __DARRAY_H__

#include <stdlib.h>
#include "../include/memory_utils.h"

typedef size_t DAItem;

typedef struct Array {
    DAItem *data;
    size_t size;
    size_t capacity; 
} Array;

void init_array(Array *a, size_t initial_capacity);
void push_back(Array *a, int value);
size_t get_length(Array *a);
void free_array(Array *a);

#endif /* __DARRAY_H__ */