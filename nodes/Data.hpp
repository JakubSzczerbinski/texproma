#include <typeinfo>

class DataT {
 public:
  template <typename dataType>
  bool isType() const {
    if (typeId() == typeid(dataType))
      return true;
    else
      return false;
  }
  virtual const std::type_info& typeId() const = 0;
  virtual DataT* copy() const = 0;
  virtual ~DataT(){};
};

template <typename dataType>
class Data : public DataT {
 public:
  Data(const dataType& data) : data_(data) {}
  dataType& getRawData() { return data_; }
  const std::type_info& typeId() const override { return typeid(dataType); }
  DataT* copy() const override { return new Data<dataType>(data_); }
  ~Data() override{};

 private:
  dataType data_;
};

bool isSameType(const DataT& a, const DataT& b) {
  return a.typeId() == b.typeId();
}