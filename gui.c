#include <assert.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "gui.h"
#include "libtexproma.h"
#include "interp.h"

#define NUM 4
#define FONT_H 16

typedef struct { uint8_t a, b, g, r; } RGBA;

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static TTF_Font *font16;

static void RenderText(SDL_Renderer *renderer, TTF_Font *font,
                       const char *str, SDL_Rect *dst, bool center)
{
  SDL_Color white = {.r = 255, .g = 255, .b = 255, .a = 0};
  SDL_Surface *surf = TTF_RenderUTF8_Blended(font, str, white);
  SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, surf);
  SDL_Rect area = {
    .y = dst->y + (dst->h - surf->h) / 2,
    .x = dst->x,
    .w = surf->w,
    .h = surf->h
  };
  if (center)
    area.x += (dst->w - surf->w) / 2;
  assert(SDL_RenderCopy(renderer, text, NULL, &area) == 0);
  SDL_DestroyTexture(text);
  SDL_FreeSurface(surf);
}

static void gui_end();

void gui_init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  atexit(SDL_Quit);

  if (TTF_Init() < 0) {
    fprintf(stderr, "Couldn't initialize SDL_ttf: %s\n", TTF_GetError());
    exit(EXIT_FAILURE);
  }

  atexit(TTF_Quit);

  /* Open Inconsolata font */
  if (!(font16 = TTF_OpenFont("data/Inconsolata.otf", 15))) {
    fprintf(stderr, "Couldn't open Insonsolata font: %s\n", TTF_GetError());
    exit(EXIT_FAILURE);
  }

  SDL_DisplayMode dm;
  assert(SDL_GetDesktopDisplayMode(0, &dm) == 0);

  int win_x = (dm.w - TP_WIDTH * NUM) / 2;
  int win_y = (dm.h - (TP_HEIGHT + FONT_H)) / 8;

  window = SDL_CreateWindow("TexProMa by Krystian Bacławski", win_x, win_y,
                            TP_WIDTH * NUM, TP_HEIGHT + FONT_H, 0);
  renderer = SDL_CreateRenderer(window, -1,
                                SDL_RENDERER_SOFTWARE);
  texture = SDL_CreateTexture(renderer, 
                              SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STREAMING,
                              TP_WIDTH, TP_HEIGHT);
  assert(texture != NULL);

  /* Clean up on exit */
  atexit(gui_end);
}

static void gui_end() {
  TTF_CloseFont(font16);

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}

static void gui_redraw(tpmi_t *interp) {
  char str[40];
  cell_t *c;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  /* clear texture header */
  for (int i = 0; i < NUM; i++) {
    SDL_Rect dst = { TP_WIDTH * i, 0, TP_WIDTH, FONT_H };
    int c = (i & 1) ? 40 : 48;
    SDL_SetRenderDrawColor(renderer, c, c, c, 0);
    SDL_RenderFillRect(renderer, &dst);
  }

  /* clear stack header */
  for (int i = 1; i <= TP_HEIGHT / FONT_H; i++) {
    SDL_Rect dst = { 0, FONT_H * i, TP_WIDTH, FONT_H };
    int c = (i & 1) ? 24 : 32;
    SDL_SetRenderDrawColor(renderer, c, c, c, 0);
    SDL_RenderFillRect(renderer, &dst);
  }

  /* display textures on top of the stack */
  unsigned n = 0;
  unsigned stkcnt = TP_HEIGHT / FONT_H;
  int imgcnt = NUM;

  TAILQ_FOREACH_REVERSE(c, &interp->stack, cell_list, list) {
    if (c->type == CT_MONO || c->type == CT_COLOR) {
      if (--imgcnt > 0) {
        int x = imgcnt * TP_WIDTH;
        SDL_Rect dst = { x, FONT_H, TP_WIDTH, TP_HEIGHT };

        /* display texture */
        int pitch;
        RGBA *pixels;

        assert(SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch) == 0);
        assert(pitch == TP_WIDTH * sizeof(RGBA));

        if (c->type == CT_MONO) {
          for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
            uint8_t p = ((uint8_t *)c->data)[i];
            pixels[i] = (RGBA){.r = p, .g = p, .b = p};
          }
        } else {
          for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
            uint8_t r = ((uint8_t **)c->data)[0][i];
            uint8_t g = ((uint8_t **)c->data)[1][i];
            uint8_t b = ((uint8_t **)c->data)[2][i];
            pixels[i] = (RGBA){.r = r, .g = g, .b = b};
          }
        }

        SDL_UnlockTexture(texture);
        assert(SDL_RenderCopy(renderer, texture, NULL, &dst) == 0);

        /* display stack position */
        snprintf(str, sizeof(str), "STACK [%d]", n);
        dst = (SDL_Rect){ x, 0, TP_WIDTH, FONT_H };
        RenderText(renderer, font16, str, &dst, true);
      }
    }

    if (n < stkcnt) {
      char *cstr = cell_stringify(c);
      snprintf(str, sizeof(str), "[%d] %s", n, cstr);
      SDL_Rect dst = { 0, (n + 1) * FONT_H, TP_WIDTH, FONT_H };
      RenderText(renderer, font16, str, &dst, false);
      free(cstr);
    }

    n++;
  }

  while (--imgcnt > 0) {
    SDL_Rect dst = { imgcnt * TP_WIDTH, 0, TP_WIDTH, FONT_H };
    RenderText(renderer, font16, "null", &dst, true);
  }

  SDL_Rect dst = { 0, 0, TP_WIDTH, FONT_H };
  RenderText(renderer, font16, "STACK LISTING", &dst, true);

  SDL_RenderPresent(renderer);
}

void gui_loop(tpmi_t *interp) {
  gui_redraw(interp);

  while (true) {
    SDL_Event event;
    SDL_WaitEvent (&event);

    if (event.type == SDL_QUIT)
      break;
    if (event.type == SDL_USEREVENT)
      gui_redraw(interp);
    SDL_RenderPresent(renderer);
  }
}

void gui_update() {
  SDL_Event event = {.type = SDL_USEREVENT};
  SDL_PushEvent(&event);
}

void gui_quit() {
  SDL_Event event = {.type = SDL_QUIT};
  SDL_PushEvent(&event);
}
