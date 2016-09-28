#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cell.h"
#include "libtexproma.h"

#define offsetof(st, m) __builtin_offsetof(st, m)

#define CT_DEF(type, name, new, copy, stringify, delete)        \
  const cell_type_t type[1] = {{                                \
    (name), (new), (copy), (stringify), (delete) }}

static cell_t *new_cell(const cell_type_t *type) {
  cell_t *c = calloc(1, sizeof(cell_t));
  c->type = type;
  return c;
}

void cell_delete(cell_t *c) {
  if (c->type->delete)
    c->type->delete(c);
  free(c);
}

cell_t *cell_copy(cell_t *c) {
  cell_t *nc = calloc(1, sizeof(cell_t));
  memcpy(nc, c, sizeof(cell_t));
  if (c->type->copy)
    c->type->copy(c, nc);
  return nc;
}

void cell_swap(cell_t *c1, cell_t *c2) {
  cell_t t;

  memcpy(&t, c1, offsetof(cell_t, list));
  memcpy(c1, c2, offsetof(cell_t, list));
  memcpy(c2, &t, offsetof(cell_t, list));
}

char *cell_stringify(cell_t *c) {
  char str[80];
  if (c == NULL) {
    strncpy(str, "null", sizeof(str));
  } else {
    assert(c->type != NULL);
    assert(c->type->stringify != NULL);
    c->type->stringify(c, str, sizeof(str));
  }
  return strdup(str);
}

void cell_print(cell_t *c) {
  char *str = cell_stringify(c);
  printf("%s ", str);
  free(str);
}

/* List of cell functions */

unsigned clist_length(cell_list_t *clist) {
  cell_t *c;
  unsigned n = 0;
  TAILQ_FOREACH(c, clist, list) n++;
  return n;
}

void clist_copy(cell_list_t *ncl, cell_list_t *ocl) {
  cell_t *c;
  TAILQ_INIT(ncl);
  TAILQ_FOREACH(c, ocl, list)
    TAILQ_INSERT_TAIL(ncl, cell_copy(c), list);
}

void clist_reset(cell_list_t *clist) {
  cell_t *c, *next;
  TAILQ_FOREACH_SAFE(c, clist, list, next)
    cell_delete(c);
  TAILQ_INIT(clist);
}

/* Integer cell */

static cell_t *ct_int_new() {
  return new_cell(CT_INT);
}

static void ct_int_stringify(cell_t *c, char *str, unsigned len) {
  snprintf(str, len, "%d", c->i);
}

CT_DEF(CT_INT, "integer",
      ct_int_new,  NULL, ct_int_stringify, NULL);

/* Float cell */

static cell_t *ct_float_new() {
  return new_cell(CT_FLOAT);
}

static void ct_float_stringify(cell_t *c, char *str, unsigned len) {
  snprintf(str, len, "%f", c->f);
}

CT_DEF(CT_FLOAT, "float",
       ct_float_new, NULL, ct_float_stringify, NULL);

/* Atom cell */

static cell_t *ct_atom_new() {
  return new_cell(CT_ATOM);
}

static void ct_atom_copy(cell_t *oc, cell_t *nc) {
  nc->data = strdup(oc->data);
}

static void ct_atom_stringify(cell_t *c, char *str, unsigned len) {
  strlcpy(str, c->data, len);
}

static void ct_atom_delete(cell_t *c) {
  free(c->data);
}

CT_DEF(CT_ATOM, "atom",
       ct_atom_new, ct_atom_copy, ct_atom_stringify, ct_atom_delete);

/* String cell */

static cell_t *ct_string_new() {
  return new_cell(CT_STRING);
}

static void ct_string_stringify(cell_t *c, char *str, unsigned len) {
  snprintf(str, len, "\"%s\"", (char *)c->data);
}

CT_DEF(CT_STRING, "string",
       ct_string_new, ct_atom_copy, ct_string_stringify, ct_atom_delete);

/* List head cell */

static cell_t *ct_list_new() {
  cell_t *c = new_cell(CT_LIST);
  TAILQ_INIT(&c->head);
  return c;
}

static void ct_list_copy(cell_t *oc, cell_t *nc) {
  clist_copy(&nc->head, &oc->head);
}

static void ct_generic_stringify(cell_t *c, char *str, unsigned len) {
  snprintf(str, len, "%s at %p", c->type->name, c->data);
}

static void ct_list_delete(cell_t *c) {
  clist_reset(&c->head);
}

CT_DEF(CT_LIST, "list",
       ct_list_new, ct_list_copy, ct_generic_stringify, ct_list_delete);

/* Function cell */

static cell_t *ct_fn_new() {
  return new_cell(CT_FN);
}

static void ct_fn_copy(cell_t *oc, cell_t *nc) {
  unsigned size = sizeof(fn_t) + oc->fn->count * sizeof(fn_arg_t);
  nc->fn = malloc(size);
  memcpy(nc->data , oc->data, size);
}

static void ct_fn_delete(cell_t *c) {
  free(c->fn);
}

CT_DEF(CT_FN, "function",
       ct_fn_new, ct_fn_copy, ct_generic_stringify, ct_fn_delete);


/* Mono image cell */

static cell_t *ct_mono_new() {
  cell_t *c = new_cell(CT_MONO);
  c->data = calloc(1, TP_WIDTH * TP_HEIGHT);
  return c;
}

static void ct_mono_copy(cell_t *oc, cell_t *nc) {
  nc->data = malloc(TP_WIDTH * TP_HEIGHT);
  memcpy(nc->data , oc->data, TP_WIDTH * TP_HEIGHT);
}

CT_DEF(CT_MONO, "mono-buf",
       ct_mono_new, ct_mono_copy, ct_generic_stringify, ct_atom_delete);

/* Color image cell */

#define COLOR(c, i) (((uint8_t **)(c)->data)[i])

static cell_t *ct_color_new() {
  cell_t *c = new_cell(CT_COLOR);
  c->data = calloc(3, sizeof(tpm_mono_buf));
  for (int i = 0; i < 3; i++)
    COLOR(c, i) = calloc(1, TP_WIDTH * TP_HEIGHT);
  return c;
}

void ct_color_copy(cell_t *oc, cell_t *nc) {
  nc->data = calloc(3, sizeof(tpm_mono_buf));
  for (int i = 0; i < 3; i++) {
    COLOR(nc, i) = malloc(TP_WIDTH * TP_HEIGHT);
    memcpy(COLOR(nc, i), COLOR(oc, i), TP_WIDTH * TP_HEIGHT);
  }
}

void ct_color_delete(cell_t *c) {
  for (int i = 0; i < 3; i++)
    free(COLOR(c, i));
  free(c->data);
}

CT_DEF(CT_COLOR, "color-buf",
       ct_color_new, ct_color_copy, ct_generic_stringify, ct_color_delete);
