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

/**
 *  @brief This function creates a Q-Digest by using some defaults
 *  values to mirror the behavior of the class constructor in
 *  the original C++ implementation. Specifically it inserts a
 *  root node initialized with a lower bound of 0 up to upper_bound.
 *  It initializes the num_nodes parameter to 1, and N to 0.
 *
 *  @param K a positive integer representing the compression
 *  parameter.
 *
 *  @param upper_bound a positive integer representing the
 *  maximum range that can contained in the root node.
 */
struct QDigest *create_tmp_q(size_t K, size_t upper_bound);

/**
 *  @brief This function safely frees memory that was dynamically
 *  allocated to build the Q-Digest. This effectively acts as a
 *  destructor of the struct.
 *
 *  @param n a pointer to a QDigestNode. This usually is the root
 *  of the Q-Digest since then the function recursively traverses
 *  the left and right subtree to free nodes and their contents.
 *
 * */
void free_tree(struct QDigestNode *n);

/**
 *  @brief This is a wrapper function around free_tree that
 *  automatically destroys and frees memory from a QDigest struct.
 *  The function applies free_tree on q->root.
 *  
 *  @param q a pointer to a QDigest struct that needs to be
 *  destroyed.
 *
 * */
void delete_qdigest(struct QDigest *q);

/** 
 *  @brief This function returns the aggregated count for a node 
 *  and its siblings.
 *
 *  @param n a pointer to a QDigestNode.
 *
 * */
size_t node_and_sibling_count(struct QDigestNode *n);

/** 
 *  @brief This function deletes a node and frees the memory that 
 *  was allocated to it.
 *
 *  @param n a pointer to a QDigestNode.
 * */
void delete_node(struct QDigestNode *n); 

/** 
 *  @brief Determines which tree nodes can be deleted. A tree node 
 *  which has a count of 0 can be deleted only if it has no 
 *  children. 
 *  Returns 'true' or 'false' depending on whether it deleted 
 *  the node n from the tree.
 *
 *  @param q a pointer to the QDigest which should be affected
 *  by the deletion.
 *
 *  @param n a pointer to a QDigestNode which should be considered
 *  for deletion.
 *
 *  @param level implemented for compatibility with the compress
 *  function
 *
 *  @param l_mac implemented for compatibility with the compress
 *  function
 *
 * */
bool delete_node_if_needed(struct QDigest *q, struct QDigestNode *n, int level, int l_max);

/** 
 *  @brief This function attempts compression on the Q-Digest
 *  structure. The function is applied recursively by traversing
 *  the left and right subtrees.
 *  The function calls delete_node_if_needed() to check which nodes
 *  should be deleted. If a node has not been deleted, and if the
 *  count of itself and its siblings is less than the compression
 *  ratio (N/K) then the children nodes are visited and it is
 *  checked if they are ready for deletion.
 *  By the time the function returns, the Q-Digest has been cleaned
 *  of all the nodes that have a low count, thus achieving a degree
 *  of compression.
 *
 *  @param q a pointer to a QDigest struct that should be changed.
 *
 *  @param n a pointer to a QDigestNode struct from which the search
 *  should begin. This is usually the root of the Q-Digest.
 *
 *  @param level an integer representing the current node level
 *  of the node in the Q-Digest
 *
 *  @param l_max an integer representing the maximum depth of the
 *  Q-Digest
 *
 *  @param nDivk a positive integer representing the compression
 *  ratio described in the paper. This makes sure that no nodes 
 *  with a low count are left in the Q-Digest after compression.
 *
 * */
void compress(struct QDigest *q, struct QDigestNode *n, int level, int l_max, size_t nDivk);

/** 
 *  @brief q a pointer to a QDigest struct whose number of nodes,
 *  N, and K parameters will be printed.
 *
 *  @param q a pointer to a QDigest struct.
 *
 * */
void print_tree(struct QDigest *q);

/** 
 *  @brief This function swaps the contents of two Q-Digests.
 *
 *  @param a pointer to a QDigest struct to be swapped
 *
 *  @param b pointer to a QDigest struct to be swapped
 *
 * */
void swap_q(struct QDigest *a, struct QDigest *b);

/** 
 *  @brief This is the function that gets called directly in most
 *  implementations and acts as a wrapper around compress().
 *  This function automatically computes the compression ratio
 *  (nDivk) and the maximum depth of the Q-Digest (l_max).
 *  Then compress() is called on the root node of the Q-Digest.
 *
 *  @param q a pointer to a QDigest struct to be compressed.
 * */
void compress_if_needed(struct QDigest *q);

/** 
 *  @brief This function expands a QDigest whose value universe is 
 *  too small by embedding its existing tree into a larger QDigest 
 *  with a higher upper bound.
 *  The QDigest data structure represents values in the range 
 *  [0, N), where N must be a power of two. When the caller 
 *  needs to insert a value outside the current universe size, the 
 *  tree must be expanded so that the digest covers a larger 
 *  power-of-two range. This function performs that expansion.
 *
 *  @param q a pointer to the QDigest struct to be expanded
 *
 *  @param upper_bound The new universe size (must be a power of 
 *  two), and must be strictly larger than the digest’s current 
 *  upper bound. The expanded digest will represent values in 
 *  [0, upper_bound).
 *
 * */
void expand_tree(struct QDigest *q, size_t upper_bound);

/** 
 *  @brief Inserts a value (or multiple occurrences of a value) 
 *  into a QDigest.
 *
 *  A QDigest represents integer values over a universe [0, N) 
 *  where N is always a power of two. Each inserted value is routed 
 *  from the root to the appropriate leaf, creating missing nodes 
 *  along the path as needed.
 *  The node corresponding to the exact key range then has its count
 *  increased. Optional post-insertion compression may occur 
 *  depending on the `try_compress` flag.
 *
 *  @param q A pointer to a struct QDigest* representing the digest 
 *  into which the value is being inserted.
 *
 *  @param key The integer value to insert. If the key exceeds the 
 *  digest’s current universe size, the digest is first expanded 
 *  so that key lies within the new universe range.
 *
 *  @param count How many occurrences of the value to insert. This 
 *  is added to the count of the leaf node that represents `key`.
 *
 *  @param try_compress A boolean flag. If true, the digest may 
 *  perform a compression pass after insertion. If false, the caller
 *  is responsible for periodically calling compression manually.
 *
 * */
void insert(struct QDigest *q, size_t key, unsigned int count,
            bool try_compress);

/**
 *  @brief Inserts an existing QDigestNode into the digest, creating
 *  intermediate nodes as needed.
 *
 *  This function takes a node `n`—typically originating from another
 *  digest during merge or expansion—and reproduces its path within the
 *  target digest `q`. The algorithm walks the tree from the root,
 *  descending left or right based on the interval boundaries of `n`
 *  until it reaches the exact interval `[n->lower_bound, n->upper_bound]`.
 *  Any missing nodes along the path are created to preserve the canonical
 *  QDigest binary interval structure. Once the corresponding node is
 *  located, its count is increased by `n->count`, and the digest’s global
 *  count is updated accordingly.
 *
 *  The function assumes that `n`’s interval is entirely contained within
 *  the universe of `q`. It does not perform universe expansion; callers
 *  must ensure compatibility beforehand.
 *
 *  @param q A pointer to the QDigest into which the node is being inserted.
 *
 *  @param n A pointer to the QDigestNode whose interval and count are to be
 *  merged into the digest `q`. The interval [n->lower_bound, n->upper_bound]
 *  must lie within the current root interval of `q`.
 *
 * */
void insert_node(struct QDigest *q, const struct QDigestNode *n);

/**
 *  @brief Performs a post-order traversal to locate the value associated
 *  with a given cumulative rank.
 *
 *  This function walks the QDigest tree in post-order (left subtree,
 *  right subtree, then node itself) while maintaining a running count
 *  of the cumulative frequency of visited nodes. The cumulative count is
 *  tracked externally through `curr_rank`, which is incremented by each
 *  node’s count as the traversal progresses.
 *
 *  The goal is to determine the smallest value whose cumulative rank
 *  meets or exceeds `req_rank`. When that condition becomes true, the
 *  function returns the upper_bound of the node that satisfied it,
 *  representing the value associated with the quantile or rank query.
 *
 *  This is a utility commonly used to answer quantile queries in QDigest,
 *  where nodes represent aggregated ranges and are visited in an order
 *  consistent with increasing value.
 *
 *  @param n A pointer to the current QDigestNode being inspected.
 *           A NULL pointer causes the function to return 0 immediately.
 *
 *  @param curr_rank A pointer to a counter tracking the cumulative rank
 *           accumulated so far. This value is incremented by `n->count`
 *           when the node is processed.
 *
 *  @param req_rank The target cumulative rank for which we are searching.
 *           Once `*curr_rank` reaches or exceeds this value, the traversal
 *           unwinds and returns the corresponding node's upper_bound.
 *
 *  @return The upper_bound of the node whose cumulative rank first meets
 *          or exceeds `req_rank`. Returns 0 if the subtree is empty.
 */
size_t postorder_by_rank(struct QDigestNode *n, size_t *curr_rank,
                         size_t req_rank);
/**
 *  @brief Merges the contents of two QDigests into one.
 *
 *  This function combines all nodes from `q2` into `q1`, producing a
 *  single QDigest that reflects the aggregated counts of both digests.
 *  The merge is performed by constructing a temporary QDigest `tmp`
 *  whose universe and compression parameter are chosen to accommodate
 *  both inputs:
 *
 *    - The merged digest uses the *maximum* K value of `q1` and `q2`,
 *      ensuring that the compression guarantee is no weaker than that
 *      of either input.
 *
 *    - The root interval (universe size) is expanded to cover the larger
 *      of the two digests’ upper bounds.
 *
 *  The function performs a breadth-first traversal of both input digests,
 *  pushing every node into `tmp` using `insert_node()`. This operation
 *  reconstructs the canonical QDigest structure while summing the counts
 *  of corresponding intervals.
 *
 *  Once all nodes have been inserted, a compression pass is applied to
 *  restore QDigest invariants. Finally, `q1` is replaced with the merged
 *  digest, and all temporary structures are freed.
 *
 *  @param q1 A pointer to the destination QDigest. After the merge,
 *            `q1` contains the combined contents of both digests.
 *
 *  @param q2 A pointer to the source QDigest. Its nodes are read but not
 *            modified. All key ranges and counts from `q2` are inserted
 *            into the merged digest.
 *
 *  @note The function does not modify `q2`; it may continue to be used
 *        independently after the merge.
 *
 *  @note The resulting digest’s accuracy guarantees follow the stronger
 *        (i.e., larger K) of the two original digests.
 */
void merge(struct QDigest *q1, const struct QDigest *q2);

/**
 *  @brief Computes the value associated with the p-th percentile of the data
 *  stored in the QDigest.
 *
 *  This function uses the cumulative count of values stored in the QDigest
 *  to estimate a percentile. Given a percentile `p` in the range [0, 1], the
 *  function calculates the corresponding rank (i.e., the number of elements
 *  that must be accumulated to reach that percentile) and then performs a
 *  post-order traversal of the digest to locate the value whose cumulative
 *  frequency first meets or exceeds that rank.
 *
 *  Internally, this delegates the search to `postorder_by_rank()`, which
 *  walks the digest in increasing-value order and returns the upper_bound of
 *  the node associated with the requested rank. Because QDigest nodes
 *  represent value ranges, the returned value corresponds to the endpoint of
 *  the interval in which the percentile falls.
 *
 *  @param q A pointer to the QDigest from which the percentile is computed.
 *
 *  @param p A floating-point percentile value in the range [0, 1]. For example,
 *           0.5 corresponds to the median, 0.9 to the 90th percentile, etc.
 *
 *  @return The value associated with the p-th percentile. This is determined
 *          by finding the smallest value whose cumulative rank satisfies
 *          `rank >= p * q->N`.
 *
 *  @note If `q` is empty (i.e., q->N == 0), the behavior is undefined. The
 *        caller must ensure that the digest contains data before requesting
 *        a percentile.
 */
size_t percentile(struct QDigest *q, double p);

/* ================= SERIALIZATION FUNCTIONS =======================*/

/**
 *  @brief Return number of digits for unsigned int value.
 *
 *  This function initialise a counter to one to then return the number 
 *  of digits by dividing the value in input by 10 until the current value
 *  is less then 10, incrementing the digits counter by one at each steps.
 *
 *  @param value A reference to the value we want to know how many digits
 *                it is composed of
 *
 *  @param digits A counter of the number of digits the input value is 
 *                composed of
 *
 *  @return A reference to the number of digits the input value is composed 
 *          of.
 */
size_t _digits(size_t value);
/**
 *  @brief Computes the number of bytes required to serialize a node subtree.
 *
 *  Recursively walks the provided node, summing the number of characters needed
 *  to serialize every descendant with a non-zero count, including delimiters
 *  between the lower bound, upper bound, and count values.
 *
 *  @param n A pointer to the node whose subtree will be measured. NULL returns 0.
 *
 *  @return The exact number of bytes required to serialize the subtree rooted at @p n.
 *
 *  @note The calculation currently adds a hard-coded 3 bytes for the two spaces
 *        and newline that separate the three values in each row. Once we allow
 *        configurable delimiters, this literal will be replaced with the
 *        length of the delimiter strings provided at runtime.
 */
size_t get_bytes_of_node(struct QDigestNode *n);

/**
 *  @brief Computes the buffer size needed to serialize an entire QDigest.
 *
 *  Sums the bytes required for the header line and all nodes with non-zero
 *  counts (by delegating to `get_bytes_of_node()`), and includes space for the
 *  terminating null byte so callers can pre-allocate an adequate buffer.
 *
 *  @param q Pointer to the QDigest whose serialized size is requested.
 *
 *  @return The total number of bytes required for the serialized digest, including
 *          the null terminator. Returns 0 if the digest or root is NULL.
 *
 *  @note The header size calculation currently assumes exactly three spaces and
 *        one newline (4 bytes) in the metadata line. This literal will be replaced
 *        by the computed length of configurable field separators once we support
 *        injecting custom formatting.
 *  @note Another intereseting update would be that for the  explicit +1 for the 
 *        null terminator to become the length of the caller-provided terminator 
 *        sequence when we move to dynamic serialization options.
 */
size_t get_num_of_bytes(struct QDigest *q);

/**
 *  @brief Serializes a QDigest tree into a string using preorder traversal.
 *
 *  This function walks the QDigestNode tree in preorder (node first,
 *  then left child, then right child) and writes the non-zero counts
 *  of each node into a provided buffer. Each line of the output contains
 *  the node's lower bound, upper bound, and count, separated by spaces.
 *
 *  The function updates the buffer pointer as it writes, and also
 *  increments the total length of the string written so far.
 *
 *  @param n A pointer to the current QDigestNode. If NULL, the function
 *           returns immediately without modifying the buffer.
 *
 *  @param buf A pointer to the character buffer where the serialized
 *           output is appended. The caller must ensure the buffer is
 *           large enough to hold the resulting string.
 *
 *  @param length A pointer to a size_t variable that tracks the total
 *           number of characters written to the buffer. This is updated
 *           in-place as nodes are serialized.
 *
 *  @return A pointer to the position in the buffer immediately after
 *          the last character written. This allows recursive calls to
 *          continue writing from the correct location.
 *
 *  @note Only nodes with count > 0 are included in the output.
 *  @note The output format for each node is:
 *        "<lower_bound> <upper_bound> <count>\n"
 */
char *preorder_to_string(struct QDigestNode *n, char *buf, size_t *buf_length);

/**
 *  @brief Serializes an entire QDigest into a string representation.
 *
 *  This function writes the metadata of the QDigest followed by the
 *  preorder traversal of all nodes with non-zero counts. The output
 *  is suitable for saving or debugging the digest.
 *
 *  The first line contains the digest-level metadata:
 *    "<total_count> <K> <root_lower_bound> <root_upper_bound>\n"
 *
 *  Each subsequent line represents a node with count > 0, in preorder:
 *    "<lower_bound> <upper_bound> <count>\n"
 *
 *  @param q A pointer to the QDigest to serialize.
 *
 *  @param buf A pointer to a character buffer where the string output
 *           will be written. The caller must ensure it is large enough
 *           to hold the entire serialized digest.
 *
 *  @param length A pointer to a size_t variable that tracks the total
 *           number of characters written into the buffer. This is updated
 *           in-place.
 *
 *  @note The serialization includes only nodes with count > 0.
 *  @note The function relies on `preorder_to_string()` to serialize
 *        the nodes recursively after writing the metadata.
 */
void to_string(struct QDigest *q, char *buf, size_t *buf_length);

/**
 *  @brief Deserializes a QDigest from a string representation.
 *
 *  This function reconstructs a QDigest from a string previously produced
 *  by `to_string()`. The string is expected to have a header line with
 *  digest metadata, followed by lines representing nodes with non-zero counts
 *  in preorder.
 *
 *  The header line format is:
 *    "<total_count> <K> <root_lower_bound> <root_upper_bound>\n"
 *
 *  Each subsequent line represents a node:
 *    "<lower_bound> <upper_bound> <count>\n"
 *
 *  @param buf A pointer to a null-terminated string containing the serialized
 *           QDigest.
 *
 *  @return A pointer to a newly allocated QDigest reconstructed from the string,
 *          or NULL if the header is invalid. The caller is responsible for
 *          freeing the returned digest using `delete_qdigest()`.
 *
 *  @note The function uses `create_tmp_q()` and `insert_node()` internally to
 *        rebuild the tree structure and maintain canonical QDigest intervals.
 *  @note Only nodes with count > 0 are expected in the serialized string.
 */
struct QDigest *from_string(char *buf);

#endif
