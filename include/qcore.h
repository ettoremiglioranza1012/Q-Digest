#ifndef QCORE
#define QCORE
#include <stdbool.h>
#include <stdio.h>

/* ======================== STRUCT DEFINITIONS ==================*/
/* Declare QDigestNode, the building block of the Data Structure */
struct QDigestNode {
  // initialize pointers to left, right and parent nodes
  struct QDigestNode *left, *right, *parent;
  size_t count;
  // variables to store the upper and lower bound of the node.
  // Range contained is [min, max], so that both bounds are included.
  size_t lower_bound, upper_bound;
};

/* Declare QDigest structure */
struct QDigest {
  // pointer to the node representing the root
  struct QDigestNode *root;
  size_t num_nodes;
  size_t N, K;
  size_t num_inserts;
};

/* ================= FUNCTION PROTOTYPES =======================*/
size_t log_2_ceil(size_t n);
struct QDigestNode *create_node(size_t lower_bound, size_t upper_bound);
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
char *preorder_to_string(struct QDigestNode *n, char *buf);
void to_string(struct QDigest *q, char *buf);
struct QDigest *from_string(char *buf);

#endif
