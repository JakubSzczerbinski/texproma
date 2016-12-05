#ifndef NODES_NODE_HPP
#define NODES_NODE_HPP

#include <vector>
#include <memory>
#include <typeindex>

#include <Logger.hpp>
#include <Function.hpp>

#include <iostream>

class ParamLink;

typedef std::vector<ParamLink> ParamLinkContainer;

class Node {
 public:
  Logger log;
  Node(Function* f) : function(f), log{"Node"} {}
  ~Node();
  std::shared_ptr<DataT> getParam(int param) {
    updateValues();
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
        std::cout << (*paramIt) << std::endl;
        auto param = (*paramIt);
        std::type_index paramType = param->typeId();
        std::type_index targetType = (*typeIt);
        if (paramType != targetType) {
          log << std::string("Unable to match params: not able to match ") + paramType.name() + " with " +
                     targetType.name();
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

class FunctionParamLink : ParamLink{
 public:
  FunctionParamLink(std::shared_ptr<Node> n, int p) : node(n), param(p) {}
  std::shared_ptr<DataT> getParam() override { return node->getParam(param); }

 private:
  std::shared_ptr<Node> node;
  int param;
};

template <typename dataType>
class DataParamLink : ParamLink
{
public:
	DataParamLink(const dataType& data) : data_(data) {}
	std::shared_ptr<DataT> getParam() override { return data_; }
private:
	dataType data_;
};

#endif