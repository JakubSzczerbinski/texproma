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

template <typename Arg, typename... Args>
struct TypeVector {
  Types getVector() {
    Types result = TypeVector<Args...>().getVector();
    result.push_front(typeid(Arg));
    return result;
  }
};

template <typename Arg>
struct TypeVector<Arg> {
  Types getVector() {
    Types result;
    result.push_front(typeid(Arg));
    return result;
  }
};

template <typename... Args>
Types makeTypeVector() {
  return TypeVector<Args...>().getVector();
}

template <typename T>
class InputFunction : public Function {
  T value_;

 public:
  InputFunction(T value) : value_(value) {}
  virtual Types getParamTypes() const { return {}; }
  virtual Types getReturnTypes() const { return makeTypeVector<T>(); }
  virtual std::string getName() const { return "InputFunction"; }
  virtual Values operator()(Values params) const {
    return {makeSharedData(value_)};
  }
};

template <typename T>
Function* makeInput(T value) {
  Function* f = new InputFunction<T>(value);
  return f;
}

#endif