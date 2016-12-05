extern "C" {
#include <libtexproma.h>
}
#include "DebugWindow.hpp"
#include "tpmWrapper.hpp"

int main() {
  DebugWindow window(1, 1);
  while (true) {
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
      return 0;
    Values params = {makeSharedData(10.0f)};
    tpmWrapper::TpmSine sine;
    Values returns = sine(params);
    auto buffer = extractRawData<tpmWrapper::MonoBuf>(returns[0]);
    window.clear();
    window.displayBuf(buffer.getRawPtr(), 0, 0);
    window.render();
  }
}