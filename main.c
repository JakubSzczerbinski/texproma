#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <editline/readline.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

#include "ansi.h"
#include "config.h"
#include "interp.h"
#include "gui.h"

static bool need_more = false;
static tpmi_t *interp = NULL;
static pthread_t shell;

static void line_append(char **dst, char *src) {
  if (*dst == NULL) {
    *dst = strdup(src);
  } else {
    size_t n = strlen(*dst) + strlen(src) + 2;
    *dst = realloc(*dst, n);
    strcat(*dst, "\n");
    strcat(*dst, src);
  }
}

static char *complete(const char *text, int state) {
  static unsigned len;
  static entry_t *match;
  
  if (state == 0) {
    match = NULL;
    len = strlen(text);

    if (len == 0)
      return NULL;
  }

  while (dict_iter(interp->words, &match))
    if (strncmp(match->key, text, len) == 0)
      return strdup(match->key);

  return NULL;
}

static char **texproma_completion(const char *text, int start, int end) {
  (void)start, (void)end;
  return rl_completion_matches(text, complete);
}

static void shell_quit(void *data) {
  tpmi_t *interp = data;
  tpmi_delete(interp);
  puts("quit");
}

static void* shell_run(void *data) {
  tpmi_t *interp = data;

  pthread_cleanup_push(shell_quit, interp);

  rl_readline_name = "texproma";
  rl_attempted_completion_function = texproma_completion;
#ifndef __MINGW32__
  rl_catch_signals = 0;
  rl_initialize();
#else
  SetConsoleOutputCP(65001);
#endif

  puts(MAGENTA BOLD "TEX" WHITE "ture " MAGENTA "PRO" WHITE "cessing "
       MAGENTA "MA" WHITE "chine" RESET);
  puts("Copyright © 1999-2016 Krystian Bacławski");
  puts("Press CTRL+C to quit");

  char *histline = NULL;
  char *line;

  while ((line = readline(need_more ? "_ " : "> "))) {
    tpmi_status_t status = tpmi_compile(interp, line);

    if (status != TPMI_ERROR) {
      need_more = (status == TPMI_NEED_MORE);

      line_append(&histline, line);

      if (!need_more) {
        add_history(histline);
        free(histline);
        histline = NULL;
      }
    }

    gui_update();

    free(line);
  }

  pthread_cleanup_pop(true);

  return NULL;
}

int main(int argc, char *argv[]) {
  (void)argc, (void)argv;

  interp = tpmi_new(); 
  gui_init();

  pthread_create(&shell, NULL, shell_run, interp);
  gui_loop(interp);
  pthread_cancel(shell);

  exit(EXIT_SUCCESS);
}
