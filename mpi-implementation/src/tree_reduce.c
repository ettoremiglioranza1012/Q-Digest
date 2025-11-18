#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../include/qcore.h"
#include "../include/tree_reduce.h"
#include "../../include/memory_utils.h"

#define DEFAULT_STR_BUFFER_SIZE 128
#define LOWER_BOUND 0
#define UPPER_BOUND 10

void initialize_data_array(int rank, int *data, int n)
{
    int i;
    srand(rank);
    for (i = 0; i < n; i++) 
        data[i] = LOWER_BOUND + rand() % (UPPER_BOUND-LOWER_BOUND + 1);
    return;
}

int *distribute_data_array(
    int *local_buf,
    int local_n,
    int rank,
    int buf_size,
    MPI_Comm comm
)
{
    if (rank == 0)
    {
        int buf[buf_size]; // TO evaluate switch to xmalloc()
        initialize_data_array(rank, buf, buf_size); // Init the random number array of data
        MPI_Scatter(buf, local_n, MPI_INT, local_buf, local_n,
            MPI_INT, 0, comm);
    } else {
        MPI_Scatter(NULL, local_n, MPI_INT, local_buf, local_n,
            MPI_INT, 0, comm);
    }
    return local_buf;
}   /* Read_vector */


void tree_reduce(
    struct QDigest *q,
    int comm_size,
    int rank,
    MPI_Comm comm)
{     
    /**
     * Reduce the number of nodes/processes to a power of 2
     * If 10 processes we reduce to 8 = 2^(num_of_layers)
     * We will keep this example for the rest of the implementations
     */
    int p = comm_size;
    int p2 = 1;
    while (p2 * 2 <= p) p2 *= 2;
    int orphans = p - p2;

    /* REDUCE */

    /**
     * Check if there are any orphans.
     * Orphans are in the head, if we have two orphans 
     * (e.g. from 10 to 8 processes), 
     * the orphans are the odd process in region [0, 2*orphans).
     * Hence, we couple them with pair process.
     * Thus, we evaluate ([0,1],[2,3]) or rank < (orphans*2==4).
     * If process is orphan, send to newly formed subset.
     */ 
    
    if (orphans > 0 && rank < 2 * orphans) { 
        if (rank % 2) {
            size_t size = get_num_of_bytes(q);
            char *buf = xmalloc(size);
            size_t length = 0; // This is currently not used, just want to see if size work
            to_string(q, buf, &length);
            MPI_Send(&size, 1, MPI_UNSIGNED_LONG, rank-1, 0, comm);
            MPI_Send(buf, size, MPI_CHAR, rank-1, 0, comm);
            free(buf);
            delete_qdigest(q);
            return; /* oprhan sender leaves the algorithm */
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

    /**
     * Since we operated transformations on the ranks, we 
     * need to consider the newly mapped logic.
     * 
     * For example:
     * 
     * p0<-p1, p2<-p3,  
     *  |        |
     *  p0       p1 
     * 
     * Thus, rank 0 stays 0, rank 2 becomes new rank 1.
     * 
     * All other ranks >= 2*orphans, must also scale to
     * new_rank = rank - num_of_orphans_removed
     * 
     * Hence, we need to build a compact communicator of survivors.
     */
    
    /* === Compact Communicator of survivors ===
     * 
     MPI_UNDEFINED to MPI_Comm_split tells MPI “I’m not part of the new communicator.” 
     * if is_orphan is MPI_UNDEFINED it is an orphan, otherwise 0 for not orphans ranks.
     * 
     * MPI_UNDEFINED to MPI_Comm_split tells MPI “I’m not part of the new communicator.” 
     * if is_orphan is MPI_UNDEFINED it is an orphan, otherwise 0 for not orphans ranks.
     * 
     * MPI_COMM_NULL initializes the handle for the communicator that 
     * will contain only the surviving (non-orphan) ranks.
     * 
     * MPI_Comm_split(comm, color, rank, &tree_comm):
     * Splits the original communicator (comm, e.g., MPI_COMM_WORLD)
     * into sub-communicators grouped by is_orphan. 
     * All ranks with is_orphan == 0 are placed into a new communicator
     * (here called tree_comm) where MPI automatically renumbers them 
     * densely from 0…,p2-1 while ranks with color == MPI_UNDEFINED
     * receive tree_comm == MPI_COMM_NULL and drop out of the reduction.
     */ 
     int is_orphan = ( (orphans > 0) && (rank < (2 * orphans)) && (rank % 2 != 0) ) ? MPI_UNDEFINED : 0;
    MPI_Comm tree_comm = MPI_COMM_NULL;
    MPI_Comm_split(comm, is_orphan, rank, &tree_comm);
    if (tree_comm == MPI_COMM_NULL) {
        return; /* already retuned the oprhans above, but just in case */
    }

    int tree_rank, tree_size;
    MPI_Comm_rank(tree_comm, &tree_rank);
    MPI_Comm_size(tree_comm, &tree_size); // equals to p2

    /**
     * What the algorithm below will do soon after: 
     * for step_size equal to 2 * K, we evaluate true for rank.
     *      
     * Below a simple example to understand considering
     * ranks between 0 to 7, e.g. tree_size == 8.
     * 
     * Step 0 (tree_size = 8 → levels = 3, k = 0, step_size = 1)
     *        rank 0: 0 % 2 == 0 → receiver; partner = 1.
     *        rank 1: 1 % 2 == 1 → sender; partner = 0.
     *        rank 2: 2 % 2 == 0 → receiver; partner = 3.
     *        rank 3: 3 % 2 == 1 → sender; partner = 2.
     *        rank 4: 4 % 2 == 0 → receiver; partner = 5.
     *        rank 5: 5 % 2 == 1 → sender; partner = 4.
     *        rank 6: 6 % 2 == 0 → receiver; partner = 7.
     *        rank 7: 7 % 2 == 1 → sender; partner = 6.
     *        Senders transmit their digest upward and then exit; receivers merge and continue.
     * 
     * Step 1 (k = 1, step_size = 2, only ranks 0,2,4,6 remain)
     *        rank 0: 0 % 4 == 0 → receiver; partner = 2.
     *        rank 2: 2 % 4 == 2 → sender; partner = 0.
     *        rank 4: 4 % 4 == 0 → receiver; partner = 6.
     *        rank 6: 6 % 4 == 2 → sender; partner = 4.
     * 
     * Step 2 (k = 2, step_size = 4, ranks 0 and 4 remain)
     *        rank 0: 0 % 8 == 0 → receiver; partner = 4.
     *        rank 4: 4 % 8 == 4 → sender; partner = 0.
     *        After this step rank 0 holds the merged digest.
     */

    int levels = (int)log2(tree_size);
    for (int k = 0; k < levels; k++) {
        int step_size = 1 << k; // 2 ^ k
        if (tree_rank % (2 * step_size)) { // if current process is a sender (or odd ONLY for the first level)
            /* sender branch */
            size_t size = 0;
            size += get_num_of_bytes(q);
            char *buf = xmalloc(size);
            size_t length = 0; // This is currently not used, just want to see if size work as well before removing
            to_string(q, buf, &length);
            int receiver = tree_rank - step_size;
            MPI_Send(&size, 1, MPI_UNSIGNED_LONG, receiver, 0, comm);
            MPI_Send(buf, size, MPI_CHAR, receiver, 0, comm);
            free(buf);
        } else {
            /* receiver branch */
            int sender = tree_rank + step_size;
            size_t recv_size;
            MPI_Recv(&recv_size, 1, MPI_UNSIGNED_LONG, sender, 0, comm,
                MPI_STATUS_IGNORE);
            char *buf = xmalloc(recv_size);
            MPI_Recv(buf, recv_size, MPI_CHAR, sender, 0, comm,
                MPI_STATUS_IGNORE);
            struct QDigest *tmp = from_string(buf);
            merge(q, tmp);
            delete_qdigest(tmp);
            free(buf);
        }
    }
    MPI_Comm_free(&tree_comm);
    return;
}
