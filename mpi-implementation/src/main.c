#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/tree_reduce.h"
#include "../../include/memory_utils.h"


/* NOTE: These are test parameters and should be removed in 
 * favor of proper user-based I/O */
// how many numbers to generate
// also the size of the array (vector) that stores them in process 0
#define DATA_SIZE 1000
#define LOWER_BOUND 0
#define UPPER_BOUND 40
#define K 200


/* ============== MAIN FUNCTION ================ */
int main(void) 
{
    int rank, comm_sz;
    double local_start, local_finish, local_elapsed, elapsed;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    /* Handling possible rest division in n/comm_sz */
    int base = DATA_SIZE/comm_sz;
    int rest = DATA_SIZE % comm_sz;

    /* Filling counts and displacements arrys with counts of 
    how many integers each nodes will receive and with offesets
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
        printf("[rank %d] starting data distribution\n", rank);
    }
    // MPI_Barrier(MPI_COMM_WORLD); // DEBUGGING
    /* This is initializing the array, technically only rank 0
     * should initialize the array and scatter it around */
    
    int *buf = NULL;
    if (rank == 0) {
        buf = xmalloc(DATA_SIZE * sizeof(int));
        initialize_data_array(rank, buf, DATA_SIZE, LOWER_BOUND, UPPER_BOUND);
    }

    // ============ Start timing ==============

    // Sync processes and start timing
    MPI_Barrier(MPI_COMM_WORLD); 
    local_start = MPI_Wtime();

    distribute_data_array(
        buf, 
        local_buf,
        counts,
        displs,
        local_n,
        rank,
        DATA_SIZE,
        MPI_COMM_WORLD
    );
    if (rank == 0) {
        printf("[rank %d] Data distributed correctly!\n", rank);
    }
    // printf("[rank %d] finished scatter, building local digest\n", rank);
    // MPI_Barrier(MPI_COMM_WORLD); // DEBUGGING 
    // From the data buffer create the q-digesti
    size_t local_upper_bound = _get_curr_upper_bound(local_buf, local_n);
    size_t global_upper_bound;
    MPI_Allreduce(
        &local_upper_bound,
        &global_upper_bound,
        1,
        MPI_UNSIGNED_LONG,
        MPI_MAX,
        MPI_COMM_WORLD
    );
    struct QDigest *q = _build_q_from_vector(local_buf, local_n, global_upper_bound, K);
    // printf("[rank %d] built q-digest, starting tree_reduce\n", rank);
    // MPI_Barrier(MPI_COMM_WORLD); // DEBUGGING 

    // data get inserted into qdigest and then compressed, ecc...
    tree_reduce(q, comm_sz, rank, MPI_COMM_WORLD);
    // printf("[rank %d] tree_reduce completed\n", rank);
    // MPI_Barrier(MPI_COMM_WORLD); // DEBUGGING 

    local_finish = MPI_Wtime();

    // ============ End timing ==============

    local_elapsed = local_finish - local_start;
    MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE,
            MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) 
        printf("[rank %d] Elapsed time = %e seconds\n", rank, elapsed);

    free(counts);
    free(displs);
    free(local_buf);
    free(buf);
    MPI_Finalize();
    
    return 0;
}
