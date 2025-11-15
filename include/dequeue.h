#ifndef __DEQUEUE_H__
#define __DEQUEUE_H__
#include "../include/memory_utils.h"
#include "../include/qdigest.h"

// Declare type Item to be a pointer to QDigestNode
typedef struct QDigestNode *Item; 

/* This is the building block of the DS */ 
typedef struct queueNode {
    Item val;
    struct queueNode *prev;
} queueNode;

/* A queue should support the following operations:
 * 1. push values at the end of the list 
 * 2. return value from the beginning without removing it 
 * 3. pop values from the beggining (FIFO protocol) 
 * 4. return wheter it is empty 
 * */
typedef struct queue {
    queueNode *front;
    queueNode *back;
    size_t size; 
} queue;

/* allocate memory for a queueNode */
queueNode *create_queue_node(Item val);
/* allocate memory for a queue */
queue *create_queue(void);
/* release queue nodes recursively */
void release_queue_node(queueNode *n);
/* release queue buffer */
void release_queue(queue *q);
/* push value at the end of the queue */
void push(queue *q, queueNode *n);
/* return value in front without popping it */
Item front(queue *q);
/* pop value and return it */
Item pop(queue *q);
/* Return whether it is empty */
bool is_empty(queue *q);

#endif /* __DEQUEUE_H__ */