// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Choose Color dialog

class _AFX_COLOR_STATE : public CNoTrackObject
{
public:
	// custom colors are held here and saved between calls
	COLORREF m_crSavedCustom[16];

	_AFX_COLOR_STATE();
};

_AFX_COLOR_STATE::_AFX_COLOR_STATE()
{
	// custom colors are initialized to white
	for (int i = 0; i < _countof(m_crSavedCustom); i++)
		m_crSavedCustom[i] = RGB(255, 255, 255);
}

EXTERN_PROCESS_LOCAL(_AFX_COLOR_STATE, _afxClrState)

CColorDialog::CColorDialog(COLORREF clrInit, DWORD dwFlags,
	CWnd* pParentWnd) : CCommonDialog(pParentWnd)
{
	memset(&m_cc, 0, sizeof(m_cc));
	m_nIDHelp = AFX_IDD_COLOR;

	m_cc.lStructSize = sizeof(m_cc);
	m_cc.lpCustColors = GetSavedCustomColors();
	m_cc.Flags = dwFlags | CC_ENABLEHOOK;
	m_cc.lpfnHook = (COMMDLGPROC)_AfxCommDlgProc;

	if ((m_cc.rgbResult = clrInit) != 0)
		m_cc.Flags |= CC_RGBINIT;
}

INT_PTR CColorDialog::DoModal()
{
	ASSERT_VALID(this);
	ASSERT(m_cc.Flags & CC_ENABLEHOOK);
	ASSERT(m_cc.lpfnHook != NULL); // can still be a user hook

	m_cc.hwndOwner = PreModal();
	INT_PTR nResult = ::AfxCtxChooseColor(&m_cc);
	PostModal();
	return nResult ? nResult : IDCANCEL;
}

BOOL CColorDialog::OnColorOK()
{
	ASSERT_VALID(this);
	// Do not call Default() if you override
	return FALSE;
}

void CColorDialog::SetCurrentColor(COLORREF clr)
{
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);

	SendMessage(_afxMsgSETRGB, 0, (DWORD)clr);
}

COLORREF* PASCAL CColorDialog::GetSavedCustomColors()
{
	return &_afxClrState->m_crSavedCustom[0];
}

////////////////////////////////////////////////////////////////////////////
// CColorDialog diagnostics

#ifdef _DEBUG
void CColorDialog::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);

	dc << "m_cc.hwndOwner = " << m_cc.hwndOwner;
	dc << "\nm_cc.rgbResult = ";
	dc.DumpAsHex(m_cc.rgbResult);
	dc << "\nm_cc.Flags = ";
	dc.DumpAsHex(m_cc.Flags);
	dc << "\nm_cc.lpCustColors ";

	for (int iClr = 0; iClr < 16; iClr++)
	{
		dc << "\n\t";
		dc.DumpAsHex(m_cc.lpCustColors[iClr]);
	}

	if (m_cc.lpfnHook == (COMMDLGPROC)_AfxCommDlgProc)
		dc << "\nhook function set to standard MFC hook function";
	else
		dc << "\nhook function set to non-standard hook function";

	dc << "\n";
}
#endif //_DEBUG


IMPLEMENT_DYNAMIC(CColorDialog, CCommonDialog)

#pragma warning(disable: 4074)
#pragma init_seg(lib)

PROCESS_LOCAL(_AFX_COLOR_STATE, _afxClrState)

////////////////////////////////////////////////////////////////////////////
