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
#include "afxoleserverdocex.h"
#include "afxoledocipframewndex.h"
#include "afxoleipframewndex.h"
#include "afxolecntrframewndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// COleServerDocEx

IMPLEMENT_DYNCREATE(COleServerDocEx, COleServerDoc)

COleServerDocEx::COleServerDocEx()
{
}

BOOL COleServerDocEx::OnNewDocument()
{
	if (!COleServerDoc::OnNewDocument())
		return FALSE;
	return TRUE;
}

COleServerDocEx::~COleServerDocEx()
{
}

COleServerItem* COleServerDocEx::OnGetEmbeddedItem()
{
	// OnGetEmbeddedItem is called by the framework to get the COleServerItem
	//  that is associated with the document.  It is only called when necessary.

	// Instead of returning NULL, return a pointer to a new COleServerItem
	//  derived class that is used in conjunction with this document, then
	//  remove the ASSERT(FALSE) below.
	//(i.e., return new CMyServerItem.)
	ASSERT(FALSE); // remove this after completing the TODO
	return NULL;
}

BEGIN_MESSAGE_MAP(COleServerDocEx, COleServerDoc)
	//{{AFX_MSG_MAP(COleServerDocEx)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COleServerDocEx diagnostics

#ifdef _DEBUG
void COleServerDocEx::AssertValid() const
{
	COleServerDoc::AssertValid();
}

void COleServerDocEx::Dump(CDumpContext& dc) const
{
	COleServerDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// COleServerDocEx serialization

void COleServerDocEx::Serialize(CArchive& ar)
{
	COleServerDoc::Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// COleServerDocEx commands
void COleServerDocEx::OnResizeBorder( LPCRECT lpRectBorder, LPOLEINPLACEUIWINDOW lpUIWindow, BOOL bFrame )
{
	ASSERT_VALID(this);
	ENSURE(lpUIWindow != NULL);

	if (!bFrame)
	{
		COleServerDoc::OnResizeBorder(lpRectBorder, lpUIWindow, bFrame);
		return;
	}

	COleCntrFrameWndEx* pMainFrame = NULL;

	COleDocIPFrameWndEx* p_IpDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, m_pInPlaceFrame);

	if (p_IpDocFrame != NULL)
	{
		pMainFrame =  p_IpDocFrame->GetContainerFrameWindow();
	}
	else
	{
		COleIPFrameWndEx* p_IpFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, m_pInPlaceFrame);

		if (p_IpFrame != NULL)
		{
			pMainFrame = p_IpFrame->GetContainerFrameWindow();
		}
		else
		{
			return;
		}
	}

	if (pMainFrame == NULL)
	{
		return;
	}

	CDockingManager* pDockManager = pMainFrame->GetDockingManager();
	ASSERT_VALID(pDockManager);

	CRect rcCurBorders;

	if (lpRectBorder == NULL)
	{
		if (lpUIWindow->GetBorder(&rcCurBorders) != S_OK)
		{
			lpUIWindow->SetBorderSpace(NULL);
			return;
		}
	}
	else
	{
		rcCurBorders = *lpRectBorder;
	}

	if (AfxGetThread()->m_pActiveWnd == m_pInPlaceFrame)
		OnShowPanes(pMainFrame, TRUE);

	pDockManager->m_rectInPlace = rcCurBorders;
	pDockManager->AdjustDockingLayout();

	CRect rectClient = pDockManager->GetClientAreaBounds();

	CRect rectRequest(abs(rectClient.left - rcCurBorders.left), abs(rectClient.top - rcCurBorders.top), abs(rectClient.right - rcCurBorders.right), abs(rectClient.bottom  - rcCurBorders.bottom)); // v.8.6 removed -1 to make it work in Excel

	CRect rectTemp;
	rectTemp = rectRequest;

	if (!rectRequest.IsRectNull() || lpUIWindow->RequestBorderSpace(&rectTemp) == S_OK)
	{
		lpUIWindow->SetBorderSpace(&rectRequest);
		pDockManager->AdjustDockingLayout();
	}
	else
	{
		OnShowPanes(pMainFrame, FALSE);
		CRect rect(0,0,0,0);
		lpUIWindow->SetBorderSpace(&rect);
	}
}

void COleServerDocEx::OnShowPanes(CFrameWnd* pFrameWnd, BOOL bShow)
{
	COleServerDoc::OnShowControlBars(pFrameWnd, bShow);
	COleCntrFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(COleCntrFrameWndEx, pFrameWnd);

	if (pMainFrame != NULL)
	{
		ASSERT_VALID(pMainFrame);
		CDockingManager* pDockManager = pMainFrame->GetDockingManager();

		if (pDockManager != NULL)
		{
			ASSERT_VALID(pDockManager);
			pDockManager->ShowPanes(bShow);
		}
	}
}

void COleServerDocEx::OnDocWindowActivate( BOOL bActivate )
{
	if (bActivate)
	{
		COleServerDoc::OnDocWindowActivate(bActivate);
		return;
	}

	COleCntrFrameWndEx* pMainFrame = NULL;

	COleDocIPFrameWndEx* p_IpDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, m_pInPlaceFrame);

	if (p_IpDocFrame != NULL)
	{
		pMainFrame =  p_IpDocFrame->GetContainerFrameWindow();
	}
	else
	{
		COleIPFrameWndEx* p_IpFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, m_pInPlaceFrame);

		if (p_IpFrame != NULL)
		{
			pMainFrame =  p_IpFrame->GetContainerFrameWindow();
		}
	}

	if (pMainFrame == NULL)
	{
		return;
	}

	CDockingManager* pDockManager = pMainFrame->GetDockingManager();
	ASSERT_VALID(pDockManager);

	pDockManager->ShowPanes(bActivate);
	COleServerDoc::OnDocWindowActivate(bActivate);
}



