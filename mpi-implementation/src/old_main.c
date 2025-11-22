#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../../include/qcore.h"

/* NOTE: These are test parameters and should be removed in 
 * favor of proper user-based I/O */
// how many numbers to generate
// also the size of the array (vector) that stores them in process 0
#define NUMS 100
#define K 5

/* =========== FUNCTION PROTOTYPES ==================== */
struct QDigest *_build_q_from_vector(int *a, int size);

/* ============== MAIN FUNCTION ======================== */

int main(void) {
    MPI_Init(NULL, NULL);
    
    int rank, n_prcs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_prcs);

    int local_n = NUMS/n_prcs;
    // rank 0 should collect the input data (a vector)
    // and scatter it to the other processes so that each
    // process gets a subset of the input data
    if (rank == 0) {
        int arr[NUMS];
        // fill array with numbers
        for (int i = 0; i < NUMS; i++) {
            arr[i] = i;
        }
        // try scatter in other process
        int local_buf[local_n];
        MPI_Scatter(arr, local_n, MPI_INT, local_buf,
                    local_n, MPI_INT, 0, MPI_COMM_WORLD);
        struct QDigest *q = _build_q_from_vector(local_buf, local_n);
        delete_qdigest(q);

    } else {
        int local_buf[local_n];
        MPI_Scatter(NULL, local_n, MPI_INT, local_buf,
                    local_n, MPI_INT, 0, MPI_COMM_WORLD);

        struct QDigest *q = _build_q_from_vector(local_buf, local_n);

        // serialize
        char ser[1024];
        size_t length = 0;
        to_string(q, ser, &length);
        printf("length of buf: %zu\n", length);

        // print serialization
        printf("=======\nSerialization from process %d\n=======\n%s",
               rank, ser);
        delete_qdigest(q);
    } // processes other than 0
    

    MPI_Finalize();
    return 0;
}

/* ============== FUNCTION IMPLEMENTATIONS =============== */

/* NOTE: helper functions are indicated with are preceded by 
 * an underscore (_) */

/* This function creates a q-digest from a vector */
struct QDigest *_build_q_from_vector(int *a, int size) {
    /* FIXED: This portion of the code was causing a segfault
     * due to the fact that when using an upper bound that is much
     * smaller than the actual received number the q-digest might
     * overflow internal nodes, causing a strange ranges in serialization. */
    struct QDigest *q = create_tmp_q(5, NUMS-1);
    for (int i = 0; i < size; i++) {
        insert(q, a[i], 1, true);
    }
    return q;
}
