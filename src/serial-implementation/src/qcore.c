/* The following program has been personally re-adapted from the following
 * GitHub repo:
 * https://github.com/dhruvbird/q-digest/tree/master/qdigest.h */

#include "../include/qcore.h"
#include "../include/memory_utils.h"
#include "../include/queue.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* This function is used to compute the base-2 logarithm of a given input n */
size_t log_2_ceil(size_t n) {
    /* Check if the given n is already a power of 2 with bitwise operations */
    // edge case
    if (n == 0) return 0;
    bool is_pow2 = (n & -n) == n;
    int res = 0;
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

/* Frees memory that was allocated to the QDigest tree */
void free_tree(struct QDigestNode *n) {
    // if NULL pointer no need to free memory
    if (!n) return;
    // destroy left subtree
    free_tree(n->left);
    // destroy right subtree
    free_tree(n->right);
    // free the remaining node
    free(n);
}

/* Deletes the entire QDigest by starting from the root
 * node and freeing recursively the left and right subtree */
void delete_qdigest(struct QDigest *q) {
    free_tree(q->root);
    free(q);
}

/* This function returns the aggregated count for a node and its siblings. */
size_t node_and_sibling_count(struct QDigestNode *n) {
    size_t ret = n->count;

    // check if the pointer is not NULL (i.e., that node exists) and then update
    // count
    if (n->left)
        ret += n->left->count;
    if (n->right)
        ret += n->right->count;

    return ret;
}

/* Determines which tree nodes can be deleted.
 * A tree node which has a count of 0 can be deleted only if it has no children.
 *
 * Returns 'true' or 'false' depending on whether it deleted the node n from the
 * tree.
 * */
bool delete_node_if_needed(struct QDigest *q, struct QDigestNode *n, int level,
                           int l_max) {
    if (n->count == 0 && (!n->left && !n->right)) {
        if (n->parent) {
            if (n->parent->left == n) {
                n->parent->left = NULL;
            } else {
                n->parent->right = NULL;
            }
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

void print_tree(struct QDigest *q) {
    printf("[TREE] num_nodes: %lu, (N, K): (%lu, %lu)\n", q->num_nodes, q->N,
           q->K);
}

void swap_q(struct QDigest *a, struct QDigest *b) {
    struct QDigestNode *tmp_root = a->root;
    a->root = b->root;
    b->root = tmp_root;

    size_t tmp_num_nodes = a->num_nodes;
    a->num_nodes = b->num_nodes;
    b->num_nodes = tmp_num_nodes;

    size_t tmp_N = a->N;
    a->N = b->N;
    b->N = tmp_N;

    size_t tmp_K = a->K;
    a->K = b->K;
    b->K = tmp_K;

    size_t tmp_inserts = a->num_inserts;
    a->num_inserts = b->num_inserts;
    b->num_inserts = tmp_inserts;
}

void compress_if_needed(struct QDigest *q) {
    if (q->num_nodes >= (q->K * 6)) {
        const size_t nDivk = (q->N / q->K);
        const int l_max = log_2_ceil(q->root->upper_bound + 1);
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
        // printf("DEBUG: before expand tree\n");
        expand_tree(q, new_upper_bound_plus_one);
    }
    size_t lower_bound = 0;
    size_t upper_bound = q->root->upper_bound;

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
        struct QDigest *old = tmp;
        swap_q(q, tmp);
        delete_qdigest(old);
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

    struct QDigest *old = tmp;
    swap_q(q, tmp);
    delete_qdigest(old);
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
    *curr_rank += n->count;
    return val;
}

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
    return postorder_by_rank(q->root, &curr_rank, req_rank);
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
    struct queue *qu = create_queue();
    push(qu, create_queue_node(q1->root));
    push(qu, create_queue_node(q2->root));
    while (!is_empty(qu)) {
        struct QDigestNode *n = front(qu);
        pop(qu);
        if (n->left) {
            push(qu, create_queue_node(n->left));
        }
        if (n->right) {
            push(qu, create_queue_node(n->right));
        }
        insert_node(tmp, n);
    }
    compress_if_needed(tmp);
    struct QDigest *old = tmp;
    swap_q(q1, tmp);
    delete_qdigest(old);
    delete_queue(qu);
}

/* ================= SERIALIZATION FUNCTIONS =======================*/

/* Functions in this section utilize a buffer (buf) to communicate */

/*
 * Perform a pre-order traversal of the tree and serialize all the
 * nodes with a non-zero count. Separates each node with a newline (\n).
 * FIXED: changed function signature to ensure that pointer to buf would be
 * returned across recursive calls so that all nodes would avoid overwriting
 * the results of previous nodes.
 */
char *preorder_to_string(struct QDigestNode *n, char *buf) {
    if (!n) return buf;
    if (n->count > 0) {
        buf += sprintf(buf, "%zu %zu %zu\n",
                       n->lower_bound,
                       n->upper_bound,
                       n->count);
    }
    buf = preorder_to_string(n->left, buf);
    buf = preorder_to_string(n->right, buf);
    return buf;
}

void to_string(struct QDigest *q, char *buf) {
    struct QDigestNode *root = q->root;
    buf += sprintf(buf, "%zu %zu %zu %zu\n",
                   q->N,
                   q->K,
                   root->lower_bound,
                   root->upper_bound);

    buf = preorder_to_string(root, buf);
}

/* Deserialize the tree from the serialized version in the string
 * 'buf'. The serialized version is obtained by calling 
 * to_string() */
struct QDigest *from_string(char *buf) {

    /* Retrieve information from header and assign values 
     * to members of the QDigest struct */
    size_t _N, _K, _lower_bound, _upper_bound;
    int chars = 0;
    if (sscanf(buf, "%zu %zu %zu %zu\n%n",
               &_N, &_K, &_lower_bound, &_upper_bound, &chars) != 4)
    {
        return NULL; // check for invalid header
    }
    buf += chars;

    /* Initialize new QDigest struct by inheriting from the serialized version */
    struct QDigest *q = create_tmp_q(_K, _upper_bound);
    struct QDigestNode *root = create_node(_lower_bound, _upper_bound);
    q->root = root;

    while (true) {
        size_t lower_bound, upper_bound, count;
        int consumed = 0;
        int n = sscanf(buf, "%zu %zu %zu%n",
                       &lower_bound, &upper_bound, &count, &consumed);
        if (n != 3) {
            break;   // no more nodes
        }
        buf += consumed;
        struct QDigestNode *node = create_node(lower_bound, upper_bound);
        node->count = count;
        insert_node(q, node);
    }

    return q;
}

#ifdef TESTCORE

int main(void) {
    struct QDigest *q = create_tmp_q(1, 1);
    insert(q, 1, 4, true);
    insert(q, 2, 5, true);
    insert(q, 0, 2, true);
    char buf[100];

    to_string(q, buf);
    struct QDigest *q_copied = from_string(buf);

    printf("%s\n", buf);
    printf("%zu\n", q->num_inserts);
    printf("%zu\n", q_copied->num_inserts);
    return 0;
}

#endif
