#ifndef __QDIGEST_H__
#define __QDIGEST_H__ 

#include <stdlib.h> 
#include <stdbool.h>
#include <stdio.h>

typedef struct QDigestNode {
   struct QDigestNode *left, *right, *parent;
   size_t count; 
   size_t lb, ub; // Range is [lb..ub] (i.e. both inclusive)
                  // E.g. values ranging from 0-10, means the    
                  // max number of value in range is: ub + 1 = 11.
} QDigestNode;

typedef struct QDigest {
   QDigestNode *root;
   size_t num_nodes;
   size_t N, k;         /* N is total count of inserted elements 
                         * k is the compression parameter */ 
   size_t num_inserts;  
} QDigest;

/* QDigest related external linked functions (visible to the linker) */
QDigest *qdigestCreate(size_t _k, size_t ub);
void qdigestRelease(QDigest *p);
void swap(QDigest *src, QDigest *other);
size_t size(QDigest *qdig);
bool empty(QDigest *qdig);
double compression_ratio(QDigest *qdig);
size_t percentile(QDigest *qdig, double p);
void merge(QDigest *src, QDigest const *other, size_t S); 

#endif /* __QDIGEST_H__ */ 
