#ifndef DYNAMIC_ARRAY
#define DYNAMIC_ARRAY
#include "../include/memory_utils.h"
#include <stdlib.h>

typedef size_t DAItem;

typedef struct {
  DAItem *data;
  size_t size;
  size_t capacity;
} Array;

void init_array(Array *a, size_t initial_capacity);

void push_back(Array *a, int value);

size_t get_length(Array *a);

void free_array(Array *a);

#endif
