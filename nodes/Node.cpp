#include <Node.hpp>

Values Node::getParamValues() {
  Values params;
  for (auto& paramLink : paramLinks) {
    params.push_back(paramLink->getParam());
  }
  return params;
}

void Node::updateValues() {
  log << "Updating values of " + function->getName();
  Values params = getParamValues();
  ParamMatcher matcher(*function.get());
  if (!matcher.match(params)) {
    log << "Unable to match arguments. Aborting.";
    throw std::runtime_error("Unable to match arguments. Aborting.");
  }
  values = (*function)(params);
}