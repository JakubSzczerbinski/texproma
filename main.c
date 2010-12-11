#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <histedit.h>
#include <signal.h>
#include <SDL.h>

#include "interp.h"
#include "ansi.h"

static bool need_more = false;

static char *prompt(EditLine *el) {
	return (need_more ? "_ " : "> ");
}

int main(int argc, char *argv[]) {
  EditLine *el;
  History *hist;
  HistEvent ev;
  tpmi_t *interp;

  /* Initialize the state to use our prompt function and emacs style editing */
  el = el_init(argv[0], stdin, stdout, stderr);
  el_set(el, EL_PROMPT, &prompt);
  el_set(el, EL_EDITOR, "emacs");

	/* Handle signals gracefully */
	el_set(el, EL_SIGNAL, 1);
	/* Source the user's defaults file */
	el_source(el, NULL);

  /* Initialize the history */
  if ((hist = history_init()) == 0) {
    fprintf(stderr, "history could not be initialized\n");
    exit(EXIT_FAILURE);
  }
  /* Set the size of the history */
  history(hist, &ev, H_SETSIZE, 200);
  /* This sets up the call back functions for history functionality */
  el_set(el, EL_HIST, history, hist);

  /* Initialize the SDL library */
  struct sigaction action;
  sigaction(SIGINT, NULL, &action);

  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0) {
    fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  sigaction(SIGINT, &action, NULL);

  /* Clean up on exit */
  atexit(SDL_Quit);

  puts("TEXture PROcessing MAchine");
  puts("Copyright © 1999-2016 Krystian Bacławski");
  puts("Press CTRL+C to exit");

  interp = tpmi_new(); 

  while (true) {
    const char *line;   /* read command line with the trailing '\n' */
    int count;          /* the number of characters read */

    line = el_gets(el, &count);

    if (count == 0) {
      putchar('\n');
      continue;
    }
    
    tpmi_status_t status = tpmi_compile(interp, line);

    if (status != TPMI_ERROR) {
      need_more = (status == TPMI_NEED_MORE);

      /* We have to explicitly add commands to the history */
      history(hist, &ev, need_more ? H_APPEND : H_ENTER, line);
    }
  }

  /* Clean up our memory */
  tpmi_delete(interp);
  history_end(hist);
  el_end(el);

  exit(EXIT_SUCCESS);
}
