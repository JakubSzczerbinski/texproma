#include <SDL.h>
#include <stdexcept>
#include "Logger.hpp"
extern "C" {
#include <libtexproma.h>
}

class DebugWindow {
 public:
  DebugWindow(unsigned rows, unsigned cols);
  ~DebugWindow();
  void clear();
  void displayBuf(tpm_mono_buf buffer, unsigned x, unsigned y);
  void displayBuf(tpm_color_buf buffer, unsigned x, unsigned y);
  void render();

 private:
  struct RGBA {
    uint8_t a, b, g, r;
  };
  RGBA* getPixelsAndLockTexture();
  void unlockTexture();
  SDL_Rect getDestinationRect(int x, int y);

  Logger SDL_log{"SDL"};
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
};