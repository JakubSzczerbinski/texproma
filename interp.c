#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <ffi.h>

#include "config.h"
#include "word.h"
#include "interp.h"
#include "ansi.h"

#include "libtexproma.h"

cell_t *stack_get_nth(cell_list_t *stack, unsigned n) {
  if (TAILQ_EMPTY(stack))
    return NULL;

  cell_t *c;
  TAILQ_FOREACH_REVERSE(c, stack, cell_list, list) {
    if (n == 0)
      return c;
    n--;
  }

  return NULL;
}

static tpmi_status_t eval_word(tpmi_t *interp, entry_t *entry);

static tpmi_status_t eval_cell(tpmi_t *interp, cell_t *c) {
  if (c->type == CT_ATOM) {
    entry_t *entry = dict_find(interp->words, c->atom);

    if (entry == NULL) {
      ERROR(interp, "unknown identifier: '%s'", c->atom);
      return TPMI_ERROR;
    }

    interp->last_word = entry->word;

    if (entry->word->type != WT_VAR)
      return eval_word(interp, entry);
  }

  stack_push(&interp->stack, cell_copy(c));
  return TPMI_OK;
}

typedef struct arg_info {
  bool do_color;
  cell_list_t args;
} arg_info_t;

static bool check_func_args(tpmi_t *interp, entry_t *entry, arg_info_t *ai) {
  fn_t *fn = entry->word->value->fn;
  unsigned args = fn_arg_count(fn, ARG_INPUT);

  TAILQ_INIT(&ai->args);
  ai->do_color = false;

  if (args == 0)
    return true;

  cell_t *arg = stack_get_nth(&interp->stack, args - 1);

  if (arg == NULL) {
    ERROR(interp, "'%s' expected %u args, but stack has %u elements",
          entry->key, args, clist_length(&interp->stack));
    return false;
  }

  /* move arguments from stack to arg_info */
  for (unsigned i = 0; i < args; i++) {
    cell_t *next = clist_next(arg);
    clist_remove(&interp->stack, arg);
    clist_append(&ai->args, arg);
    arg = next;
  }

  /* check arguments */
  arg = clist_first(&ai->args);

  for (unsigned i = 0, j = 0; i < fn->count; i++) {
    const fn_arg_t *fn_arg = &fn->args[i];

    if (fn_arg->flags & ARG_INPUT) {
      /* HACK: if color-buf was passed instead of mono-buf, check if argument
       *       is coercible, if so mark function to be executed for each
       *       buffer channel separately */
      if ((fn_arg->type == CT_MONO) && (fn_arg->flags & ARG_COERCIBLE) && 
          (arg->type == CT_COLOR)) {
        ai->do_color = true;
      } else {
        /* HACK: automatic coercion from mono-buf to color-buf */
        if ((fn_arg->type == CT_COLOR) && (arg->type == CT_MONO)) {
          cell_t *color = CT_COLOR->new();
          tpm_color(color->data, arg->data);
          cell_swap(color, arg);
          cell_delete(color);
        }

        if ((fn_arg->type != NULL) && (fn_arg->type != arg->type)) {
          ERROR(interp, "'%s' argument %u type mismatch - expected "
                BOLD "%s" RESET ", got " BOLD "%s" RESET,
                entry->key, j, fn_arg->type->name, arg->type->name);
          TAILQ_CONCAT(&interp->stack, &ai->args, list);
          return false;
        }
      }
      arg = clist_next(arg);
      j++;
    }
  }

  return true;
}

static void call_func(fn_t *fn, cell_list_t *args, cell_list_t *res)
{
  unsigned n = fn_arg_count(fn, ARG_INPUT|ARG_OUTPUT);

  /* construct a call */
  ffi_type *arg_ctype[n];
  void *arg_value[n];

  cell_t *arg = clist_first(args);

  for (unsigned i = 0; i < n; i++) {
    const fn_arg_t *fn_arg = &fn->args[i];

    if (fn_arg->flags & ARG_INPUT) {
      if (fn_arg->type == CT_INT) {
        arg_ctype[i] = &ffi_type_sint;
        arg_value[i] = &arg->i;
      } else if (fn_arg->type == CT_FLOAT) {
        arg_ctype[i] = &ffi_type_float;
        arg_value[i] = &arg->f;
      } else {
        arg_ctype[i] = &ffi_type_pointer;
        arg_value[i] = &arg->data;
      }

      cell_t *prev = arg;
      arg = clist_next(arg);

      /* Add placeholder for input-output arguments */
      if (fn_arg->flags & ARG_OUTPUT) {
        clist_remove(args, prev);
        clist_append(res, prev);
      }
    } else {
      /* pass output arguments, but firstly push them on top of stack */
      cell_t *c = fn_arg->type->new();

      arg_ctype[i] = &ffi_type_pointer;

      if (fn_arg->type == CT_INT)
        arg_value[i] = &c->i;
      else if (fn_arg->type == CT_FLOAT)
        arg_value[i] = &c->f;
      else if (fn_arg->type == CT_MONO)
        arg_value[i] = &c->data;
      else if (fn_arg->type == CT_COLOR)
        arg_value[i] = &c->data;
      else
        abort();

      clist_append(res, c);
    }
  }

  ffi_cif cif;
  ffi_arg result;

  assert(ffi_prep_cif(&cif, FFI_DEFAULT_ABI,
                      n, &ffi_type_void, arg_ctype) == FFI_OK);

  ffi_call(&cif, FFI_FN(fn->ptr), &result, arg_value);
}

static tpmi_status_t eval_word(tpmi_t *interp, entry_t *entry) {
  word_t *word = entry->word;

  if (word->type == WT_BUILTIN) {
    arg_info_t ai;
    if (!check_func_args(interp, entry, &ai))
      return TPMI_ERROR;
    TAILQ_CONCAT(&interp->stack, &ai.args, list);
    return ((tpmi_fn_t)word->value->fn->ptr)(interp);
  }

  if (word->type == WT_DEF) {
    cell_t *c;
    tpmi_status_t status = TPMI_OK;
    TAILQ_FOREACH(c, &word->value->head, list)
      if (!(status = eval_cell(interp, c)))
        break;
    return status;
  }
  
  if (word->type == WT_CFUNC) {
    fn_t *fn = word->value->fn;
    arg_info_t ai;

    if (!check_func_args(interp, entry, &ai))
      return TPMI_ERROR;

    if (ai.do_color) {
      cell_list_t args;
      cell_list_t res;

      TAILQ_INIT(&res);

      for (unsigned j = 0; j < 3; j++) {
        clist_copy(&args, &ai.args);

        cell_t *arg = clist_first(&args);

        for (unsigned i = 0; i < fn->count; i++) {
          const fn_arg_t *fn_arg = &fn->args[i];
          if (fn_arg->flags & ARG_INPUT) {
            if ((fn_arg->type == CT_MONO) && (fn_arg->flags & ARG_COERCIBLE) && 
                (arg->type == CT_COLOR)) {
              cell_t *mono = CT_MONO->new();
              tpm_extract(arg->data, mono->data, j);
              cell_swap(mono, arg);
              cell_delete(mono);
            }
            arg = clist_next(arg);
          }
        }
        call_func(fn, &args, &res);
        clist_reset(&args);
      }

      cell_t *color = CT_COLOR->new();
      cell_t *r = clist_first(&res);
      cell_t *g = clist_next(r);
      cell_t *b = clist_next(g);

      tpm_implode(color->data, r->data, g->data, b->data);
      stack_push(&interp->stack, color);

      clist_reset(&res);
    } else {
      call_func(fn, &ai.args, &interp->stack);
    }

    clist_reset(&ai.args);

    return TPMI_OK;
  }

  abort();
}

extern void tpmi_init(tpmi_t *interp);

tpmi_t *tpmi_new() {
  tpmi_t *interp = calloc(1, sizeof(tpmi_t));
  TAILQ_INIT(&interp->stack);
  ARRAY_INIT(&interp->tokens);
  interp->words = dict_new();

  tpmi_init(interp);

  return interp;
}

void tpmi_reset(tpmi_t *interp) {
  interp->mode = NULL;
  interp->curr_word = NULL;

  clist_reset(&interp->stack);
  dict_reset(interp->words);
  tpmi_init(interp);
}

void tpmi_delete(tpmi_t *interp) {
  clist_reset(&interp->stack);
  dict_delete(interp->words);
  free(interp);
}

static bool read_float(const char *str, float *f) {
  char *end;
  *f = strtof(str, &end);
  return *end == '\0';
}

static bool read_int(const char *str, int *i) {
  char *end;
  bool hex = (str[0] == '0' && tolower(str[1]) == 'x');
  *i = strtol(hex ? str + 2 : str, &end, hex ? 16 : 10);
  return *end == '\0';
}

static cell_t make_cell(const char *token) {
  int i; float f;
  int n = strlen(token);

  if (read_int(token, &i))
    return DEF_INT(i);
  if (read_float(token, &f))
    return DEF_FLOAT(f);
  if (token[0] == '"' && token[n - 1] == '"')
    return DEF_STRING(strndup(token + 1, n - 2));
  return DEF_ATOM(strdup(token));
}

static tpmi_status_t tpmi_compile_token(tpmi_t *interp, const char *token) {
  tpmi_status_t status = TPMI_OK;

  cell_t c = make_cell(token);

  interp->last_word = NULL;

  switch (*interp->mode) {
    case TPMI_EVAL: 
      /* evaluation mode */
      status = TPMI_NEED_MORE;

      if (strcmp(token, ":") == 0) {
        *interp->mode = TPMI_COMPILE;
        interp->curr_word = NULL;
      } else if (strcmp(token, "'") == 0)
        *interp->mode = TPMI_FUNCREF;
      else if (strcasecmp(token, "variable") == 0)
        *interp->mode = TPMI_DEFVAR;
      else if (strcasecmp(token, "immediate") == 0) {
        if (interp->curr_word != NULL)
          interp->curr_word->immediate = true;
        status = TPMI_OK;
      } else
        status = eval_cell(interp, &c);
      break;

    case TPMI_COMPILE:
      /* compilation mode */

      if (strcmp(token, ";") == 0) {
        *interp->mode = TPMI_EVAL;
        status = TPMI_OK;
      } else if (interp->curr_word == NULL) {
        if (c.type == CT_ATOM) {
          entry_t *entry = dict_add(interp->words, c.atom); 

          if (entry->word == NULL) {
            word_t *word = calloc(1, sizeof(word_t));
            word->type = WT_DEF;
            word->value = CT_LIST->new();
            word->immediate = false;
            interp->curr_word = entry->word = word;
            status = TPMI_NEED_MORE;
          } else {
            ERROR(interp, "word '%s' has been already defined", c.atom);
            status = TPMI_ERROR;
          }
        } else {
          ERROR(interp, "expected word name");
          status = TPMI_ERROR;
        }
      } else {
        if (c.type == CT_ATOM && 
            dict_add(interp->words, c.atom)->word->immediate) {
          status = eval_cell(interp, &c);
        } else {
          stack_push(&interp->curr_word->value->head, cell_copy(&c));
          status = TPMI_NEED_MORE;
        }
      }
      break;

    case TPMI_DEFVAR:
      *interp->mode = TPMI_EVAL;

      if (c.type == CT_ATOM) {
        dict_add(interp->words, c.atom)->word->type = WT_VAR;
        status = TPMI_OK;
      } else {
        ERROR(interp, "'variable' expects name");
        status = TPMI_ERROR;
      }
      break;

    case TPMI_FUNCREF:
      *interp->mode = TPMI_EVAL;
      status = TPMI_ERROR;

      if (c.type == CT_ATOM)
        if (dict_add(interp->words, c.atom)->word->type == WT_CFUNC) {
          stack_push(&interp->stack, cell_copy(&c));
          status = TPMI_OK;
        }

      if (status == TPMI_ERROR)
        ERROR(interp, "'tick' expects C function name");
      break;
  }

  if (interp->ready && status != TPMI_ERROR) {
    if (!(interp->last_word && interp->last_word->immediate))
      ARRAY_APPEND(&interp->tokens, strdup(token));
    if (interp->last_word && interp->last_word->type == WT_CFUNC)
      ARRAY_APPEND(&interp->tokens, NULL);
  }

  if ((c.type != NULL) && (c.type->delete != NULL))
    c.type->delete(&c);

  return status;
}

tpmi_status_t tpmi_compile(tpmi_t *interp, const char *prog) {
  tpmi_status_t status = TPMI_OK;
  unsigned n = 0;

  while (true) {
    /* skip spaces */
    prog += strspn(prog, " \t\n");

    /* find token */
    unsigned toklen;

    if (*prog == '"') {
      char *closing = strchr(prog + 1, '"');

      if (closing == NULL) {
        ERROR(interp, "missing closing quote character");
        status = TPMI_ERROR;
        goto error;
      }

      toklen = closing + 1 - prog;
    } else {
      toklen = strcspn(prog, " \t\n");
    }

    if (toklen == 0)
      break;

    char *token;
    
    token = strndup(prog, toklen);
    status = tpmi_compile_token(interp, token);
    free(token);

    prog += toklen;

error:

    if (status == TPMI_ERROR) {
      fprintf(stderr, RED "failure at token %u\n" RESET, n + 1);
      fprintf(stderr, RED "error: " RESET "%s\n", interp->errmsg);
      break;
    }

    if (status == TPMI_RESET) {
      tpmi_reset(interp);
    }
  }

  return status;
}
