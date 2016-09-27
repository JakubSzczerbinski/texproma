#include <stdlib.h>
#include <stdio.h>

#include "ansi.h"
#include "word.h"

void word_print(const char *key, word_t *word) {
  if (word->type == WT_VAR) {
    printf(MAGENTA "%s" RESET " = " BOLD, key);
    cell_print(word->value);
    printf(RESET);
  } else if (word->type == WT_DEF) {
    cell_t *c;
    printf(BOLD ": %s ", key);
    TAILQ_FOREACH(c, &word->value->head, list)
      cell_print(c);
    printf(";" RESET);
  } else if (word->type == WT_BUILTIN) {
    printf(BLUE "%s" RESET " : ", key); fn_sig_print(word->value->fn);
  } else if (word->type == WT_CFUNC) {
    printf(GREEN "%s" RESET " : ", key); fn_sig_print(word->value->fn);
  } else {
    abort();
  }

  printf(RED "%s" RESET "\n", word->immediate ? " immediate" : "");
}

void word_delete(word_t *word) {
  if (word->value)
    cell_delete(word->value);
  free(word);
}
