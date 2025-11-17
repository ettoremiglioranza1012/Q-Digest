#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/qcore.h"
#include "../include/allreduce.h"
#include "../include/memory_utils.h"

#define DEFAULT_STR_BUFFER_SIZE 128

void Distribute_vector(
    int *sendbuf,
    int scounts,
    int n,
    int rank,
    MPI_Comm comm
)
{
    if (rank == 0) {
        MPI_Scatter(sendbuf, scounts, MPI_INT, sendbuf, scounts,
            MPI_INT, 0, comm);
    } else {
        MPI_Scatter(sendbuf, scounts, MPI_CHAR, sendbuf, scounts,
            MPI_CHAR, 0, comm);
    }
}   /* Read_vector */


void TreeAllreduce(
    struct QDigest *q,
    int comm_size,
    int rank,
    MPI_Comm comm)
{   
    int size = comm_size;
    
    MPI_Status recv_status;
    MPI_Request recv_req;
    MPI_Datatype datatype = MPI_INT;
    
    // Compute tree depth complexity
    int num_of_layers = (int)(log2(size));
    // Reduce the number of nodes/processes to a power of 2
    // If 10 processes we reduce to 8 = 2^(num_of_layers)
    int p = comm_size;
    int p2 = 1;
    while (p2 * 2 <= p) p2 *= 2;
    int orphans = p - p2;

    bool local_orphan_activity_flag = 1;
    int new_rank;

    /* REDUCE */

    // The new max_rank of processes/nodes is the == reduced_num-1.
    // Thus, we need to send the orphan nodes data to the new subset
    // of nodes, if there are any.
    if (orphans > 0) { // Check if there are any orphans.
        // Oprhans are in the head, if orphans == 2, the orphans are rank = (0, 2)
        // We couple them with rank = (1, 3)
        // thus we evaluate (0,1,2,3) or rank < orphans*2
        if (rank < 2*orphans) { // if orphan, send to newly formed subset.
            if (rank % 2 != 0) {
                char buf[DEFAULT_STR_BUFFER_SIZE]; 
                size_t length = sizeof(buf)/sizeof(char);
                to_string(q, buf); // to implement
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

    return;
}
