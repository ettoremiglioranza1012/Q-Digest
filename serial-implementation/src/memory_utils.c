#include "../../include/memory_utils.h"
#include <stdlib.h>

void *xmalloc(size_t size) {
  void *ret = malloc(size);
  if (!ret) {
    fprintf(stderr, "OOM error while calling malloc\n");
    exit(EXIT_FAILURE);
  } else
    return ret;
}
