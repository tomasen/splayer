#ifndef MAINFRM_H
#define MAINFRM_H
#pragma once

#include "UserInterface/wtl_aero.h"
#include "UserInterface/generic_app_frm.h"
#include "UserInterface/splayer_frm_styler.h"
#include "UserInterface/splayer_widget_host.h"

class MainFrame:
  public GenericAppFrame<MainFrame>,
  public SPlayerFrameStyler<MainFrame>,
  public SPlayerWidgetHost<MainFrame>,
  public CUpdateUI<MainFrame>,
  public CMessageFilter,
  public CIdleHandler
{
public:
  DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

  MainFrame(void);

  virtual BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnIdle();

  BEGIN_UPDATE_UI_MAP(MainFrame)
  END_UPDATE_UI_MAP()

  BEGIN_MSG_MAP(MainFrame)
    // note: no messages should be handled before these CHAIN_MSG_MAPs
    CHAIN_MSG_MAP(GenericAppFrame<MainFrame>)
    CHAIN_MSG_MAP(SPlayerFrameStyler<MainFrame>)
    CHAIN_MSG_MAP(SPlayerWidgetHost<MainFrame>)

    // frame messages goes here and below
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

    // respond to aero/theme changes
    MESSAGE_HANDLER(WM_THEMECHANGED, OnThemeChanged)
    MESSAGE_HANDLER(WM_DWMCOMPOSITIONCHANGED, OnThemeChanged)

    // special message to show hide controls
    MESSAGE_HANDLER(UWM_FADEIN_CONTROLS, OnFadeInControls)
    MESSAGE_HANDLER(UWM_FADEOUT_CONTROLS, OnFadeOutControls)

    COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
    COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
    COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
    CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
  END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

  // GenericAppFrame<T>::PaintFrame(HDC hdc, LPRECT rcPaint)
  // must be implemented in derived client to support painting of
  // the window frame
  virtual void PaintFrame(HDC hdc, LPRECT rcPaint);

  LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
  
  LRESULT OnThemeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  LRESULT OnFadeInControls(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnFadeOutControls(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
  void InitFrameStyle();
};

#endif // MAINFRM_H