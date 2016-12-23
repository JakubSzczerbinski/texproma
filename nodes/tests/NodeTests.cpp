#include <Node.hpp>
#include <tpmWrapper.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(MatchingParmetersFailingDiffrentParams) {
  auto v = makeSharedData(10u);
  Values values = {v};
  tpmWrapper::TpmSine tpm;
  ParamMatcher matcher(tpm);
  BOOST_TEST(matcher.match(values) == false);
}

BOOST_AUTO_TEST_CASE(MatchingParmetersFailingDiffrentNumberOfParams) {
  auto v = makeSharedData(100u);
  Values values = {v};
  tpmWrapper::TpmNoise tpm;
  ParamMatcher matcher(tpm);
  BOOST_TEST(matcher.match(values) == false);
}

BOOST_AUTO_TEST_CASE(MatchingParmetersPassingOneParam) {
  auto v = makeSharedData(10.0f);
  Values values = {v};
  tpmWrapper::TpmSine tpm;
  ParamMatcher matcher(tpm);
  BOOST_TEST(matcher.match(values) == true);
}

BOOST_AUTO_TEST_CASE(MatchingParmetersPassingOneParamTwoParams) {
  auto v1 = makeSharedData(10u), v2 = makeSharedData(100u);
  Values values = {v1, v2};
  tpmWrapper::TpmNoise tpm;
  ParamMatcher matcher(tpm);
  BOOST_TEST(matcher.match(values) == true);
}

BOOST_AUTO_TEST_CASE(CallNodeWithValidParamsGetValidResult)
{
  Values v = {makeSharedData(10.0f)};
  Node node(new tpmWrapper::TpmSine());
  Values ret = node(v);
  BOOST_TEST(ret.size() == 1);
  BOOST_TEST(ret[0]);
}

BOOST_AUTO_TEST_CASE(CallNodeWithInvalidParamsCatchException) {
  Values v = {makeSharedData(10u)};
  Node node(new tpmWrapper::TpmSine());

  bool caughtExcept = false;
  try{
    node(v);
  }
  catch (std::runtime_error err)
  {
    caughtExcept = true;
  }
  BOOST_TEST(caughtExcept);
}

BOOST_AUTO_TEST_CASE(CallNodeWithTooManyParamsCatchException) {
  Values v = {makeSharedData(10.0f), makeSharedData(10u)};
  Node node(new tpmWrapper::TpmSine());

  bool caughtExcept = false;
  try{
    node(v);
  }
  catch (std::runtime_error err)
  {
    caughtExcept = true;
  }
  BOOST_TEST(caughtExcept);
}

BOOST_AUTO_TEST_CASE(CallNodeWithNotEnoughParamsCatchException) {
  Values v = {};
  Node node(new tpmWrapper::TpmSine());

  bool caughtExcept = false;
  try{
    node(v);
  }
  catch (std::runtime_error err)
  {
    caughtExcept = true;
  }
  BOOST_TEST(caughtExcept);
}

