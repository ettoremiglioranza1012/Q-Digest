/* This module implements a linked list and then declares functions to
 * use it mainly as a stack */
#ifndef LINKED
#define LINKED
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef size_t LLItem;

/* The building block of the linked list implementation.
 * It stores a value and a reference to the next node in
 * the sequence. */
struct ListNode {
  LLItem val;            // The value to be stored inside the node
  struct ListNode *next; // The next node connected to this one
};

/* The idea of a linked list is to have nodes connected to the one adjacent to
 * them so that nodes can be visited by looking iteratively at the nodes */
struct LinkedList {
  struct ListNode *head; // Keep a pointer to the head of the structure
  struct ListNode *tail; // Keep a pointer to the tail of the structure
  size_t len;            // Length of the list
};

/* The main operations to be performed on a linked list are as follows:
 * 1. add node to the list
 * 2. remove node from the list
 * 3. look for a node value
 *
 * Potential function prototypes for each of the above actions are listed
 * below */

struct ListNode *create_linked_node(LLItem value);
struct LinkedList *create_linked_list(void);
void LLpush(struct LinkedList *list, struct ListNode *n);
void read_list(struct LinkedList *list);
bool is_empty_list(struct LinkedList *list);
size_t get_length(struct LinkedList *list);
LLItem LLpop(struct LinkedList *list);

#endif
