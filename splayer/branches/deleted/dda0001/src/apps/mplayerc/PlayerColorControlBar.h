#pragma once

#include "SVPSliderCtrl.h"
#include "SVPButton.h"
#include "SVPStatic.h"
#include "SVPDialog.h"

// CPlayerColorControlBar

class CPlayerColorControlBar : public CSVPDialog
{
	DECLARE_DYNAMIC(CPlayerColorControlBar)

public:
	CPlayerColorControlBar();
	virtual ~CPlayerColorControlBar();



	
	void Relayout();
	
	// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	CSVPStatic csBrightLabel;
	CSVPStatic csConstLabel;

	CSVPSliderCtrl csl_bright;
	CSVPSliderCtrl csl_const;

	CSVPButton  cb_reset;
	//CSVPButton  cb_enablectrl;

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
	
	//afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnColorControlButtonEnable();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

};


