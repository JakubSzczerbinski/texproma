
#ifndef __CELL_H__
#define __CELL_H__

#include "tailq.h"
#include "fn.h"

typedef struct cell cell_t;

typedef cell_t* (*cell_new_fn)();
typedef void (*cell_copy_fn)(cell_t *from, cell_t *to);
typedef void (*cell_stringify_fn)(cell_t *c, char *buf, unsigned len);
typedef void (*cell_delete_fn)(cell_t *c);

typedef struct cell_type {
  const char *name;
  cell_new_fn new;
  cell_copy_fn copy;
  cell_stringify_fn stringify;
  cell_delete_fn delete;
} cell_type_t;

extern const cell_type_t CT_INT[1];
extern const cell_type_t CT_FLOAT[1];
extern const cell_type_t CT_ATOM[1];
extern const cell_type_t CT_STRING[1];
extern const cell_type_t CT_LIST[1];
extern const cell_type_t CT_FN[1];

/* Not builtin cell types... but declared here. */
extern const cell_type_t CT_MONO[1];
extern const cell_type_t CT_COLOR[1];

typedef TAILQ_HEAD(cell_list, cell) cell_list_t;

struct cell {
  const cell_type_t *type;
  union {
    int i;
    float f;
    char *atom;
    void *data;
    fn_t *fn;
    cell_list_t head;
  };
  TAILQ_ENTRY(cell) list;
};

cell_t *cell_copy(cell_t *c);
void cell_swap(cell_t *c1, cell_t *c2);
void cell_delete(cell_t *c);
char *cell_stringify(cell_t *c);
void cell_print(cell_t *c);

static inline cell_t *clist_prev(cell_t *c) {
  return TAILQ_PREV(c, cell_list, list);
}

static inline cell_t *clist_next(cell_t *c) {
  return TAILQ_NEXT(c, list);
}

static inline cell_t *clist_first(cell_list_t *clist) {
  return TAILQ_FIRST(clist);
}

static inline void clist_append(cell_list_t *clist, cell_t *c) {
  TAILQ_INSERT_TAIL(clist, c, list);
}

static inline void clist_remove(cell_list_t *clist, cell_t *c) {
  TAILQ_REMOVE(clist, c, list);
}

void clist_copy(cell_list_t *ncl, cell_list_t *ocl);
unsigned clist_length(cell_list_t *clist);
void clist_reset(cell_list_t *clist);

#endif
