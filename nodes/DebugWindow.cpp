#include "DebugWindow.hpp"

DebugWindow::DebugWindow(unsigned rows, unsigned cols) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::string error = SDL_GetError();
    SDL_log << "Couldn't initialize SDL: " << error << '\n';
    throw std::runtime_error(error);
  }

  unsigned windowWidth = rows * TP_WIDTH;
  unsigned windowHeight = cols * TP_HEIGHT;

  window_ =
      SDL_CreateWindow("Debug Window", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0);

  if (window_ == nullptr) {
    std::string error = SDL_GetError();
    SDL_log << "Couldn't create window: " << error << '\n';
    throw std::runtime_error(error);
  }

  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
  texture_ =
      SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_STREAMING, TP_WIDTH, TP_HEIGHT);
}

void DebugWindow::clear() {
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_RenderClear(renderer_);
}

void DebugWindow::displayBuf(tpm_mono_buf buffer, unsigned x, unsigned y) {
  RGBA* pixels = getPixelsAndLockTexture();

  for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
    uint8_t p = ((uint8_t*)buffer)[i];
    pixels[i] = (RGBA){255, p, p, p};
  }

  unlockTexture();
  auto renderDest = getDestinationRect(x, y);
  SDL_RenderCopy(renderer_, texture_, NULL, &renderDest);
}

void DebugWindow::displayBuf(tpm_color_buf buffer, unsigned x, unsigned y) {
  RGBA* pixels = getPixelsAndLockTexture();

  for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
    uint8_t r = ((uint8_t**)buffer)[0][i];
    uint8_t g = ((uint8_t**)buffer)[1][i];
    uint8_t b = ((uint8_t**)buffer)[2][i];
    pixels[i] = (RGBA){r, g, b, 255};
  }

  unlockTexture();
  auto renderDest = getDestinationRect(x, y);
  SDL_RenderCopy(renderer_, texture_, NULL, &renderDest);
}

void DebugWindow::render() {
  SDL_RenderPresent(renderer_);
}

DebugWindow::RGBA* DebugWindow::getPixelsAndLockTexture() {
  RGBA* pixels;
  int pitch;
  SDL_LockTexture(texture_, NULL, (void**)&pixels, &pitch);
  return pixels;
}

void DebugWindow::unlockTexture() {
  SDL_UnlockTexture(texture_);
}

SDL_Rect DebugWindow::getDestinationRect(int x, int y) {
  int posX = x * TP_WIDTH;
  int posY = y * TP_HEIGHT;
  SDL_Rect dest = {posX, posY, TP_WIDTH, TP_HEIGHT};
  return dest;
}