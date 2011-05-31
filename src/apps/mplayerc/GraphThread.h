#pragma once

class CMainFrame;

class CGraphThread : public CWinThread
{
  CMainFrame* m_pMainFrame;

  DECLARE_DYNCREATE(CGraphThread);

public:
  CGraphThread() : m_pMainFrame(NULL) {}

  void SetMainFrame(CMainFrame* pMainFrame) {m_pMainFrame = pMainFrame;}

  BOOL InitInstance();
  int ExitInstance();

  enum {TM_EXIT=WM_APP, TM_OPEN, TM_CLOSE};
  DECLARE_MESSAGE_MAP()
  afx_msg void OnExit(WPARAM wParam, LPARAM lParam);
  afx_msg void OnOpen(WPARAM wParam, LPARAM lParam);
  afx_msg void OnClose(WPARAM wParam, LPARAM lParam);
};