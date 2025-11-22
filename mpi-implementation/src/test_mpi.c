#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/tree_reduce.h"
#include "../../include/memory_utils.h"
#include "../../include/qcore.h"

/* NOTE: These are test parameters and should be removed in 
 * favor of proper user-based I/O */
// how many numbers to generate
// also the size of the array (vector) that stores them in process 0
#define DATA_SIZE 1024
#define K 1

/* ============== MAIN FUNCTION ======================== */
int main(void) 
{
    int rank, comm_sz;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    /* Handling possible rest division in n/comm_sz */
    int base = DATA_SIZE / comm_sz;
    int rest = DATA_SIZE % comm_sz;

    /* Filling counts and displacements arrys with counts of 
    how many integers each nodes will receive and with offsets
    describing how to scatter the original array into the 
    different processes. */
    int *counts = xmalloc(comm_sz*sizeof(int));
    int *displs = xmalloc(comm_sz*sizeof(int));
    int offset = 0;
    for (int i = 0; i < comm_sz; i++) {
        counts[i] = base + (i < rest ? 1 : 0);
        displs[i] = offset;
        offset += counts[i];
    }

    /* Computing local dimension of the array for the given rank
    and allocating proper memory space to held the scattered data. */
    int local_n = counts[rank];
    int *local_buf = xmalloc(local_n*sizeof(int));

    /* Scatter the array around the nodes */ 
    if (rank == 0) {
        int arr[DATA_SIZE];
        for (int i = 0; i < DATA_SIZE; i++)
            arr[i] = i;

        printf("The last number in the array is %d\n",
                arr[DATA_SIZE-1]);

        printf("[rank %d] starting data distribution\n", rank);

        distribute_data_array(
            arr, 
            local_buf,
            counts,
            displs,
            local_n,
            rank,
            DATA_SIZE,
            true,
            MPI_COMM_WORLD
        );
    } else {
        distribute_data_array(
            NULL, 
            local_buf,
            counts,
            displs,
            local_n,
            rank,
            DATA_SIZE,
            true,
            MPI_COMM_WORLD
        );
 
    }

    printf("[rank %d] finished scatter, building local digest\n", rank);

    // From the data buffer create the q-digest
    size_t upper_bound = _get_curr_upper_bound(local_buf, local_n);
    struct QDigest *q = _build_q_from_vector(local_buf, local_n, upper_bound, K);
    printf("[rank %d] built q-digest, starting tree_reduce\n", rank);

    // printf("Process %d received buffer of size %zu bytes\n",
    //        rank, sizeof(*local_buf));
    
    printf("Process %d: first element contained in array: %d\n",
           rank, local_buf[0]);

    printf("The root of the tree built in rank %d has upper_bound: %zu\n",
           rank, q->root->upper_bound);
    
    MPI_Barrier(MPI_COMM_WORLD);
    // data get inserted into qdigest and then compressed, ecc...
    tree_reduce(q, comm_sz, rank, MPI_COMM_WORLD);

    printf("[rank %d] tree_reduce completed\n", rank);

# define TEST
# ifdef TEST
    /* ============== TESTS ============= */
    // check if size of the communicator is as expected
    if (rank == 0) {
        size_t min = percentile(q, 0);
        size_t max = percentile(q, 1);
        size_t median = percentile(q, 0.5);
        printf("median is %zu\n", median);
        printf("max is %zu\n", max);
        printf("min is %zu\n", min);

        // // test suite works only for array declared above
        // assert(min == 0);
        // assert(max == DATA_SIZE-1);
        // assert(median == 5);
    }
#endif

    MPI_Finalize();
    // printf("Apparently all worked fine!\n"); // DEBUGGING 
    return 0;
}
