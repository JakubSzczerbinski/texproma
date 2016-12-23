#ifndef NODES_NODE_TREE_HPP
#define NODES_NODE_TREE_HPP
#include <Logger.hpp>
#include <Node.hpp>
#include <memory>
#include <map>

struct Return {
  std::shared_ptr<Node> node;
  unsigned returnIndex;
};

struct Param {
  std::shared_ptr<Node> node;
  unsigned paramIndex;
  bool operator<(Param other) const {
    if (node < other.node)
      return true;
    if (node > other.node)
      return false;
    if (paramIndex < other.paramIndex)
      return true;
    else
      return false;
  }
};

typedef std::map<Param, Return> NodeLinkContainer;
typedef std::vector<std::shared_ptr<Node>> NodeContainer;
typedef std::map<std::shared_ptr<Node>, Values> CachedValuesContainer;

class NodeTree {
 public:
  std::shared_ptr<Node> addNode(Function* f) {
    cachedValues.clear();
    auto node = std::make_shared<Node>(f);
    nodes.push_back(node);
    return node;
  }
  void linkNodes(std::shared_ptr<Node> inputNode,
                 unsigned inputIndex,
                 std::shared_ptr<Node> outputNode,
                 unsigned outputIndex) {
    Param param{inputNode, inputIndex};
    Return ret{outputNode, outputIndex};
    nodeLinks[param] = ret;
  }
  std::shared_ptr<DataT> getResult(std::shared_ptr<Node> node,
                                   unsigned paramIndex) {
    return getResult({node, paramIndex});
  }

 private:
  std::shared_ptr<DataT> getResult(Return ret) {

    auto it = cachedValues.find(ret.node);
    if(it != cachedValues.end())
    {
      return it->second[ret.returnIndex];
    }

    Values dataParams;
    unsigned arity = ret.node->getArity();

    for (unsigned i = 0; i < arity; i++) {
      Param param = {ret.node, i};
      auto it = nodeLinks.find(param);
      if (it == nodeLinks.end()) {
        log << "Can't evaluate result. Missing return to match param " +
                   std::to_string(i) + " of " + ret.node->getName();
        throw std::runtime_error("Evaulation return missing.");
      }
      auto data = getResult(it->second);
      dataParams.push_back(data);
    }
    Values returns = (*ret.node)(dataParams);
    cachedValues[ret.node] = returns;
    return returns[ret.returnIndex];
  }
  Logger log{"NodeTree"};
  NodeContainer nodes;
  NodeLinkContainer nodeLinks;
  CachedValuesContainer cachedValues;
};

#endif