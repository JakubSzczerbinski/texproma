#include "DebugWindow.hpp"
#include "tpmWrapper.hpp"
#include "NodeTree.hpp"

int main() {
  DebugWindow window(3, 1);
  while (true) {
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT)
      return 0;
    NodeTree tree;
    auto sineNode = tree.addNode(new tpmWrapper::TpmSine());
    auto floatInputNode = tree.addNode(makeInput(10.0f));
    auto revertNode = tree.addNode(new tpmWrapper::TpmInvert());
    auto addNode = tree.addNode(new tpmWrapper::TpmAdd());

    tree.linkNodes(sineNode, 0, floatInputNode, 0);
    tree.linkNodes(revertNode, 0, sineNode, 0);
    tree.linkNodes(addNode, 0, sineNode, 0);
    tree.linkNodes(addNode, 1, revertNode, 0);

    auto addedResult = tree.getResult(addNode, 0);
    auto revertResult = tree.getResult(revertNode, 0);
    auto sineResult = tree.getResult(sineNode, 0);

    window.clear();
    window.displayBuf(extractRawData<tpmWrapper::MonoBuf>(addedResult).getRawPtr(), 2, 0);
    window.displayBuf(extractRawData<tpmWrapper::MonoBuf>(revertResult).getRawPtr(), 1, 0);
    window.displayBuf(extractRawData<tpmWrapper::MonoBuf>(sineResult).getRawPtr(), 0, 0);
    window.render();
  }
}