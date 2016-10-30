#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <editline/readline.h>

#include "ansi.h"
#include "config.h"
#include "interp.h"
#include "gui.h"

static bool need_more = false;
static tpmi_t *interp = NULL;
static sigjmp_buf ctrlc_buf;

static void sigint(int signo) {
  if (signo == SIGINT)
    siglongjmp(ctrlc_buf, 1);
}

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

int main(int argc, char *argv[]) {
  (void)argc, (void)argv;

  interp = tpmi_new(); 

  gui_init();
  gui_update(interp);

  /* Set up our own handler for CTRL + C */
  signal(SIGINT, sigint);

  rl_catch_signals = 0;
  rl_readline_name = "texproma";
  rl_attempted_completion_function = texproma_completion;
  rl_initialize();

  puts(MAGENTA BOLD "TEX" WHITE "ture " MAGENTA "PRO" WHITE "cessing "
       MAGENTA "MA" WHITE "chine" RESET);
  puts("Copyright © 1999-2016 Krystian Bacławski");
  puts("Press CTRL+C to quit");

  char *histline = NULL;
  char *line;

  bool quit = sigsetjmp(ctrlc_buf, 1);

  while (!quit && (line = readline(need_more ? "_ " : "> "))) {
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

    gui_update(interp);

    free(line);
  }

  puts("quit");

  /* Clean up our memory */
  tpmi_delete(interp);

  exit(EXIT_SUCCESS);
}
