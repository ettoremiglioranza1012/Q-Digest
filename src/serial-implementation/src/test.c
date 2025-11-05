#include "../include/dynamic_array.h"
#include "../include/memory_utils.h"
#include "../include/qcore.h"
#include <stddef.h>
#include <string.h>

void compare_percentiles(double p, const Array *a, struct QDigest *q) {
  int ip = (p * 1000 + 9) / 10;
  printf("%dth percentile: %lu v/s %lu\n", ip, a->data[(int)(p * a->size - 1)],
         percentile(q, p));
}

void test_poisson_distribution(int n, int k, int seed) {
  printf("<< test_poisson_distribution >>\n");
  int number = 1;
  int repeat = 1;
  bool flipped = false;
  Array *a = xmalloc(sizeof(Array));
  init_array(a, 128);
  for (; get_length(a) != n; ++number) {
    for (int i = 0; i < repeat && get_length(a) != n; ++i) {
      push_back(a, number);
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

  Array *b = malloc(sizeof(Array));
  init_array(b, a->capacity);
  memcpy(b->data, a->data, sizeof(DAItem) * a->size);
  b->size = a->size;

  // missing sort and random shuffle here

  // use the default parameter of for the upper bound 1 declared in the c++
  // constructor
  struct QDigest *q = create_tmp_q(k, 1);
  for (size_t i = 0; i < get_length(b); ++i) {
    insert(q, b->data[i], 1, true);
  }
  compare_percentiles(0.01, a, q);
  compare_percentiles(0.02, a, q);
  compare_percentiles(0.03, a, q);
  for (double d = 0.05; d < 1.0; d += 0.05) {
    compare_percentiles(d, a, q);
  }
}

#ifdef TEST
int main(void) {
  const int K = 100;
  int seed = 377;
  int N = 65535;

  test_poisson_distribution(N, K, seed);
  return 0;
}
#endif
