//
// HEADER NAME.
//      run_all_unittests.h : unittest entry point.
//
// FUNCTIONAL DESCRIPTION.
//    unittest entry point.
//
//

#include "logging.h"

#include "test_suite.h"
#include "common.h"

int main(int argc, char** argv) {
    // ...
    //_CrtSetBreakAlloc(967);
    int i = TestSuite(argc, argv).Run();
    logging::CloseLogFile();
    return i;
}
