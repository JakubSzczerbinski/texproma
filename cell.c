#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cell.h"
#include "libtexproma.h"

#define CT_DEF(type, name, copy, stringify, delete)                     \
  const cell_type_t type[1] = {{(name), (copy), (stringify), (delete)}}

cell_t *new_cell(const cell_type_t *type, void *value) {
  cell_t *c = calloc(1, sizeof(cell_t));
  c->type = type;
  c->ptr = value;
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

  memcpy(&t, c1, sizeof(cell_t));
  memcpy(c1, c2, sizeof(cell_t));
  memcpy(c2, &t, sizeof(cell_t));
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

/* Integer cell */

cell_t *cell_int(int i) {
  cell_t *c = calloc(1, sizeof(cell_t));
  c->type = CT_INT;
  c->i = i;
  return c;
}

static void ct_int_stringify(cell_t *c, char *str, unsigned len) {
  snprintf(str, len, "%d", c->i);
}

CT_DEF(CT_INT, "integer",
       NULL, ct_int_stringify, NULL);

/* Float cell */

cell_t *cell_float(float f) {
  cell_t *c = calloc(1, sizeof(cell_t));
  c->type = CT_FLOAT;
  c->f = f;
  return c;
}

static void ct_float_stringify(cell_t *c, char *str, unsigned len) {
  snprintf(str, len, "%f", c->f);
}

CT_DEF(CT_FLOAT, "float",
       NULL, ct_float_stringify, NULL);

/* Atom cell */

cell_t *cell_atom(const char *atom) {
  return new_cell(CT_ATOM, strdup(atom));
}

static void ct_atom_copy(cell_t *oc, cell_t *nc) {
  nc->ptr = strdup(oc->ptr);
}

static void ct_atom_stringify(cell_t *c, char *str, unsigned len) {
  strlcpy(str, c->ptr, len);
}

static void ct_atom_delete(cell_t *c) {
  free(c->ptr);
}

CT_DEF(CT_ATOM, "atom",
       ct_atom_copy, ct_atom_stringify, ct_atom_delete);

/* String cell */

cell_t *cell_string(const char *str) {
  return new_cell(CT_STRING, strdup(str));
}

static void ct_string_stringify(cell_t *c, char *str, unsigned len) {
  snprintf(str, len, "\"%s\"", (char *)c->ptr);
}

CT_DEF(CT_STRING, "string",
       ct_atom_copy, ct_string_stringify, ct_atom_delete);

/* Mono image cell */

cell_t *cell_mono() {
  return new_cell(CT_MONO, calloc(1, TP_WIDTH * TP_HEIGHT));
}

static void ct_mono_copy(cell_t *oc, cell_t *nc) {
  nc->ptr = malloc(TP_WIDTH * TP_HEIGHT);
  memcpy(nc->ptr , oc->ptr, TP_WIDTH * TP_HEIGHT);
}

static void ct_mono_stringify(cell_t *c, char *str, unsigned len) {
  snprintf(str, len, "%s at %p", c->type->name, c->ptr);
}

CT_DEF(CT_MONO, "mono-buf",
       ct_mono_copy, ct_mono_stringify, ct_atom_delete);

/* Color image cell */

#define COLOR(c, i) (((uint8_t **)(c)->ptr)[i])

cell_t *cell_color() {
  cell_t *c = new_cell(CT_COLOR, calloc(3, sizeof(tpm_mono_buf)));
  for (int i = 0; i < 3; i++)
    COLOR(c, i) = calloc(1, TP_WIDTH * TP_HEIGHT);
  return c;
}

void ct_color_copy(cell_t *oc, cell_t *nc) {
  nc->ptr = calloc(3, sizeof(tpm_mono_buf));
  for (int i = 0; i < 3; i++) {
    COLOR(nc, i) = malloc(TP_WIDTH * TP_HEIGHT);
    memcpy(COLOR(nc, i), COLOR(oc, i), TP_WIDTH * TP_HEIGHT);
  }
}

void ct_color_delete(cell_t *c) {
  for (int i = 0; i < 3; i++)
    free(COLOR(c, i));
}

CT_DEF(CT_COLOR, "color-buf",
       ct_color_copy, ct_mono_stringify, ct_color_delete);
