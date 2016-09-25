#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <editline/readline.h>

#include "ansi.h"
#include "config.h"
#include "interp.h"
#include "gui.h"

static bool quit = false;
static bool need_more = false;
static tpmi_t *interp = NULL;

static void sigint() {
  quit = true;
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
  static word_t *match;
  
  if (state == 0) {
    match = NULL;
    len = strlen(text);

    if (len == 0)
      return NULL;
  }

  if (dict_match(interp->words, &match, text))
    return strdup(match->key);

	return NULL;
}

static char **texproma_completion(const char *text, int start, int end) {
  return rl_completion_matches(text, complete);
}

int main(int argc, char *argv[]) {
  interp = tpmi_new(); 

  gui_init();
  gui_update(interp);

  rl_initialize();
  rl_readline_name = "texproma";
  rl_attempted_completion_function = texproma_completion;

  /* Set up our own handler for CTRL + C */
  struct sigaction action = {{sigint}, 0, 0};
  sigaction(SIGINT, &action, NULL);

  puts(MAGENTA BOLD "TEX" WHITE "ture " MAGENTA "PRO" WHITE "cessing "
       MAGENTA "MA" WHITE "chine" RESET);
  puts("Copyright © 1999-2016 Krystian Bacławski");
  puts("Press CTRL+C to exit");


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

      gui_update(interp);
    }

    free(line);
  }

  puts("\nQuit.");

  /* Clean up our memory */
  tpmi_delete(interp);

  exit(EXIT_SUCCESS);
}
