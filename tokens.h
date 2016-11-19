#ifndef __TOKENS_H__
#define __TOKENS_H__

#include <stdbool.h>

#include "array.h"

typedef ARRAY(char *) tokens_t;

bool tokens_feed(tokens_t *tokens, const char *program);
void tokens_trim(tokens_t *tokens, unsigned n);
void tokens_swap(tokens_t *tokens1, tokens_t *tokens2);

#endif
