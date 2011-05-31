#include "stdafx.h"
#include "GraphThread.h"

#include "MainFrm.h"

//
// CGraphThread
//

IMPLEMENT_DYNCREATE(CGraphThread, CWinThread)

BOOL CGraphThread::InitInstance()
{
  AfxSocketInit();

  return SUCCEEDED(CoInitialize(0)) ? TRUE : FALSE;
}

int CGraphThread::ExitInstance()
{
  CoUninitialize();
  return __super::ExitInstance();
}

BEGIN_MESSAGE_MAP(CGraphThread, CWinThread)
  ON_THREAD_MESSAGE(TM_EXIT, OnExit)
  ON_THREAD_MESSAGE(TM_OPEN, OnOpen)
  ON_THREAD_MESSAGE(TM_CLOSE, OnClose)
END_MESSAGE_MAP()

void CGraphThread::OnExit(WPARAM wParam, LPARAM lParam)
{
  if(m_pMainFrame){
    m_pMainFrame->OnPlayPause();
    m_pMainFrame->CloseMediaPrivate();
    m_pMainFrame->ShowWindow(SW_HIDE);
  }
  if(CAMEvent* e = (CAMEvent*)lParam) e->Set();
}

void CGraphThread::OnOpen(WPARAM wParam, LPARAM lParam)
{
  if(m_pMainFrame)
  {
    CAutoPtr<OpenMediaData> pOMD((OpenMediaData*)lParam);
    m_pMainFrame->OpenMediaPrivate(pOMD);
  }
}

void CGraphThread::OnClose(WPARAM wParam, LPARAM lParam)
{
  if(m_pMainFrame){
    m_pMainFrame->OnPlayPause();
    m_pMainFrame->CloseMediaPrivate();
  }
  if(CAMEvent* e = (CAMEvent*)lParam) e->Set();

}

