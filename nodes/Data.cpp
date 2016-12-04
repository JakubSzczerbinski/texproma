#include <Data.hpp>

bool isSameType(const DataT& a, const DataT& b) {
  return a.typeId() == b.typeId();
}