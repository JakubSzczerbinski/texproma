#ifndef __WORD_H__
#define __WORD_H__

#include <stdbool.h>

#include "cell.h"
#include "fn.h"

typedef enum { WT_VAR, WT_DEF, WT_BUILTIN, WT_CFUNC, WT_DRCT } word_type_t;

typedef struct word {
  word_type_t type;
  bool immediate;
  cell_t *value;
} word_t;

void word_print(const char *key, word_t *word);
void word_delete(word_t *word);

#endif
