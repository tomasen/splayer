

#pragma once

#include "DhtmlDlgBase.h"
#include "OAuthDlg.h"

#define ID_MOVIESHARE_RESPONSE 32932

class MovieComment : public DhtmlDlgBase
{
  DECLARE_DYNAMIC(MovieComment)

public:
  MovieComment();
  virtual ~MovieComment();

  BOOL OnEventNewLink(IDispatch **ppDisp, VARIANT_BOOL *Cancel,
    DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);

  HRESULT OpenNewLink(IHTMLElement *pElement);
  HRESULT OnEventClose(IHTMLElement *pElement);

  HRESULT OnEventCapture(IHTMLElement* pElement);

  void CloseOAuth();
  void OpenOAuth(LPCTSTR str);

  void ShowFrame();
  void ClearFrame();
  void CalcWndPos();
  void OpenNewLink(LPCTSTR url);

  BSTR CallSPlayer(LPCTSTR p, LPCTSTR param);

  std::wstring GetMovieTime(int i);
  int m_initialize;

  BOOL AdjustMainWnd();
  void OnSize(UINT nType, int cx, int cy);

  OAuthDlg* m_oadlg;

protected:
  virtual BOOL OnInitDialog();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);

  DECLARE_MESSAGE_MAP()
  DECLARE_DHTML_EVENT_MAP()
  DECLARE_EVENTSINK_MAP()
  DECLARE_DISPATCH_MAP()

private:
  void RenderAni();

  std::wstring m_oauthurl;
  RECT m_mainrc;
  CRgn m_rgn;

  // animation
  DWORD m_st;
  DWORD m_detalt;
  DWORD m_ut;

  float m_dt;
  float m_offset;
  int m_oauthHeight;
};


