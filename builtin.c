#include <stdio.h>
#include <stdlib.h>

#include "libtexproma.h"
#include "word.h"
#include "interp.h"

static tpmi_status_t do_print(tpmi_t *interp) {
  cell_t *c = stack_top(&interp->stack);
  cell_print(c);
  putchar('\n');
  return TPMI_OK;
}

static tpmi_status_t do_drop(tpmi_t *interp) {
  cell_delete(stack_pop(&interp->stack));
  return TPMI_OK;
}

static tpmi_status_t do_roll(tpmi_t *interp) {
  cell_t *top = stack_top(&interp->stack);

  if (top->type != CT_INT) {
    ERROR(interp, "'roll' requires integer at the top of stack");
    return TPMI_ERROR;
  }
  
  unsigned n = top->i;
  cell_t *arg = clist_prev(top);

  while (arg && (n-- > 0))
    arg = clist_prev(arg);

  if (arg == NULL) {
    ERROR(interp,
          "'roll' expected the stack to have %d elements", top->i + 1);
    return TPMI_ERROR;
  }

  clist_remove(&interp->stack, top);
  clist_remove(&interp->stack, arg);
  stack_push(&interp->stack, arg);
  cell_delete(top);

  return TPMI_OK;
}

static tpmi_status_t do_pick(tpmi_t *interp) {
  cell_t *top = stack_top(&interp->stack);

  if (top->type != CT_INT) {
    ERROR(interp, "'pick' requires integer at the top of stack");
    return TPMI_ERROR;
  }
  
  unsigned n = top->i;
  cell_t *arg = clist_prev(top);

  while (arg && (n-- > 0))
    arg = clist_prev(arg);

  if (arg == NULL) {
    ERROR(interp,
          "'pick' expected the stack to have %d elements", top->i + 1);
    return TPMI_ERROR;
  }

  clist_remove(&interp->stack, top);
  stack_push(&interp->stack, cell_copy(arg));
  cell_delete(top);

  return TPMI_OK;
}

static tpmi_status_t do_depth(tpmi_t *interp) {
  cell_t *c = CT_INT->new();
  c->i = clist_length(&interp->stack);
  stack_push(&interp->stack, c);
  return TPMI_OK;
}

static tpmi_status_t do_emit(tpmi_t *interp) {
  cell_t *top = stack_top(&interp->stack);

  if (top->type != CT_INT) {
    ERROR(interp, "'emit' requires integer at the top of stack");
    return TPMI_ERROR;
  }

  putchar(top->i);
  return do_drop(interp);
}

static tpmi_status_t do_print_stack(tpmi_t *interp) {
  unsigned n = clist_length(&interp->stack);
  cell_t *c;
  TAILQ_FOREACH(c, &interp->stack, list) {
    printf("[%u] ", --n);
    cell_print(c);
    putchar('\n');
  }
  return TPMI_OK;
}

static void print_words(tpmi_t *interp, word_type_t type) {
  entry_t *entry = NULL;

  while (dict_iter(interp->words, &entry))
    if (entry->word->type == type)
      word_print(entry->key, entry->word);
}

static tpmi_status_t do_print_dict(tpmi_t *interp) {
  print_words(interp, WT_DEF);
  print_words(interp, WT_VAR);
  print_words(interp, WT_BUILTIN);
  print_words(interp, WT_CFUNC);
  return TPMI_OK;
}

static entry_t *find_var(tpmi_t *interp, char *key) {
  entry_t *entry = dict_find(interp->words, key);
  if (entry != NULL)
    return entry;
  ERROR(interp, "'%s': no such variable", key);
  return NULL;
}

static tpmi_status_t do_load(tpmi_t *interp) {
  char *key = stack_top(&interp->stack)->atom;
  entry_t *entry = find_var(interp, key);
  if (entry == NULL)
    return TPMI_ERROR;

  word_t *word = entry->word;
  if (word->value == NULL) {
    ERROR(interp, "'%s': variable has not been initialized", key);
    return TPMI_ERROR;
  }

  cell_delete(stack_pop(&interp->stack));
  stack_push(&interp->stack, cell_copy(word->value));
  return TPMI_OK;
}

static tpmi_status_t do_store(tpmi_t *interp) {
  entry_t *entry = find_var(interp, stack_top(&interp->stack)->atom);
  if (entry == NULL)
    return TPMI_ERROR;
  cell_delete(stack_pop(&interp->stack));
  cell_t *value = stack_pop(&interp->stack);
  word_t *word = entry->word;
  if (word->value != NULL) {
    cell_swap(word->value, value);
    cell_delete(value);
  } else {
    word->value = value;
  }
  return TPMI_OK;
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
  ": get-red 0 extract ;",
  ": get-green 1 extract ;",
  ": get-blue 2 extract ;",
  ": put-red 0 insert ;",
  ": put-green 1 insert ;",
  ": put-blue 2 insert ;",
  NULL
};

static fn_ctor_t cfuncs[] = {
  { "insert", &tpm_insert, "<>cmi" },
  { "extract", &tpm_extract, "<>c>mi" },
  { "explode", &tpm_explode, ">m>m>mc" },
  { "implode", &tpm_implode, ">cmmm" },
  { "save-mono", &tpm_mono_buf_save, "ms" },
  { "save-color", &tpm_color_buf_save, "cs" },
  { "sine", &tpm_sine, ">mf" },
  { "noise", &tpm_noise, ">mii" },
  { "plasma", &tpm_plasma, ">m" },
  { "light", &tpm_light, ">mif" },
  { "perlin-noise", &tpm_perlin_noise, ">mi" },
  { "add", &tpm_add, ">@m@m@m" },
  { "mul", &tpm_mul, ">@m@m@m" },
  { "mix", &tpm_mix, ">@m@m@mi" },
  { "max", &tpm_max, ">@m@m@m" },
  { "shade", &tpm_shade, ">mmm" },
  { "mix-map", &tpm_mix_map, ">mmmm" },
  { "repeat", &tpm_repeat, ">@m@mii" },
  { "flip", &tpm_flip, ">@m@m" },
  { "rotate", &tpm_rotate, ">@m@m" },
  { "twist", &tpm_twist, ">@m@mf" },
  { "move", &tpm_move, ">@m@mff" },
  { "distort", &tpm_distort, ">@m@mmmff" },
  { "invert", &tpm_invert, ">@m@m" },
  { "sine-color", &tpm_sine_color, ">@m@mi" },
  { "hsv-modify", &tpm_hsv_modify, ">ccff" },
  { "brightness", &tpm_brightness, ">@m@mf" },
  { "contrast", &tpm_contrast, ">@m@mf" },
  { "colorize", &tpm_colorize, ">cmii" },
  { "grayscale", &tpm_grayscale, ">mc" },
  { "blur-3x3", &tpm_blur_3x3, ">@m@m" },
  { "blur-5x5", &tpm_blur_5x5, ">@m@m" },
  { "gaussian-3x3", &tpm_gaussian_3x3, ">@m@m" },
  { "gaussian-5x5", &tpm_gaussian_5x5, ">@m@m" },
  { "sharpen", &tpm_sharpen, ">@m@m" },
  { "emboss", &tpm_emboss, ">@m@m" },
  { "edges", &tpm_edges, ">@m@m" },
  { "median-3x3", &tpm_median_3x3, ">@m@m" },
  { "median-5x5", &tpm_median_5x5, ">@m@m" },
  { NULL }
};

void tpmi_init(tpmi_t *interp) {
  /* Initialize builtin words */
  for (fn_ctor_t *builtin = builtins; builtin->id; builtin++) {
    word_t *word = calloc(1, sizeof(word_t));
    word->type = WT_BUILTIN;
    word->value = CT_FN->new();
    word->value->fn = new_fn(builtin);
    word->immediate = builtin->immediate;
    dict_add(interp->words, builtin->id)->word = word;
  }

  /* Initialize C function calls */
  for (fn_ctor_t *cfunc = cfuncs; cfunc->id; cfunc++) {
    word_t *word = calloc(1, sizeof(word_t));
    word->type = WT_CFUNC;
    word->value = CT_FN->new();
    word->value->fn = new_fn(cfunc);
    word->immediate = cfunc->immediate;
    dict_add(interp->words, cfunc->id)->word = word;
  }

  /* Add special variables */
  word_t *state = calloc(1, sizeof(word_t));
  state->type = WT_VAR;
  state->value = CT_INT->new();
  dict_add(interp->words, "state")->word = state;
  interp->mode = (tpmi_mode_t *)&state->value->i;

  /* Compile initial program */
  for (char **line = initprog; *line; line++)
    tpmi_compile(interp, *line);
}
