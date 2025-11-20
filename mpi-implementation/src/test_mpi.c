#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../../include/qcore.h"
#include "../include/tree_reduce.h"
#include "../../include/memory_utils.h"

#define NUMS 1024

void debug_distribute_data_array(int *local_buf, int local_n, int rank, int buf_size, MPI_Comm comm)
{
    printf("DEBUG: distribute data array function\n");
    distribute_data_array(local_buf, local_n, rank, NUMS, MPI_COMM_WORLD);
    printf("\n[Rank %d] Size of local buf:%d\n", rank, local_n);
    printf("[DEBUG] local buffer of process %d\n", rank);
    for (int i = 0; i < 10; i++) printf("[%d]:%d\n",i,local_buf[i]);
    MPI_Barrier(MPI_COMM_WORLD);
    printf("\n");
}

void debug_build_q_from_vector(int *local_buf, int local_n)
{
    printf("DEBUG: build q from vector\n");
    size_t upper_bound = _get_curr_upper_bound(local_buf, local_n);
    struct QDigest *q = _build_q_from_vector(local_buf, local_n, upper_bound);
    size_t size = get_num_of_bytes(q);
    char *serial_q = xmalloc(size);
    size_t length = 0;
    to_string(q, serial_q, &length);
    printf("DEBUG: built q dig:\n%s\n", serial_q);
    assert(q->root->upper_bound == upper_bound);
}

int main(void) {
    int rank, comm_sz;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int local_n = NUMS/comm_sz;
    int local_buf[local_n];
    MPI_Barrier(MPI_COMM_WORLD);

    debug_distribute_data_array(local_buf, local_n, rank, NUMS, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    debug_build_q_from_vector(local_buf, local_n);
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    printf("All test passed!\n");
    return 0;
}


