#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/tree_reduce.h"

/* NOTE: These are test parameters and should be removed in 
 * favor of proper user-based I/O */
// how many numbers to generate
// also the size of the array (vector) that stores them in process 0
#define NUMS 1024

/* ============== MAIN FUNCTION ======================== */
int main(void) 
{
    int rank, comm_sz;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int local_n = NUMS/comm_sz;
    int local_buf[local_n];

    // Scatter the array around the nodes
    if (rank == 0) {
        printf("[rank %d] starting data distribution\n", rank);
    }
    MPI_Barrier(MPI_COMM_WORLD); // DEBUGGING
    distribute_data_array(local_buf, local_n, rank, NUMS, MPI_COMM_WORLD);
    printf("[rank %d] finished scatter, building local digest\n", rank);
    MPI_Barrier(MPI_COMM_WORLD); // DEBUGGING 

    // From the data buffer create the q-digest
    size_t upper_bound = _get_curr_upper_bound(local_buf, local_n);
    struct QDigest *q = _build_q_from_vector(local_buf, local_n, upper_bound);
    printf("[rank %d] built q-digest, starting tree_reduce\n", rank);
    MPI_Barrier(MPI_COMM_WORLD); // DEBUGGING 

    // data get inserted into qdigest and then compressed, ecc...
    tree_reduce(q, comm_sz, rank, MPI_COMM_WORLD);
    printf("[rank %d] tree_reduce completed\n", rank);
    MPI_Barrier(MPI_COMM_WORLD); // DEBUGGING 

    MPI_Finalize();
    printf("Apparenly alla worked fine!\n"); // DEBUGGING 
    return 0;
}