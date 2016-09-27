#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <ffi.h>

#include "config.h"
#include "interp.h"
#include "ansi.h"

#define ERROR(interp, format, ...)                                      \
  snprintf((interp)->errmsg, ERRMSG_LENGTH, format, ##__VA_ARGS__)

#define STACK_TOP(stack)                                                \
  TAILQ_LAST((stack), cell_list)

#define STACK_POP(stack) ({                                             \
    cell_t *tmp = TAILQ_LAST((stack), cell_list);                       \
    TAILQ_REMOVE((stack), tmp, list);                                   \
    tmp;                                                                \
  })

#define STACK_PUSH(stack, cell) ({                                      \
    cell_t *tmp = (cell);                                               \
    TAILQ_INSERT_TAIL((stack), tmp, list);                              \
  })

static unsigned stack_depth(cell_list_t *stack) {
  cell_t *c;
  unsigned n = 0;
  TAILQ_FOREACH(c, stack, list) n++;
  return n;
}

static cell_t *stack_get_nth(cell_list_t *stack, unsigned n) {
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

static tpmi_status_t do_print(tpmi_t *interp) {
  cell_t *c = STACK_TOP(&interp->stack);
  cell_print(c);
  putchar('\n');
  return TPMI_OK;
}

static tpmi_status_t do_drop(tpmi_t *interp) {
  cell_delete(STACK_POP(&interp->stack));
  return TPMI_OK;
}

static tpmi_status_t do_roll(tpmi_t *interp) {
  cell_t *top = STACK_TOP(&interp->stack);

  if (top->type != CT_INT) {
    ERROR(interp, "'roll' requires integer at the top of stack");
    return TPMI_ERROR;
  }
  
  unsigned n = top->i;
  cell_t *arg = CELL_PREV(top);

  while (arg && (n-- > 0))
    arg = CELL_PREV(arg);

  if (arg == NULL) {
    ERROR(interp,
          "'roll' expected the stack to have %d elements", top->i + 1);
    return TPMI_ERROR;
  }

  CELL_REMOVE(&interp->stack, top);
  CELL_REMOVE(&interp->stack, arg);
  STACK_PUSH(&interp->stack, arg);
  cell_delete(top);

  return TPMI_OK;
}

static tpmi_status_t do_pick(tpmi_t *interp) {
  cell_t *top = STACK_TOP(&interp->stack);

  if (top->type != CT_INT) {
    ERROR(interp, "'pick' requires integer at the top of stack");
    return TPMI_ERROR;
  }
  
  unsigned n = top->i;
  cell_t *arg = CELL_PREV(top);

  while (arg && (n-- > 0))
    arg = CELL_PREV(arg);

  if (arg == NULL) {
    ERROR(interp,
          "'pick' expected the stack to have %d elements", top->i + 1);
    return TPMI_ERROR;
  }

  CELL_REMOVE(&interp->stack, top);
  STACK_PUSH(&interp->stack, cell_copy(arg));
  cell_delete(top);

  return TPMI_OK;
}

static tpmi_status_t do_depth(tpmi_t *interp) {
  STACK_PUSH(&interp->stack, cell_int(stack_depth(&interp->stack)));
  return TPMI_OK;
}

static tpmi_status_t do_emit(tpmi_t *interp) {
  cell_t *top = STACK_TOP(&interp->stack);

  if (top->type != CT_INT) {
    ERROR(interp, "'emit' requires integer at the top of stack");
    return TPMI_ERROR;
  }

  putchar(top->i);
  return do_drop(interp);
}

static tpmi_status_t do_print_stack(tpmi_t *interp) {
  unsigned n = stack_depth(&interp->stack);
  cell_t *c;
  TAILQ_FOREACH(c, &interp->stack, list) {
    printf("[%u] ", --n);
    cell_print(c);
    putchar('\n');
  }
  return TPMI_OK;
}

static tpmi_status_t do_print_dict(tpmi_t *interp) {
  print_dict(interp->words);
  return TPMI_OK;
}

static tpmi_status_t do_load(tpmi_t *interp) {
  cell_t *key = STACK_POP(&interp->stack);
  word_t *word = dict_find(interp->words, key->atom);
  if (word->var == NULL) {
    ERROR(interp, "variable '%s' has no value", key->atom);
    cell_delete(key);
    return TPMI_ERROR;
  }
  cell_delete(key);
  STACK_PUSH(&interp->stack, cell_copy(word->var));
  return TPMI_OK;
}

static tpmi_status_t do_store(tpmi_t *interp) {
  cell_t *key = STACK_POP(&interp->stack);
  cell_t *value = STACK_POP(&interp->stack);
  word_t *word = dict_find(interp->words, key->atom);
  cell_delete(key);
  if (word->var != NULL) {
    cell_swap(word->var, value);
    cell_delete(value);
  } else {
    word->var = value;
  }
  return TPMI_OK;
}

static tpmi_status_t do_mono(tpmi_t *interp) {
  STACK_PUSH(&interp->stack, cell_mono());
  return TPMI_OK;
}

static tpmi_status_t do_color(tpmi_t *interp) {
  STACK_PUSH(&interp->stack, cell_color());
  return TPMI_OK;
}

static tpmi_status_t eval_word(tpmi_t *interp, word_t *word);

static tpmi_status_t eval_cell(tpmi_t *interp, cell_t *c) {
  if (c->type == CT_ATOM) {
    word_t *word = dict_find(interp->words, (char *)c->ptr);

    if (word == NULL) {
      ERROR(interp, "unknown identifier: '%s'", c->atom);
      return TPMI_ERROR;
    }

    if (word->type != WT_VAR)
      return eval_word(interp, word);
  }

  STACK_PUSH(&interp->stack, cell_copy(c));
  return TPMI_OK;
}

typedef struct arg_info {
  unsigned args;
  cell_t *first;
} arg_info_t;

static bool check_func_args(tpmi_t *interp, word_t *word, arg_info_t *ai) {
  unsigned args = fn_arg_count(word->func, ARG_INPUT);

  ai->args = args;

  if (args == 0)
    return true;

  cell_t *stk_arg = stack_get_nth(&interp->stack, args - 1);

  if (stk_arg == NULL) {
    ERROR(interp, "'%s' expected %u args, but stack has %u elements",
          word->key, args, stack_depth(&interp->stack));
    return false;
  }

  ai->first = stk_arg;

  /* check arguments */
  unsigned i = 0;

  for (const fn_arg_t *arg = word->func->args; arg->flags; arg++) {
    if (arg->flags & ARG_INPUT) {
      if ((arg->type != NULL) && (arg->type != stk_arg->type)) {
        ERROR(interp, "'%s' argument %u type mismatch - expected "
              BOLD "%s" RESET ", got " BOLD "%s" RESET,
              word->key, i, arg->type->name, stk_arg->type->name);
        return false;
      }
      stk_arg = CELL_NEXT(stk_arg);
      i++;
    }
  }

  return true;
}

static tpmi_status_t eval_word(tpmi_t *interp, word_t *word) {
  if (word->type == WT_BUILTIN) {
    arg_info_t ai;
    if (!check_func_args(interp, word, &ai))
      return TPMI_ERROR;
    return ((tpmi_fn_t)word->func->fn)(interp);
  }

  if (word->type == WT_DEF) {
    cell_t *c;
    tpmi_status_t status = TPMI_OK;
    TAILQ_FOREACH(c, &word->def, list)
      if (!(status = eval_cell(interp, c)))
        break;
    return status;
  }
  
  if (word->type == WT_CFUNC) {
    arg_info_t ai;

    if (!check_func_args(interp, word, &ai))
      return TPMI_ERROR;

    unsigned n = fn_arg_count(word->func, ARG_INPUT|ARG_OUTPUT);

    /* construct a call */
    ffi_type *arg_ctype[n];
    void *arg_value[n];

    cell_t *arg = ai.first;

    for (int i = 0; i < n; i++) {
      const fn_arg_t *fn_arg = &word->func->args[i];

      if (fn_arg->flags & ARG_INPUT) {
        if (fn_arg->type == CT_INT) {
          arg_ctype[i] = &ffi_type_sint;
          arg_value[i] = &arg->i;
        } else if (fn_arg->type == CT_FLOAT) {
          arg_ctype[i] = &ffi_type_float;
          arg_value[i] = &arg->f;
        } else {
          arg_ctype[i] = &ffi_type_pointer;
          arg_value[i] = &arg->ptr;
        }

        arg = CELL_NEXT(arg);
      } else {
        /* pass output arguments, but firstly push them on top of stack */
        cell_t *c;

        arg_ctype[i] = &ffi_type_pointer;

        if (fn_arg->type == CT_INT) {
          c = cell_int(0);
          arg_value[i] = &c->i;
        } else if (fn_arg->type == CT_FLOAT) {
          c = cell_float(0.0);
          arg_value[i] = &c->f;
        } else if (fn_arg->type == CT_MONO) {
          c = cell_mono();
          arg_value[i] = &c->ptr;
        } else if (fn_arg->type == CT_COLOR) {
          c = cell_color();
          arg_value[i] = &c->ptr;
        } else {
          abort();
        }

        STACK_PUSH(&interp->stack, c);
      }
    }

    ffi_cif cif;
    ffi_arg result;

    assert(ffi_prep_cif(&cif, FFI_DEFAULT_ABI,
                        n, &ffi_type_void, arg_ctype) == FFI_OK);

    ffi_call(&cif, FFI_FN(word->func->fn), &result, arg_value);

    /* remove input arguments from stack */
    arg = ai.first;

    for (unsigned i = 0; i < ai.args; i++) {
      cell_t *c = arg;
      arg = CELL_NEXT(arg);
      CELL_REMOVE(&interp->stack, c);
      cell_delete(c);
    }

    return TPMI_OK;
  }

  abort();
}

static fn_ctor_t builtins[] = {
  { "depth", &do_depth, "" },
  { "drop", &do_drop, "?" },
  { "roll", &do_roll, "i" },
  { "pick", &do_pick, "i" },
  { "emit", &do_emit, "i" },
  { "!", &do_store, "?a" },
  { "@", &do_load, "a" },
  { ".p", &do_print, "?" },
  { ".s", &do_print_stack, "", true },
  { "?", &do_print_dict, "", true },
  { "mono", &do_mono, "" },
  { "color", &do_color, "" },
  { NULL }
};

static fn_ctor_t cfuncs[] = {
  { "explode", &tpm_explode, "MMMc" },
  { "implode", &tpm_implode, "Cmmm" },
  { "save-mono", &tpm_mono_buf_save, "ms" },
  { "save-color", &tpm_color_buf_save, "cs" },
  { "sine", &tpm_sine, "Mf" },
  { "noise", &tpm_noise, "Mii" },
  { "plasma", &tpm_plasma, "M" },
  { "light", &tpm_light, "Mif" },
  { "perlin-noise", &tpm_perlin_noise, "Mi" },
  { "add", &tpm_add, "Mmm" },
  { "mul", &tpm_mul, "Mmm" },
  { "mix", &tpm_mix, "Mmmi" },
  { "max", &tpm_max, "Mmm" },
  { "shade", &tpm_shade, "Mmm" },
  { "mix-map", &tpm_mix_map, "Mmmm" },
  { "repeat", &tpm_repeat, "Mmii" },
  { "flip", &tpm_flip, "Mm" },
  { "rotate", &tpm_rotate, "Mm" },
  { "twist", &tpm_twist, "Mmf" },
  { "move", &tpm_move, "Mmff" },
  { "distort", &tpm_distort, "Mmmmff" },
  { "invert", &tpm_invert, "Mm" },
  { "sine-color", &tpm_sine_color, "Mmi" },
  { "hsv-modify", &tpm_hsv_modify, "Ccff" },
  { "brightness", &tpm_brightness, "Mmf" },
  { "contrast", &tpm_contrast, "Mmf" },
  { "colorize", &tpm_colorize, "Cmii" },
  { "grayscale", &tpm_grayscale, "Mc" },
  { "blur-3x3", &tpm_blur_3x3, "Mm" },
  { "blur-5x5", &tpm_blur_5x5, "Mm" },
  { "gaussian-3x3", &tpm_gaussian_3x3, "Mm" },
  { "gaussian-5x5", &tpm_gaussian_5x5, "Mm" },
  { "sharpen", &tpm_sharpen, "Mm" },
  { "emboss", &tpm_emboss, "Mm" },
  { "edges", &tpm_edges, "Mm" },
  { "median-3x3", &tpm_median_3x3, "Mm" },
  { "median-5x5", &tpm_median_5x5, "Mm" },
  { NULL }
};

static char *initprog[] = {
  ": [ 0 state ! ; immediate",
  ": ] 1 state ! ;",
  ": _ .p drop ;",
  ": dup 0 pick ;",
  ": over 1 pick ;",
  ": swap 1 roll ;",
  ": rot 2 roll ;",
  ": tuck swap over ;",
  ": nip swap drop ;",
  NULL
};

tpmi_t *tpmi_new() {
  tpmi_t *interp = calloc(1, sizeof(tpmi_t));
  TAILQ_INIT(&interp->stack);
  interp->words = dict_new();

  /* Initialize builtin words */
  for (fn_ctor_t *builtin = builtins; builtin->id; builtin++) {
    word_t *word = dict_add(interp->words, builtin->id);
    word->type = WT_BUILTIN;
    word->func = new_fn(builtin);
    word->immediate = builtin->immediate;
  }

  /* Initialize C function calls */
  for (fn_ctor_t *cfunc = cfuncs; cfunc->id; cfunc++) {
    word_t *word = dict_add(interp->words, cfunc->id);
    word->type = WT_CFUNC;
    word->func = new_fn(cfunc);
    word->immediate = cfunc->immediate;
  }

  /* Add special variables */
  word_t *state = dict_add(interp->words, "state");
  state->type = WT_VAR;
  state->var = cell_int(0);
  interp->mode = (tpmi_mode_t *)&state->var->i;

  /* Compile initial program */
  for (char **line = initprog; *line; line++)
    tpmi_compile(interp, *line);

  return interp;
}

void tpmi_delete(tpmi_t *interp) {
  dict_delete(interp->words);
  free(interp);
}

static bool read_float(const char *str, unsigned span, float *f) {
  char *end;
  *f = strtof(str, &end);
  return end == str + span;
}

static bool read_int(const char *str, unsigned span, int *i) {
  char *end;
  bool hex = (str[0] == '0' && tolower(str[1]) == 'x');
  *i = strtol(hex ? str + 2 : str, &end, hex ? 16 : 10);
  return end == str + span;
}

static cell_t make_cell(const char *line, unsigned span) {
  int i; float f;

  if (read_int(line, span, &i))
    return (cell_t){CT_INT, {.i = i}};
  if (read_float(line, span, &f))
    return (cell_t){CT_FLOAT, {.f = f}};
  if (line[0] == '"' && line[span - 1] == '"')
    return (cell_t){CT_STRING, {.ptr = strndup(line + 1, span - 2)}};
  return (cell_t){CT_ATOM, {.ptr = strndup(line, span)}};
}

tpmi_status_t tpmi_compile(tpmi_t *interp, const char *line) {
  tpmi_status_t status = TPMI_OK;
  unsigned n = 0;

  while (true) {
    /* skip spaces */
    line += strspn(line, " \t\n");

    /* find token */
    unsigned len;

    if (*line == '"') {
      char *closing = strchr(line + 1, '"');

      if (closing == NULL) {
        ERROR(interp, "missing closing quote character");
        status = TPMI_ERROR;
        goto error;
      }

      len = closing + 1 - line;
    } else {
      len = strcspn(line, " \t\n");
    }

    if (len == 0)
      break;

    /* parse token */
    cell_t c = make_cell(line, len);

    switch (*interp->mode) {
      case TPMI_EVAL: 
        /* evaluation mode */
        status = TPMI_NEED_MORE;

        if (strncmp(line, ":", len) == 0) {
          *interp->mode = TPMI_COMPILE;
          interp->curr_word = NULL;
        } else if (strncmp(line, "'", len) == 0)
          *interp->mode = TPMI_FUNCREF;
        else if (strncasecmp(line, "variable", len) == 0)
          *interp->mode = TPMI_DEFVAR;
        else if (strncasecmp(line, "immediate", len) == 0) {
          if (interp->curr_word != NULL)
            interp->curr_word->immediate = true;
          status = TPMI_OK;
        } else
          status = eval_cell(interp, &c);
        break;

      case TPMI_COMPILE:
        /* compilation mode */

        if (strncmp(line, ";", len) == 0) {
          *interp->mode = TPMI_EVAL;
          status = TPMI_OK;
        } else if (interp->curr_word == NULL) {
          if (c.type == CT_ATOM) {
            word_t *word = dict_add(interp->words, c.atom); 

            if (word->type == WT_NULL) {
              word->type = WT_DEF;
              word->immediate = false;
              TAILQ_INIT(&word->def);
              interp->curr_word = word;
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
          if (c.type == CT_ATOM && dict_add(interp->words, c.atom)->immediate) {
            status = eval_cell(interp, &c);
          } else {
            STACK_PUSH(&interp->curr_word->def, cell_copy(&c));
            status = TPMI_NEED_MORE;
          }
        }
        break;

      case TPMI_DEFVAR:
        *interp->mode = TPMI_EVAL;

        if (c.type == CT_ATOM) {
          dict_add(interp->words, c.atom)->type = WT_VAR;
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
          if (dict_add(interp->words, c.atom)->type == WT_CFUNC) {
            STACK_PUSH(&interp->stack, cell_copy(&c));
            status = TPMI_OK;
          }

        if (status == TPMI_ERROR)
          ERROR(interp, "'tick' expects C function name");
        break;
    }

    line += len;

error:

    if (status == TPMI_ERROR) {
      fprintf(stderr, RED "failure at token %u\n" RESET, n + 1);
      fprintf(stderr, RED "error: " RESET "%s\n", interp->errmsg);
      break;
    }
  }

  return status;
}
