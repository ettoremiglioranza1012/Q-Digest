#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/tree_reduce.h"

#define BUFFER_SIZE 1024
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

int main(void) 
{
    int rank, comm_sz;
    int data[BUFFER_SIZE]; // TO evaluate switch to xmalloc(); 

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // Init the random number array of data
    initialize_data_array(rank, data, BUFFER_SIZE);

    int local_n = BUFFER_SIZE/comm_sz;

    // Scatter the array around the nodes
    distribute_data_array(data, local_n, rank, MPI_COMM_WORLD);

    // From the data buffer create the q-digest
    struct QDigest *q = from_buff_to_q(); // To DO;

    // data get inserted into qdigest and then compressed, ecc...
    tree_reduce(q, comm_sz, rank, MPI_COMM_WORLD);
    
    MPI_Finalize();
    return 0;
}