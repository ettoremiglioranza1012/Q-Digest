/** 
 *  @file This file contains the structs and function prototypes
 *  to implement a queue data structure.
 *
 * */
#ifndef QUEUE
#define QUEUE
#include "../include/memory_utils.h"
#include "../include/qcore.h"
#include <stdbool.h>

/** 
 *  @brief Declares Item to be an alias for a pointer to a 
 *  struct QDigestNode.
 *
 * */
typedef struct QDigestNode *Item;

/** 
 *  @brief The struct implementing a node in the queue.
 *
 * */
struct queueNode {
  Item val;                 /**< The value to be stored in the queue. */
  struct queueNode *next;   /**< A pointer to the next queueNode. */
};

/** 
 *  @brief The struct implementing a queue.
 *
 * */
struct queue {
  struct queueNode *first;  /**< The pointer to the first node in the queue. */
  struct queueNode *last;   /**< The pointer to the last node in the queue. */
  size_t len;               /**< A positive integer representing the length of the queue. */
};

/** 
 *  @brief This function allocates memory for a queueNode and initializes
 *  it to contain the specified value (`val`).
 *
 *  @param val A pointer to a QDigestNode to be added as a queueNode.
 *
 * */
struct queueNode *create_queue_node(Item val);

/**
 *  @brief Creates and initializes an empty queue.
 *
 *  This function allocates a new `struct queue` and initializes its fields
 *  so that it represents an empty FIFO queue. Both the `first` and `last`
 *  pointers are set to NULL, and the length is initialized to zero.
 *
 *  The queue is typically used to support breadth-first traversal of
 *  QDigest trees during operations such as merging.
 *
 *  @return A pointer to a newly allocated, empty queue. The caller is
 *          responsible for freeing the queue using `delete_queue()` when
 *          it is no longer needed.
 *
 *  @note Memory is allocated using `xmalloc()`, which is expected to
 *        abort on allocation failure.
 */
struct queue *create_queue(void);

/**
 *  @brief Enqueues a node at the end of a queue.
 *
 *  This function appends a `struct queueNode` to the tail of the given
 *  queue. If the queue is currently empty, the new node becomes both the
 *  head and tail. Otherwise, it is linked to the existing last node and
 *  becomes the new tail.
 *
 *  The node's `next` pointer is assumed to be properly initialized
 *  (typically NULL) by the caller or by the node creation function.
 *
 *  @param q A pointer to the queue into which the node will be inserted.
 *  @param n The node to enqueue. Must not be NULL.
 *
 *  @note This function increments the queue length counter.
 */
void push(struct queue *q, struct queueNode *n);

/**
 *  @brief Retrieves the value stored at the front of the queue.
 *
 *  This function returns the `val` field of the first node in the queue,
 *  without removing that node. It assumes that the queue is not empty;
 *  callers must ensure this (e.g., by checking `is_empty(q)` beforehand).
 *
 *  @param q A pointer to the queue whose front value is requested.
 *
 *  @return The value stored in the first node of the queue.
 *
 *  @warning Calling this function on an empty queue results in
 *  dereferencing a NULL pointer and therefore undefined behavior.
 */
Item front(struct queue *q);

/**
 *  @brief Removes and returns the value at the front of the queue.
 *
 *  This function removes the first node from the queue, returns its stored
 *  value, and frees the node. The queue’s length and first/last pointers
 *  are updated accordingly.  
 *
 *  If the queue is empty, the function prints an error message to stderr
 *  and aborts the program.
 *
 *  @param q A pointer to the queue from which the front element should be removed.
 *
 *  @return The value stored in the node that was removed from the front
 *  of the queue.
 *
 *  @warning Calling this function on an empty queue causes program termination.
 */
Item pop(struct queue *q);

/**
 *  @brief Checks whether a queue is empty.
 *
 *  This function returns true if the queue contains no elements
 *  (i.e., its length is zero), and false otherwise.
 *
 *  @param q A pointer to the queue to be checked.
 *
 *  @return `true` if the queue has no elements;  
 *          `false` if it contains one or more elements.
 */
bool is_empty(struct queue *q);

/**
 *  @brief Deletes a queue and frees all associated memory.
 *
 *  This function repeatedly removes and frees all elements
 *  from the queue using `pop()`, ensuring that no queue nodes
 *  remain allocated. After the queue is emptied, the queue
 *  structure itself is freed.
 *
 *  @param q A pointer to the queue to be deleted.  
 *           Must not be NULL.
 */
void delete_queue(struct queue *q);
#endif
