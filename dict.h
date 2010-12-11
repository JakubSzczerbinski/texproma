#ifndef __DICT_H__
#define __DICT_H__

#include "cell.h"

typedef enum { WT_NULL, WT_DEF, WT_BUILTIN, WT_CFUNC } word_type_t;

typedef struct {
  const char *key;
  word_type_t type;
  union {
    cell_list_t def;
    struct {
      void (*fn)();
      size_t args;
    } builtin;
    struct {
      void (*fn)();
      const char *sig;
    } cfunc;
  };
} word_t;

typedef struct dict dict_t;

dict_t *dict_new();
void dict_delete(dict_t *);
word_t *dict_find(dict_t *, const char *);
word_t *dict_add(dict_t *, const char *);

void print_dict(dict_t *);

#endif /* !__DICT_H__ */
