#include <assert.h>
#include <signal.h>
#include <SDL.h>

#include "gui.h"
#include "libtexproma.h"
#include "interp.h"

#define NUM 4

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static color_t *pixels;

void gui_init() {
  /* Workaround for libedit SIGINT handler */
  struct sigaction action;
  sigaction(SIGINT, NULL, &action);

  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0) {
    fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  sigaction(SIGINT, &action, NULL);

  window = SDL_CreateWindow("TexProMa by Krystian Bacławski",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            TP_WIDTH * NUM, TP_HEIGHT, 0);
  renderer = SDL_CreateRenderer(window, -1,
                                SDL_RENDERER_ACCELERATED);
  texture = SDL_CreateTexture(renderer, 
                              SDL_PIXELFORMAT_RGB24,
                              SDL_TEXTUREACCESS_STREAMING,
                              TP_WIDTH, TP_HEIGHT);
  assert(texture != NULL);

  pixels = malloc(TP_WIDTH * TP_HEIGHT * sizeof(color_t));

  /* Clean up on exit */
  atexit(gui_end);
}

void gui_end() {
  free(pixels);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
}

void gui_update(tpmi_t *interp) {
  SDL_Rect dst = { TP_WIDTH * (NUM - 1), 0, TP_WIDTH, TP_HEIGHT };
  cell_t *c;

  SDL_RenderClear(renderer);

  TAILQ_FOREACH_REVERSE(c, &interp->stack, cell_list, list) {
    if (c->type == CT_MONO || c->type == CT_COLOR) {
      int pitch = TP_WIDTH * sizeof(color_t);

      if (c->type == CT_MONO) {
        uint8_t *mono = c->mono;
        for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
          uint8_t p = mono[i];
          pixels[i] = (color_t){.r = p, .g = p, .b = p};
        }
        assert(SDL_UpdateTexture(texture, NULL, pixels, pitch) == 0);
      } else {
        assert(SDL_UpdateTexture(texture, NULL, c->color, pitch) == 0);
      }

      SDL_RenderCopy(renderer, texture, NULL, &dst);

      dst.x -= TP_WIDTH;
      
      if (dst.x < 0)
        break;
    }
  }

  SDL_RenderPresent(renderer);
}
