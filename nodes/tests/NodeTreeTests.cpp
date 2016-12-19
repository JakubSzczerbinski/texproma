#include <boost/test/unit_test.hpp>
#include "NodeTree.hpp"
#include "tpmWrapper.hpp"

BOOST_AUTO_TEST_CASE(calculateTreeWithOneNode) {
  NodeTree tree;
  auto node = tree.addNode(new tpmWrapper::TpmSine());
  auto node2 = tree.addNode(makeInput(10.0f));
  tree.linkNodes(node, 0, node2, 0);
  auto result = tree.getResult(node, 0);
  auto xd = extractRawData<tpmWrapper::ColorBuf>(result);
  BOOST_TEST(true);
}

// BOOST_AUTO_TEST_CASE(WRITE_TESTS)
// {
// 	BOOST_TEST(false);
// }

// BOOST_AUTO_TEST_CASE(WRITE_TESTS)
// {
// 	BOOST_TEST(false);
// }

// Logger log{"DEBUG"};

// BOOST_AUTO_TEST_CASE(TreeWithOneNodeIsCalculatedProperly)
// {
// 	log << "0";
// 	NodeTree nodeTree;
// 	log << "1/4";
// 	auto node = nodeTree.makeNode(new tpmWrapper::TpmSine());
// 	log << "1/2";
// 	nodeTree.setConstantInput(node, 0, 10.0f);
// 	log << "3/4";
// 	bool test1 = nodeTree.calculateReturn(node, 0) != nullptr;
// 	log << "7/8";
// 	BOOST_TEST(test1);
// 	log << "1";
// }