#ifndef __INTERP_H__
#define __INTERP_H__

#include "cell.h"
#include "dict.h"

/* TEXture PROcessing MAchine interpeter */

typedef enum { TPMI_ERROR, TPMI_OK, TPMI_NEED_MORE } tpmi_status_t;

#define ERRMSG_LENGTH 128

typedef struct tpmi {
  cell_list_t stack;    /* working stack */
  dict_t *words;        /* word dictionary */

  char errmsg[ERRMSG_LENGTH];
  bool compilation;     /* is compilation mode active ? */
  word_t *curr_word;    /* word being compiled now */
} tpmi_t;

typedef tpmi_status_t (*tpmi_fn_t)(tpmi_t *);

tpmi_t *tpmi_new();
void tpmi_delete(tpmi_t *interp);
tpmi_status_t tpmi_compile(tpmi_t *interp, const char *line);

#endif
