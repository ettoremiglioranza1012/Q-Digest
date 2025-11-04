#ifndef LINKED
#define LINKED

typedef int LLItem;

struct ListNode {
  LLItem val;            // The value to be stored inside the node
  struct ListNode *next; // The next node connected to this one
};

/* The idea of a linked list is to have nodes connected to the one adjacent to
 * them so that nodes can be visited by looking iteratively at the nodes*/
struct LinkedList {
  struct ListNode *head; // Keep a pointer to the head of the structure
  struct ListNode *tail; // Keep a pointer to the tail of the structure
  int len;               // Length of the list
};

/* The main operations to be performed on a linked list are as follows:
 * 1. add node to the list
 * 2. remove node from the list
 * 3. look for a node value
 *
 * Potential function prototypes for each of the above actions are listed
 * below*/

// is there a way i can avoid using a struct node * argument?
// the real issue is that i do not understand how connect a node that has not
// been created yet. Maybe I should just create the first node and make it point
// to NULL and see how it evolves from there.
struct LinkedList *create_linked_list(void);
void read_list(struct LinkedList *list);
struct ListNode *create_linked_node(LLItem value, struct ListNode *n);
void add_node(struct LinkedList *list,
              struct ListNode *n); // working with pointers to avoid copying the
                                   // whole structure
void remove_node(struct LinkedList *list);
LLItem find_value(struct LinkedList *list);

#endif
