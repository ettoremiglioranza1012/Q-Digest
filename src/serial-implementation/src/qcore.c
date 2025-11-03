#include "../include/qcore.h"
#include "../include/memory_utils.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This function is used to compute the base-2 logarithm of a given input n */
size_t log_2_ceil(size_t n) {
  /* Check if the given n is already a power of 2 with bitwise operations */
  bool is_pow2 = (n & -n) == n;
  /* Initialize return value to 0 */
  int res = 0;
  /* While n is greater than 1 increase result by 1 */
  while (n > 1) {
    n /= 2;
    res++;
  }
  /* Return the result as is if n was a power of 2, otherwise add 1 to it */
  return res + (is_pow2 ? 0 : 1);
}

/* This function allocates memory for a node and initializes some of its
 * members. */
struct QDigestNode *create_node(size_t lower_bound, size_t upper_bound) {
  struct QDigestNode *ret = xmalloc(sizeof(struct QDigestNode));

  // make all pointers NULL pointers
  ret->left = ret->right = ret->parent = NULL;
  // make count start from 0
  ret->count = 0;

  // assign both a lower and upper bound to respective struct members
  ret->lower_bound = lower_bound;
  ret->upper_bound = upper_bound;

  return ret;
}

/* This function deletes a node and frees the memory that was allocated to it */
void delete_node(struct QDigestNode *n) { free(n); }

/* TODO: implement the create_q function similar to what done above */
struct QDigest *create_q(struct QDigestNode *root, size_t num_nodes, size_t N,
                         size_t K, size_t num_inserts) {
  struct QDigest *ret = xmalloc(sizeof(struct QDigest));
  ret->root = root;
  ret->num_nodes = num_nodes;
  ret->N = N;
  ret->K = K;
  ret->num_inserts = num_inserts;

  return ret;
}

/* Constructor for a special case used inside the expand_tree function */
struct QDigest *create_tmp_q(size_t K, size_t upper_bound) {
  struct QDigest *tmp = xmalloc(sizeof(struct QDigest));
  struct QDigestNode *n = create_node(0, upper_bound);
  tmp->root = n;
  tmp->num_nodes = 1;
  tmp->N = 0;
  tmp->K = K;
  return tmp;
}
/* This function returns the aggregated count for a node and its siblings. */
size_t node_and_sibling_count(struct QDigestNode *n) {
  size_t ret = n->count;

  // check if the pointer is not NULL (i.e., that node exists) and then update
  // count
  if (n->left)
    ret += n->left->count;
  if (n->right)
    ret += n->left->count;

  return ret;
}

/* Determines which tree nodes can be deleted.
 * A tree node which has a count of 0 can be deleted only if it has no children.
 *
 * Returns 'true' or 'false' depending on whether it deleted the node n from the
 * tree.
 * TODO: check if memory allocation logic is correct */
bool delete_node_if_needed(struct QDigest *q, struct QDigestNode *n, int level,
                           int l_max) {
  if (n->count == 0 && (!n->left && !n->right)) {
    if (n->parent->left == n) {
      n->parent->left = NULL;
    } else {
      n->parent->right = NULL;
    }

    delete_node(n);
    (q->num_nodes)--;
    return true;
  }
  return false;
}

void compress(struct QDigest *q, struct QDigestNode *n, int level, int l_max,
              size_t nDivk) {
  if (!n)
    return;

  compress(q, n->left, level + 1, l_max, nDivk);
  compress(q, n->right, level + 1, l_max, nDivk);

  if (level > 0) {
    bool deleted = delete_node_if_needed(q, n, level, l_max);
    if (!deleted && node_and_sibling_count(n->parent) < nDivk) {
      struct QDigestNode *par = n->parent;
      par->count = node_and_sibling_count(par);

      if (par->left) {
        par->left->count = 0;
        delete_node_if_needed(q, par->left, level, l_max);
      }
      if (par->right) {
        par->right->count = 0;
        delete_node_if_needed(q, par->right, level, l_max);
      }

    } // if (!deleted && ...)
  } // if (level > 0)
}

/* TODO: implement print_tree */
void print_tree() { ; }

/* TODO: implement swap function which should mirror the std::swap from C++ */
void swap_q(struct QDigest *q1, struct QDigest *q2) {
  struct QDigest *tmp = xmalloc(sizeof(struct QDigest));
  tmp = q1;
  q1 = q2;
  q2 = tmp;
  free(tmp);
}

void compress_if_needed(struct QDigest *q) {
  // TODO: why fixed to 6 (?)
  if (q->num_nodes >= (q->K * 6)) {
    const size_t nDivk = (q->N / q->K);
    const int l_max = log_2_ceil(q->root->upper_bound + 1);
    // TODO: check if this is problematic in terms of modifying struct in place
    compress(q, q->root, 0, l_max, nDivk);
  }
}

void expand_tree(struct QDigest *q, size_t upper_bound);

/* Bump up the count for key by count.
 *
 * If try_compact is true then attempt compaction if
 * applicable. Don't compact when we want to build a tree
 * which has a specific shape since it is assumed that certain
 * nodes will be present at specific positions (for example when called by
 * expand_tree()).
 * */
void insert(struct QDigest *q, size_t key, unsigned int count,
            bool try_compress) {
  if (key > q->root->upper_bound) {
    size_t new_upper_bound_plus_one = 1 << log_2_ceil(key);
    if (q->root->upper_bound + 1 == new_upper_bound_plus_one) {
      new_upper_bound_plus_one *= 2;
    }
    expand_tree(q, new_upper_bound_plus_one);
  }
  size_t lower_bound = 0;
  size_t upper_bound = q->root->upper_bound;

  // TODO: should these be managed by malloc (?)
  struct QDigestNode *prev = q->root;
  struct QDigestNode *curr = prev;

  while (lower_bound != upper_bound) {
    size_t mid = lower_bound + (upper_bound - lower_bound) / 2;
    prev = curr;
    if (key <= mid) {
      // go left
      if (!curr->left) {
        struct QDigestNode *new_node = create_node(lower_bound, mid);
        prev->left = new_node;
        prev->left->parent = prev;
        (q->num_nodes)++;
      }
      curr = prev->left;
      upper_bound = mid;
    } else {
      // go right
      assert(mid + 1 <= upper_bound);
      if (!curr->right) {
        struct QDigestNode *new_node = create_node(mid + 1, upper_bound);
        prev->right = new_node;
        prev->right->parent = prev;
        (q->num_nodes)++;
      }
      curr = prev->right;
      lower_bound = mid + 1;
    }
  } // while()
  curr->count += count;
  q->N += count;
  if (try_compress) {
    compress_if_needed(q);
  }
}

/*
 * Insert the equivalent of the values present in node n into
 * the current tree. This will either create new nodes along the
 * way and then create the final node or will update the count in
 * the destination node if that node is already present in the
 * tree. No compression is attempted after the new node is inserted
 * since this function is assumed to be called by the
 * deserialization routine.
 * */

void insert_node(struct QDigest *q, const struct QDigestNode *n) {
  struct QDigestNode *r = q->root;
  assert(n->lower_bound >= r->lower_bound);
  assert(n->upper_bound <= r->upper_bound);

  struct QDigestNode *prev = q->root;
  struct QDigestNode *curr = prev;

  while (curr->lower_bound != n->lower_bound ||
         n->upper_bound != curr->upper_bound) {
    size_t mid =
        curr->lower_bound + (curr->upper_bound - curr->lower_bound) / 2;
    prev = curr;
    if (n->upper_bound <= mid) {
      // go left
      if (!prev->left) {
        struct QDigestNode *new_node = create_node(curr->lower_bound, mid);
        prev->left = new_node;
        prev->left->parent = prev;
        (q->num_nodes)++;
      }
      curr = prev->left;
    } else {
      // go right
      assert(mid + 1 <= curr->upper_bound);
      if (!prev->right) {
        struct QDigestNode *new_node = create_node(mid + 1, curr->upper_bound);
        prev->right = new_node;
        prev->right->parent = prev;
        (q->num_nodes)++;
      }
      curr = prev->right;
    }
  } // while()
  assert(curr->lower_bound == n->lower_bound);

  // curr should get the contents of n
  curr->count += n->count;
  q->N += n->count;
}

void expand_tree(struct QDigest *q, size_t upper_bound) {
  assert(upper_bound - 1 > q->root->upper_bound);
  // check that the upper_bound is a power of 2
  assert((upper_bound & (-upper_bound)) == upper_bound);

  upper_bound--;

  struct QDigest *tmp = create_tmp_q(q->K, upper_bound);

  if (q->N == 0) {
    swap_q(q, tmp);
    return;
  }

  const bool try_compress = false;
  insert(tmp, q->root->upper_bound, 1, try_compress);

  // Intuitively, we keep going down the left child until we find
  // that no left child exists. This is the point where we
  // branched off to the right, and we need to trim the tree below
  // this node and replace this node with the root node of the
  // original tree.

  struct QDigestNode *n = tmp->root;
  while (n->upper_bound != q->root->upper_bound) {
    n = n->left;
  }
  struct QDigestNode *par = n->parent;
  int to_remove = 0;
  while (n) {
    n = n->right;
    ++to_remove;
  }
  par->left = q->root;
  // this is a workaround to emulate what .release() does in C++ for smart
  // pointers
  q->root = NULL;
  par->left->parent = par;

  tmp->num_nodes -= to_remove;
  tmp->num_nodes += q->num_nodes;
  tmp->N = q->N;

  swap_q(q, tmp);
}

/*
 * Perform a post-order traversal of the tree and fetch the
 * element at rank `req_rank` starting from the smallest
 * element in the structure.
 * */
size_t postorder_by_rank(struct QDigestNode *n, size_t *curr_rank,
                         size_t req_rank) {
  if (!n)
    return 0;
  size_t val = postorder_by_rank(n->left, curr_rank, req_rank);
  if (*curr_rank >= req_rank)
    return val;
  val = postorder_by_rank(n->right, curr_rank, req_rank);
  if (*curr_rank >= req_rank)
    return val;

  val = n->upper_bound;
  curr_rank += n->count;
  return val;
}

/*
 * Perform a pre-order traversal of the tree and serialize all the
 * nodes with a non-zero count. Separates each node with a newline
 * (\n).
 *
 * */
// TODO: implement this
// void preorder_to_string(struct QDigestNode *n)

/*
 * Returns the approximate 100p'th percentile element in the
 * structure. i.e., passing in 0.7 will return the 70th percentile
 * element (which is the 70th percentile element starting from the
 * smallest element).
 *
 * */
size_t percentile(struct QDigest *q, double p) {
  // p is in the range [0,1]
  size_t curr_rank = 0;
  const size_t req_rank = p * q->N;
  // TODO: unsure about this way of implementing this
  return postorder_by_rank(q->root, &curr_rank, req_rank);
}

/*
 * Serialized format consists of newline separated entries which
 * are triplets of the form: (LB, UB, COUNT)
 *
 * That means that we have a node which has COUNT elements which
 * have values in the range [LB, UB]. Only non-empty ranges will
 * be serialized (i.e., the serialized tree will be sparse). Also,
 * the ranges will be serialized in pre-order tree traversal so
 * that re-construction is easy.
 *
 * */
char *to_string(struct QDigest *q) {
  struct QDigestNode *r = q->root;
  // TODO: declare this 1000 with a macro so that it can be changed easily
  char *buf = malloc(1000);
  sprintf(buf, "%lu %lu %lu %lu\n", q->N, q->K, r->lower_bound, r->upper_bound);
  size_t str_len = strlen(buf);
  // TODO: finish implementing this, have a look at code I wrote to write
  // strings to buffer using fgets
}

/*
 * Merge two qdigests with q2 being the one that is merged into q1.
 * Therefore, q2 is declared constant since it should not be modified
 * */
void merge(struct QDigest *q1, const struct QDigest *q2) {
  // pick the maximum K between the two QDigests
  const size_t max_k = (q1->K > q2->K) ? q1->K : q2->K;
  const size_t max_upper_bound = (q1->root->upper_bound > q2->root->upper_bound)
                                     ? q1->root->upper_bound
                                     : q2->root->upper_bound;

  struct QDigest *tmp = create_tmp_q(max_k, max_upper_bound);

  // TODO: implement queue
}
