#ifndef __FN_H__
#define __FN_H__

#include "cell.h"

typedef enum { ARG_LAST = 0, ARG_INPUT = 1, ARG_OUTPUT = 2 } fn_arg_flags_t;

typedef struct {
  const cell_type_t *type; /* may be NULL to denote any type */
  fn_arg_flags_t flags;
} fn_arg_t;

typedef struct {
  void (*fn)();
  fn_arg_t args[0];
} fn_t;

typedef struct {
  const char *id;
  void *fn;
  const char *sig;
  bool immediate;
} fn_ctor_t;

fn_t *new_fn(fn_ctor_t *ctor);
void fn_sig_print(const fn_t *fn);
unsigned fn_arg_count(const fn_t *fn, fn_arg_flags_t flags);

#endif
