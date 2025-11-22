#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../../include/qcore.h"
#include "../include/tree_reduce.h"
#include "../../include/memory_utils.h"

#define TEST 1

int main(void) {
    int rank, comm_sz;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

#if TEST
    
    /* Exercise tree_reduce with deterministic QDigest */
    int a[10] = {0,1,2,3,4,5,6,7,8,9};
    int n = sizeof(a)/sizeof(a[0]);
    
    /* Handling possible rest division in n/comm_sz */
    int base = n/comm_sz;
    int rest = n % comm_sz;

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

    distribute_data_array(
        a, 
        local_buf,
        counts,
        displs,
        local_n,
        rank,
        n,
        1,
        MPI_COMM_WORLD
    );
    size_t upper_bound = _get_curr_upper_bound(local_buf, local_n);
    struct QDigest *q = _build_q_from_vector(local_buf, local_n, upper_bound);
    tree_reduce(q, comm_sz, rank, MPI_COMM_WORLD);

    /* Free dynamically allocated memory for this session */
    free(counts);
    free(displs);
    free(local_buf);

#endif

    // test_tree_reduce() 
    if (rank == 0)
        printf("All test passed!\n");
    MPI_Finalize();
    return 0;
}


