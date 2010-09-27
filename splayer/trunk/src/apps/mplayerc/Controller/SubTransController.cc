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
  Stop(true);
}

void SubTransController::SetFrame(HWND hwnd)
{

}

void SubTransController::Start(const wchar_t* video_filename, 
                               SubTransOperation operation, 
                               StringList files_upload /*= StringList()*/)
{
  // we should stop running tasks first
  Stop(true);
  // record parameters
  m_operation = operation;
  // create thread
  m_thread = (HANDLE)::_beginthread(_thread_dispatch, 0, (void*)this);
}

void SubTransController::Stop(bool synced /*= false*/)
{
  ::SetEvent(m_stopevent);
  if (synced && m_thread && m_thread != INVALID_HANDLE_VALUE)
  {
    ::WaitForSingleObject(m_thread, INFINITE);
    m_thread = NULL;
  }
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
}

void SubTransController::_thread_upload()
{
  refptr<pool> pool = pool::create_instance();
}