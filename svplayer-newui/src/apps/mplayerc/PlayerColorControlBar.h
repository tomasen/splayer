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

	CStatic csBrightLabel;
	CStatic csConstLabel;

	CSliderCtrl csl_bright;
	CSliderCtrl csl_const;

	CButton  cb_reset;
	CButton  cb_enablectrl;

	CFont m_font;

	void CheckAbility();
private:

	float fDefaultBright;
	float fDefaultConst;
	float fDefaultHue;
	float fDefaultSaturation;

	bool m_bAbleControl;

protected:

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnButtonReset();
	afx_msg void OnButtonEnable();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	
};


