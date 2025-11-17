/*! \file qcore.h
 *  \brief A collection of the core structures needed for the
 *  Q-Digest implementation.
 *
 *  This header file contains a collection of C structs and methods
 *  to operate on them. Methods are implemented as functions taking
 *  a pointer to a struct QDigest as input to avoid passing the
 *  entire structure by value and making a copy every time.
 *
 */

#ifndef QCORE
#define QCORE
#include <stdbool.h>
#include <stdio.h>

/* ======================== STRUCT DEFINITIONS ==================*/
/* Declare QDigestNode, the building block of the Data Structure */

/** 
  *  @brief A struct representing a node in the Q-Digest data
  *  structure.
  *
  *  This struct represents the main building block of the Q-Digest
  *  data structure.
  *
  *  The node can contain a range going from [lower, upper], so
  *  that both bounds are included. 
  */
struct QDigestNode {
  // initialize pointers to left, right and parent nodes
  struct QDigestNode *left;     /**< Pointer to the left child. */
  struct QDigestNode *right;    /**< Pointer to the right child. */
  struct QDigestNode *parent;   /**< Pointer to the parent node */
  size_t count;                 /**< Number of items aggregated */
  size_t lower_bound;           /**< Lower bound of covered range. */
  size_t upper_bound;           /**< Upper bound of covered range. */
};

/**
 *  @brief A struct representing the Q-Digest data structure.
 */
struct QDigest {
  struct QDigestNode *root;     /**< Pointer to the root node */
  size_t num_nodes;             /**< The number of nodes currently in the Q-Digest. */
  size_t N;                     /**< The size of the `universe` (i.e., the maximum size that can appear in the stream.) */
  size_t K;                     /**< The compression parameter, a tunable accuracy-memory tradeoff parameter. Smaller K => more compression => higher error, lower memory. The opposite is true. */ 
  size_t num_inserts;           /**< The total number of inserted values (used to enforce the compression invariant) \f$count < num\_inserts / K\f$ */
};

/* ================= FUNCTION PROTOTYPES =======================*/

/**
 *  @brief This function computes the logarithm base 2 of a number.
 *  The ceiling function is applied to return the closest 
 *  upper integer.
 *
 *  @param n a positive integer.
 */
size_t log_2_ceil(size_t n);

/**
 *  @brief This function creates a QDigestNode, allocating memory 
 *  for it and initializing its lower and upper bound.
 *
 *  @param lower_bound a positive integer representing the lower
 *  bound that can be represented by this node.
 *
 *  @param upper_bound a positive integer representing the upper
 *  bound that can be represented by this node.
 */
struct QDigestNode *create_node(size_t lower_bound, size_t upper_bound);

/**
 *  @brief This function creates a QDigest, allocating memory for
 *  it and initializing its root, the number of nodes contained,
 *  the maximum value of the considered universe (N), the 
 *  compression parameter (K), and the number of inserts performed.
 *
 *  @param root a pointer to a QDigestNode which will represent
 *  the root of the tree.
 *
 *  @param num_nodes an integer representing the number of nodes 
 *  contained in the Q-Digest.
 *
 *  @param N a positive integer representing the largest possible 
 *  number that can be stored in the Q-Digest.
 *
 *  @param K an integer representing the compression factor.
 *
 *  @param num_inserts the total number of insertions.
 */
struct QDigest *create_q(struct QDigestNode *root, size_t num_nodes, size_t N,
                         size_t K, size_t num_inserts);
struct QDigest *create_tmp_q(size_t K, size_t upper_bound);
void free_tree(struct QDigestNode *n);
void delete_qdigest(struct QDigest *q);
size_t node_and_sibling_count(struct QDigestNode *n);
void delete_node(struct QDigestNode *n); 
bool delete_node_if_needed(struct QDigest *q, struct QDigestNode *n, int level,
                           int l_max);
void compress(struct QDigest *q, struct QDigestNode *n, int level, int l_max,
              size_t nDivk);
void print_tree(struct QDigest *q);
void swap_q(struct QDigest *a, struct QDigest *b);
void compress_if_needed(struct QDigest *q);
void expand_tree(struct QDigest *q, size_t upper_bound);
void insert(struct QDigest *q, size_t key, unsigned int count,
            bool try_compress);
void insert_node(struct QDigest *q, const struct QDigestNode *n);
void expand_tree(struct QDigest *q, size_t upper_bound);
size_t postorder_by_rank(struct QDigestNode *n, size_t *curr_rank,
                         size_t req_rank);
void merge(struct QDigest *q1, const struct QDigest *q2);
size_t percentile(struct QDigest *q, double p);

/* ================= SERIALIZATION FUNCTIONS =======================*/
char *preorder_to_string(struct QDigestNode *n, char *buf, size_t *buf_length);
void to_string(struct QDigest *q, char *buf, size_t *buf_length);
struct QDigest *from_string(char *buf);

#endif
