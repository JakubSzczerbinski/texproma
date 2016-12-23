#include "DebugWindow.hpp"
#include "tpmWrapper.hpp"
#include "NodeTree.hpp"

int main() {
  DebugWindow window(1, 1);
  while (true) {
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
      return 0;
    NodeTree tree;
    auto node = tree.addNode(new tpmWrapper::TpmNoise());
    auto node2 = tree.addNode(makeInput(10u));
    tree.linkNodes(node, 1, node2, 0);
    tree.linkNodes(node, 0, node2, 0);
    auto result = tree.getResult(node, 0);
    auto buffer = extractRawData<tpmWrapper::MonoBuf>(result);
    window.clear();
    window.displayBuf(buffer.getRawPtr(), 0, 0);
    window.render();
  }
}