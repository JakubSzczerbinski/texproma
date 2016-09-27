
#ifndef __CELL_H__
#define __CELL_H__

#include "tailq.h"
#include "fn.h"

typedef struct cell cell_t;

typedef void (*cell_copy_fn)(cell_t *from, cell_t *to);
typedef void (*cell_stringify_fn)(cell_t *c, char *buf, unsigned len);
typedef void (*cell_delete_fn)(cell_t *c);

typedef struct cell_type {
  const char *name;
  cell_copy_fn copy;
  cell_stringify_fn stringify;
  cell_delete_fn delete;
} cell_type_t;

extern const cell_type_t CT_INT[1];
extern const cell_type_t CT_FLOAT[1];
extern const cell_type_t CT_ATOM[1];
extern const cell_type_t CT_STRING[1];
extern const cell_type_t CT_LIST[1];
extern const cell_type_t CT_MONO[1];
extern const cell_type_t CT_COLOR[1];

typedef TAILQ_HEAD(cell_list, cell) cell_list_t;

struct cell {
  const cell_type_t *type;
  union {
    int i;
    float f;
    char *atom;
    void *ptr;
    cell_list_t head;
  };
  TAILQ_ENTRY(cell) list;
};

#define CELL_PREV(cell)                         \
  TAILQ_PREV(cell, cell_list, list)

#define CELL_NEXT(cell)                         \
  TAILQ_NEXT(cell, list)

#define CELL_REMOVE(stack, cell)                \
  TAILQ_REMOVE((stack), (cell), list);

cell_t *cell_copy(cell_t *c);
void cell_swap(cell_t *c1, cell_t *c2);
void cell_delete(cell_t *c);
char *cell_stringify(cell_t *c);
void cell_print(cell_t *c);

cell_t *cell_int(int i);
cell_t *cell_float(float f);
cell_t *cell_atom(const char *atom);
cell_t *cell_string(const char *str);
cell_t *cell_list();
cell_t *cell_mono();
cell_t *cell_color();

unsigned clist_length(cell_list_t *clist);
void clist_reset(cell_list_t *clist);

#endif
