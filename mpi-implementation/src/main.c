#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/tree_reduce.h"
#include "../../include/memory_utils.h"
#include "../../include/dataset_reader.h"


/* NOTE: These are test parameters and should be removed in 
 * favor of proper user-based I/O */
// how many numbers to generate
// also the size of the array (vector) that stores them in process 0
#define K 200

int main(int argc, char **argv) 
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int rank, comm_sz;
    int count_out = 0; 
    double local_start, local_finish, local_elapsed, elapsed;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int total_data_size = 0;
    int *buf = NULL;

    if (rank == 0) {
        printf("[rank %d] Reading from file: %s\n", rank, argv[1]);

        buf = read_ints_from_file(argv[1], &count_out);
        
        if (buf == NULL) {
            fprintf(stderr, "Error: Could not read file.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        total_data_size = count_out;
        printf("First entry: %d\n", buf[0]);
        printf("Total entries read: %d\n", total_data_size);
    }

    MPI_Bcast(&total_data_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int base = total_data_size / comm_sz;
    int rest = total_data_size % comm_sz;

    int *counts = xmalloc(comm_sz * sizeof(int));
    int *displs = xmalloc(comm_sz * sizeof(int));
    int offset = 0;
    for (int i = 0; i < comm_sz; i++) {
        counts[i] = base + (i < rest ? 1 : 0);
        displs[i] = offset;
        offset += counts[i];
    }

    int local_n = counts[rank];
    int *local_buf = xmalloc(local_n * sizeof(int));

    // ============ Start timing ==============
    MPI_Barrier(MPI_COMM_WORLD); 
    local_start = MPI_Wtime();

    distribute_data_array(
        buf, 
        local_buf,
        counts,
        displs,
        local_n,
        rank,
        total_data_size, 
        MPI_COMM_WORLD
    );

    if (rank == 0) {
        printf("[rank %d] Data distributed correctly!\n", rank);
        free(buf); // Rank 0 can free the big buffer now, it's been scattered
    } else {
        printf("[rank %d] Gathered %d data points\n", rank, local_n);
    }

    size_t local_upper_bound = _get_curr_upper_bound(local_buf, local_n);
    size_t global_upper_bound;
    MPI_Allreduce(&local_upper_bound, &global_upper_bound, 1, MPI_UNSIGNED_LONG, MPI_MAX, MPI_COMM_WORLD);

    struct QDigest *q = _build_q_from_vector(local_buf, local_n, global_upper_bound, K);
    
    tree_reduce(q, comm_sz, rank, MPI_COMM_WORLD);

    local_finish = MPI_Wtime();
    // ============ End timing ==============

    local_elapsed = local_finish - local_start;
    MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // TODO: decide if remove MPI_Barrier, I added it because I wanted to make sure that
    // the final reduced tree was completed.
    MPI_Barrier(MPI_COMM_WORLD);
    // TODO: decide if percentile computation should be considered as part of the timed portion!
    if (rank == 0) {
        printf("[rank %d] Elapsed time = %e seconds\n", rank, elapsed);
        printf("The median of the distribution is %zu\n", percentile(q, 0.5));
    }

    free(counts);
    free(displs);
    free(local_buf);
    
    MPI_Finalize();
    return 0;
}
