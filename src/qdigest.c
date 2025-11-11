 /* Copyright (C) 2025 Ettore Miglioranza <ettoremiglioranza1012>
 * This software is released under the GPL license version 2.0 */

#include <stdbool.h> 
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "qdigest.h"


/*=====================================*/ 
/* Static private functions prototypes */
/*=====================================*/ 

static size_t log2Ceil(size_t n);
static QDigestNode *qdigestnodeCreate(size_t lb, size_t ub);
static void qdigestnodeRelease(QDigestNode *qnode);
static void _insert(QDigest *qdig, size_t key, unsigned int count, bool try_compact, size_t S);
static void expandTree(QDigest *qdig, size_t ub, size_t S);
static void compress_if_needed(QDigest *qdig, size_t S);
static void compress(QDigest *qdig, QDigestNode *n, int level, int l_max, size_t nDivK);
static bool delete_node_if_needed(QDigest *qdig, QDigestNode *qnode);
static size_t node_and_sibbling_count(QDigestNode *qnode);
static size_t postorder_by_rank(QDigestNode *qnode, size_t *curr_rank, size_t req_rank);
// TO DO!

static void _insert_node(const QDigestNode *n);
void preorder_toString(QDigestNode *n, FILE *out); 


/*======================================*/ 
/* Public API of QDigest Data Structure */
/*======================================*/ 
 
/* This function allocate memory space for QDigest Data Structure 
 * and return a pointer to this memory space */
QDigest *qdigestCreate(size_t k, size_t ub) 
{
   struct QDigest *qdig;
   if ((qdig = malloc(sizeof(*qdig))) == NULL) {
      return NULL;
   }
   qdig->root = qdigestnodeCreate(0, ub);
   qdig->num_nodes = 1;
   qdig->N = 0;
   qdig->k = k;
   qdig->num_inserts = 0; 
   return qdig;
} 

/* This function release allocated memory space for 
 * QDigest data structure */ 
void qdigestRelease(QDigest *qdig)
{
   if (!qdig) return;
   qdigestnodeRelease(qdig->root);
   free(qdig);
   return; 
}

/* This function swap the content of two QDigest 
 * data structure with each other */ 
void swap(QDigest *src, QDigest *other)
{
   QDigestNode *tmp_root = src->root;
   src->root = other->root;
   other->root = tmp_root;
   
   size_t tmp;
   
   tmp = src->num_nodes;
   src->num_nodes = other->num_nodes;
   other->num_nodes = tmp;
   
   tmp = src->N;
   src->N = other->N;
   other->N = tmp;
   
   tmp = src->k;
   src->k = other->k;
   other->k = tmp;   
}

/* This function inserts a key with a certain count in a binary tree 
 * representing the QDigest data structure. 
 * The actual mechanism is opaque and really implemented in the static 
 * function _insert. */ 
void insert(QDigest *qdig, size_t key, unsigned int count, size_t S)
{
   const bool try_compact = true;
   _insert(qdig, key, count, try_compact, S); 
}

/* This function extracts the total number of counts 
 * of a QDigest Tree and return it. */
size_t size(QDigest *qdig)
{
   return qdig->N;
}

/* This function evaluates the emptiness of a QDigest tree
 * and return true if the size is 0, false otherwise. */
bool empty(QDigest *qdig)
{
   return (size(qdig) == 0);
}

/* This function return the compression ratio */
double compression_ratio(QDigest *qdig)
{
   return (double)qdig->num_nodes/(double)qdig->N;
}

/* Returns the approximate 100p'th percentile element in the
 * structure. i.e. passing in 0.7 will return the 70th percentile
 * element (which is the 70th percentile element starting from the
 * smallest element). */
size_t percentile(QDigest *qdig, double p) {
   // Calculate the Target Rank
   size_t req_rank = (size_t)floor(qdig->N*p);
   size_t curr_rank = 0;
   return inorder_by_rank(qdig->root, &curr_rank, req_rank);
}


/*======================================*/ 
/* Static private functions declaration */ 
/*======================================*/

/* This function compute log in base 2 rounded for excess.
 * Used to compute the binary tree depth. */ 
static size_t log2Ceil(size_t n) 
{  
   bool is_pow2 = (n & -n) == n;
   int ret = 0; 
   while (n > 1) { n /= 2; ++ret; }
   return ret + (is_pow2 ? 0: 1);
}

/* This function create a QDigestNode data structure, init of its members
 * and return a pointer to it. */
static QDigestNode *qdigestnodeCreate(size_t _lb, size_t _ub)
{
   struct QDigestNode *qnode;
   if (qnode = malloc(sizeof(*qnode)) == NULL) {
      return NULL;
   }
   qnode->left = qnode->right = qnode->parent = NULL;
   qnode->lb = _lb;
   qnode->ub = _ub;

   return qnode;
}

/* This function releases the memory allocated for 
 * each nodes of the Qdigest binary tree recursively */ 
static void qdigestnodeRelease(QDigestNode *qnode)
{
   if (!qnode) return;
   qdigestnodeRelease(qnode->left);
   qdigestnodeRelease(qnode->right);
   free(qnode);
   return; 
}

/* This function implement the insertion of a key with a certain count
 * in a binary tree representing the QDigest data structure.
 *
 * The key is the numerical value we want to insert, 
 * (e.g. the new sensor measurements) while the count is the 
 * number of times we've seen the measurements. 
 *
 * Expand the tree if necessary: if the key is bigger than the 
 * upper boundary (root->ub), expands the tree to cover a broader range.
 * 
 * Navigate the binary tree: starting from the route, descend the 
 * tree using a binary search algorithm:
 *    - If key <= mid: go left (lower bound);
 *    - If key > mid: go right (upper bound);
 *
 * Create missing nodes on-demand: during the descend, if a sibling node 
 * does not exists, it is created with the proper range [lb, mid] o 
 * [mid+1, ub] 
 *
 * Reach the leaf node: proceed until lb == ub, meaning until the node 
 * with the exact range [key, key] is found (single value node).
 * 
 * Bump up the count for 'key' by 'count'. 
 *
 * Update statistics: Increment N (total number of inserted elements).
 *
 * If 'try_compact' is 'true' then attemp compaction if applicable. 
 * 
 * We don't compact when we want to build a tree which has a 
 * specific shape since we assume that certain nodes will be present
 * at specific position (for example when called by expantTree()). */ 
static void _insert(
    QDigest *qdig, 
    size_t key,
    unsigned int count,
    bool try_compact,
    size_t S)
{
   if (key > qdig->root->ub) { 
      size_t new_ub_plus_one = 1 << log2Ceil(key); // next power of 2 not minor of key
                                                   // in other words, new ub' for key that is a power of 2;
                                                   // Useful to preserve binary tree structure.
      if (qdig->root->ub + 1 == new_ub_plus_one) {
         // Handling the limit case of ub + 1 == new_ub_plus_one:
         // if the new_ub is old_ub + 1, to avoid not necessary tree expansion
         // we double the new_ub granting more future space and reducting
         // number of future expansion. 
         new_ub_plus_one *= 2;
      }
      expandTree(qdig*, new_ub_plus_one, S); // keep in mind we are passing [new_ub + 1] ~ exclusive upper bound!
   } 
   size_t lb = 0;
   size_t ub = qdig->root->ub; 
   QDigestNode *prev = qdig->root; // Instantiating prev node ptr as qdig root
   QDigestNode *curr = prev; // Instantiating curr node ptr to point to prev
   while (lb != ub) {
      size_t mid = lb + (ub - lb) / 2;
      prev = curr; // Update prev to point to what curr is pointing to (Useful for 2nd iteration and on)
      if (key <= mid) {
         // Go left
         if (!curr->left) {
            prev->left = qdigestnodeCreate(lb, mid); // Create new sibling node
            prev->left->parent = prev; // Update parent node ptr of the new sibling node
            ++qdig->num_nodes; // Update count of nodes in QDigest Struct
         }
         curr = prev->left; // Update current node ptr
         ub = mid; // Update ub for left part
         // Keep going down if lb != new_ub === mid;
      } else {
         // Go right
         assert(mid + 1 <= ub);
         if (!curr->right) {
            prev->right = qdigestnodeCreate(mid, ub);
            prev->right->parent = prev;
            ++qdig->num_nodes;
         }
         curr = prev->right;
         lb = mid + 1; // else is evaluating key > mid, so we restart from mid + 1;
         // Keep going
      }
   } // while()
   curr->count += count; // Increase count of the leaf node
   qdig->N += count;  // Increase overl count
   if (try_compact) compress_if_needed(*qdig);
}

/* This function is called when we want to insert a new key value  
 * in the current QDigest data structure, but the key value > ub. 
 * Thus, we expand the tree to [lb, ub'= new_ub_plus_one-1]. Again, we assume ub 
 * passed is exclusive, e.g. ub' = 12 means values rangin from 0 to 11,
 * thus new upper bound will be key-1. */ 
static void expandTree(QDigest *qdig, size_t ub, size_t S)
{
   assert((ub & (-ub)) == ub); // Check if ub is a power of two
   assert(ub - 1 > qdig->root->ub); // Check if inclusive key (or 'new_ub_plus_one') is bigger than actual ub
   --ub; // Switch from exclusive key to inclusive key ('new_ub_plus_one')
   QDigest *tmp = qdigestCreate(qdig->k, ub);

   if (qdig->N == 0) {
      swap(qdig, tmp);
      return;
   }

   const bool try_compact = false;
   /* While expanding the tree, we have created the tmp bigger tree
    * where we have to innest the older tree */
   _insert(tmp, qdig->root->ub, 1, try_compact, S);

   QDigestNode *n = tmp->root;
   while (n->ub != qdig->root->ub) {
      n = n->left;
   }
   QDigestNode *par = n->parent;
   int to_remove = 0;
   while (n) {
      n = n->right; ++to_remove;
   }
   // First: save the reference to the node is going to be freed
   struct QDigestNode *old_subtree = par->left;
   // Graft the original tree 
   par->left = qdig->root;
   par->left->parent = par;
   qdig->root = NULL; 
   // AFTER: release recursively the old temporary subtree
   qdigestnodeRelease(tmp);
   // Update metadata regarding new grafted tree
   tmp->num_nodes -= to_remove;
   tmp->num_nodes += qdig->num_nodes;
   tmp->N = qdig->N;
   // Finally, swap ownership of resources from the tmp new qdig and our main qdig
   swap(qdig, tmp);
   qdigestRelease(tmp);
}

/* This function assess if there is the necessity to compact the 
 * current qdigest tree. 
 * 
 * In the program execution this functions get called when a
 * modification is perfomed on the QDigest tree structure 
 * (e.g. inserting a node ) and we want to compact the new tree. 
 * 
 * To avoid compacting too frequently we add a slack variable:
 * the num of nodes of the new tree must be greater or equal than
 * S times the compression parameter, this way we avoid compacting 
 * after each insertion o too often, ammortising the cost greatly. 
 *
 * More in depth, we know the theorically QDigest grants, with the compression 
 * k parameter, that the number of nodes is kept limited. This is a theoretical 
 * assumption - with the S variable we add some "slack" before forcing compression. 
 * We wait K*S more nodes insted of just K, accumulating insertions and then 
 * we perform a single compression far more efficient. 
 *
 * Like everything in engineering, it's a TRADE_OFF: 
 * the parameter must be enough bigger to avoid too frequent COMPRESSION,
 * but at least smaller enough to avoid using too much MEMORY. 
 *
 * Recomended value in the orignal cpp build is S = 6; 
 * */ 
static void compress_if_needed(QDigest *qdig, size_t S)
{
   if (qdig->num_nodes >= qdig->k*S) {
      const size_t nDivk = (qdig->N / qdig->k); 
      const size_t n = qdig->root->ub + 1;
      const int max_depth = log2Ceil(n);  
      compress(qdig, qdig->root, 0, max_depth, nDivk);
    }
}

/* This function perform compression. Specifically, ensure that no node is too
 * small. i.e. apart from the root node, try to see if we can compress 
 * counts from a set of 3 nodes (i.e. a node, its parent and its sibling)
 * and promote them to the parent node.  
 * 
 * Important: each recursive step comproesses bottom-up:
 * - Children are processed first, so by the time we reach this node,
 *    its subtree is already minimized.
 * - Internal nodes then act as temporary leaves for the next level.
 * - If parent+child total count < nDivK, we merge them into parent.
 * This wau each layer becomes the "Leaf Layer" if the next compression step. */
static void compress(QDigest *qdig, QDigestNode *qnode, int level, int l_max, size_t nDivK)
{
   if (!qnode) return; // Consider only existing node, used to stop post-order search
   
   // Post order search -> first the leafs, then the rest
   compress(qdig, qnode->left, level+1, l_max, nDivK);
   compress(qdig, qnode->right, level+1, l_max, nDivK);

   if (level > 0) { // Skip the root
      bool deleted = delete_node_if_needed(qdig, qnode); // Try to delete empty leaves
      if (!deleted) { // Proceed only if the node wasn't just removed 
         QDigestNode *par = qnode->parent;
         // Next: if the node survived and its parent group is too light (< nDivK), merge
         // that group into the parent
         if (par && node_and_sibbling_count(par) < nDivK) { // Check the parent and its two children's total weight
            /* A leaf reaches this point only if it wasn't deleted earlier (non zero count)
             * An internal node reaches this point because it has at least once child
             * and a parent, but the combined weight of that local trio is below the treshold. 
             * Remember, the childs of the interla have been killed already at this point 
             * by the post order traversing, thus we are evaluating the internal node like
             * a leaf. */
            par->count = node_and_sibbling_count(par); // Increment the count
            // Then safely remove any empty child nodes
            if (par->left) { 
               par->left->count = 0;
               delete_node_if_needed(qdig, par->left);
               }
            if (par->right) {
               par->right->count = 0;
               delete_node_if_needed(qdig, par->right);
            }
         } // if (!deleted && ...)
      }  // if (level > 0)
   }
}

/* This function delete node if the subsequent condition is met:
 *    - A tree which has a no children has a count of 0;
 * Return 'true' or 'false' depending on wheter it deleted the 
 * node 'n' from the tree. */
static bool delete_node_if_needed(QDigest *qdig, QDigestNode *qnode)
{
   if ((!qnode->left && !qnode->right) && qnode->count == 0) {
      qnode->parent->count += qnode->count;
      if (qnode == qnode->parent->left) {
         qnode->parent->left = NULL;
      } else {
         qnode->parent->right = NULL;
      }
      qdigestnodeRelease(qnode);
      --qdig->num_nodes;
      return true;
   }
   return false;
}

/* This function initialise the return count values as the count 
 * of the node itself, which could be == 0 or != 0 as well. Then,
 * it performs two checks, since in each case we perfom different actions.
 * The two checks verify left and right node not NULL state and sum their 
 * current count to the return count value.
 * Function return the total count between node and its sibblings. */
static size_t node_and_sibbling_count(QDigestNode *qnode)
{
   size_t ret = qnode->count;
   if (qnode->left) { ret += qnode->left->count; }
   if (qnode->right) { ret += qnode->right->count; }
   return ret;
}

/* This function perfoms a inorder traversal of a binary tree using the 
 * rank of the nodes to find the range it's looking for. */
static size_t inorder_by_rank(QDigestNode *qnode, size_t *curr_rank, size_t req_rank)
{
   if (!qnode) return 0;
   
   size_t res_left = inorder_by_rank(qnode->left, curr_rank, req_rank);
   if (res_left != 0) return res_left;
   *curr_rank += qnode->count;
   if (*curr_rank >= req_rank) 
      return (qnode->lb + qnode->ub) / 2; 
   
   return inorder_by_rank(qnode->right, curr_rank, req_rank);
}
