// MainFrm.cpp : implmentation of the MainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "main_frm.h"
#include "UserInterface/bitmap_svc.h"
#include "Controller/resource_bridge.h"

MainFrame::MainFrame(void)
{
}

BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
  return CFrameWindowImpl<MainFrame>::PreTranslateMessage(pMsg);
}

BOOL MainFrame::OnIdle()
{
  return FALSE;
}

void MainFrame::PaintFrame(HDC hdc, LPRECT rcPaint)
{
  SPlayerFrameStyler::PaintFrame(hdc, rcPaint);
}

LRESULT MainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  // register object for message filtering and idle updates
  CMessageLoop* pLoop = _Module.GetMessageLoop();
  ATLASSERT(pLoop != NULL);
  pLoop->AddMessageFilter(this);
  pLoop->AddIdleHandler(this);

//   ResourceBridge::GetInstance()->SetLocale(L"ja");

  // Note: if you would like to change window text during running, you should call
  // GenericAppFrame<MainFrame>::RedrawFrame after SetWindowText. We used
  // mechanisms to avoid refreshing of window text due to a Windows flaw.
  // For more information, see GenericAppFrame<T>::OnSetText and ScopedRedrawLock
  SetWindowText(ResourceBridge::GetInstance()->LoadString(L"window_title").c_str());

  SPlayerWidgetHost::PrepareWidgets(m_hWnd);

  InitFrameStyle();

  return 0;
}

LRESULT MainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
  // unregister message filtering and idle updates
  CMessageLoop* pLoop = _Module.GetMessageLoop();
  ATLASSERT(pLoop != NULL);
  pLoop->RemoveMessageFilter(this);
  pLoop->RemoveIdleHandler(this);

  bHandled = FALSE;
  return 1;
}

LRESULT MainFrame::OnThemeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  InitFrameStyle();
  return 0;
}

LRESULT MainFrame::OnFadeInControls(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  if (IsAeroFrameEnabled())
  {
    m_hud_widget->Show();
    m_playback_widget->Show();
  }
  return 0;
}

LRESULT MainFrame::OnFadeOutControls(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  if (IsAeroFrameEnabled())
  {
    m_hud_widget->Hide();
    m_playback_widget->Hide();
  }
  return 0;
}

LRESULT MainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  PostMessage(WM_CLOSE);
  return 0;
}

LRESULT MainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  // TODO: add code to initialize document

  switch(GetFrameStyle())
  {
  case -1:
    SetFrameStyle(0);
    SetWorkAsLayered(false);
    break;
  case 0:
    SetFrameStyle(1);
    SetWorkAsLayered(false);
    break;
  case 1:
    SetFrameStyle(2);
    SetWorkAsLayered(false);
    break;
  case 2:
    SetFrameStyle(-1);
    SetWorkAsLayered(true);
    break;
  }

  EnableAeroFrame(GetFrameStyle() == -1?true:false);

  return 0;
}

LRESULT MainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  return 0;
}

void MainFrame::InitFrameStyle()
{
  if (IsCompositionEnabled())
  {
    SetFrameStyle(-1);
    SetWorkAsLayered(true);
    EnableAeroFrame(true);
    return;
  }
  SetFrameStyle(0);
  SetWorkAsLayered(false);
  EnableAeroFrame(false);
  m_hud_widget->Show();
  m_playback_widget->Show();

}
