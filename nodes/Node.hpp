#ifndef NODES_NODE_HPP
#define NODES_NODE_HPP

#include <vector>
#include <memory>
#include <typeindex>

#include <Logger.hpp>
#include <Function.hpp>

class ParamLink;

typedef std::vector<std::shared_ptr<ParamLink> > ParamLinkContainer;

class Node {
 public:
  Logger log;
  Node(Function* f, const ParamLinkContainer& pLinks)
      : function(f), paramLinks(pLinks), log{"Node"} {}
  std::shared_ptr<DataT> getParam(int param) {
    updateValues();
    if (param >= values.size() || param < 0) {
      log << "Unable to return param. Param of value " + std::to_string(param) +
                 " out of boundaries ";
      return nullptr;
    }
    return values[param];
  }

 private:
  void updateValues();
  Values getParamValues();
  Values values;
  std::unique_ptr<Function> function;
  ParamLinkContainer paramLinks;
};

class ParamMatcher {
 public:
  ParamMatcher(const Function& f)
      : types(f.getParamTypes()), log{"ParamMatcher"} {}
  bool match(const Values& params) {
    if (types.size() == params.size()) {
      auto typeIt = types.begin();
      auto paramIt = params.begin();
      while (paramIt != params.end() && typeIt != types.end()) {
        auto param = (*paramIt);
        std::type_index paramType = param->typeId();
        std::type_index targetType = (*typeIt);
        if (paramType != targetType) {
          log << std::string("Unable to match params: not able to match ") +
                     paramType.name() + " with " + targetType.name();
          return false;
        }
        typeIt++;
        paramIt++;
      }
      return true;

    } else {
      log << std::string("Unable to match params: diffrent number of params ") +
                 std::to_string(types.size()) + " vs " +
                 std::to_string(params.size());
      return false;
    }
  }

 private:
  Types types;
  Logger log;
};

class ParamLink {
 public:
  virtual std::shared_ptr<DataT> getParam() = 0;
};

class FunctionParamLink : public ParamLink {
 public:
  FunctionParamLink(std::shared_ptr<Node> n, int p) : node(n), param(p) {}
  std::shared_ptr<DataT> getParam() override { return node->getParam(param); }

 private:
  std::shared_ptr<Node> node;
  int param;
};

class DataParamLink : public ParamLink {
 public:
  DataParamLink(std::shared_ptr<DataT> data) : data_(data) {}
  std::shared_ptr<DataT> getParam() override { return data_; }

 private:
  std::shared_ptr<DataT> data_;
};

#endif