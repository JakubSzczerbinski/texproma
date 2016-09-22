#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <avcall.h>

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

static size_t stack_depth(cell_list_t *stack) {
  cell_t *c;
  size_t n = 0;
  TAILQ_FOREACH(c, stack, list) n++;
  return n;
}

static tpmi_status_t do_print(tpmi_t *interp) {
  cell_t *c = STACK_TOP(&interp->stack);
  print_cell(c);
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
  
  size_t n = top->i;
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
  
  size_t n = top->i;
  cell_t *arg = CELL_PREV(top);

  while (arg && (n-- > 0))
    arg = CELL_PREV(arg);

  if (arg == NULL) {
    ERROR(interp,
          "'pick' expected the stack to have %d elements", top->i + 1);
    return TPMI_ERROR;
  }

  CELL_REMOVE(&interp->stack, top);
  STACK_PUSH(&interp->stack, cell_dup(arg));
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
  size_t n = stack_depth(&interp->stack);
  cell_t *c;
  TAILQ_FOREACH(c, &interp->stack, list) {
    printf("[%zu] ", --n);
    print_cell(c);
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
  STACK_PUSH(&interp->stack, cell_dup(word->var));
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
    word_t *word = dict_find(interp->words, c->atom);

    if (word == NULL) {
      ERROR(interp, "unknown identifier: '%s'", c->atom);
      return TPMI_ERROR;
    }

    if (word->type != WT_VAR)
      return eval_word(interp, word);
  }

  STACK_PUSH(&interp->stack, cell_dup(c));
  return TPMI_OK;
}

typedef struct arg_info {
  size_t args;
  cell_t *first;
} arg_info_t;

static bool check_func_args(tpmi_t *interp, word_t *word, arg_info_t *ai) {
  const char *sig = word->func.sig;
  size_t depth = stack_depth(&interp->stack);
  size_t n = strlen(sig);
  size_t args = 0;

  for (int i = n - 1; i >= 0; i--)
    if (!isupper(sig[i]))
      args++;

  if (depth < args) {
    ERROR(interp, "'%s' expected %zu args, but stack has %zu elements",
          word->key, args, depth);
    return false;
  }

  /* check arguments */
  cell_t *arg = NULL;

  for (int i = n - 1; i >= 0; i--) {
    if (!isupper(sig[i])) {
      arg = (arg == NULL) ? STACK_TOP(&interp->stack) : CELL_PREV(arg);

      if (sig[i] == CT_ANY)
        continue;

      if (arg->type != sig[i]) {
        ERROR(interp, "'%s' argument %u type mismatch - expected %c, got %c",
              word->key, i, sig[i], arg->type);
        return false;
      }
    }
  }

  ai->first = arg;
  ai->args = args;

  return true;
}

static tpmi_status_t eval_word(tpmi_t *interp, word_t *word) {
  if (word->type == WT_BUILTIN) {
    arg_info_t ai;
    if (!check_func_args(interp, word, &ai))
      return TPMI_ERROR;
    return ((tpmi_fn_t)word->func.fn)(interp);
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

    const char *sig = word->func.sig;
    size_t n = strlen(sig);

    /* construct a call */
    av_alist alist;
    av_start_void (alist, word->func.fn);

    cell_t *arg = ai.first;

    for (int i = 0; i < n; i++) {
      if (!isupper(sig[i])) {
        /* pass input arguments */
        if (arg->type == CT_INT)
          av_int(alist, arg->i);
        else if (arg->type == CT_FLOAT)
          av_float(alist, arg->f);
        else if (arg->type == CT_MONO)
          av_ptr(alist, tpm_mono_buf, arg->mono);
        else if (arg->type == CT_COLOR)
          av_ptr(alist, tpm_color_buf, arg->color);
        else
          abort();

        arg = CELL_NEXT(arg);
      } else {
        /* pass output arguments, but firstly push them on top of stack */
        char type = tolower(sig[i]);
        cell_t *c;

        if (type == CT_INT) {
          c = cell_int(0);
          av_ptr(alist, int *, &c->i);
        } else if (type == CT_FLOAT) {
          c = cell_float(0.0);
          av_ptr(alist, float *, &c->f);
        } else if (type == CT_MONO) {
          c = cell_mono();
          av_ptr(alist, tpm_mono_buf, c->mono);
        } else if (type == CT_COLOR) {
          c = cell_color();
          av_ptr(alist, tpm_color_buf, c->color);
        } else
          abort();

        STACK_PUSH(&interp->stack, c);
      }
    }

    av_call(alist);

    /* remove input arguments from stack */
    arg = ai.first;

    for (size_t i = 0; i < ai.args; i++) {
      cell_t *c = arg;
      arg = CELL_NEXT(arg);
      CELL_REMOVE(&interp->stack, c);
      cell_delete(c);
    }

    return TPMI_OK;
  }

  abort();
}

typedef struct {
  const char *id;
  void *fn;
  const char *sig;
  bool immediate;
} func_ctor_t;

static func_ctor_t builtins[] = {
  { "depth", &do_depth, "" },
  { "drop", &do_drop, "?" },
  { "roll", &do_roll, "i" },
  { "pick", &do_pick, "i" },
  { "emit", &do_emit, "i" },
  { "!", &do_store, "?a" },
  { "@", &do_load, "a" },
  { ".p", &do_print, "?" },
  { ".s", &do_print_stack, "", true },
  { ".w", &do_print_dict, "", true },
  { "mono", &do_mono, "" },
  { "color", &do_color, "" },
  { NULL }
};

static func_ctor_t cfuncs[] = {
  { "explode", &tpm_explode, "MMMc" },
  { "implode", &tpm_implode, "Cmmm" },
  { "noise", &tpm_noise, "Mi" },
  { "plasma", &tpm_plasma, "Mii" },
  { "light", &tpm_light, "Mif" },
  { "perlin-plasma", &tpm_perlin_plasma, "Mii" },
  { "perlin-noise", &tpm_perlin_noise, "Mi" },
  { "add", &tpm_add, "Mmm" },
  { "mul", &tpm_mul, "Mmm" },
  { "mix", &tpm_mix, "Mmmi" },
  { "max", &tpm_max, "Mmm" },
  { "shade", &tpm_shade, "Mmm" },
  { "mix-map", &tpm_mix_map, "Mmmm" },
  { "twist", &tpm_twist, "Mmf" },
  { "move", &tpm_move, "Mmff" },
  { "uvmap", &tpm_uvmap, "Mmmmff" },
  { "invert", &tpm_invert, "Mm" },
  { "sine-color", &tpm_sine_color, "Mmi" },
  { "brightness", &tpm_brightness, "Mmf" },
  { "contrast", &tpm_contrast, "Mmf" },
  { "colorize", &tpm_colorize, "Cmii" },
  { "grayscale", &tpm_grayscale, "Mc" },
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
  for (func_ctor_t *builtin = builtins; builtin->id; builtin++) {
    word_t *word = dict_add(interp->words, builtin->id);
    word->type = WT_BUILTIN;
    word->func.fn = (void *)builtin->fn;
    word->func.sig = builtin->sig;
    word->immediate = builtin->immediate;
  }

  /* Initialize C function calls */
  for (func_ctor_t *cfunc = cfuncs; cfunc->id; cfunc++) {
    word_t *word = dict_add(interp->words, cfunc->id);
    word->type = WT_CFUNC;
    word->func.fn = cfunc->fn;
    word->func.sig = cfunc->sig;
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

  if (read_int(token, &i))
    return (cell_t){CT_INT, {.i = i}};
  else if (read_float(token, &f))
    return (cell_t){CT_FLOAT, {.f = f}};
  else
    return (cell_t){CT_ATOM, {.atom = token}};
}

tpmi_status_t tpmi_compile(tpmi_t *interp, const char *line) {
  const char *token;
  char *state, *orig_state;
  size_t n = 0;
  cell_t c;
  tpmi_status_t status = TPMI_OK;

  state = orig_state = strdup(line);

  while ((token = strsep(&state, " \n")) != NULL) {
    if (strlen(token) == 0)
      continue;

    c = make_cell(token);

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
          continue;
        }

        if (interp->curr_word == NULL) {
          if (c.type == CT_ATOM) {
            word_t *word = dict_add(interp->words, token); 

            if (word->type == WT_NULL) {
              word->type = WT_DEF;
              word->immediate = false;
              TAILQ_INIT(&word->def);
              interp->curr_word = word;
              status = TPMI_NEED_MORE;
            } else {
              ERROR(interp, "word '%s' has been already defined", token);
              status = TPMI_ERROR;
            }
          } else {
            ERROR(interp, "expected word name");
            status = TPMI_ERROR;
          }
        } else {
          if (c.type == CT_ATOM && dict_add(interp->words, token)->immediate) {
            status = eval_cell(interp, &c);
          } else {
            STACK_PUSH(&interp->curr_word->def, cell_dup(&c));
            status = TPMI_NEED_MORE;
          }
        }
        break;

      case TPMI_DEFVAR:
        *interp->mode = TPMI_EVAL;

        if (c.type == CT_ATOM) {
          dict_add(interp->words, token)->type = WT_VAR;
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
          if (dict_add(interp->words, token)->type == WT_CFUNC) {
            STACK_PUSH(&interp->stack, cell_dup(&c));
            status = TPMI_OK;
          }

        if (status == TPMI_ERROR)
          ERROR(interp, "'tick' expects C function name");
        break;
    }

    if (status == TPMI_ERROR) {
      fprintf(stderr, RED "failure at token %zu\n" RESET, n + 1);
      fprintf(stderr, RED "error: " RESET "%s\n", interp->errmsg);
      break;
    }
  }

  free(orig_state);

  return status;
}
