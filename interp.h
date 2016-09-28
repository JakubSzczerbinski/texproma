#ifndef __INTERP_H__
#define __INTERP_H__

#include <stdbool.h>

#include "cell.h"
#include "dict.h"

static inline cell_t *stack_top(cell_list_t *stack) {
  return TAILQ_LAST(stack, cell_list);
}

static inline cell_t *stack_pop(cell_list_t *stack) {
  cell_t *c = TAILQ_LAST(stack, cell_list);
  TAILQ_REMOVE(stack, c, list);
  return c;
}

static inline void stack_push(cell_list_t *stack, cell_t *c) {
  TAILQ_INSERT_TAIL(stack, c, list);
}

cell_t *stack_get_nth(cell_list_t *stack, unsigned n);

/* TEXture PROcessing MAchine interpeter */

typedef enum { TPMI_ERROR, TPMI_OK, TPMI_NEED_MORE } tpmi_status_t;
typedef enum { TPMI_EVAL, TPMI_COMPILE, TPMI_DEFVAR, TPMI_FUNCREF } tpmi_mode_t;

#define ERRMSG_LENGTH 128

#define ERROR(interp, format, ...)                                      \
  snprintf((interp)->errmsg, ERRMSG_LENGTH, format, ##__VA_ARGS__)

typedef struct tpmi {
  cell_list_t stack;    /* working stack */
  dict_t *words;        /* word dictionary */

  char errmsg[ERRMSG_LENGTH];
  bool reset;           /* if true then please reset interpreter */
  tpmi_mode_t *mode;    /* current working mode */
  word_t *curr_word;    /* word being compiled now */
} tpmi_t;

typedef tpmi_status_t (*tpmi_fn_t)(tpmi_t *);

tpmi_t *tpmi_new();
void tpmi_reset(tpmi_t *interp);
void tpmi_delete(tpmi_t *interp);
tpmi_status_t tpmi_compile(tpmi_t *interp, const char *line);

#endif
