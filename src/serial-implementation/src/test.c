#include "../include/dynamic_array.h"
#include "../include/memory_utils.h"
#include "../include/qcore.h"
#include <stddef.h>
#include <string.h>

/* This is a helper function that is used by the qsort function */
int comp(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

/* This is a function that shuffles a given array */
void shuffle(DAItem *array, size_t n) {
  if (n > 1) {
    size_t i;
    for (i = 0; i < n - 1; i++) {
      size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}

/* This function prints the real vs computed percentiles according to QDigest
 * Args:
 *  double p: the percentile to compute (a value between 0 and 1, inclusive)
 *  const Array *a: a pointer to a variable of type array containing the data
 *  struct QDigest *q: a pointer to the QDigest structure
 * Returns:
 *  Nothing */
void compare_percentiles(double p, const Array *a, struct QDigest *q) {
  int ip = (p * 1000 + 9) / 10;
  printf("%dth percentile: %lu v/s %lu\n", ip, a->data[(int)(p * a->size - 1)],
         percentile(q, p));
}

/* This function tests a QDigest against a Poisson distribution */
void test_poisson_distribution(int n, int k, int seed) {
  printf("<< test_poisson_distribution >>\n");
  int number = 1;
  int repeat = 1;
  bool flipped = false;
  Array *a = xmalloc(sizeof(Array)); // allocate memory for Array
  init_array(a, 128); // initialize the Array with an initial capacity
  for (; get_length(a) != n;
       ++number) { // start loop to add numbers to dynamic array
    for (int i = 0; i < repeat && get_length(a) != n; ++i) {
      push_back(a, number); // add number to dynamic array
    }
    if (get_length(a) <= n / 2) {
      if (!flipped)
        repeat += 3;
    } else {
      if (!flipped)
        repeat += 3;
      flipped = true;
      repeat -= 3;
    }
    if (repeat < 1)
      repeat = 2;
  }

  Array *b = xmalloc(sizeof(Array)); // allocate memory for array b
  init_array(b, a->capacity); // initialize array of b making sure it retains
                              // the same capacity of a
  memcpy(b->data, a->data, sizeof(DAItem) * a->size); // copy the data in a to b
  b->size = a->size;                                  // copy size as well

  // missing sort and random shuffle
  qsort(a->data, a->size, sizeof(a->data[0]), comp);
  shuffle(b->data, b->size);

  // use the default parameter of for the upper bound 1 declared in the c++
  // constructor
  // printf("DEBUG: Before creating tree\n");
  struct QDigest *q = create_tmp_q(
      k, 1); // create a QDigest structure using the shorter constructor
  // printf("DEBUG: After creating tree\n");
  for (size_t i = 0; i < get_length(b); ++i) {
    insert(q, b->data[i], 1, true); // insert data inside the QDigest
  }

  /* Compare percentiles between true values and using the QDigest */
  compare_percentiles(0.01, a, q);
  compare_percentiles(0.02, a, q);
  compare_percentiles(0.03, a, q);
  for (double d = 0.05; d < 1.0; d += 0.05) {
    compare_percentiles(d, a, q);
  }
}

#ifdef TESTALL
int main(void) {
  // higher values of K -> higher accuracy, lesser compression
  const int K = 5;
  int seed = 377;
  int N = 1999911; // # of numbers generated

  test_poisson_distribution(N, K, seed);
  return 0;
}
#endif
