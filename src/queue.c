#include "../include/queue.h"
#include "../include/memory_utils.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef TEST
#include "../include/qcore.h"
#endif

struct queueNode *create_queue_node(Item val) {
  struct queueNode *res = (struct queueNode *)xmalloc(sizeof(struct queueNode));
  res->val = val;
  // res->prev = NULL;
  res->next = NULL;
  return res;
}

struct queue *create_queue(void) {
  struct queue *q = xmalloc(sizeof(struct queue));
  q->first = NULL;
  q->last = NULL;
  q->len = 0;

  return q;
}

bool is_empty(struct queue *q) { return (q->len == 0); }

void push(struct queue *q, struct queueNode *n) {
  // in case list is empty
  if (is_empty(q)) {
    q->first = q->last = n;
  } else {
    // get the current last node and assign its next pointer to the new element
    q->last->next = n;
    // now last points to the newest node
    q->last = n;
  }
  (q->len)++;
}

Item front(struct queue *q) { return q->first->val; }

Item pop(struct queue *q) {
  if (q->len == 0) {
    fprintf(stderr, "Nothing to pop. Aborting...\n");
    exit(EXIT_FAILURE);
  }

  struct queueNode *old_first = q->first;
  Item ret = old_first->val;

  // Move head pointer forward
  q->first = old_first->next;
  (q->len)--;

  // If queue becomes empty, update last too
  if (q->first == NULL)
    q->last = NULL;

  free(old_first);
  return ret;
}

void delete_queue(struct queue *q) {
  while (!is_empty(q)) {
    pop(q);
  }
  free(q);
}

#ifdef TEST
int main(void) {
  struct QDigestNode *qdn = xmalloc(sizeof(struct QDigestNode));
  struct QDigestNode *qdn1 = xmalloc(sizeof(struct QDigestNode));
  struct QDigestNode *qdn2 = xmalloc(sizeof(struct QDigestNode));
  qdn->upper_bound = 65;
  qdn1->upper_bound = 100;
  qdn2->upper_bound = 1003;
  struct queueNode *n = create_queue_node(qdn);
  struct queueNode *n1 = create_queue_node(qdn1);
  struct queueNode *n2 = create_queue_node(qdn2);
  struct queue *q = create_queue();

#if 0
  printf("is queue empty %d\n", is_empty(q));
  push(q, n);
  printf("is queue empty %d\n", is_empty(q));

  printf("The upper bound from the popped value is %lu\n",
         front(q)->upper_bound);
  printf("is queue empty %d\n", is_empty(q));
  printf("The upper bound from the popped value is %lu\n", pop(q)->upper_bound);
  printf("is queue empty %d\n", is_empty(q));
#endif
  push(q, n);
  push(q, n1);
  push(q, n2);

  while (q->len > 0) {
    printf("The popped value is: %lu\n", pop(q)->upper_bound);
  }
  return 0;
}
#endif
