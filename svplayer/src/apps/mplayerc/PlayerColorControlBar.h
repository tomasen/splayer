#pragma once


// CPlayerColorControlBar

class CPlayerColorControlBar : public CDialogBar
{
	DECLARE_DYNAMIC(CPlayerColorControlBar)

public:
	CPlayerColorControlBar();
	virtual ~CPlayerColorControlBar();



	
	void Relayout();
	
	// Overrides
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);


protected:

	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

};


