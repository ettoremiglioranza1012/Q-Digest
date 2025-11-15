#include "../include/qdigest.h"
#include "../include/dequeue.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef TESTALL
#include "../src/qdigest.c"
#endif

#ifdef TESTCORE
#include "../src/qdigest.c"
#endif

void insert_all_nodes(QDigest *dest, QDigestNode *src) {
    if (!src) return;
    if (src->count > 0) _insert_node(dest, src);
    insert_all_nodes(dest, src->left);
    insert_all_nodes(dest, src->right);
}

void print_sep(const char *msg) {
    printf("\n==== %s ====\n", msg);
}

void test_log_2_ceil() {
    print_sep("Testing log_2_ceil");
    assert(log2Ceil(0) == 0);
    assert(log2Ceil(1) == 0);
    assert(log2Ceil(2) == 1);
    assert(log2Ceil(3) == 2);
    assert(log2Ceil(4) == 2);
    assert(log2Ceil(5) == 3);
    printf("log_2_ceil tests passed\n");
}

/* Test node creation and deletion */
void test_node_create_delete() {
    print_sep("Testing create_node and delete_node");
    struct QDigestNode *n = qdigestnodeCreate(0, 10);
    assert(n != NULL);
    assert(n->count == 0);
    assert(n->left == NULL && n->right == NULL && n->parent == NULL);
    assert(n->lb == 0 && n->ub == 10);
    qdigestnodeRelease(n);
    printf("Node creation/deletion passed\n");
}

/* Test QDigest creation and deletion */
void test_qdigest_create_delete() {
    print_sep("Testing QDigest create/delete");
    QDigest *q = qdigestCreate(5, 10);
    assert(q != NULL);
    assert(q->root != NULL);
    assert(q->num_nodes == 1);
    qdigestRelease(q); // also frees root
    printf("QDigest create/delete passed\n");
}

/* Test basic insertion and percentile */
void test_insert_and_percentile() {
    print_sep("Testing insert and percentile");
    QDigest *q = qdigestCreate(5, 15);
    size_t S = 6;
    _insert(q, 5, 1, false, S);
    _insert(q, 7, 1, false, S);
    _insert(q, 3, 1, false, S);
    assert(q->N == 3);
    size_t p50 = percentile(q, 0.5);
    printf("Approx 50th percentile: %lu\n", p50);
    qdigestRelease(q);
}

/* Test insert_node and postorder traversal */
void test_insert_node() {
    print_sep("Testing insert_node and traversal");
    struct QDigest *q1 = qdigestCreate(5, 15);
    struct QDigest *q2 = qdigestCreate(5, 15);
    size_t S = 6;
    _insert(q2, 2, 1, false, S);
    _insert(q2, 4, 1, false, S);
    insert_all_nodes(q1, q2->root);
    assert(q1->N == 2);
    qdigestRelease(q1);
    qdigestRelease(q2);
}

/* Test expand_tree */
void test_expand_tree() {
    print_sep("Testing expand_tree");
    struct QDigest *q = qdigestCreate(5, 3);
    size_t S = 6;
    _insert(q, 1, 1, false, S);
    _insert(q, 3, 1, false, S);
    expandTree(q, 8, S); // expand upper bound
    _insert(q, 7, 1, false, S);
    qdigestRelease(q);
}

/* Test compress_if_needed */
void test_compress() {
    print_sep("Testing compress_if_needed");
    struct QDigest *q = qdigestCreate(1, 7); // small K to trigger compression easily
    size_t S = 6;
    for (size_t i = 0; i < 10; i++) _insert(q, i, 1, true, S);
    qdigestRelease(q);
}

/* Test merge */
void test_merge() {
    print_sep("Testing merge");
    size_t S = 6;
    struct QDigest *q1 = qdigestCreate(5, 7);
    struct QDigest *q2 = qdigestCreate(5, 7);
    _insert(q1, 1, 1, false, S);
    _insert(q1, 3, 1, false, S);
    _insert(q2, 2, 1, false, S);
    _insert(q2, 4, 1, false, S);
    merge(q1, q2, S);
    printf("After merge, q1 N = %lu\n", q1->N);
    qdigestRelease(q1);
    qdigestRelease(q2);
}

/* Test swap_q */
void test_swap_q() {
    print_sep("Testing swap_q");
    struct QDigest *q1 = qdigestCreate(5, 3);
    struct QDigest *q2 = qdigestCreate(10, 7);
    swap(q1, q2);
    assert(q1->k == 10 && q2->k == 5);
    assert(q1->root->ub == 7 && q2->root->ub == 3);
    qdigestRelease(q1);
    qdigestRelease(q2);
}

int main(void) {
    test_log_2_ceil();
    test_node_create_delete();
    test_qdigest_create_delete();
    test_insert_and_percentile();
    test_insert_node();
    test_expand_tree();
    test_compress();
    test_merge();
    test_swap_q();

    printf("\nAll tests completed successfully.\n");

    // Check for leaks automatically
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "leaks %d", getpid());
    system(cmd);
}