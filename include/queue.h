#ifndef QUEUE
#define QUEUE
#include "../include/memory_utils.h"
#include "../include/qcore.h"
#include <stdbool.h>
// declare type Item to be a pointer to QDigestNode
typedef struct QDigestNode *Item;

/* This is the building block of the DS */
struct queueNode {
  Item val;
  // struct queueNode *prev;
  struct queueNode *next;
};

/* A queue should support the following operations:
 * 1. push values at the end of the list
 * 2. return value from the beginning without removing it
 * 3. pop values from the beginning (FIFO protocol)
 * 4. return whether it is empty
 * */
struct queue {
  struct queueNode *first;
  struct queueNode *last;
  size_t len;
};

/* allocate memory for a queueNode */
struct queueNode *create_queue_node(Item val);
/* allocate memory for a queue */
struct queue *create_queue(void);
/* push value at the end of the queue */
void push(struct queue *q, struct queueNode *n);
/* return value in front without popping it */
Item front(struct queue *q);
/* pop value and return it */
Item pop(struct queue *q);
/* Return whether it is empty */
bool is_empty(struct queue *q);

struct queue *create_queue(void);
struct queueNode *create_queue_node(Item val);
Item front(struct queue *q);
bool is_empty(struct queue *q);
Item pop(struct queue *q);
void push(struct queue *q, struct queueNode *n);
void delete_queue(struct queue *q);
#endif
