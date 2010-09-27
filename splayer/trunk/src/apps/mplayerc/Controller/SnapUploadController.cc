#include "stdafx.h"
#include "SnapUploadController.h"

SnapUploadController::SnapUploadController(void):
  m_stopevent(::CreateEvent(NULL, TRUE, FALSE, NULL)),
  m_thread(NULL)
{

}

SnapUploadController::~SnapUploadController(void)
{
  Stop(true);
}

std::wstring SnapUploadController::GetTempDir()
{
  return L"";
}

void SnapUploadController::SetFrame(HWND hwnd)
{

}

void SnapUploadController::Start(const wchar_t* hash_str, 
                                 const unsigned int curr_pos_sec, const unsigned int total_sec)
{
  // we should stop running tasks first
  Stop(true);
  // record values, and current time
  // create thread
  m_thread = (HANDLE)::_beginthread(_thread_dispatch, 0, (void*)this);
}

void SnapUploadController::Stop(bool synced /*= false*/)
{
  ::SetEvent(m_stopevent);
  if (synced && m_thread && m_thread != INVALID_HANDLE_VALUE)
  {
    ::WaitForSingleObject(m_thread, INFINITE);
    m_thread = NULL;
  }
}

void SnapUploadController::_thread_dispatch(void* param)
{
  static_cast<SnapUploadController*>(param)->_thread();
}

void SnapUploadController::_thread()
{
  // allow immediate cancel, pause execution for 1 sec
  if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
    return;

  // step 1. use sinet to retrieve upload requirements
  // declare sinet pool here
  std::vector<unsigned int> time_to_shot;
  while (true)
  {
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return;
    // check task finish, if it's finished, parse buffer to |time_to_shot|, 
    // and break while loop
  }

  if (time_to_shot.empty())
    return;

  // step 2. iterate through time_to_shot and calculate 
  //         sleep period before taking a snapshot
  for (std::vector<unsigned int>::iterator it = time_to_shot.begin();
    it != time_to_shot.end(); it++)
  {
    unsigned int sleep_period = 1*1000;
    if (::WaitForSingleObject(m_stopevent, sleep_period) == WAIT_OBJECT_0)
      return;
    // we should have reached the point to take snapshot
    // step 3. send message to main frame to take snapshot
    // wait for 5 seconds
    // step 4. read temporary directory to locate this file, if failed, return
    // step 5. if successful, use sinet to upload (similar to step 1's loop)
  }
}