#include <deque>
#include <memory>
#include <typeindex>
#include <Data.hpp>

#ifndef NODES_FUNCTION_HPP
#define NODES_FUNCTION_HPP

typedef std::deque<std::shared_ptr<DataT>> Values;
typedef std::deque<std::type_index> Types;

class Function {
 public:
  virtual Types getParamTypes() const = 0;
  virtual Types getReturnTypes() const = 0;
  virtual std::string getName() const = 0;
  virtual Values operator()(Values params) const = 0;
};

#endif