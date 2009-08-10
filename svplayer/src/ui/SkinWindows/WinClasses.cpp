// WinClasses.cpp: implementation of the CWinClasses class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WinClasses.h"
#include "wclassdefines.h"

#include <afxpriv.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// helper function for everyone to use
void TRACEWND(LPCTSTR szFunctionName, HWND hWnd)
{
#ifdef _DEBUG
	if (hWnd)
	{
		CString sText; 
		//		pWnd->GetWindowText(sText); 
		TRACE ("%s(%s, %s, id = %d)\n", szFunctionName, CWinClasses::GetClass(hWnd), sText, GetDlgCtrlID(hWnd)); 
	}
#endif
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMap<LPCTSTR, LPCTSTR, int, int&> CWinClasses::s_mapCtrlClasses;

CString CWinClasses::GetClass(HWND hWnd)
{
	static TCHAR szWndClass[128] = L"";
	
	if (hWnd)
		::GetClassName(hWnd, szWndClass, 127);
	
	return CString(szWndClass);
}

BOOL CWinClasses::IsClass(HWND hWnd, LPCTSTR szClass)
{
	if (hWnd)
	{
		TCHAR szWndClass[128] = L"";

		::GetClassName(hWnd, szWndClass, 127);
		return (lstrcmpi(szClass, szWndClass) == 0);
	}

	return FALSE;
}

CString CWinClasses::GetClassEx(HWND hWnd)
{
	CString sClass = GetClass(hWnd);
	
	if (sClass.Find(L"Afx") == 0) // its an mfc framework base or derived class
	{
		// can do the check if pWnd is permanent else mfc will not yet
		// have hooked up
		CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);

		if (pWnd)
		{
			// must do the check in order of most derived class first
			if (pWnd->IsKindOf(RUNTIME_CLASS(CView)))
				return WC_MFCVIEW;
			
			else if (pWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)))
				return WC_MFCMDIFRAME;
			
			else if (pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
				return WC_MFCMDICHILD;
			
			else if (pWnd->IsKindOf(RUNTIME_CLASS(CMiniDockFrameWnd)))
				return WC_MFCMINIDOCKFRAME;
			
			else if (pWnd->IsKindOf(RUNTIME_CLASS(CMiniFrameWnd)))
				return WC_MFCMINIFRAME;
			
			else if (pWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd))) // this is the catch all for frame wnds
				return WC_MFCFRAME;
			
			else if (pWnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
				return WC_MFCSPLITTER;
			
			else if (pWnd->IsKindOf(RUNTIME_CLASS(CDialogBar)))
				return WC_MFCDIALOGBAR;
			
			else if (pWnd->IsKindOf(RUNTIME_CLASS(CControlBar)))
				return WC_CONTROLBAR;
			
			else 
				return WC_MFCWND; // catch all for all window classes
		}
	}
	
	return sClass;
}

BOOL CWinClasses::IsControlClass(HWND hWnd)
{
	return IsControlClass(GetClass(hWnd));
}

BOOL CWinClasses::IsControlClass(LPCTSTR szClass)
{
	// init
	if (!s_mapCtrlClasses.GetCount())
	{
		s_mapCtrlClasses[WC_BUTTON] = 1;
		s_mapCtrlClasses[WC_STATIC] = 1;
		s_mapCtrlClasses[WC_EDIT] = 1;  
		s_mapCtrlClasses[WC_COMBOBOX] = 1;
		s_mapCtrlClasses[WC_COMBOLBOX] = 1;
		s_mapCtrlClasses[WC_LISTBOX] = 1;  
		s_mapCtrlClasses[WC_SCROLLBAR] = 1;
		s_mapCtrlClasses[WC_TOOLBAR] = 1;  
		s_mapCtrlClasses[WC_SPIN] = 1;     
		s_mapCtrlClasses[WC_PROGRESS] = 1; 
		s_mapCtrlClasses[WC_SLIDER] = 1;   
		s_mapCtrlClasses[WC_HOTKEY] = 1;   
		s_mapCtrlClasses[WC_SHELLDLLDEFVIEW] = 1;  
		s_mapCtrlClasses[WC_STATUSBAR] = 1;        
		s_mapCtrlClasses[WC_ANIMATE] = 1;          
		s_mapCtrlClasses[WC_RICHEDIT] = 1;         
		s_mapCtrlClasses[WC_RICHEDIT20] = 1;       
		s_mapCtrlClasses[WC_DATETIMEPICK] = 1;     
		s_mapCtrlClasses[WC_MONTHCAL] = 1;         
		s_mapCtrlClasses[WC_REBAR] = 1;            
		s_mapCtrlClasses[WC_TOOLTIPS] = 1; 
		s_mapCtrlClasses[WC_THUMBNAILVIEW] = 1; 
		s_mapCtrlClasses[WC_IE] = 1; 
		s_mapCtrlClasses[WC_SHDOCOBJVW] = 1; 
		s_mapCtrlClasses[WC_SHELLEMBEDDING] = 1; 
	}

	int nTemp;

	return s_mapCtrlClasses.Lookup(szClass, nTemp);
}
