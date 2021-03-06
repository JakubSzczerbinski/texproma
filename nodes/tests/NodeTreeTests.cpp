#include <boost/test/unit_test.hpp>
#include "NodeTree.hpp"
#include "tpmWrapper.hpp"

BOOST_AUTO_TEST_CASE(calculateTreeWithOneNode) {
  NodeTree tree;
  auto node = tree.addNode(new tpmWrapper::TpmSine());
  auto node2 = tree.addNode(makeInput(10.0f));
  tree.linkNodes(node, 0, node2, 0);
  auto result = tree.getResult(node, 0);
  auto test = result != nullptr;
  BOOST_TEST(test);
}

BOOST_AUTO_TEST_CASE(calculateTwiceTreeWithOneNode) {
  NodeTree tree;
  auto node = tree.addNode(new tpmWrapper::TpmSine());
  auto node2 = tree.addNode(makeInput(10.0f));
  tree.linkNodes(node, 0, node2, 0);
  auto result = tree.getResult(node, 0);
  auto result2 = tree.getResult(node, 0);
  bool test = result != nullptr;
  bool test2 = result2 != nullptr;
  BOOST_TEST(test);
  BOOST_TEST(test2);
}

BOOST_AUTO_TEST_CASE(calculateInvalidTreeResultsWithExcept)
{
	NodeTree tree;
  auto node = tree.addNode(new tpmWrapper::TpmSine());
  auto node2 = tree.addNode(makeInput(10.0f));
  tree.linkNodes(node2, 0, node, 0);
  bool caughtExcept = false;
  try{
  	tree.getResult(node, 0);	
  }
  catch (std::runtime_error err)
  {
  	caughtExcept = true;
  }
  BOOST_TEST(caughtExcept);
}


BOOST_AUTO_TEST_CASE(calculateBigTreeResults)
{
  NodeTree tree;
  auto sineNode = tree.addNode(new tpmWrapper::TpmSine());
  auto floatInputNode = tree.addNode(makeInput(10.0f));
  auto revertNode = tree.addNode(new tpmWrapper::TpmInvert());
  auto addNode = tree.addNode(new tpmWrapper::TpmAdd());

  tree.linkNodes(sineNode, 0, floatInputNode, 0);
  tree.linkNodes(revertNode, 0, sineNode, 0);
  tree.linkNodes(addNode, 0, sineNode, 0);
  tree.linkNodes(addNode, 1, revertNode, 0);

  auto result = tree.getResult(addNode, 0);

  auto test = result != nullptr;
  BOOST_TEST(test);
}
