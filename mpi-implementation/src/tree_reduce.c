#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "../../include/qcore.h"
#include "../include/tree_reduce.h"
#include "../../include/memory_utils.h"

#define LOWER_BOUND 0
#define UPPER_BOUND 10

#define MAX(a, b) ((a) > (b) ? (a) : (b));

void _initialize_data_array(int rank, int *data, int n)
{
    int i;
    srand(rank);
    for (i = 0; i < n; i++) 
        data[i] = LOWER_BOUND + rand() % (UPPER_BOUND-LOWER_BOUND + 1);
    return;
}

size_t _get_curr_upper_bound(int *buf, int n)
{
    size_t max = 0;
    for (int i = 0; i < n; i++) {
        max = MAX(buf[i], max);
    }
    return max;   
}

struct QDigest *_build_q_from_vector(int *a, int size, size_t upper_bound) {
    /* FIXED: This portion of the code was causing a segfault
     * due to the fact that when using an upper bound that is much
     * smaller than the actual received number the q-digest might
     * overflow internal nodes, causing a strange ranges in serialization. */
    struct QDigest *q = create_tmp_q(5, upper_bound);
    for (int i = 0; i < size; i++) {
        insert(q, a[i], 1, true);
    }
    return q;
}

int *distribute_data_array(
    int *src_values,
    int *local_buf,
    int *counts,
    int *displs,
    int local_n,
    int rank, 
    int buf_size,
    bool use_src,
    MPI_Comm comm
)
{
    if (rank == 0)
    {
        // If the caller provides a source buffer and requests its use, scatter from that buffer.
        if (src_values && use_src) {
            MPI_Scatterv(
                src_values,   // sendbuf (root routine)
                counts,     
                displs,     
                MPI_INT,
                local_buf,  
                local_n,    
                MPI_INT,
                0,          // root
                comm
            );
        } else {
            // For convenience in testing, allocate and initialize a temporary buffer internally.
            int *buf = xmalloc(buf_size*sizeof(int));
            _initialize_data_array(rank, buf, buf_size);
            MPI_Scatterv(
                buf,   // sendbuf (root routine)
                counts,     
                displs,     
                MPI_INT,
                local_buf,  
                local_n,    
                MPI_INT,
                0,          // root
                comm
            );
            free(buf);
        }
    } else {
        MPI_Scatterv(
            NULL,   // recvbuff (root routine)
            counts,     
            displs,     
            MPI_INT,
            local_buf,  
            local_n,    
            MPI_INT,
            0,          // root
            comm
            );
    }
    return local_buf;
}   /* Read_vector */


void tree_reduce(
    struct QDigest *q,
    int comm_size,
    int rank,
    MPI_Comm comm)
{     
    /* checking for the number of orphan processes (i.e., those in excess of a power of two). */
    int p = comm_size;
    int p2 = 1;
    while (p2 * 2 <= p) p2 *= 2;
    int orphans = p - p2;

    // Init a flag var to signal whether the curr rank is 
    // orphan and needs to be trimmed,
    // under baseline assumption that every process is not an orphan
    int is_orphan = 0;
    
    /* REDUCE */
    
    /* === Trim of the communicator === */
    if ((orphans > 0) && (rank < 2 * orphans)) { 
        if (rank % 2 != 0) {
            // Odd ranks branch or orphans -> sender 
            size_t size = get_num_of_bytes(q);
            char *buf = xmalloc(size);
            size_t length = 0; // TODO
            to_string(q, buf, &length);
            MPI_Send(&size, 1, MPI_UNSIGNED_LONG, rank-1, 0, comm);
            MPI_Send(buf, size, MPI_CHAR, rank-1, 0, comm);
            free(buf);
            delete_qdigest(q);
            is_orphan = 1;
        } else {
            // Pair ranks branch or siblings -> receiver 
            size_t recv_size;
            MPI_Recv(&recv_size, 1, MPI_UNSIGNED_LONG, rank + 1, 0, comm,
                MPI_STATUS_IGNORE);
            char *buf = xmalloc(recv_size);
            MPI_Recv(buf, recv_size, MPI_CHAR, rank + 1, 0, comm,
                MPI_STATUS_IGNORE);
            struct QDigest *tmp = from_string(buf);
            merge(q, tmp);
            delete_qdigest(tmp);
            free(buf);
        }
    }
    
    /* === Compact Communicator of survivors === */
    /* is_orphan begins as a boolean; here it becomes the color for
     * MPI_Comm_split, so ranks marked orphan turn into MPI_UNDEFINED
     * (excluded) while color 0 places all surviving processes in the same
     * communicator. */
    if (is_orphan)
        is_orphan = MPI_UNDEFINED;
    MPI_Comm tree_comm = MPI_COMM_NULL;
    // printf("DEBUG: before comm split\n");
    MPI_Comm_split(comm, is_orphan, rank, &tree_comm);
    // printf("DEBUG: after comm split\n");
    if (tree_comm == MPI_COMM_NULL) {
        // printf("DEBUG: tree comm is NULL\n");
        return;
    }
    
    /*====== SECTION ON REDUCED TREE =========*/
    int tree_rank, tree_size;
    MPI_Comm_rank(tree_comm, &tree_rank);
    MPI_Comm_size(tree_comm, &tree_size);

    printf("reduced tree communicator size is %d\n", tree_size);

    /* === Power-of-two tree reduction === */
    int levels = log_2_ceil(tree_size);
    for (int k = 0; k < levels; k++) {
        uint64_t step_size = 0b0001 << k;
        if (tree_rank % (2 * step_size)) {
            /* sender branch */
            size_t size = 0;
            size += get_num_of_bytes(q);
            char *buf = xmalloc(size);
            size_t length = 0; // TODO
            to_string(q, buf, &length);
            int receiver = tree_rank - step_size;
            MPI_Send(&size, 1, MPI_UNSIGNED_LONG, receiver, 0, tree_comm);
            MPI_Send(buf, size, MPI_CHAR, receiver, 0, tree_comm);
            free(buf);
            break;
        } else {
            /* receiver branch */
            int sender = tree_rank + step_size;
            size_t recv_size;
            MPI_Recv(&recv_size, 1, MPI_UNSIGNED_LONG, sender, 0, tree_comm,
                MPI_STATUS_IGNORE);
            char *buf = xmalloc(recv_size);
            MPI_Recv(buf, recv_size, MPI_CHAR, sender, 0, tree_comm,
                MPI_STATUS_IGNORE);
            struct QDigest *tmp = from_string(buf);
            merge(q, tmp);
            delete_qdigest(tmp);
            free(buf);
        }
    }
    
    MPI_Comm_free(&tree_comm);
    return;
} /* tree_reduce */
