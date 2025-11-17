#ifndef __ALLREDUCE_H__
#define __ALLREDUCE_H__

#include <stdlib.h>
#include "../include/qcore.h"

typedef struct QDigest *Item;

void TreeAllreduce(
    size_t *data,
    size_t length,
    int comm_size,
    int rank);

#endif