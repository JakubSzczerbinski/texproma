#ifndef NODES_DATA_HPP
#define NODES_DATA_HPP

#include <typeindex>
#include <stdexcept>
#include <string>
#include <memory>

class DataT {
 public:
  template <typename dataType>
  bool isType() const {
    if (typeId() == typeid(dataType))
      return true;
    else
      return false;
  }
  virtual std::type_index typeId() const = 0;
  virtual DataT* copy() const = 0;
  virtual ~DataT(){};
};

template <typename dataType>
class Data : public DataT {
 public:
  Data(const dataType& data) : data_(data) {}
  dataType& getRawData() { return data_; }
  std::type_index typeId() const override { return typeid(dataType); }
  DataT* copy() const override { return new Data<dataType>(data_); }
  ~Data() override{};

 private:
  dataType data_;
};

template <typename dataType>
dataType extractRawData(std::shared_ptr<DataT> srcData) {
  auto castedData = std::dynamic_pointer_cast<Data<dataType>>(srcData);
  if (castedData == nullptr) {
    auto error = std::string("Unable to cast ") +
                 std::string(typeid(dataType).name()) + " into " +
                 std::string(srcData->typeId().name());
    throw std::runtime_error(error);
  }
  return castedData->getRawData();
}

template <typename dataType>
Data<dataType> makeData(dataType data) {
  return Data<dataType>(data);
}

template <typename dataType>
std::shared_ptr<DataT> makeSharedData(dataType data) {
  return std::shared_ptr<Data<dataType>>(new Data<dataType>(data));
}

bool isSameType(const DataT& a, const DataT& b);

#endif