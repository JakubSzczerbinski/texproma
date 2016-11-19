#include <stdlib.h>
#include <string.h>

#include "array.h"

#define N 40

static unsigned fib[N] = {
  13,        21,        34,        55,         89,        144,       233,
  377,       610,       987,       1597,       2584,      4181,      6765,
  10946,     17711,     28657,     46368,      75025,     121393,    196418,
  317811,    514229,    832040,    1346269,    2178309,   3524578,   5702887,
  9227465,   14930352,  24157817,  39088169,   63245986,  102334155, 165580141,
  267914296, 433494437, 701408733, 1134903170, 1836311903};

static unsigned determine_capacity(unsigned size) {
  for (unsigned i = 0; i < N; i++) {
    if (size < fib[i])
      return fib[i];
  }
  abort();
}

void _array_maybe_resize(_array_t *array, unsigned new_size) {
  /* does user want to release memory occupied by the array? */
  if (new_size == 0) {
    free(array->data);

    array->size = 0;
    array->capacity = 0;
    array->data = NULL;
    return;
  }

  /* calculate new capacity and resize array if needed */
  unsigned new_capacity = determine_capacity(new_size);
  if (new_capacity != array->capacity) {
    array->data = realloc(array->data, new_capacity * array->elemsize);
    if (array->capacity < new_capacity) {
      memset(array->data + array->capacity * array->elemsize, 0,
             (new_capacity - array->capacity) * array->elemsize);
    }
    if (array->size > new_capacity)
      array->size = new_capacity;
    array->capacity = new_capacity;
  }
  if (new_size < array->size) {
    memset(array->data + new_size * array->elemsize, 0,
           (array->size - new_size) * array->elemsize);
  }
  array->size = new_size;
}
