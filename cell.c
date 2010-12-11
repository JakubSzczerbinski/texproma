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
  } else if (nc->type == CT_MONO) {
    nc->mono = malloc(TP_WIDTH * TP_HEIGHT);
    memcpy(nc->mono, c->mono, TP_WIDTH * TP_HEIGHT);
  } else if (nc->type == CT_COLOR) {
    nc->color = malloc(TP_WIDTH * TP_HEIGHT * sizeof(color_t));
    memcpy(nc->color, c->color, TP_WIDTH * TP_HEIGHT * sizeof(color_t));
  }

  return nc;
}

void cell_delete(cell_t *c) {
  if (c->type == CT_ATOM)
    free((void *)c->atom);
  else if (c->type == CT_MONO)
    free(c->mono);
  else if (c->type == CT_COLOR)
    free(c->color);
  free(c);
}

void print_cell(cell_t *cell) {
  if (cell->type == CT_INT)
    printf("%d ", cell->i);
  else if (cell->type == CT_FLOAT)
    printf("%f ", cell->f);
  else if (cell->type == CT_ATOM)
    printf("%s ", cell->atom);
  else if (cell->type == CT_MONO)
    printf("<grayscale image at %p> ", cell->mono);
  else if (cell->type == CT_COLOR)
    printf("<color image at %p> ", cell->color);
  else
    printf("??? ");
}
