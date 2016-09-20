#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <avcall.h>
#include <SDL.h>

#include "interp.h"
#include "ansi.h"

#define ERROR(interp, format, ...)                                      \
  snprintf((interp)->errmsg, ERRMSG_LENGTH, format, ##__VA_ARGS__)

#define STACK_TOP(stack)                                                \
  TAILQ_LAST((stack), cell_list)

#define STACK_PREV(cell)                                                \
  TAILQ_PREV(cell, cell_list, list)

#define STACK_NEXT(cell)                                                \
  TAILQ_NEXT(cell, list)

#define STACK_REMOVE(stack, cell)                                       \
  TAILQ_REMOVE((stack), (cell), list);

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

static tpmi_status_t do_display(tpmi_t *interp) {
  cell_t *c = STACK_TOP(&interp->stack);

  if (c->type != CT_MONO && c->type != CT_COLOR) {
    ERROR(interp, "not at image at the top of stack");
    return TPMI_ERROR;
  }

  SDL_Window *window = SDL_CreateWindow("TexProMa: top of stack display",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        TP_WIDTH, TP_HEIGHT, 0);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                              SDL_RENDERER_ACCELERATED);
  SDL_Texture *texture = SDL_CreateTexture(renderer, 
                                           SDL_PIXELFORMAT_RGB24,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           TP_WIDTH, TP_HEIGHT);
  assert(texture != NULL);

  int pitch = TP_WIDTH * sizeof(color_t);

  if (c->type == CT_MONO) {
    color_t *pixels = malloc(pitch * TP_HEIGHT);
    uint8_t *mono = c->mono;
    for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
      uint8_t p = mono[i];
      pixels[i] = (color_t){.r = p, .g = p, .b = p};
    }
    assert(SDL_UpdateTexture(texture, NULL, pixels, pitch) == 0);
    free(pixels);
  } else {
    assert(SDL_UpdateTexture(texture, NULL, c->color, pitch) == 0);
  }
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

  SDL_Event event;
  while (SDL_WaitEvent(&event))
    if (event.type == SDL_QUIT || event.type == SDL_KEYUP)
      break;

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  return TPMI_OK;
}

static tpmi_status_t do_drop(tpmi_t *interp) {
  cell_delete(STACK_POP(&interp->stack));
  return TPMI_OK;
}

static tpmi_status_t do_dup(tpmi_t *interp) {
  STACK_PUSH(&interp->stack, cell_dup(STACK_TOP(&interp->stack)));
  return TPMI_OK;
}

static tpmi_status_t do_swap(tpmi_t *interp) {
  cell_t *a = STACK_POP(&interp->stack);
  cell_t *b = STACK_POP(&interp->stack);
  STACK_PUSH(&interp->stack, a);
  STACK_PUSH(&interp->stack, b);
  return TPMI_OK;
}

static tpmi_status_t do_roll(tpmi_t *interp) {
  cell_t *top = STACK_TOP(&interp->stack);

  if (top->type != CT_INT) {
    ERROR(interp, "'roll' requires integer at the top of stack");
    return TPMI_ERROR;
  }
  
  size_t n = top->i;
  cell_t *arg = STACK_PREV(top);

  while (arg && (n-- > 0))
    arg = STACK_PREV(arg);

  if (arg == NULL) {
    ERROR(interp,
          "'roll' expected the stack to have %d elements", top->i + 1);
    return TPMI_ERROR;
  }

  STACK_REMOVE(&interp->stack, top);
  STACK_REMOVE(&interp->stack, arg);
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
  cell_t *arg = STACK_PREV(top);

  while (arg && (n-- > 0))
    arg = STACK_PREV(arg);

  if (arg == NULL) {
    ERROR(interp,
          "'pick' expected the stack to have %d elements", top->i + 1);
    return TPMI_ERROR;
  }

  STACK_REMOVE(&interp->stack, top);
  STACK_PUSH(&interp->stack, cell_dup(arg));
  cell_delete(top);

  return TPMI_OK;
}

static tpmi_status_t do_over(tpmi_t *interp) {
  STACK_PUSH(&interp->stack, cell_dup(STACK_PREV(STACK_TOP(&interp->stack))));
  return TPMI_OK;
}

static tpmi_status_t do_depth(tpmi_t *interp) {
  STACK_PUSH(&interp->stack, cell_int(stack_depth(&interp->stack)));
  return TPMI_OK;
}

static tpmi_status_t do_print_stack(tpmi_t *interp) {
  int i = 0;
  cell_t *c;
  TAILQ_FOREACH(c, &interp->stack, list) {
    printf("<%d> ", i++);
    print_cell(c);
    putchar('\n');
  }
  return TPMI_OK;
}

static tpmi_status_t do_print_dict(tpmi_t *interp) {
  print_dict(interp->words);
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

    return eval_word(interp, word);
  }

  STACK_PUSH(&interp->stack, cell_dup(c));
  return TPMI_OK;
}

static tpmi_status_t eval_word(tpmi_t *interp, word_t *word) {
  size_t depth = stack_depth(&interp->stack);

  if (word->type == WT_BUILTIN) {
    if (depth >= word->builtin.args)
      return ((tpmi_fn_t)word->builtin.fn)(interp);
    ERROR(interp, "'%s' expected %zu args, but stack has %zu elements",
          word->key, word->builtin.args, depth);
  } else if (word->type == WT_DEF) {
    cell_t *c;
    tpmi_status_t status = TPMI_OK;
    TAILQ_FOREACH(c, &word->def, list)
      if (!(status = eval_cell(interp, c)))
        break;
    return status;
  } else if (word->type == WT_CFUNC) {
    const char *sig = word->cfunc.sig;
    size_t n = strlen(sig);
    size_t args = 0;

    for (int i = n - 1; i >= 0; i--)
      if (islower(sig[i]))
        args++;

    if (depth < args) {
      ERROR(interp, "'%s' expected %zu args, but stack has %zu elements",
            word->key, args, depth);
      return TPMI_ERROR;
    }
 
    /* check arguments */
    cell_t *arg = NULL;

    for (int i = n - 1; i >= 0; i--) {
      if (islower(sig[i])) {
        arg = (arg == NULL) ? STACK_TOP(&interp->stack) : STACK_PREV(arg);

        if (arg->type != sig[i]) {
          ERROR(interp, "'%s' argument %u type mismatch - expected %c, got %c",
                word->key, i, sig[i], arg->type);
          return TPMI_ERROR;
        }
      }
    }

    /* construct a call */
    av_alist alist;
    av_start_void (alist, word->cfunc.fn);

    cell_t *carg = arg;

    for (int i = 0; i < n; i++) {
      if (islower(sig[i])) {
        /* pass input arguments */
        if (carg->type == CT_INT)
          av_int(alist, carg->i);
        else if (carg->type == CT_FLOAT)
          av_float(alist, carg->f);
        else if (carg->type == CT_MONO)
          av_ptr(alist, tpm_mono_buf, carg->mono);
        else if (carg->type == CT_COLOR)
          av_ptr(alist, tpm_color_buf, carg->color);
        else
          abort();

        carg = STACK_NEXT(carg);
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
    for (size_t i = 0; i < args; i++) {
      cell_t *c = arg;
      arg = STACK_NEXT(arg);
      STACK_REMOVE(&interp->stack, c);
      cell_delete(c);
    }

    return TPMI_OK;
  }

  return TPMI_ERROR;
}

typedef struct {
  const char *id;
  tpmi_fn_t fn;
  size_t args;
} builtin_ctor_t;

static builtin_ctor_t builtins[] = {
  { "depth", &do_depth, 0 },
  { "drop", &do_drop, 1 },
  { "dup", &do_dup, 1 },
  { "swap", &do_swap, 2 },
  { "over", &do_over, 2 },
  { "roll", &do_roll, 1 },
  { "pick", &do_pick, 1 },
  { ".p", &do_print, 1 },
  { ".d", &do_display, 1 },
  { ".s", &do_print_stack, 0 },
  { ".w", &do_print_dict, 0 },
  { "mono", &do_mono, 0 },
  { "color", &do_color, 0 },
  { NULL }
};

typedef struct {
  const char *id;
  void *fn;
  const char *sig;
} cfunc_ctor_t;

void tpm_light(tpm_mono_buf dst, uint8_t type, float radius);
void tpm_perlin_plasma(tpm_mono_buf dst, uint8_t step, uint32_t seed);

static cfunc_ctor_t cfuncs[] = {
  { "explode", &tpm_explode, "MMMc" },
  { "implode", &tpm_implode, "Cmmm" },
  { "plasma", &tpm_plasma, "Miiff" },
  { "light", &tpm_light, "Mif" },
  { "perlin-plasma", &tpm_perlin_plasma, "Mii" },
  { "add", &tpm_add, "Mmm" },
  { "mul", &tpm_mul, "Mmm" },
  { "mix", &tpm_mix, "Mmmi" },
  { "max", &tpm_max, "Mmm" },
  { "shade", &tpm_shade, "Mmm" },
  { "mix-map", &tpm_mix_map, "Mmmm" },
  { "twist", &tpm_twist, "Mmf" },
  { "move", &tpm_move, "Mmii" },
  { "uvmap", &tpm_uvmap, "Mmmff" },
  { NULL }
};

static char *initprog[] = {
  ": . .p drop ;",
  ": tuck swap over ;",
  ": nip swap drop ;",
  NULL
};

tpmi_t *tpmi_new() {
  tpmi_t *interp = calloc(1, sizeof(tpmi_t));
  TAILQ_INIT(&interp->stack);
  interp->words = dict_new();

  /* Initialize builtin words */
  for (builtin_ctor_t *builtin = builtins; builtin->id; builtin++) {
    word_t *word = dict_add(interp->words, builtin->id);
    word->type = WT_BUILTIN;
    word->builtin.fn = (void *)builtin->fn;
    word->builtin.args = builtin->args;
  }

  /* Initialize C function calls */
  for (cfunc_ctor_t *cfunc = cfuncs; cfunc->id; cfunc++) {
    word_t *word = dict_add(interp->words, cfunc->id);
    word->type = WT_CFUNC;
    word->cfunc.fn = cfunc->fn;
    word->cfunc.sig = cfunc->sig;
  }

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
  *i = strtol(str, &end, 10);
  return *end == '\0';
}

tpmi_status_t tpmi_compile(tpmi_t *interp, const char *line) {
  const char *token;
  char *state, *orig_state;
  size_t c = 0;
  tpmi_status_t status = TPMI_OK;

  state = orig_state = strdup(line);

  while ((token = strsep(&state, " \n")) != NULL) {
    int i; float f;

    if (strlen(token) == 0)
      continue;

    if (strcmp(token, ":") == 0) {
      interp->compilation = true;
      status = TPMI_NEED_MORE;
      continue;
    }

    if (strcmp(token, ";") == 0) {
      interp->compilation = false;
      interp->curr_word = NULL;
      status = TPMI_OK;
      continue;
    }

    if (interp->compilation) {
      /* compilation mode */
      if (read_int(token, &i)) {
        if (interp->curr_word) {
          STACK_PUSH(&interp->curr_word->def, cell_int(i));
          status = TPMI_NEED_MORE;
        } else {
          ERROR(interp, "expected word name, got integer %d", i);
          status = TPMI_ERROR;
        }
      } else if (read_float(token, &f)) {
        if (interp->curr_word) {
          STACK_PUSH(&interp->curr_word->def, cell_float(f));
          status = TPMI_NEED_MORE;
        } else {
          ERROR(interp, "expected word name, got float %f", f);
          status = TPMI_ERROR;
        }
      } else {
        if (interp->curr_word == NULL) {
          word_t *word = dict_add(interp->words, token); 

          if (word->type == WT_NULL) {
            word->type = WT_DEF;
            TAILQ_INIT(&word->def);
            interp->curr_word = word;
            status = TPMI_NEED_MORE;
          } else {
            ERROR(interp, "word '%s' has been already defined", token);
            status = TPMI_ERROR;
          }
        } else {
          STACK_PUSH(&interp->curr_word->def, cell_atom(token));
          status = TPMI_NEED_MORE;
        }
      }
    } else {
      /* evaluation mode */
      if (read_int(token, &i)) {
        cell_t c_i = {CT_INT, {.i = i}};
        status = eval_cell(interp, &c_i);
      } else if (read_float(token, &f)) {
        cell_t c_f = {CT_FLOAT, {.f = f}};
        status = eval_cell(interp, &c_f);
      } else {
        cell_t c_id = {CT_ATOM, {.atom = token}};
        status = eval_cell(interp, &c_id);
      }
    }

    if (status == TPMI_ERROR) {
      fprintf(stderr, RED "failure at token %zu\n" RESET, c + 1);
      fprintf(stderr, RED "error: " RESET "%s\n", interp->errmsg);
      break;
    }
  }

  free(orig_state);

  return status;
}
