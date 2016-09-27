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
  c->color = calloc(3, sizeof(tpm_mono_buf));
  for (int i = 0; i < 3; i++)
    c->color[i] = calloc(1, TP_WIDTH * TP_HEIGHT);
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
    nc->color = calloc(3, sizeof(tpm_mono_buf));
    for (int i = 0; i < 3; i++) {
      nc->color[i] = malloc(TP_WIDTH * TP_HEIGHT);
      memcpy(nc->color[i], c->color[i], TP_WIDTH * TP_HEIGHT);
    }
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
  else if (c->type == CT_COLOR) {
    for (int i = 0; i < 3; i++)
      free(c->color[i]);
    free(c->color);
  }
  free(c);
}

char *stringify_cell(cell_t *c) {
  char str[80];
  if (c == NULL)
    strncpy(str, "null", sizeof(str));
  else if (c->type == CT_INT)
    snprintf(str, sizeof(str), "%d", c->i);
  else if (c->type == CT_FLOAT)
    snprintf(str, sizeof(str), "%f", c->f);
  else if (c->type == CT_ATOM)
    strncpy(str, c->atom, sizeof(str));
  else if (c->type == CT_STRING)
    snprintf(str, sizeof(str), "\"%s\"", c->str);
  else if (c->type == CT_MONO)
    snprintf(str, sizeof(str), "mono image at %p", c->mono);
  else if (c->type == CT_COLOR)
    snprintf(str, sizeof(str), "color image at %p", c->color);
  else
    abort();
  return strdup(str);
}

void print_cell(cell_t *c) {
  char *str = stringify_cell(c);
  printf("%s ", str);
  free(str);
}
