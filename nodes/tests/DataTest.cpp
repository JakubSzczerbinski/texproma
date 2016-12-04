#define BOOST_TEST_MODULE DataTests
#include <boost/test/included/unit_test.hpp>
#include <memory>
#include <Data.hpp>

BOOST_AUTO_TEST_CASE(EnsureUnderlyingTypeComparisionCorrect) {
  auto intData = std::unique_ptr<DataT>(new Data<int>(1));
  auto intData2 = std::unique_ptr<DataT>(new Data<int>(2));
  auto floatData = std::unique_ptr<DataT>(new Data<float>(1.0));

  BOOST_TEST(floatData->isType<int>() == false);
  BOOST_TEST(floatData->isType<float>() == true);
  BOOST_TEST(intData->isType<int>() == true);
  BOOST_TEST(intData->isType<float>() == false);

  BOOST_TEST(isSameType(*intData, *floatData) == false);
  BOOST_TEST(isSameType(*intData, *intData2) == true);
}

BOOST_AUTO_TEST_CASE(EnsureRawDataValueCorrect) {
  auto intData = std::unique_ptr<Data<int>>(new Data<int>(1));
  BOOST_TEST(intData->getRawData() == 1);
}

BOOST_AUTO_TEST_CASE(EnsureRawDataCopyAndAssginment) {
  auto intData = std::unique_ptr<Data<int>>(new Data<int>(2));
  auto intData2 = std::unique_ptr<Data<int>>(new Data<int>(*intData));
  intData2->getRawData() = 3;
  BOOST_TEST(intData->getRawData() == 2);
  BOOST_TEST(intData2->getRawData() == 3);
}
