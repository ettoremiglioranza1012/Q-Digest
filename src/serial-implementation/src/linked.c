#include "../include/linked.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* This function is to be used every time a new ListNode is to be created*/
struct ListNode *create_linked_node(LLItem value) {
  struct ListNode *s;
  s = malloc(sizeof(struct ListNode));
  if (s == NULL) {
    printf("Error in initialize_list, malloc did not allocate memory\n");
    exit(EXIT_FAILURE);
  }
  s->val = value;
  s->next = NULL;

  return s;
}

struct LinkedList *create_linked_list(void) {
  struct LinkedList *list;
  list = malloc(sizeof(struct LinkedList));
  if (list == NULL) {
    printf("Error in initialize_list, malloc did not allocate memory\n");
    exit(EXIT_FAILURE);
  }
  list->head = NULL;
  list->tail = NULL;
  list->len = 0;

  return list;
}

void LLpush(struct LinkedList *list, struct ListNode *n) {
  n->next = NULL; // always terminate the list

  if (list->len == 0) {
    list->head = n;
    list->tail = n;
  } else {
    list->tail->next = n; // link old tail to new node
    list->tail = n;
  }

  list->len++;
}

/* This function traverses the linked list and prints all values of the
 * ListNodes*/
void read_list(struct LinkedList *list) {
  struct ListNode *n = list->head;
  printf("Linked list values are reported below:\n");
  while (n != NULL) {
    printf(" %lu ->", n->val);
    n = n->next;
  }
  printf(" END\n");
}

bool is_empty_list(struct LinkedList *list) { return list->len == 0; }

size_t get_length(struct LinkedList *list) { return list->len; }

LLItem LLpop(struct LinkedList *list) {
  if (is_empty_list(list)) {
    fprintf(stderr, "Nothing to LLpop in linked list. Aborting...\n");
    exit(EXIT_FAILURE);
  }

  struct ListNode *prev = NULL;
  struct ListNode *curr = list->head;

  while (curr->next != NULL) {
    prev = curr;
    curr = curr->next;
  }

  LLItem res = curr->val;

  if (prev == NULL) {
    // only one element in list
    list->head = NULL;
    list->tail = NULL;
  } else {
    prev->next = NULL;
    list->tail = prev;
  }

  free(curr);
  list->len--;

  return res;
}

// #define TEST
#ifdef TEST
int main(void) {
  // initialize array
  int len = 10;
  struct LinkedList *l = create_linked_list();
  struct ListNode *n;
  for (int i = 0; i < len; i++) {
    n = create_linked_node(i);
    LLpush(l, n);
  }
  read_list(l);
  // LLpop 8 values
  LLpop(l);
  LLpop(l);
  LLpop(l);
  LLpop(l);
  LLpop(l);
  LLpop(l);
  LLpop(l);
  LLpop(l);

  read_list(l);
  return 0;
}
#endif
