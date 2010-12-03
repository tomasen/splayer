// ThreadHelper_UnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <process.h>
#include <threadhelper.h>

#define THREAD_NUM 50

class TestThreadHelper : public ThreadHelperImpl<TestThreadHelper>
{
public:
  static int num;
  void _Thread()
  {
    num++;
    int i = 0;
    printf("[%d]TestThreadHelper thread start\n", num);
    while(i<5)
    {
      if (_Exit_state(300))
        break;
      i++;
      Sleep(300);
    }
    printf("[%d]TestThreadHelper thread end\n", num);
  }
};

int TestThreadHelper::num = 0;

int _tmain(int argc, _TCHAR* argv[])
{
  TestThreadHelper mythread;

  for (int i=1;i<=THREAD_NUM;i++)
  {
    mythread._Stop();
    mythread.num++;
    mythread._Start();
  }

  while (mythread._Is_alive())
  {
    Sleep(1000);
  }
  printf("over\n");
  scanf_s("%d");
	return 0;
}

