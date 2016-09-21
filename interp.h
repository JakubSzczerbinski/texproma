#ifndef __INTERP_H__
#define __INTERP_H__

#include <stdbool.h>

#include "cell.h"
#include "dict.h"

/* TEXture PROcessing MAchine interpeter */

typedef enum { TPMI_ERROR, TPMI_OK, TPMI_NEED_MORE } tpmi_status_t;
typedef enum { TPMI_EVAL, TPMI_COMPILE, TPMI_DEFVAR, TPMI_FUNCREF } tpmi_mode_t;

#define ERRMSG_LENGTH 128

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
