#ifndef __ARRAY_H__
#define __ARRAY_H__

#define ARRAY(type)                                                            \
  struct {                                                                     \
    unsigned size;                                                             \
    unsigned capacity;                                                         \
    unsigned elemsize;                                                         \
    type *data;                                                                \
  }

typedef ARRAY(void) _array_t;

void _array_maybe_resize(_array_t *array, unsigned new_size);

#define ARRAY_INIT(array)                                                      \
  do {                                                                         \
    (array)->size = 0;                                                         \
    (array)->capacity = 0;                                                     \
    (array)->elemsize = sizeof(*(array)->data);                                \
    (array)->data = NULL;                                                      \
  } while (0)

#define ARRAY_APPEND(array, elem)                                              \
  do {                                                                         \
    if ((array)->size == (array)->capacity)                                    \
      _array_maybe_resize((_array_t *)(array), (array)->size + 1);             \
    (array)->data[(array)->size++] = (elem);                                   \
  } while (0)

#define ARRAY_RESIZE(array, count)                                             \
  _array_maybe_resize((_array_t *)(array), (count));

#define ARRAY_FOREACH(var, array)                                              \
  for ((var) = &((array)->data[0]); (var) != &((array)->data[(array)->size]);  \
       (var) = (var) + 1)

#define ARRAY_FOREACH_REVERSE(var, array)                                      \
  for ((var) = &((array)->data[(array)->size - 1]);                            \
       (var) != &((array)->data[-1]); (var) = (var)-1)

#define ARRAY_AT(array, n)                                                     \
  ({                                                                           \
    typeof(n) _n = n;                                                          \
    if (((int)(_n) < 0) || ((int)(_n) >= (int)(array)->size))                  \
      abort();                                                                 \
    &((array)->data[_n]);                                                      \
  })

#endif /* !__ARRAY_H__ */
