#include <stdio.h>
#include <stdlib.h>

#include "word.h"
#include "interp.h"

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
  STACK_PUSH(&interp->stack, cell_int(clist_length(&interp->stack)));
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
  char *key = STACK_TOP(&interp->stack)->atom;
  entry_t *entry = find_var(interp, key);
  if (entry == NULL)
    return TPMI_ERROR;

  word_t *word = entry->word;
  if (word->var == NULL) {
    ERROR(interp, "'%s': variable has not been initialized", key);
    return TPMI_ERROR;
  }

  cell_delete(STACK_POP(&interp->stack));
  STACK_PUSH(&interp->stack, cell_copy(word->var));
  return TPMI_OK;
}

static tpmi_status_t do_store(tpmi_t *interp) {
  entry_t *entry = find_var(interp, STACK_TOP(&interp->stack)->atom);
  if (entry == NULL)
    return TPMI_ERROR;
  cell_delete(STACK_POP(&interp->stack));
  cell_t *value = STACK_POP(&interp->stack);
  word_t *word = entry->word;
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

void tpmi_init(tpmi_t *interp) {
  /* Initialize builtin words */
  for (fn_ctor_t *builtin = builtins; builtin->id; builtin++) {
    word_t *word = calloc(1, sizeof(word_t));
    word->type = WT_BUILTIN;
    word->func = new_fn(builtin);
    word->immediate = builtin->immediate;
    dict_add(interp->words, builtin->id)->word = word;
  }

  /* Initialize C function calls */
  for (fn_ctor_t *cfunc = cfuncs; cfunc->id; cfunc++) {
    word_t *word = calloc(1, sizeof(word_t));
    word->type = WT_CFUNC;
    word->func = new_fn(cfunc);
    word->immediate = cfunc->immediate;
    dict_add(interp->words, cfunc->id)->word = word;
  }

  /* Add special variables */
  word_t *state = calloc(1, sizeof(word_t));
  state->type = WT_VAR;
  state->var = cell_int(0);
  dict_add(interp->words, "state")->word = state;
  interp->mode = (tpmi_mode_t *)&state->var->i;

  /* Compile initial program */
  for (char **line = initprog; *line; line++)
    tpmi_compile(interp, *line);
}
