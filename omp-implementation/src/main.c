/* Compiler CLI example  */
/* $ gcc-15 -g -Wall -fopenmp 
 *      ./src/test_review.c ./src/ompcore.c 
 *      ../src/memory_utils.c ../src/qcore.c 
 *      ../src/queue.c ../src/dynamic_array.c -o test_try
 *  $ ./omp_hello 4
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/ompcore.h"
#include "../../include/qcore.h"
#include "../../include/memory_utils.h"


// OpenMP safe-check, loading, and  directives' declaration 
#ifdef _OPENMP
    #include <omp.h>
    // UDR -> Merge reduction of Q-Digests
    #pragma omp declare reduction(merge_q:struct QDigest*: \
        merge(omp_out, omp_in)) \
        initializer( \
        omp_priv = create_tmp_q( \
            omp_orig->K, omp_orig->root->upper_bound))
/* UDR Notes */
#endif


// User's defined Macros 
#define DATA_SIZE 100000000
#define LOWER_BOUND 0
#define UPPER_BOUND 40
#define K 5 


/* ============== MAIN FUNCTION ================ */
int main(int argc, char* argv[]) 
{
    // Check for input parameters
    if (!argv[1]) {
        fprintf(stderr, "Please provide thread_count in input\n");
        return 1;
    } 
    
    /* Get number of threads from command line */
    int thread_count  = strtol(argv[1], NULL, 10);
    // Debug 
    printf("Thread count is %d\n", thread_count);
    
    /* Handling possible rest division in n/comm_sz */
    int base = DATA_SIZE / thread_count; 
    int rest = DATA_SIZE % thread_count;
    /* Creating shared-memory indexes to divide
     * data among threads */
    int *counts = xmalloc(thread_count*sizeof(int));
    int *displs = xmalloc(thread_count*sizeof(int));
    int offset = 0;
    
    // Populating counts and dispalcementes arrays
    for (int i = 0; i < thread_count; i++) {
        counts[i] = base + (i < rest ? 1 : 0);
        displs[i] = offset;
        printf("Counts[%d]: %d\n", i, counts[i]);
        printf("Displs[%d]: %d\n", i, displs[i]);
        offset += counts[i];
    }
    
    // Allocate memory for DATA_SIZE
    int *buf = xmalloc(DATA_SIZE*sizeof(int));
    
    // Init full dataset
    #pragma omp master  
    {
        int master_id = omp_get_thread_num();
        initialize_data_array_for_omp(
            master_id,
            buf, 
            DATA_SIZE,
            LOWER_BOUND,
            UPPER_BOUND);
    }   
    
    // Variables and containers for parallel directive
    size_t *upper_bounds = xmalloc(thread_count*sizeof(size_t));
    struct QDigest *local_results[thread_count]; // Array of QDigest's pointers
    struct QDigest *final_q; // Global QDigest accumulator
    size_t n = DATA_SIZE-1; 
    size_t glob_mx = 0;

    double start = omp_get_wtime();

    // Threads execution
    #pragma omp parallel num_threads(thread_count) \
        default(none) shared(buf, counts, displs,  \
                upper_bounds, n, thread_count,     \
                local_results, glob_mx,  final_q)
    {
        // Local boundaries index variables definitions
        int thread_id = omp_get_thread_num();
        size_t thread_ix_start = displs[thread_id];
        size_t thread_ix_end;
        
        // Local end index initialisation 
        if (thread_id+1 == thread_count) {
            thread_ix_end = n;
        } else {
            thread_ix_end = displs[thread_id+1];
        }
        // Debug
        printf("For thread %d data start at %zu and end at %zu\n", 
                thread_id, thread_ix_start, thread_ix_end);
        
        // Private local upper bound
        size_t local_mx = 0;
        for (size_t i = thread_ix_start; 
                i < thread_ix_end; i++) {
            if (buf[i] > local_mx) local_mx = buf[i]; 
        }
        // Collecting local upper boundaries
        upper_bounds[thread_id] = local_mx;
        
        // Reduce local upper boundaries array to global upper boundary
        #pragma omp master 
        {
            for (int thr_i = 0; thr_i < thread_count; thr_i++) {
                if (glob_mx < upper_bounds[thr_i]) {
                    glob_mx = upper_bounds[thr_i];
                }                
                printf("Thread %d 's local UB is %zu\n", 
                        thr_i, upper_bounds[thr_i]);
            }
            printf("Global upper boundary is: %zu\n", 
                    glob_mx);
        }
        // Ensure every thread has access to global upper bound
        #pragma omp barrier

        // Init global accumulator shared in heap memory 
        #pragma omp single
        {
            final_q = create_tmp_q(K, glob_mx);
        }

        // Build thread's private Q-Digest from shared buffer
        local_results[thread_id] = _build_q_from_vector_for_omp(
                buf,
                thread_ix_start,
                thread_ix_end,
                counts[thread_id],
                glob_mx,
                K);
        
        // Debug
        printf("Thread %d correctly initialised Q-Digest\n", thread_id);
        printf("    Data range: [%zu, %zu)\n", thread_ix_start, thread_ix_end);
        printf("    Count of value: %d\n", counts[thread_id]);
        
        // Ensure every thread has completed building its local digest
        #pragma omp barrier

        printf("Thread %d Proceed to reduction...\n", thread_id);

        // Parallel Reduction
        #pragma omp for reduction(merge_q: final_q)
        for (int k = 0; k < thread_count; k++) 
        {
            merge(final_q, local_results[k]);
            delete_qdigest(local_results[k]);
        }   
    }
    double end = omp_get_wtime();
    printf("\n");
    printf("Reduction completed successfully\n");
    printf("Execution time: %f seconds\n", end - start);
    printf("\n");    
    
    return 0;
}
