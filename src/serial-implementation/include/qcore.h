#ifndef QCORE
#define QCORE
#include <stdio.h>

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
#endif
