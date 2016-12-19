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
  Node(Function* f) : log{"Node", f->getName()}, function(f) {}
  size_t getArity() { return function->getParamTypes().size(); }
  size_t getReturnArity() { return function->getReturnTypes().size(); }
  std::string getName() { return function->getName(); }
  Values operator()(Values params);

 private:
  std::unique_ptr<Function> function;
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
        if (param == nullptr) {
          log << "Unable to match params: one of the params is null";
          return false;
        }
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

#endif