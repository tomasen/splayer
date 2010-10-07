#include "stdafx.h"
#include "SnapUploadController.h"

SnapUploadController::SnapUploadController(void):
  m_stopevent(::CreateEvent(NULL, TRUE, FALSE, NULL)),
  m_thread(NULL)
{

}

SnapUploadController::~SnapUploadController(void)
{
  Stop();
}

std::wstring SnapUploadController::GetTempDir()
{
  return L"";
}

void SnapUploadController::SetFrame(HWND hwnd)
{
  m_frame = hwnd;
}

void SnapUploadController::Start(const wchar_t* hash_str, 
                                 const unsigned int curr_pos_sec, const unsigned int total_sec)
{
  // we should stop running tasks first
  Stop();
  // record values, and current time
  // create thread
  m_thread = (HANDLE)::_beginthread(_thread_dispatch, 0, (void*)this);
}

void SnapUploadController::Stop()
{
  unsigned long thread_exitcode;
  if (m_thread && m_thread != INVALID_HANDLE_VALUE &&
    GetExitCodeThread(m_thread, &thread_exitcode) &&
    thread_exitcode == STILL_ACTIVE)
  {
    ::SetEvent(m_stopevent);
    ::WaitForSingleObject(m_thread, INFINITE);
  }
  m_thread = NULL;
  ::ResetEvent(m_stopevent);
}

void SnapUploadController::_thread_dispatch(void* param)
{
  static_cast<SnapUploadController*>(param)->_thread();
}

void SnapUploadController::_thread()
{
  printf("0. thread start\n");
  // allow immediate cancel, pause execution for 1 sec
  if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
    return;

  OutputDebugStringA("SnapUploadController::_thread attempt to send WM_COMMAND\n");
  ::SendMessage(m_frame, WM_COMMAND, MAKEWPARAM(5201, 0), 0);
  OutputDebugStringA("SnapUploadController::_thread after WM_COMMAND\n");

  printf("1. step 1\n");
  // step 1. use sinet to retrieve upload requirements
  // declare sinet pool here
  std::vector<unsigned int> time_to_shot;
  while (true)
  {
    printf("2. entering while of step 1\n");
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return;
    // check task finish, if it's finished, parse buffer to |time_to_shot|, 
    // and break while loop
    ::SendMessage(m_frame, WM_COMMAND, MAKEWPARAM(5201, 0), 0);
    time_to_shot.push_back(10);
    time_to_shot.push_back(110);
    printf("3. leaving while of step 1\n");
    break;
  }

  if (time_to_shot.empty())
    return;

  printf("4. step 2\n");
  // step 2. iterate through time_to_shot and calculate 
  //         sleep period before taking a snapshot
  for (std::vector<unsigned int>::iterator it = time_to_shot.begin();
    it != time_to_shot.end(); it++)
  {
    printf("5. entering for of step 2\n");
    ::SendMessage(m_frame, WM_COMMAND, MAKEWPARAM(5201, 0), 0);
    unsigned int sleep_period = 1*1000;
    if (::WaitForSingleObject(m_stopevent, sleep_period) == WAIT_OBJECT_0)
      return;
    // we should have reached the point to take snapshot
    // step 3. send message to main frame to take snapshot
    // wait for 5 seconds
    // step 4. read temporary directory to locate this file, if failed, return
    // step 5. if successful, use sinet to upload (similar to step 1's loop)
    printf("5. leaving for of step 2\n");
  }
}