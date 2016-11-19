#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tokens.h"

bool tokens_feed(tokens_t *tokens, const char *program) {
  const char *token = program;

  while (true) {
    /* move to the next token */
    token += strspn(token, " \t\n");

    /* find token length */
    unsigned toklen;

    if (*token == '"') {
      char *closing = strchr(token + 1, '"');

      if (closing == NULL) {
        fprintf(stderr, "missing closing quote character");
        tokens_trim(tokens, 0);
        return false;
      }

      toklen = closing + 1 - token;
    } else {
      toklen = strcspn(token, " \t\n");
    }

    if (toklen == 0)
      return true;

    ARRAY_APPEND(tokens, strndup(token, toklen));

    token += toklen;
  }
}

void tokens_trim(tokens_t *tokens, unsigned n) {
  if (n >= tokens->size)
    return;

  for (unsigned i = n; i < tokens->size; i++) {
    char **token_p = ARRAY_AT(tokens, i);
    free(*token_p);
    *token_p = NULL;
  }

  ARRAY_RESIZE(tokens, n);
}

void tokens_swap(tokens_t *t1, tokens_t *t2) {
  tokens_t t = *t1; *t1 = *t2; *t2 = t;
}
