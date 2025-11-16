#include "../include/dynamic_array.h"

void init_array(Array *a, size_t initial_capacity) {
  a->data = (DAItem *)xmalloc(initial_capacity * sizeof(DAItem));
  a->size = 0;
  a->capacity = initial_capacity;
}

void push_back(Array *a, int value) {
  if (a->size == a->capacity) {
    a->capacity *= 2;
    a->data = (DAItem *)realloc(a->data, a->capacity * sizeof(DAItem));
  }
  a->data[a->size++] = value;
}

size_t get_length(Array *a) { return a->size; }

void free_array(Array *a) {
    if (!a) return;
    free(a->data);
    free(a);
}
