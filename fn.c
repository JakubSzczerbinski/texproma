#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "ansi.h"
#include "cell.h"

typedef struct {
  char t;
  const cell_type_t *ct;
} fn_sig_t;

static const fn_sig_t fn_sig_map[] = {
  {'i', CT_INT},
  {'f', CT_FLOAT},
  {'a', CT_ATOM},
  {'s', CT_STRING},
  {'m', CT_MONO},
  {'c', CT_COLOR},
  {'?', NULL},
  {0, NULL}
};

static unsigned sig_length(const char *sig) {
  unsigned n = 0;

  for (char c; (c = *sig); sig++)
    if (c != '@' && c != '<' && c != '>')
      n++;

  return n;
}

static void sig_decode(const char *sig, fn_arg_t *arg) {
  const fn_sig_t *fn_sig;

  for (char c; (c = *sig); sig++) {
    if (c == '@')
      arg->flags |= ARG_COERCIBLE;
    else if (c == '>')
      arg->flags |= ARG_OUTPUT;
    else if (c == '<')
      arg->flags |= ARG_INPUT;
    else {
      for (fn_sig = fn_sig_map; fn_sig->t; fn_sig++) {
        if (c == fn_sig->t) {
          arg->type = fn_sig->ct;
          break;
        }
      }

      assert(fn_sig->t != 0);

      if (!(arg->flags & ARG_OUTPUT))
        arg->flags |= ARG_INPUT;

      arg++;
    }
  }
}

fn_t *new_fn(fn_ctor_t *ctor) {
  unsigned n = sig_length(ctor->sig);
  fn_t *fn = calloc(1, sizeof(fn_t) + n * sizeof(fn_arg_t));

  fn->ptr = ctor->ptr;
  fn->count = n;

  sig_decode(ctor->sig, fn->args);

  return fn;
}

unsigned fn_arg_count(const fn_t *fn, fn_arg_flags_t flags) {
  unsigned count = 0;

  for (unsigned i = 0; i < fn->count; i++)
    if (fn->args[i].flags & flags)
      count++;

  return count;
}

static void print_args(const fn_t *fn, fn_arg_flags_t flags) {
  bool first = true;

  fputs(BOLD "(" RESET, stdout);
  for (unsigned i = 0; i < fn->count; i++) {
    const fn_arg_t *arg = &fn->args[i];
    if (arg->flags & flags) {
      if (first)
        first = false;
      else
        fputs(BOLD "," RESET " ", stdout);
      if (arg->flags & ARG_COERCIBLE)
        fputs(MAGENTA "@" RESET, stdout);
      fputs(arg->type ? arg->type->name : "?", stdout);
    }
  }
  fputs(BOLD ")" RESET, stdout);
}

void fn_sig_print(const fn_t *fn) {
  print_args(fn, ARG_INPUT);
  fputs(BOLD " ↦ " RESET, stdout);
  print_args(fn, ARG_OUTPUT);
}
