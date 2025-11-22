#include "../../include/memory_utils.h"
#include "../../include/qcore.h"
#include "../include/tree_reduce.h"
#include <assert.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST 1

int main(void) {
  int rank, comm_sz;

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

#if TEST
  /* Exercise tree_reduce with deterministic QDigest */
  int a[10] = {0, 1, 2, 4, 5, 6, 7, 8, 9};
  int n = sizeof(a) / sizeof(a[0]);

  /* Handling possible rest division in n/comm_sz */
  int base = n / comm_sz;
  int rest = n % comm_sz;
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

  distribute_data_array(a, local_buf, counts, displs, local_n, rank, n, true,
                        MPI_COMM_WORLD);

  size_t upper_bound = _get_curr_upper_bound(local_buf, local_n);
  struct QDigest *q = _build_q_from_vector(local_buf, local_n, upper_bound);
  tree_reduce(q, comm_sz, rank, MPI_COMM_WORLD);

#endif

  // test_tree_reduce()

  MPI_Finalize();
  printf("All tests passed!\n");
  return 0;
}
