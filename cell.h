#ifndef __CELL_H__
#define __CELL_H__

#include "tailq.h"
#include "libtexproma.h"

typedef enum { 
  CT_ANY = '?',
  CT_INT = 'i',
  CT_FLOAT = 'f', 
  CT_ATOM = 'a',
  CT_MONO = 'm', 
  CT_COLOR = 'c'
} cell_type_t;

typedef struct cell {
  cell_type_t type;
  union {
    int i;
    float f;
    const char *atom;
    tpm_mono_buf mono;
    tpm_color_buf color;
  };
  TAILQ_ENTRY(cell) list;
} cell_t;

typedef TAILQ_HEAD(cell_list, cell) cell_list_t;

#define CELL_PREV(cell)                         \
  TAILQ_PREV(cell, cell_list, list)

#define CELL_NEXT(cell)                         \
  TAILQ_NEXT(cell, list)

#define CELL_REMOVE(stack, cell)                \
  TAILQ_REMOVE((stack), (cell), list);

cell_t *cell_int(int i);
cell_t *cell_float(float f);
cell_t *cell_atom(const char *atom);
cell_t *cell_mono();
cell_t *cell_color();

cell_t *cell_dup(cell_t *c);
void cell_delete(cell_t *c);

void print_cell(cell_t *cell);

#endif
