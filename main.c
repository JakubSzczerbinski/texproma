#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

#include "ansi.h"
#include "interp.h"
#include "gui.h"
#include "linenoise.h"

static bool quit = false;
static tpmi_t *interp = NULL;

static void sigint(int num) {
  quit = true;
}

static void append(char **dst, char *src) {
  if (*dst == NULL) {
    *dst = strdup(src);
  } else {
    size_t n = strlen(*dst) + strlen(src) + 2;
    *dst = realloc(*dst, n);
    strcat(*dst, " ");
    strcat(*dst, src);
  }
}

static void completion(const char *buf, linenoiseCompletions *lc) {
  const char *token;
  word_t *match = NULL;

  if ((token = strrchr(buf, ' ')) == NULL)
    token = buf;
  else
    token++;

  while (dict_match(interp->words, &match, token)) {
    size_t n = (token - buf) + strlen(match->key) + 1;
    char *line = malloc(n);
    strncpy(line, buf, token - buf);
    strcat(line, match->key);
    linenoiseAddCompletion(lc, line);
    free(line);
  }
}

int main(int argc, char *argv[]) {
  interp = tpmi_new(); 

  gui_init();
  gui_update(interp);

  puts(MAGENTA BOLD "TEX" WHITE "ture " MAGENTA "PRO" WHITE "cessing "
       MAGENTA "MA" WHITE "chine" RESET);
  puts("Copyright © 1999-2016 Krystian Bacławski");
  puts("Press CTRL+C to exit");

  linenoiseSetMultiLine(true);
  linenoiseSetCompletionCallback(completion);

  /* Capture CTRL + C */
  struct sigaction action = {{sigint}, 0, 0};
  sigaction(SIGINT, &action, NULL);

  bool need_more = false;
  char *line, *histline = NULL;

  while ((line = linenoise(need_more ? "_ " : "> ")) && !quit) {
    if (line[0] != '\0') {
      tpmi_status_t status = tpmi_compile(interp, line);

      if (status != TPMI_ERROR) {
        need_more = (status == TPMI_NEED_MORE);

        /* We have to explicitly add commands to the history */
        if (need_more) {
          append(&histline, line);
        } else {
          append(&histline, line);
          linenoiseHistoryAdd(histline);
          free(histline);
          histline = NULL;
        }
      }

      gui_update(interp);
    }

    free(line);
  }

  /* Clean up our memory */
  tpmi_delete(interp);

  exit(EXIT_SUCCESS);
}
