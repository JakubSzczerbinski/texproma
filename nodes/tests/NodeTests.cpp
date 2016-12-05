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

//TODO write tests for Node