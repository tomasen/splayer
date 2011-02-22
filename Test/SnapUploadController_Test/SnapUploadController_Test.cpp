// SnapUploadController_Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../src/apps/mplayerc/Controller/SnapUploadController.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
  SnapUploadController con;
  cout << "SnapUploadController test case: start at any time (auto thread joining)" << endl;
  cout << "  pre-sleeping ..." << endl;
  ::Sleep(1000);

  int total_passes = 10;
  int sleep_period = 6000;
  for (int i = 0; i < total_passes; i++)
  {
    cout << "  calling SnapUploadController::Start (pass " << (i+1) << " of " << (total_passes) << ") ..." << endl;
    con.Start(NULL, 0, 0);
    cout << "  sleep " << sleep_period << "ms ..." << endl;
    ::Sleep(sleep_period);
  }

  cout << "End of test" << endl;
	return 0;
}

