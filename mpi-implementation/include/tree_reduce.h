#ifndef TREE_REDUCE_H
#define TREE_REDUCE_H

#include <stdlib.h>
#include <mpi.h>
#include "../../include/qcore.h"

void initialize_data_array(int rank, int *data, int n);
int *distribute_data_array(int *local_buf, int local_n, int rank, int buf_size, MPI_Comm comm);
void tree_reduce(struct QDigest *q, int comm_size, int rank, MPI_Comm comm);

#endif