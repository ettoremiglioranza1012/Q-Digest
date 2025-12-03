#ifndef TREE_REDUCE_H
#define TREE_REDUCE_H

#include <stdlib.h>
#include <mpi.h>
#include "../../include/qcore.h"


/* ============== HELPER FUNCTIONS =============== */
/**
 *  @brief Fills a buffer with pseudo-random integers for a given rank.
 *
 *  Seeds the C RNG with @p rank so every process chosen for the generation
 *  (default case is rank 0 process) creates a reproducible sequence, then 
 *  writes @p n values between the given @lb and @ub (lower bound and upper bound)
 *  (inclusive) into @p data.
 *
 *  @param rank Calling process rank; used both for RNG seeding and to decorrelate buffers.
 *  @param data Pointer to the buffer that will receive @p n integers.
 *  @param n Number of elements to generate.
 *  @param lb lower bound of data.
 *  @param up upper bound of data.
 *
 */
void initialize_data_array(int rank, int *data, size_t n, size_t lb, size_t ub);

/**
 *  @brief Scatter variable-sized segments across ranks using MPI_Scatterv.
 *
 *  Distributes variable-sized segments of an integer array across ranks using MPI_Scatterv.
 *
 *  The caller is responsible for providing the source data array (@p src_values) on rank 0,
 *  as well as the counts and displacements arrays. The function simply scatters the data
 *  using MPI_Scatterv, and each rank receives @p local_n integers into @p local_buf.
 *
 *  This function does not generate or allocate the source buffer; it only performs the scatter.
 *  The caller must ensure that @p src_values is valid and populated on rank 0.
 *
 *  After scattering, the receive buffer (@p local_buf) is returned for convenience.
 *
 *  @param src_values source buffer on rank 0; ignored on other ranks. 
 *                    This should be set to NULL if the caller does not intend 
 *                    to pass a pre-allocated buffer containing the data distribution to process.
 *  @param local_buf Caller-provided receive buffer.
 *  @param counts Per-rank receive counts (length comm_size) passed to MPI_Scatterv.
 *  @param displs Displacements matching @p counts.
 *  @param local_n Number of integers expected locally (recvcount).
 *  @param rank Rank of the calling process.
 *  @param buf_size Total number of integers available on rank 0. Used when @p use_src is false.
 *  @param comm Communicator used for the scatter.
 *
 *  @return @p local_buf for chaining.
 */
int *distribute_data_array(
    int *src_values,
    int *local_buf,
    int *counts,
    int *displs,
    int local_n,
    int rank, 
    size_t buf_size,
    MPI_Comm comm
);

/**
 *  @brief Compute the maximum value in a buffer segment.
 *
 *  Scans the first @p n entries of @p buf and returns the largest value found,
 *  convenient for deriving an upper bound before constructing a digest.
 *
 *  @param buf Pointer to the integer buffer to inspect.
 *  @param n   Number of elements from @p buf to consider.
 *
 *  @return The maximum value among the inspected elements.
 */
size_t _get_curr_upper_bound(int *buf, int n);

/**
 *  @brief Build a QDigest from an integer vector.
 *
 *  Allocates and initializes a QDigest sized for @p upper_bound, then inserts
 *  the @p size values from @p a into it, returning the populated digest.
 *
 *  @param a           Pointer to the input array of integers.
 *  @param size        Number of elements in @p a.
 *  @param upper_bound Maximum value expected in @p a (used to size the digest).
 *
 *  @return Newly allocated QDigest containing all values from @p a.
 */
struct QDigest *_build_q_from_vector(int *a, int size, size_t upper_bound, int k);


/* ============== PARALLE Q-DIGEST FUNCTION =============== */
/**
 *  @brief Performs a power-of-two tree reduction over distributed QDigests.
 *
 *  The routine first trims the communicator to the largest power of two not
 *  exceeding @p comm_size. Ranks in the prefixed [0, 2*orphans) window are
 *  paired (odd sends to the preceding even rank) and their digests are merged,
 *  after which odd senders exit the algorithm. Survivors form a compact
 *  communicator via `MPI_Comm_split()` so their ranks become dense in [0, p2).
 *
 *  The remaining ranks execute a log2(tree_size)-step binary-tree reduction:
 *    - On each level k, ranks with `tree_rank % (2 * step_size) != 0` serialize
 *      their digest via `to_string()` and send it (size first, then payload) to
 *      the partner `tree_rank - step_size`.
 *    - Receivers collect the serialized digest, reconstruct it with
 *      `from_string()`, merge it into their local digest via `merge()`, and
 *      continue to the next level.
 *    - Senders drop out after transmitting.
 *
 *  @param q Pointer to the local QDigest that participates in the reduction.
 *  @param comm_size Total number of ranks in the input communicator.
 *  @param rank Rank of the calling process within the input communicator.
 *  @param comm Input communicator that contains all participants (e.g., MPI_COMM_WORLD).
 *
 *  @note Serialization uses the current `get_num_of_bytes()`/`to_string()` pair,
 *        which assumes fixed field separators; once configurable delimiters are
 *        supported those literals will be replaced with runtime values.
 *  @note All MPI messages use tag 0 and blocking send/recv; future work may
 *        introduce configurable tags or non-blocking operations.
 */
void tree_reduce(struct QDigest *q, int comm_size, int rank, MPI_Comm comm);

#endif
