/** 
 *  @file This header file includes struct declarations and
 *  function prototypes to implement a dynamic array.
 *
 * */

#ifndef DYNAMIC_ARRAY
#define DYNAMIC_ARRAY
#include "../include/memory_utils.h"
#include <stdlib.h>

/** 
 *  @brief create an alias for the size_t type to be explicit
 *  about the fact that variables declared with this type will
 *  belong inside the dynamic array.
 *
 * */
typedef size_t DAItem;

/** 
 *  @brief Declares a dynamic array struct and aliases with the
 *  `Array` type.
 *
 * */
typedef struct {
  DAItem *data;     /**< A pointer to a dynamically allocated array. */
  size_t size;      /**< The current size of the list (positive integer). */
  size_t capacity;  /**< The maximum number of elements that can be contained in the `Array`. This parameter grows in powers of two. every time the maximum is reached. */
} Array;

/** 
 *  @brief Initialize an array capable of containing `initial_capacity`
 *  elements.
 *
 *  @param a A pointer to an `Array` which should be initialized.
 *
 *  @param initial_capacity A positive integer indicating how many
 *  elements can be added to the array after initialization.
 *
 * */
void init_array(Array *a, size_t initial_capacity);

/** 
 *  @brief Implements push_back, a function that appends an element
 *  to the `Array` after checking if there is enough space.
 *  In case an append operation is about to be performed but the array
 *  has reached its max capacity, the data pointer expands using `realloc`
 *  effectively making space for new data to be appended to the end
 *  of the list.
 *
 *  @param a A pointer to an `Array` where data should be appended
 *
 *  @param value An integer to add to the `Array`
 *
 * */
void push_back(Array *a, int value);

/** 
 *  @brief Returns the length of the `Array`.
 *
 *  @param a A pointer to an `Array` whose length should be
 *  returned.
 *
 * */
size_t get_length(Array *a);

/** 
 *  @brief Implements a destructor of the Array which frees dynamically
 *  allocated memory.
 *
 *  @param a A pointer to an `Array` whose memory needs to be freed.
 *
 * */
void free_array(Array *a);

#endif
