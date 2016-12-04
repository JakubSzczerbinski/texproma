#define BOOST_TEST_MODULE NODES_TESTS
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <tpmWrapper.hpp>

BOOST_AUTO_TEST_CASE(TpmSineCallTest) {
  tpm_mono_buf cbuf = (tpm_mono_buf)malloc(TP_WIDTH * TP_HEIGHT);
  tpm_sine(cbuf, 10.0);

  Values params;
  params.push_front(makeSharedData(10.0f));

  tpmWrapper::TpmSine sine;
  Values returns = sine(params);
  auto bufObj = extractRawData<tpmWrapper::MonoBuf>(returns[0]);
  auto rawBuf = bufObj.getRawPtr();

  for (size_t i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
    BOOST_TEST(cbuf[i] == rawBuf[i]);
  }
}

BOOST_AUTO_TEST_CASE(TpmSineReturnsTest) {
  std::unique_ptr<Function> f(new tpmWrapper::TpmSine());
  auto returns = f->getReturnTypes();
  BOOST_TEST(returns.size() == 1);
  bool isMonoBufReturn = (returns[0] == typeid(tpmWrapper::MonoBuf));
  BOOST_TEST(isMonoBufReturn);
}

BOOST_AUTO_TEST_CASE(TpmSineParamsTest) {
  std::unique_ptr<Function> f(new tpmWrapper::TpmSine());
  auto params = f->getParamTypes();
  BOOST_TEST(params.size() == 1);
  bool isFloatParam = (params[0] == typeid(float));
  BOOST_TEST(isFloatParam);
}