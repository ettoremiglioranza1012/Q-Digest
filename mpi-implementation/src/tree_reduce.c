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
                MPI_COMM_WORLD
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
            MPI_COMM_WORLD
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
    
    int p = comm_size;
    int p2 = 1;
    while (p2 * 2 <= p) p2 *= 2;
    int orphans = p - p2;
    
    /* REDUCE */
    
    /* === Trim of the communicator === */
    if (orphans > 0 && rank < 2 * orphans) { 
        if (rank % 2) {
            size_t size = get_num_of_bytes(q);
            char *buf = xmalloc(size);
            size_t length = 0; // This is currently not used
            to_string(q, buf, &length);
            MPI_Send(&size, 1, MPI_UNSIGNED_LONG, rank-1, 0, comm);
            MPI_Send(buf, size, MPI_CHAR, rank-1, 0, comm);
            free(buf);
            delete_qdigest(q);
            return;
        } else {
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
    int is_orphan = ( (orphans > 0) && (rank < (2 * orphans)) && (rank % 2 != 0) ) ? MPI_UNDEFINED : 0;
    MPI_Comm tree_comm = MPI_COMM_NULL;
    MPI_Comm_split(comm, is_orphan, rank, &tree_comm);
    if (tree_comm == MPI_COMM_NULL) {
        return;
    }
    
    int tree_rank, tree_size;
    MPI_Comm_rank(tree_comm, &tree_rank);
    MPI_Comm_size(tree_comm, &tree_size);
    
    /* === Power-of-two tree reduction === */
    int levels = log_2_ceil(tree_size);
    for (int k = 0; k < levels; k++) {
        int step_size = 1 << k;
        if (tree_rank % (2 * step_size)) {
            /* sender branch */
            size_t size = 0;
            size += get_num_of_bytes(q);
            char *buf = xmalloc(size);
            size_t length = 0; // This is currently not used
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
