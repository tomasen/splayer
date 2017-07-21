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
#include "occimpl.h"
#include "sal.h"

/////////////////////////////////////////////////////////////////////////////
// CDataExchange member functions (contructor is in wincore.cpp for swap tuning)

HWND CDataExchange::PrepareEditCtrl(int nIDC)
{
	HWND hWndCtrl = PrepareCtrl(nIDC);
	m_bEditLastControl = TRUE;

   return hWndCtrl;
}

HWND CDataExchange::PrepareCtrl(int nIDC)
{
	ASSERT(nIDC != 0);
	ASSERT(nIDC != -1); // not allowed
	HWND hWndCtrl;
   COleControlSite* pSite = NULL;
	m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (hWndCtrl == NULL)
	{
	  // Could be a windowless OCX
	  pSite = m_pDlgWnd->GetOleControlSite(nIDC);
	  if (pSite == NULL)
	  {
		   TRACE(traceAppMsg, 0, "Error: no data exchange control with ID 0x%04X.\n", nIDC);
		   ASSERT(FALSE);
		   AfxThrowNotSupportedException();
	  }
	}
	m_idLastControl = nIDC;
	m_bEditLastControl = FALSE; // not an edit item by default

   return hWndCtrl;
}

void CDataExchange::Fail()
{
	if (!m_bSaveAndValidate)
	{
		TRACE(traceAppMsg, 0, "Warning: CDataExchange::Fail called when not validating.\n");
		// throw the exception anyway
	}
	else if (m_idLastControl != NULL)
	{
		// restore focus and selection to offending field
	  HWND hWndLastControl;
	  m_pDlgWnd->GetDlgItem(m_idLastControl, &hWndLastControl);
	  if (hWndLastControl != NULL)
	  {
		   ::SetFocus(hWndLastControl);
		   if (m_bEditLastControl) // select edit item
			   ::SendMessage(hWndLastControl, EM_SETSEL, 0, -1);
	  }
	}
	else
	{
		TRACE(traceAppMsg, 0, "Error: fail validation with no control to restore focus to.\n");
		// do nothing more
	}

	AfxThrowUserException();
}

/////////////////////////////////////////////////////////////////////////////
// Notes for implementing dialog data exchange and validation procs:
//  * always start with PrepareCtrl or PrepareEditCtrl
//  * always start with 'pDX->m_bSaveAndValidate' check
//  * pDX->Fail() will throw an exception - so be prepared
//  * avoid creating temporary HWNDs for dialog controls - i.e.
//      use HWNDs for child elements
//  * validation procs should only act if 'm_bSaveAndValidate'
//  * use the suffices:
//      DDX_ = exchange proc
//      DDV_ = validation proc
//
/////////////////////////////////////////////////////////////////////////////

AFX_STATIC void AFX_CDECL _Afx_DDX_TextWithFormat(CDataExchange* pDX, int nIDC,
	LPCTSTR lpszFormat, UINT nIDPrompt, ...)
	// only supports windows output formats - no floating point
{
	va_list pData;
	va_start(pData, nIDPrompt);

	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
   ASSERT( hWndCtrl != NULL );

	const int SZT_SIZE = 64;
	TCHAR szT[SZT_SIZE];
	if (pDX->m_bSaveAndValidate)
	{
		void* pResult;

		pResult = va_arg( pData, void* );
		// the following works for %d, %u, %ld, %lu
		::GetWindowText(hWndCtrl, szT, _countof(szT));
		if (_sntscanf_s(szT, _countof(szT), lpszFormat, pResult) != 1)
		{
			AfxMessageBox(nIDPrompt);
			pDX->Fail();        // throws exception
		}
	}
	else
	{
		
		ATL_CRT_ERRORCHECK_SPRINTF(_vsntprintf_s(szT, _countof(szT), _countof(szT) - 1, lpszFormat, pData));
			// does not support floating point numbers - see dlgfloat.cpp
		AfxSetWindowText(hWndCtrl, szT);
	}

	va_end(pData);
}

/////////////////////////////////////////////////////////////////////////////
// Simple formatting to text item

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, BYTE& value)
{
	int n = (int)value;
	if (pDX->m_bSaveAndValidate)
	{
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%u"), AFX_IDP_PARSE_BYTE, &n);
		if (n > 255)
		{
			AfxMessageBox(AFX_IDP_PARSE_BYTE);
			pDX->Fail();        // throws exception
		}
		value = (BYTE)n;
	}
	else
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%u"), AFX_IDP_PARSE_BYTE, n);
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, short& value)
{
	if (pDX->m_bSaveAndValidate)
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%hd"), AFX_IDP_PARSE_INT, &value);
	else
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%hd"), AFX_IDP_PARSE_INT, value);
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, int& value)
{
	if (pDX->m_bSaveAndValidate)
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%d"), AFX_IDP_PARSE_INT, &value);
	else
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%d"), AFX_IDP_PARSE_INT, value);
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, UINT& value)
{
	if (pDX->m_bSaveAndValidate)
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%u"), AFX_IDP_PARSE_UINT, &value);
	else
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%u"), AFX_IDP_PARSE_UINT, value);
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, long& value)
{
	if (pDX->m_bSaveAndValidate)
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%ld"), AFX_IDP_PARSE_INT, &value);
	else
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%ld"), AFX_IDP_PARSE_INT, value);
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, DWORD& value)
{
	if (pDX->m_bSaveAndValidate)
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%lu"), AFX_IDP_PARSE_UINT, &value);
	else
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%lu"), AFX_IDP_PARSE_UINT, value);
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, LONGLONG& value)
{
	if (pDX->m_bSaveAndValidate)
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%I64d"), AFX_IDP_PARSE_INT, &value);
	else
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%I64d"), AFX_IDP_PARSE_INT, value);
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, ULONGLONG& value)
{
	if (pDX->m_bSaveAndValidate)
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%I64u"), AFX_IDP_PARSE_UINT, &value);
	else
		_Afx_DDX_TextWithFormat(pDX, nIDC, _T("%I64u"), AFX_IDP_PARSE_UINT, value);
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, CString& value)
{
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
   if (pDX->m_bSaveAndValidate)
	{
		int nLen = ::GetWindowTextLength(hWndCtrl);
		::GetWindowText(hWndCtrl, value.GetBufferSetLength(nLen), nLen+1);
		value.ReleaseBuffer();
	}
	else
	{
		AfxSetWindowText(hWndCtrl, value);
	}
}

void AFXAPI DDX_Text(_Inout_ CDataExchange* pDX, _In_ int nIDC, _Out_z_cap_(nMaxLen) LPTSTR value, _In_ int nMaxLen)
{
	ASSERT(nMaxLen != 0);

	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		int nLen = ::GetWindowTextLength(hWndCtrl);
		int nRetrieved = ::GetWindowText(hWndCtrl, value, nMaxLen);
		if (nLen > nRetrieved)
			TRACE(traceAppMsg, 0, "Text in control ID %d is too long. Call DDV_MaxChars()!\n", nIDC);
	}
	else
	{
		AfxSetWindowText(hWndCtrl, value);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Data exchange for special control

void AFXAPI DDX_Check(CDataExchange* pDX, int nIDC, int& value)
{
	pDX->PrepareCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		value = (int)::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);
		ASSERT(value >= 0 && value <= 2);
	}
	else
	{
		if (value < 0 || value > 2)
		{
			TRACE(traceAppMsg, 0, "Warning: dialog data checkbox value (%d) out of range.\n",
				 value);
			value = 0;  // default to off
		}
		::SendMessage(hWndCtrl, BM_SETCHECK, (WPARAM)value, 0L);
	}
}

void AFXAPI DDX_Radio(CDataExchange* pDX, int nIDC, int& value)
	// must be first in a group of auto radio buttons
{
	pDX->PrepareCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);

	ASSERT(::GetWindowLong(hWndCtrl, GWL_STYLE) & WS_GROUP);
	ASSERT(::SendMessage(hWndCtrl, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON);

	if (pDX->m_bSaveAndValidate)
		value = -1;     // value if none found

	// walk all children in group
	int iButton = 0;
	do
	{
		if (::SendMessage(hWndCtrl, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON)
		{
			// control in group is a radio button
			if (pDX->m_bSaveAndValidate)
			{
				if (::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L) != 0)
				{
					ASSERT(value == -1);    // only set once
					value = iButton;
				}
			}
			else
			{
				// select button
				::SendMessage(hWndCtrl, BM_SETCHECK, (iButton == value), 0L);
			}
			iButton++;
		}
		else
		{
			TRACE(traceAppMsg, 0, "Warning: skipping non-radio button in group.\n");
		}
		hWndCtrl = ::GetWindow(hWndCtrl, GW_HWNDNEXT);

	} while (hWndCtrl != NULL &&
		!(GetWindowLong(hWndCtrl, GWL_STYLE) & WS_GROUP));
}

/////////////////////////////////////////////////////////////////////////////
// Listboxes, comboboxes

void AFXAPI DDX_LBString(CDataExchange* pDX, int nIDC, CString& value)
{
	pDX->PrepareCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		int nIndex = (int)::SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);
		if (nIndex != -1)
		{
			int nLen = (int)::SendMessage(hWndCtrl, LB_GETTEXTLEN, nIndex, 0L);
			::SendMessage(hWndCtrl, LB_GETTEXT, nIndex,
					(LPARAM)(LPVOID)value.GetBufferSetLength(nLen));
		}
		else
		{
			// no selection
			value.Empty();
		}
		value.ReleaseBuffer();
	}
	else
	{
		// set current selection based on data string
		if (::SendMessage(hWndCtrl, LB_SELECTSTRING, (WPARAM)-1,
		  (LPARAM)(LPCTSTR)value) == LB_ERR)
		{
			// no selection match
			TRACE(traceAppMsg, 0, "Warning: no listbox item selected.\n");
		}
	}
}

void AFXAPI DDX_LBStringExact(CDataExchange* pDX, int nIDC, CString& value)
{
	pDX->PrepareCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		DDX_LBString(pDX, nIDC, value);
	}
	else
	{
		// set current selection based on data string
		int i = (int)::SendMessage(hWndCtrl, LB_FINDSTRINGEXACT, (WPARAM)-1,
		  (LPARAM)(LPCTSTR)value);
		if (i < 0)
		{
			// no selection match
			TRACE(traceAppMsg, 0, "Warning: no listbox item selected.\n");
		}
		else
		{
			// select it
			SendMessage(hWndCtrl, LB_SETCURSEL, i, 0L);
		}
	}
}

void AFXAPI DDX_CBString(CDataExchange* pDX, int nIDC, CString& value)
{
	pDX->PrepareCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		// just get current edit item text (or drop list static)
		int nLen = ::GetWindowTextLength(hWndCtrl);
		if (nLen > 0)
		{
			// get known length
			::GetWindowText(hWndCtrl, value.GetBufferSetLength(nLen), nLen+1);
		}
		else
		{
			// for drop lists GetWindowTextLength does not work - assume
			//  max of 255 characters
			::GetWindowText(hWndCtrl, value.GetBuffer(255), 255+1);
		}
		value.ReleaseBuffer();
	}
	else
	{
		// set current selection based on model string
		if (::SendMessage(hWndCtrl, CB_SELECTSTRING, (WPARAM)-1,
			(LPARAM)(LPCTSTR)value) == CB_ERR)
		{
			// just set the edit text (will be ignored if DROPDOWNLIST)
			AfxSetWindowText(hWndCtrl, value);
		}
	}
}

void AFXAPI DDX_CBStringExact(CDataExchange* pDX, int nIDC, CString& value)
{
	pDX->PrepareCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		DDX_CBString(pDX, nIDC, value);
	}
	else
	{
		// set current selection based on data string
		int i = (int)::SendMessage(hWndCtrl, CB_FINDSTRINGEXACT, (WPARAM)-1,
		  (LPARAM)(LPCTSTR)value);
		if (i < 0)
		{
			// just set the edit text (will be ignored if DROPDOWNLIST)
			AfxSetWindowText(hWndCtrl, value);
		}
		else
		{
			// select it
			SendMessage(hWndCtrl, CB_SETCURSEL, i, 0L);
		}
	}
}

void AFXAPI DDX_LBIndex(CDataExchange* pDX, int nIDC, int& index)
{
	pDX->PrepareCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
		index = (int)::SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);
	else
		::SendMessage(hWndCtrl, LB_SETCURSEL, (WPARAM)index, 0L);
}

void AFXAPI DDX_CBIndex(CDataExchange* pDX, int nIDC, int& index)
{
	pDX->PrepareCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
		index = (int)::SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0L);
	else

		::SendMessage(hWndCtrl, CB_SETCURSEL, (WPARAM)index, 0L);
}

void AFXAPI DDX_Scroll(CDataExchange* pDX, int nIDC, int& value)
{
	pDX->PrepareCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
		value = GetScrollPos(hWndCtrl, SB_CTL);
	else
		SetScrollPos(hWndCtrl, SB_CTL, value, TRUE);
}

void AFXAPI DDX_Slider(CDataExchange* pDX, int nIDC, int& value)
{
	pDX->PrepareCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
		value = (int) ::SendMessage(hWndCtrl, TBM_GETPOS, 0, 0l);
	else
		::SendMessage(hWndCtrl, TBM_SETPOS, TRUE, value);
}

void AFXAPI DDX_IPAddress(CDataExchange* pDX, int nIDC, DWORD& value)
{
	pDX->PrepareCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
		::SendMessage(hWndCtrl, IPM_GETADDRESS, 0, (LPARAM) &value);
	else
		::SendMessage(hWndCtrl, IPM_SETADDRESS, 0, (LPARAM) value);
}

/////////////////////////////////////////////////////////////////////////////
// Range Dialog Data Validation

AFX_STATIC void AFXAPI _AfxFailMinMaxWithFormat(CDataExchange* pDX,
	 LONGLONG minVal, LONGLONG maxVal, LPCTSTR lpszFormat, UINT nIDPrompt)
	// error string must have '%1' and '%2' strings for min and max values
	// since minVal and maxVal are 64-bit, lpszFormat should be "%I64d" or "%I64u"
{
	ASSERT(lpszFormat != NULL);

	if (!pDX->m_bSaveAndValidate)
	{
		TRACE(traceAppMsg, 0, "Warning: initial dialog data is out of range.\n");
		return;     // don't stop now
	}
	
	const int MINMAX_BUFFER_SIZE = 64;
	TCHAR szMin[MINMAX_BUFFER_SIZE];
	TCHAR szMax[MINMAX_BUFFER_SIZE];
	
	ATL_CRT_ERRORCHECK_SPRINTF(_sntprintf_s(szMin, _countof(szMin), _countof(szMin) - 1, lpszFormat, minVal));
	ATL_CRT_ERRORCHECK_SPRINTF(_sntprintf_s(szMax, _countof(szMax), _countof(szMax) - 1, lpszFormat, maxVal));

	CString prompt;
	AfxFormatString2(prompt, nIDPrompt, szMin, szMax);
	AfxMessageBox(prompt, MB_ICONEXCLAMATION, nIDPrompt);
	prompt.Empty(); // exception prep
	pDX->Fail();
}

//NOTE: don't use overloaded function names to avoid type ambiguities
void AFXAPI DDV_MinMaxByte(CDataExchange* pDX, BYTE value, BYTE minVal, BYTE maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		_AfxFailMinMaxWithFormat(pDX, minVal, maxVal, _T("%I64u"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxShort(CDataExchange* pDX, short value, short minVal, short maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		_AfxFailMinMaxWithFormat(pDX, minVal, maxVal, _T("%I64d"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxInt(CDataExchange* pDX, int value, int minVal, int maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		_AfxFailMinMaxWithFormat(pDX, minVal, maxVal, _T("%I64d"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxLong(CDataExchange* pDX, long value, long minVal, long maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		_AfxFailMinMaxWithFormat(pDX, minVal, maxVal, _T("%I64d"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxUInt(CDataExchange* pDX, UINT value, UINT minVal, UINT maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		_AfxFailMinMaxWithFormat(pDX, minVal, maxVal, _T("%I64u"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxDWord(CDataExchange* pDX, DWORD value, DWORD minVal, DWORD maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		_AfxFailMinMaxWithFormat(pDX, minVal, maxVal, _T("%I64u"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxLongLong(CDataExchange* pDX, LONGLONG value, LONGLONG minVal, LONGLONG maxVal)
{
	ASSERT(minVal <= maxVal);
	if ((value < minVal) || (value > maxVal))
		_AfxFailMinMaxWithFormat(pDX, minVal, maxVal, _T("%I64d"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxULongLong(CDataExchange* pDX, ULONGLONG value, ULONGLONG minVal, ULONGLONG maxVal)
{
	ASSERT(minVal <= maxVal);
	if ((value < minVal) || (value > maxVal))
		_AfxFailMinMaxWithFormat(pDX, minVal, maxVal, _T("%I64u"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxSlider(CDataExchange* pDX, DWORD value, DWORD minVal, DWORD maxVal)
{
	ASSERT(minVal <= maxVal);

	if (!pDX->m_bSaveAndValidate)
	{
		if (minVal > value || maxVal < value)
		{
			TRACE(traceAppMsg, 0, "Warning: initial dialog data is out of "
				"range in control ID %d.\n", pDX->m_idLastControl);
			return;     // don't stop now
		}
	}

   HWND hWndLastControl;
   pDX->m_pDlgWnd->GetDlgItem(pDX->m_idLastControl, &hWndLastControl);
	::SendMessage(hWndLastControl, TBM_SETRANGEMIN, FALSE, (LPARAM) minVal);
	::SendMessage(hWndLastControl, TBM_SETRANGEMAX, TRUE, (LPARAM) maxVal);
}

/////////////////////////////////////////////////////////////////////////////
// Max Chars Dialog Data Validation

void AFXAPI DDV_MaxChars(CDataExchange* pDX, CString const& value, int nChars)
{
	ASSERT(nChars >= 1);        // allow them something
	if (pDX->m_bSaveAndValidate && value.GetLength() > nChars)
	{
		TCHAR szT[32];
		_stprintf_s(szT, _countof(szT), _T("%d"), nChars);
		CString prompt;
		AfxFormatString1(prompt, AFX_IDP_PARSE_STRING_SIZE, szT);
		AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_STRING_SIZE);
		prompt.Empty(); // exception prep
		pDX->Fail();
	}
	else if (pDX->m_idLastControl != 0 && pDX->m_bEditLastControl)
	{
	  HWND hWndLastControl;
	  pDX->m_pDlgWnd->GetDlgItem(pDX->m_idLastControl, &hWndLastControl);
		// limit the control max-chars automatically
		::SendMessage(hWndLastControl, EM_LIMITTEXT, nChars, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Special DDX_ proc for subclassing controls

void AFXAPI DDX_Control(CDataExchange* pDX, int nIDC, CWnd& rControl)
{
	if ((rControl.m_hWnd == NULL) && (rControl.GetControlUnknown() == NULL))    // not subclassed yet
	{
		ASSERT(!pDX->m_bSaveAndValidate);

		pDX->PrepareCtrl(nIDC);
	  HWND hWndCtrl;
	  pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
		if ((hWndCtrl != NULL) && !rControl.SubclassWindow(hWndCtrl))
		{
			ASSERT(FALSE);      // possibly trying to subclass twice?
			AfxThrowNotSupportedException();
		}
#ifndef _AFX_NO_OCC_SUPPORT
		else
		{
		 if (hWndCtrl == NULL)
		 {
			if (pDX->m_pDlgWnd->GetOleControlSite(nIDC) != NULL)
			{
			   rControl.AttachControlSite(pDX->m_pDlgWnd, nIDC);
			}
		 }
		 else
		 {
			   // If the control has reparented itself (e.g., invisible control),
			   // make sure that the CWnd gets properly wired to its control site.
			   if (pDX->m_pDlgWnd->m_hWnd != ::GetParent(rControl.m_hWnd))
				   rControl.AttachControlSite(pDX->m_pDlgWnd);
		 }
		}
#endif //!_AFX_NO_OCC_SUPPORT

	}
}

/////////////////////////////////////////////////////////////////////////////
// Global failure dialog helpers (used by database classes)

void AFXAPI AfxFailMaxChars(CDataExchange* pDX, int nChars)
{
	TCHAR lpszTemp[32];
	_stprintf_s(lpszTemp, _countof(lpszTemp), _T("%d"), nChars);
	CString prompt;
	AfxFormatString1(prompt, AFX_IDP_PARSE_STRING_SIZE, lpszTemp);
	AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_STRING_SIZE);
	prompt.Empty(); // exception prep
	pDX->Fail();
}

void AFXAPI AfxFailRadio(CDataExchange* pDX)
{
	CString prompt;
	AfxFormatStrings(prompt, AFX_IDP_PARSE_RADIO_BUTTON, NULL, 0);
	AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_RADIO_BUTTON);
	prompt.Empty(); // exception prep
	pDX->Fail();
}

/////////////////////////////////////////////////////////////////////////////
