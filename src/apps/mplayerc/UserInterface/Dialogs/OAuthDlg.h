#pragma once

#include "DhtmlDlgBase.h"

class CircleBtn : public CWnd
{
	DECLARE_DYNAMIC(CircleBtn)
public:
	CircleBtn();
	~CircleBtn();

	void SetCircleWnd();
	void OnPaint();

	CBitmap m_over;
	CBitmap m_out;

protected:
	virtual LRESULT OnMouseLeave(WPARAM, LPARAM);
	virtual LRESULT OnMouseMove(WPARAM, LPARAM);

	void OnSize(UINT nType, int cx, int cy);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
private:
	BOOL m_trackleave;
};


class OAuthDlg : public DhtmlDlgBase
{
  DECLARE_DYNAMIC(OAuthDlg)

public:
  OAuthDlg();
  virtual ~OAuthDlg();

  void CalcOauthPos(BOOL display = TRUE);
  void OnSize(UINT nType, int cx, int cy);
  void SetUrl(std::wstring url);

  void HideFrame();

  BSTR CallSPlayer(LPCTSTR p, LPCTSTR param);
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  CRect m_currect;
  CircleBtn m_btnclose;
protected:
  virtual BOOL OnInitDialog();
  STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);

  DECLARE_MESSAGE_MAP()
  DECLARE_DHTML_EVENT_MAP()
  DECLARE_DISPATCH_MAP()

private:
  CRgn m_rgn;
};
