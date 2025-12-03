#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "../../include/qcore.h"
#include "../include/tree_reduce.h"
#include "../../include/memory_utils.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b));

void initialize_data_array(
    int rank,
    int *data,
    size_t n,
    size_t lb,
    size_t ub)
{
    int i;
    srand(rank);
    for (i = 0; i < n; i++) 
        data[i] = lb + rand() % (ub-lb + 1);
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

struct QDigest *_build_q_from_vector(int *a, int size, size_t upper_bound, int k) {
    /* FIXED: This portion of the code was causing a segfault
     * due to the fact that when using an upper bound that is much
     * smaller than the actual received number the q-digest might
     * overflow internal nodes, causing a strange ranges in serialization. */
    struct QDigest *q = create_tmp_q(k, upper_bound);
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
    size_t buf_size,
    MPI_Comm comm
)
{
    if (rank == 0)
    {
        if (src_values == NULL)
            printf("ERROR: provided pointer to the rank 0's buffer must be not NULL");
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
    
    // printf("DEBUG: number of orphans is %d\n", orphans);
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
            // printf("DEBUG: found orphan, sending data to process %d and flagging as orphan\n", rank - 1);
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
            // printf("DEBUG: after creation of q digest from string.\n");
            merge(q, tmp);
            // printf("DEBUG: after merge.\n");
            delete_qdigest(tmp);
            free(buf);
            // printf("DEBUG: rank %d received from rank %d during orphan processes detection\n",
                   // rank, rank + 1);

        }
    }
    
    /* === Compact Communicator of survivors === */
    /* is_orphan begins as a boolean; here it becomes the color for
     * MPI_Comm_split, so ranks marked orphan turn into MPI_UNDEFINED
     * (excluded) while color 0 places all surviving processes in the same
     * communicator. */

    if (is_orphan)
        is_orphan = MPI_UNDEFINED;
    // printf("rank %d is_orphan: %d\n", rank, is_orphan);
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

    // printf("reduced tree communicator size is %d\n", tree_size);

    /* === Power-of-two tree reduction === */

    /* Outline of the algorithm: 
     * 1. get the levels available in a full binary tree (depth)
     * 2. for each of these levels check if the rank of the process
     * in the new communicator is not a multiple of two of the current step size
     * 3. If it is not, then that rank is identified as a sender and
     * its receiver is found by subtracting the current step size from the
     * current rank. Once a node sends its data it drops out (break).
     * 4. Conversely, if the current rank is a multiple of the current step size
     * then it is identified as a receiver. This makes it so that this rank can
     * receive data from a sender identified by adding the current step size
     * to its rank.
     *
     * Example with 4 processes.
     * ============ Step size = 1 ==================
     * rank 0: qualifies as a receiver. Prepares to receive
     * from rank 1
     * rank 1: qualifies as a sender. Sends to rank 0. Breaks
     * from loop.
     * rank 2: qualifies as a receiver. Prepares to receive
     * from rank 3
     * rank 3: qualifies as a sender. Prepares to send to
     * rank 2.
     *
     * =========== Step size = 2 ===================
     * rank 0: qualifies as a receiver. Prepares to receive
     * from rank 2.
     * rank 2: qualifies as a sender. Prepares to send to
     * rank 0. Breaks from loop.
     * 
     * k now is equal to the maximum depth of the tree. Exit
     * for loop. Proceed with other steps.
     * 
     * */
    int levels = log_2_ceil(tree_size);
    printf("total depth of the tree is %d\n", levels);
    for (int k = 0; k < levels; k++) {
        uint64_t step_size = 0b0001 << k;
        // printf("DEBUG: current step size is %llu\n", step_size);
        if (tree_rank % (2 * step_size) != 0) {
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
            // printf("DEBUG: rank %d is preparing to receive from %d\n", tree_rank, sender);
            size_t recv_size;
            MPI_Recv(&recv_size, 1, MPI_UNSIGNED_LONG, sender, 0, tree_comm,
                MPI_STATUS_IGNORE);
            char *buf = xmalloc(recv_size);
            MPI_Recv(buf, recv_size, MPI_CHAR, sender, 0, tree_comm,
                MPI_STATUS_IGNORE);
            // printf("DEBUG: rank %d received from %d\n", tree_rank, sender);
            struct QDigest *tmp = from_string(buf);
            merge(q, tmp);
            delete_qdigest(tmp);
            free(buf);
        }
    }
    
    /* DEBUG: When run with 5 processes only 2 processes reach this stage. 
     * These are the processes that send at the first level and break from
     * the loop. */
    // MPI_Barrier(tree_comm);
    // printf("DEBUG: completed tree reduce\n");
    MPI_Comm_free(&tree_comm);
    return;
} /* tree_reduce */
