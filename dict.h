#ifndef __DICT_H__
#define __DICT_H__

#include <stdbool.h>

typedef struct word word_t;
typedef struct dict dict_t;

typedef struct {
  char *key;
  word_t *word;
} entry_t;

dict_t *dict_new();
void dict_reset(dict_t *);
void dict_delete(dict_t *);
entry_t *dict_find(dict_t *, const char *);
entry_t *dict_add(dict_t *, const char *);
bool dict_iter(dict_t *, entry_t **);

#endif /* !__DICT_H__ */
