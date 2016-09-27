#ifndef __WORD_H__
#define __WORD_H__

#include <stdbool.h>

#include "cell.h"
#include "fn.h"

typedef enum { WT_VAR, WT_DEF, WT_BUILTIN, WT_CFUNC } word_type_t;

typedef struct word {
  word_type_t type;
  bool immediate;
  union {
    cell_t *value;
    fn_t *func;
  };
} word_t;

void word_print(const char *key, word_t *word);
void word_delete(word_t *word);

#endif
