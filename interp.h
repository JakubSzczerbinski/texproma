#ifndef __INTERP_H__
#define __INTERP_H__

#include <stdbool.h>

#include "cell.h"
#include "dict.h"

#define STACK_TOP(stack)                                                \
  TAILQ_LAST((stack), cell_list)

#define STACK_POP(stack) ({                                             \
    cell_t *tmp = TAILQ_LAST((stack), cell_list);                       \
    TAILQ_REMOVE((stack), tmp, list);                                   \
    tmp;                                                                \
  })

#define STACK_PUSH(stack, cell) ({                                      \
    cell_t *tmp = (cell);                                               \
    TAILQ_INSERT_TAIL((stack), tmp, list);                              \
  })

unsigned stack_depth(cell_list_t *stack);
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
  tpmi_mode_t *mode;    /* current working mode */
  word_t *curr_word;    /* word being compiled now */
} tpmi_t;

typedef tpmi_status_t (*tpmi_fn_t)(tpmi_t *);

tpmi_t *tpmi_new();
void tpmi_delete(tpmi_t *interp);
tpmi_status_t tpmi_compile(tpmi_t *interp, const char *line);

#endif
