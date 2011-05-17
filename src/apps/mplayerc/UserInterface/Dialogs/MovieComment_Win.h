

#pragma once

#include "DhtmlDlgBase.h"

#define ID_MOVIESHARE_RESPONSE 32932

class MovieComment : public DhtmlDlgBase
{
  DECLARE_DYNAMIC(MovieComment)

public:
  MovieComment();
  virtual ~MovieComment();

  HRESULT OpenNewLink(IHTMLElement *pElement);
  HRESULT OnEventClose(IHTMLElement *pElement);

  HRESULT OnEventCapture(IHTMLElement* pElement);

  void ClearFrame();
  void CalcWndPos();
  void OpenNewLink(LPCTSTR url);

  BSTR CallSPlayer(LPCTSTR p, LPCTSTR param);

  std::wstring GetMovieTime(int i);
  int m_initialize;

  void OnSize(UINT nType, int cx, int cy);
  CRgn m_rgn;

protected:
  virtual BOOL OnInitDialog();

  DECLARE_MESSAGE_MAP()
  DECLARE_DHTML_EVENT_MAP()
  DECLARE_DISPATCH_MAP()

};


