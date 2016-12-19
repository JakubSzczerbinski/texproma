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

BOOST_AUTO_TEST_CASE(CHECK_TESTS) {
  BOOST_TEST(false);
}

// BOOST_AUTO_TEST_CASE(CallingFunctionWithNodesAndParamLinks) {
//   auto valueParamLink =
//       std::shared_ptr<ParamLink>(new DataParamLink(makeSharedData(10.0f)));
//   ParamLinkContainer v = {valueParamLink};
//   auto node = std::shared_ptr<Node>(new Node(new tpmWrapper::TpmSine(), v));
//   auto nodeLink = std::unique_ptr<ParamLink>(new FunctionParamLink(node, 0));
//   bool test1 = nodeLink->getParam() != nullptr;
//   BOOST_TEST(test1);
// }

// BOOST_AUTO_TEST_CASE(CallingFunctionWithNodesAndInvalidOutputParamLinks) {
//   auto valueParamLink =
//       std::shared_ptr<ParamLink>(new DataParamLink(makeSharedData(100.0f)));
//   ParamLinkContainer v = {valueParamLink};
//   auto node = std::shared_ptr<Node>(new Node(new tpmWrapper::TpmSine(), v));
//   auto nodeLink = std::unique_ptr<ParamLink>(new FunctionParamLink(node, 1));
//   bool test1 = nodeLink->getParam() == nullptr;
//   BOOST_TEST(test1);
// }

// BOOST_AUTO_TEST_CASE(CallingFunctionWithNodesAndInvalidInputParamLinks) {
//   auto valueParamLink =
//       std::shared_ptr<ParamLink>(new DataParamLink(makeSharedData(100.0)));
//   ParamLinkContainer v = {valueParamLink};
//   auto node = std::shared_ptr<Node>(new Node(new tpmWrapper::TpmSine(), v));
//   auto nodeLink = std::unique_ptr<ParamLink>(new FunctionParamLink(node, 0));
//   bool caughtError = 0;
//   try {
//     nodeLink->getParam();
//   } catch (std::runtime_error err) {
//     caughtError = 1;
//   }
//   BOOST_TEST(caughtError);
// }

// BOOST_AUTO_TEST_CASE(CallingFunctionWithNodesAndInvalidEmptyParamLinks) {
//   auto node = std::shared_ptr<Node>(new Node(new tpmWrapper::TpmSine()));
//   auto nodeLink = std::unique_ptr<ParamLink>(new FunctionParamLink(node, 0));
//   bool caughtError = 0;
//   try {
//     nodeLink->getParam();
//   } catch (std::runtime_error err) {
//     caughtError = 1;
//   }
//   BOOST_TEST(caughtError);
// }

// BOOST_AUTO_TEST_CASE(CallingFunctionWithNodesAndAddedValidInputParamLinks) {
//   auto valueParamLink =
//       std::shared_ptr<ParamLink>(new DataParamLink(makeSharedData(100.0f)));
//   auto node = std::shared_ptr<Node>(new Node(new tpmWrapper::TpmSine()));
//   node->setParamLink(valueParamLink, 0);
//   auto nodeLink = std::unique_ptr<ParamLink>(new FunctionParamLink(node, 0));
//   bool test1 = nullptr != nodeLink->getParam();
//   BOOST_TEST(test1);
// }

// TODO write tests for Node