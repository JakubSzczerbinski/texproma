
class DataT {
 public:
  template <typename dataType>
  bool hasType() {
    if (typeId() == type_info.id(dataType))
      return 1;
  }
  virtual unsigned typeId() const = 0;
  virtual DataT* copy() const = 0;
  virtual ~DataT() = 0;
};

template <typename dataType>
class Data : public DataT {
 public:
  Data(const dataType& data) : data_(data) {}
  dataType& getRawData() { return data_; }
  unsigned typeId() const override;

 private:
  dataType data_;
};