#pragma once

#include "SUIButton.h"
// MOSDWnd

class CMOSDWnd : public CWnd
{
	DECLARE_DYNAMIC(CMOSDWnd)
	enum {IDT_HIDE = 32};
public:
	CMOSDWnd();
	virtual ~CMOSDWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	CAtlList <CString> m_msgList;
	void SendOSDMsg(CString szMsg, UINT timeOut = 2500);
	virtual BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPVOID lpParam = NULL);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CSize mSize ;
	CFont m_statft;
	CString m_osdStr;
	void CountSize();
	CWnd* m_wndView;
	afx_msg void OnPaint();
};


