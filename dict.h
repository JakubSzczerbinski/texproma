#ifndef __DICT_H__
#define __DICT_H__

#include <stdbool.h>

#include "cell.h"
#include "fn.h"

typedef enum { WT_NULL, WT_VAR, WT_DEF, WT_BUILTIN, WT_CFUNC } word_type_t;

typedef struct {
  const char *key;
  word_type_t type;
  bool immediate;
  union {
    cell_t *var;
    cell_list_t def;
    fn_t *func;
  };
} word_t;

typedef struct dict dict_t;

dict_t *dict_new();
void dict_delete(dict_t *);
word_t *dict_find(dict_t *, const char *);
word_t *dict_add(dict_t *, const char *);
bool dict_match(dict_t *, word_t **, const char *);

void print_dict(dict_t *);

#endif /* !__DICT_H__ */
