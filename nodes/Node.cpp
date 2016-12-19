#include <Node.hpp>

Values Node::operator()(Values params) {
  ParamMatcher matcher(*function.get());
  if (!matcher.match(params)) {
    log << "Unable to match arguments. Aborting.";
    throw std::runtime_error("Unable to match arguments. Aborting.");
  }
  auto values = (*function)(params);
  return values;
}