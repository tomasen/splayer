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
#include "afxcontrolbarutil.h"
#include "afxglobals.h"
#include "afxusertool.h"

#include "afxribbonres.h"

IMPLEMENT_SERIAL(CUserTool, CObject, VERSIONABLE_SCHEMA | 1)

// Construction/Destruction
CUserTool::CUserTool()
{
	m_uiCmdId = 0;
	m_hIcon = NULL;
}

CUserTool::~CUserTool()
{
	DeleteIcon();
}

void CUserTool::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> m_strLabel;

		CString strCmd;
		ar >> strCmd;
		SetCommand(strCmd);

		ar >> m_strArguments;
		ar >> m_strInitialDirectory;
		ar >> m_uiCmdId;
	}
	else
	{
		ar << m_strLabel;
		ar << m_strCommand;
		ar << m_strArguments;
		ar << m_strInitialDirectory;
		ar << m_uiCmdId;
	}
}

BOOL CUserTool::Invoke()
{
	if (m_strCommand.IsEmpty())
	{
		TRACE(_T("Empty command in user-defined tool: %d\n"), m_uiCmdId);
		return FALSE;
	}

	if (::ShellExecute(AfxGetMainWnd()->GetSafeHwnd(), NULL, m_strCommand, m_strArguments, m_strInitialDirectory, SW_SHOWNORMAL) <(HINSTANCE) 32)
	{
		TRACE(_T("Can't invoke command: %s\n"), m_strCommand);
		return FALSE;
	}

	return TRUE;
}

void CUserTool::DrawToolIcon(CDC* pDC, const CRect& rectImage)
{
	ASSERT_VALID(pDC);

	int cx = afxGlobalData.m_sizeSmallIcon.cx;
	int cy = afxGlobalData.m_sizeSmallIcon.cy;

	int x = max(0, (rectImage.Width() - cx) / 2);
	int y = max(0, (rectImage.Height() - cy) / 2);

	::DrawIconEx(pDC->GetSafeHdc(), rectImage.left + x, rectImage.top + y, m_hIcon, 0, 0, 0, NULL, DI_NORMAL);
}

BOOL CUserTool::CopyIconToClipboard()
{
	try
	{
		CWindowDC dc(NULL);

		int cx = afxGlobalData.m_sizeSmallIcon.cx;
		int cy = afxGlobalData.m_sizeSmallIcon.cy;

		//----------------------
		// Create a bitmap copy:
		//----------------------
		CDC memDCDest;
		memDCDest.CreateCompatibleDC(NULL);

		CBitmap bitmapCopy;
		if (!bitmapCopy.CreateCompatibleBitmap(&dc, cx, cy))
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			return FALSE;
		}

		CBitmap* pOldBitmapDest = memDCDest.SelectObject(&bitmapCopy);

		CRect rectIcon(0, 0, cx, cy);
		memDCDest.FillRect(rectIcon, &afxGlobalData.brBtnFace);

		DrawToolIcon(&memDCDest, rectIcon);

		memDCDest.SelectObject(pOldBitmapDest);

		if (!AfxGetMainWnd()->OpenClipboard())
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			return FALSE;
		}

		if (!::EmptyClipboard())
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			::CloseClipboard();
			return FALSE;
		}

		HANDLE hclipData = ::SetClipboardData(CF_BITMAP, bitmapCopy.Detach());
		if (hclipData == NULL)
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			TRACE(_T("CUserTool::CopyIconToClipboard error. Error code = %x\n"), GetLastError());
		}

		::CloseClipboard();
		return TRUE;
	}
	catch(...)
	{
		AfxMessageBox(IDP_AFXBARRES_INTERLAL_ERROR);
	}

	return FALSE;
}

void CUserTool::SetCommand(LPCTSTR lpszCmd)
{
	if (lpszCmd == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (m_strCommand != lpszCmd)
	{
		m_strCommand = lpszCmd;

		DeleteIcon();
		m_hIcon = SetToolIcon();
	}
}

HICON CUserTool::SetToolIcon()
{
	CString strPath = m_strCommand;

	//--------------------------------------------
	// If the image path is not defined, found it:
	//--------------------------------------------
	if (strPath.Find(_T("\\")) == -1 && strPath.Find(_T("/")) == -1 && strPath.Find(_T(":")) == -1)
	{
		TCHAR lpszPath [MAX_PATH];
		CString strFileName = m_strCommand;

		if (::SearchPath(NULL, strFileName, NULL, MAX_PATH,
			lpszPath, NULL) == 0)
		{
			return LoadDefaultIcon();
		}

		strPath = lpszPath;
	}

	//----------------------------------------
	// Try to obtain a default icon from file:
	//----------------------------------------
	SHFILEINFO sfi;
	if (::SHGetFileInfo(strPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SHELLICONSIZE | SHGFI_SMALLICON))
	{
		return sfi.hIcon;
	}

	return LoadDefaultIcon();
}

HICON CUserTool::LoadDefaultIcon()
{
	if (afxGlobalData.m_hiconTool == NULL)
	{
		LPCTSTR lpszResourceName = MAKEINTRESOURCE(IDI_AFXRES_TOOL);
		ENSURE(lpszResourceName != NULL);

		afxGlobalData.m_hiconTool = (HICON) ::LoadImage(
			AfxFindResourceHandle(lpszResourceName, RT_ICON),
			lpszResourceName, IMAGE_ICON, afxGlobalData.m_sizeSmallIcon.cx, afxGlobalData.m_sizeSmallIcon.cy, LR_SHARED);
	}

	return afxGlobalData.m_hiconTool;
}

void CUserTool::DeleteIcon()
{
	if (m_hIcon != NULL && m_hIcon != afxGlobalData.m_hiconTool)
	{
		::DestroyIcon(m_hIcon);
	}

	m_hIcon = NULL;
}


