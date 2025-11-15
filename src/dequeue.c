#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../include/dequeue.h"
#include "../include/memory_utils.h"

#ifdef TEST
#include "../include/qdigest.h"
#include "../src/qdigest.c"
#endif

queueNode *create_queue_node(Item item)
{
    queueNode *ret;
    ret = (queueNode*)xmalloc(sizeof(queueNode));
    ret->val = item;
    ret->prev = NULL;
    return ret;
}

queue *create_queue()
{
    queue *ret;
    ret = (queue*)xmalloc(sizeof(queue));
    ret->front = NULL;
    ret->back= NULL;
    ret->size = 0;
    return ret;
}

void release_queue_node(queueNode *n)
{
    if (!n) return;
    release_queue_node(n->prev);
    qdigestnodeRelease(n->val);
    free(n);
    return;
}

void release_queue(queue *q)
{
    if (q->size == 0) { free(q); return; } 
    release_queue_node(q->front);
    free(q);
    return;
}

Item front(queue *queue) { return queue->front->val; }

bool is_empty(queue *queue) { return (queue->size == 0); }

Item pop(queue *queue)
{
    if (queue->size == 0) { 
        fprintf(stderr, "Err: queue is empty.\n");
        exit(EXIT_FAILURE);
    } 
    
    queueNode *old_front = queue->front;
    Item ret = old_front->val;
    
    // Move head pointer forward
    queue->front = old_front->prev;
    --(queue->size);
    
    if (queue->front == NULL) queue->back = NULL;

    free(old_front);
    return ret;
}

void push(queue *queue, queueNode *queueNode)
{
    // In case list is empty
    if (is_empty(queue)) 
        queue->back = queue->front = queueNode;
    else {
        queue->back->prev = queueNode;
        queue->back = queueNode;
    }        
    ++(queue->size);
    return; 
}

#ifdef TESTQUEUE
int main(void) {
    QDigestNode *qdn = xmalloc(sizeof(QDigestNode));
    QDigestNode *qdn1 = xmalloc(sizeof(QDigestNode));
    QDigestNode *qdn2 = xmalloc(sizeof(QDigestNode));
    qdn->ub= 65;
    qdn1->ub = 100;
    qdn2->ub = 1003;
    queueNode *n = create_queue_node(qdn);
    queueNode *n1 = create_queue_node(qdn1);
    queueNode *n2 = create_queue_node(qdn2);
    queue *q = create_queue();
#if 1
    printf("is queue empty %d\n", is_empty(q));
    push(q, n);
    printf("is queue empty %d\n", is_empty(q));

    printf("FRONT: The upper bound from the front value of the queue is %zu\n",
        front(q)->ub);
    printf("is queue empty %d\n", is_empty(q));
    printf("POP: The upper bound from the popped value is %zu\n",
        pop(q)->ub);
    printf("is queue empty %d\n", is_empty(q));
#endif 
    push(q, n1);
    push(q, n2);

    while (q->size > 0)
        printf("The popped value's upper bound is: %zu\n",
             pop(q)->ub);
    return 0;
}
#endif /* TEST */