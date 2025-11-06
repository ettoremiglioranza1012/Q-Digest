 /* Copyright (C) 2025 Ettore Miglioranza <ettoremiglioranza1012>
 * This software is released under the GPL license version 2.0 */

#include <stdbool.h> 
#include <stdlib.h>
#include "qdigest.h"


/*=====================================*/ 
/* Static private functions prototypes */
/*=====================================*/ 

static size_t log2Ceil(size_t n);
static void _insert(QDigest *qdig, size_t key, unsigned int count, bool try_compact, size_t S);
static void compress_if_needed(QDigest *qdig, size_t S);
static void compress(QDigestNode *n, int level, int l_max, size_t nDivK);
size_t node_and_sibbling_count(QDigestNode *n);
bool delete_node_if_needed(QDigestNode *n, int level, int l_max);
void expandTree(size_t ub);
void _insert_node(const QDigestNode *n);
void preorder_toString(QDigestNode *n, FILE *out); 


/*======================================*/ 
/* Public API of QDigest Data Structure */
/*======================================*/ 
 
/* This function allocate memory space for QDigest Data Structure 
 * and return a pointer to this memory space */
QdigestNode *qdigestCreate(size_t lb, size_t ub) 
{
   struct QDigest *qdig;
   if ((qdig = malloc(sizeof(*qdig))) == NULL) {
      return NULL;
   }
   qdig->root = NULL;
   qdig->num_nodes = 1;
   qdig->N = 0;
   qdig->k = lb;
   qdig->num_inserts = 0; 
   return qdig;
} 

/* This function releases the memory allocated for 
 * each nodes of the Qdigest binary tree recursively */ 
void qdigestnodeRelease(QDigestNode *qnode)
{
   if (!qnode) return;
   qdigestnodeRelease(qnode->left);
   qdigestnodeRelease(qnode->right);
   free(qnode);
   return; 
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
   /* Swap must be performed by using the reference to where
    * pointers point to. */ 
   QDigest tmp = *src;  // Creates a temporary QDigest structure and 
                        // copies the contents of *src into it.
   *src = *other;       // Copies the contents of *other into the location
                        // *src points to.      
   *other = tmp;        // Copies the saved contents from tmp into the 
                        // location *other points to. 
}

/* This function inserts a key with a certain count in a binary tree 
 * representing the QDigest data structure. 
 * The actual mechanism is opaque and really implemented in the static 
 * function _insert. */ 
void insert(QDigest *qdig, size_t key, unsigned int count, size_t S)
{
   const bool try_compact = true;
   _insert(*qdig, key, count, try_compact, S); 
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
   if (key > qdig->root->ub) { expandTree(key); } 
   /* To Implement */  
   if (try_compact) compress_if_needed(*qdig);
}

/* This function is called when we want to insert a new key value  
 * in the current QDigest data structure, but the key value > ub. 
 * Thus, we expand the tree to [lb, ub'= key]. */ 
static void expandTree(size_t ub)
{
   // To DO 
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
      compress(qdig->root, 0, max_depth, nDivk);
    }
}

static void compress(QDigestNode *n, int level, int l_max, size_t nDivK)
{
   // To DO; 
}

