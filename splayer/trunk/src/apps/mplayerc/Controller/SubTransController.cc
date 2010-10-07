#include "stdafx.h"
#include "SubTransController.h"
#include <sinet.h>

using namespace sinet;

SubTransController::SubTransController(void):
  m_stopevent(::CreateEvent(NULL, TRUE, FALSE, NULL)),
  m_thread(NULL),
  m_operation(Unknown)
{
  refptr<pool> pool = pool::create_instance();
}

SubTransController::~SubTransController(void)
{
  Stop();
}

void SubTransController::SetFrame(HWND hwnd)
{

}

void SubTransController::Start(const wchar_t* video_filename, 
                               SubTransOperation operation, 
                               StringList files_upload /*= StringList()*/)
{
  // we should stop running tasks first
  Stop();
  // record parameters
  m_operation = operation;
  // create thread
  m_thread = (HANDLE)::_beginthread(_thread_dispatch, 0, (void*)this);
}

void SubTransController::Stop()
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

void SubTransController::_thread_dispatch(void* param)
{
  static_cast<SubTransController*>(param)->_thread();
}

void SubTransController::_thread()
{
  switch (m_operation)
  {
  case DownloadSubtitle:
    _thread_download();
    break;
  case UploadSubtitle:
    _thread_upload();
    break;
  }
}

void SubTransController::_thread_download()
{
  while (true)
  {
    // exiting thread
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return;
    // check download task finish
  }
}

void SubTransController::_thread_upload()
{
  refptr<pool> pool = pool::create_instance();
}