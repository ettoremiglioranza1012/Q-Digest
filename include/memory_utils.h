/**
 *  @file This header file contains memory allocation utilities, mainly
 *  wrappers implemented around stdlib functions.
 *
 * */
#ifndef MEMORY_UTILS
#define MEMORY_UTILS
#include <stdio.h>
#include <stdlib.h>

/** 
 *  @brief A function that checks whether the memory allocation with
 *  malloc returned a NULL pointer implying an error with the memory
 *  allocation. If an error occurs the program safely exits returning
 *  an error code.
 *
 *  @param size The size (number of bytes) that should be dynamically
 *  allocated.
 *
 * */
void *xmalloc(size_t size);
#endif
