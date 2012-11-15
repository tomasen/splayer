//
// HEADER NAME.
//      test_suite.h : Defines a basic test suite framework for running gtest based tests.
//
// FUNCTIONAL DESCRIPTION.
//    Defines a basic test suite framework for running gtest based tests. You can
//    instantiate this class in your main function and call its Run method to run
//    any gtest based tests that are linked into your executable.
//
//

#ifndef BASE_TEST_SUITE_H_
#define BASE_TEST_SUITE_H_

#include "gtest/gtest.h"
#include "common.h"

//
// Defines a basic test suite framework for running gtest based tests.  You can
// instantiate this class in your main function and call its Run method to run
// any gtest based tests that are linked into your executable.
//

class TestSuite {
public:
  TestSuite(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
  }

  virtual ~TestSuite() {
  }

  int Run() {
    Initialize();
    int result = RUN_ALL_TESTS();
    Shutdown();
    return result;
  }

protected:

  //
  // Override these for custom initialization and shutdown handling.  Use these
  // instead of putting complex code in your constructor/destructor.
  //

  virtual void Initialize() {

  }

  virtual void Shutdown() {
  }

};

#endif  // BASE_TEST_SUITE_H_
