#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cell.h"
#include "libtexproma.h"

cell_t *cell_int(int i) {
  cell_t *c = calloc(1, sizeof(cell_t));
  c->type = CT_INT;
  c->i = i;
  return c;
}

cell_t *cell_float(float f) {
  cell_t *c = calloc(1, sizeof(cell_t));
  c->type = CT_FLOAT;
  c->f = f;
  return c;
}

cell_t *cell_atom(const char *atom) {
  cell_t *c = calloc(1, sizeof(cell_t));
  c->type = CT_ATOM;
  c->atom = strdup(atom);
  return c;
}

cell_t *cell_string(const char *str) {
  cell_t *c = calloc(1, sizeof(cell_t));
  c->type = CT_STRING;
  c->str = strdup(str);
  return c;
}

cell_t *cell_mono() {
  cell_t *c = calloc(1, sizeof(cell_t));
  c->type = CT_MONO;
  c->mono = calloc(1, TP_WIDTH * TP_HEIGHT);
  return c;
}

cell_t *cell_color() {
  cell_t *c = calloc(1, sizeof(cell_t));
  c->type = CT_COLOR;
  c->color = calloc(1, TP_WIDTH * TP_HEIGHT * sizeof(color_t));
  return c;
}

cell_t *cell_dup(cell_t *c) {
  cell_t *nc = calloc(1, sizeof(cell_t));
  memcpy(nc, c, sizeof(cell_t));
  if (nc->type == CT_ATOM) {
    nc->atom = strdup(c->atom);
  } else if (nc->type == CT_STRING) {
    nc->atom = strdup(c->str);
  } else if (nc->type == CT_MONO) {
    nc->mono = malloc(TP_WIDTH * TP_HEIGHT);
    memcpy(nc->mono, c->mono, TP_WIDTH * TP_HEIGHT);
  } else if (nc->type == CT_COLOR) {
    nc->color = malloc(TP_WIDTH * TP_HEIGHT * sizeof(color_t));
    memcpy(nc->color, c->color, TP_WIDTH * TP_HEIGHT * sizeof(color_t));
  }

  return nc;
}

void cell_swap(cell_t *c1, cell_t *c2) {
  cell_t t;

  memcpy(&t, c1, sizeof(cell_t));
  memcpy(c1, c2, sizeof(cell_t));
  memcpy(c2, &t, sizeof(cell_t));
}

void cell_delete(cell_t *c) {
  if (c->type == CT_ATOM)
    free((void *)c->atom);
  else if (c->type == CT_STRING)
    free((void *)c->str);
  else if (c->type == CT_MONO)
    free(c->mono);
  else if (c->type == CT_COLOR)
    free(c->color);
  free(c);
}

char *stringify_cell(cell_t *c) {
  char *str;
  if (c == NULL)
    str = strdup("null");
  else if (c->type == CT_INT)
    asprintf(&str, "%d", c->i);
  else if (c->type == CT_FLOAT)
    asprintf(&str, "%f", c->f);
  else if (c->type == CT_ATOM)
    str = strdup(c->atom);
  else if (c->type == CT_STRING)
    asprintf(&str, "\"%s\"", c->str);
  else if (c->type == CT_MONO)
    asprintf(&str, "mono image at %p", c->mono);
  else if (c->type == CT_COLOR)
    asprintf(&str, "color image at %p", c->color);
  else
    abort();
  return str;
}

void print_cell(cell_t *c) {
  char *str = stringify_cell(c);
  printf("%s ", str);
  free(str);
}
