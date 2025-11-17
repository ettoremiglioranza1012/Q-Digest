#ifndef ALLREDUCE_H
#define ALLREDUCE_H

#include <stdlib.h>
#include "../../include/qcore.h"

void tree_reduce(
    size_t *data,
    size_t length,
    int comm_size,
    int rank);

#endif