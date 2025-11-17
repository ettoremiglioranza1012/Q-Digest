#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../../include/qcore.h"
#include "../include/tree_reduce.h"
#include "../../include/memory_utils.h"

#define DEFAULT_STR_BUFFER_SIZE 128

void distribute_data_array(
    int *sendbuf,
    int local_n,
    int n,
    int rank,
    MPI_Comm comm
)
{
    // TO do; 
    int local_buf[local_n];
    if (rank == 0) {
        MPI_Scatter(sendbuf, local_n, MPI_INT, local_buf, local_n,
            MPI_INT, 0, comm);
    } else {
        MPI_Scatter(NULL, local_n, MPI_CHAR, local_buf, local_n,
            MPI_CHAR, 0, comm);
    }
}   /* Read_vector */


void tree_reduce(
    struct QDigest *q,
    int comm_size,
    int rank,
    MPI_Comm comm)
{   
    
    // Reduce the number of nodes/processes to a power of 2
    // If 10 processes we reduce to 8 = 2^(num_of_layers)
    // We will keep this example for the rest of the implementations
    int p = comm_size;
    int p2 = 1;
    while (p2 * 2 <= p) p2 *= 2;
    int orphans = p - p2;

    bool local_orphan_activity_flag = 1;
    int new_rank;

    /* REDUCE */
    if (orphans > 0) { // Check if there are any orphans.
        // Oprhans are in the head, if we have two orphans (from 10 to 8 procs), 
        // the orphans are odd process in region [0, 2*orphans]
        // We couple them with pair process,
        // thus we evaluate ([0,1],[2,3]) or rank < orphans*2
        if (rank < 2*orphans) { // if orphan, send to newly formed subset.
            if (rank % 2 != 0) {
                char buf[DEFAULT_STR_BUFFER_SIZE]; 
                size_t length = 0;
                to_string(q, buf, &length); // to implement
                MPI_Send(length, 1, MPI_UNSIGNED_LONG, rank-1, 0, comm);
                MPI_Send(buf, length, MPI_CHAR, rank-1, 0, comm);
                local_orphan_activity_flag = 0;
            } else {
                size_t recv_length;
                char buf[DEFAULT_STR_BUFFER_SIZE];
                MPI_Recv(recv_length, 1, MPI_UNSIGNED_LONG, rank+1, 0, comm,
                    MPI_STATUS_IGNORE);
                MPI_Recv(buf, recv_length, MPI_CHAR, rank+1, 0, comm,
                    MPI_STATUS_IGNORE);
                struct QDigest *tmp = from_string(buf); // To implement
                merge(q, tmp);
                new_rank = rank / 2; // p0<-p1, p2<-p3, thus rank 0 stays 0, rank 2 becomes new rank 1
                                     // p0, p1.
            }
        } else {
            // Here we handle from p4 to pN, thus we only scale rank by N orphans
            new_rank = rank - orphans;
        }
    }

    if (!local_orphan_activity_flag) {
        // These ranks do not partecipate to the tree phase;
        //
        return;
    }

    // Here we have p2 processes, 
    // here we implement the tree reducing recursive doubling with the new_rank;

    return;
}
