extern "C" {
#include <libtexproma.h>
}
#include "DebugWindow.hpp"

int main() {
  DebugWindow window(5, 3);
  while (true) {
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
      return 0;
    tpm_mono_buf buffer = (tpm_mono_buf)malloc(256 * 256);
    tpm_sine(buffer, 4.0);
    tpm_mono_buf buffer1 = (tpm_mono_buf)malloc(256 * 256);
    tpm_noise(buffer1, 10, 13);
    tpm_mono_buf buffer2 = (tpm_mono_buf)malloc(256 * 256);
    tpm_plasma(buffer2);
    window.clear();
    window.displayBuf(buffer, 0, 0);
    window.displayBuf(buffer1, 1, 1);
    window.displayBuf(buffer2, 2, 2);
    window.render();
  }
}