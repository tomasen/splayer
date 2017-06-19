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
#include <afxwfrmcmd.h>

namespace Microsoft{
	namespace VisualC{
		namespace MFC {


IMPLEMENT_DYNAMIC(CWinFormsView,CView)

BEGIN_MESSAGE_MAP(CWinFormsView,CView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CWinFormsView::OnInitialUpdate()
{
	System::Windows::Forms::ScrollableControl ^scrlCtrl = dynamic_cast<System::Windows::Forms::ScrollableControl^>(GetControl());
	if (scrlCtrl != nullptr)
	{	
		CRect rcView;
		GetClientRect(&rcView);
		System::Drawing::Size size(rcView.Width(),rcView.Height());
		scrlCtrl->AutoScrollMinSize = size;			
		scrlCtrl->AutoScroll = true;
	}

	IView ^pIView = dynamic_cast<IView^>(GetControl());
	if (pIView != nullptr)
	{
		pIView->OnInitialUpdate();
	}

	__super::OnInitialUpdate();
}


BOOL CWinFormsView::OnCmdMsg(UINT nID, int nCode, void* pExtra,
						AFX_CMDHANDLERINFO* pHandlerInfo)
{
	System::Object^ p=static_cast<System::Object^>(m_CmdSource);
	if (p!=nullptr)
	{		
		CCommandSource^ pCmdSource = safe_cast<CCommandSource^>(p);
		if (nCode == CN_UPDATE_COMMAND_UI)
		{
			CommandUIHandler^ handler = pCmdSource->FindCommandUIHandler(nID);

			if (handler)
			{
				ENSURE_ARG(pExtra != NULL);
				CCmdUI &cmdUIMFC = *(CCmdUI*)pExtra;
				ASSERT(!cmdUIMFC.m_bContinueRouting);    // idle - not set
				CCommandUI^ cmdUI = gcnew CCommandUI(cmdUIMFC);
				handler->Invoke(nID, cmdUI);
				BOOL bResult = !cmdUIMFC.m_bContinueRouting;
				cmdUIMFC.m_bContinueRouting = FALSE;

				return bResult;
			}
		}
		else
			if (nCode == CN_COMMAND)
			{

				CommandHandler^ handler = pCmdSource->FindCommandHandler(nID);
				if (handler)
				{
					if (pHandlerInfo == NULL)
					{
						handler->Invoke(nID);
					}
					else
					{
						//
						// ensure the menu entry is enabled
						//
						pHandlerInfo->pTarget = NULL; //we don't have target
						pHandlerInfo->pmf = NULL;    // and pfnHandler
					}
					return TRUE;
				}
			}
	}

	return __super::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


BOOL CWinFormsView::Create(LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect,
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext)
{
	m_nFlags |= WF_ISWINFORMSVIEWWND;
	BOOL fSucceeded=__super::Create(lpszClassName, lpszWindowName, dwStyle, rect,pParentWnd, nID, pContext);
	ASSERT(fSucceeded);
	fSucceeded=fSucceeded && m_control.CreateManagedControl(m_pManagedViewType,WS_VISIBLE, rect, this,nID);
	ASSERT(fSucceeded);	
	if(fSucceeded)
	{
		ICommandTarget^ pICmdTarget = dynamic_cast<ICommandTarget^>(GetControl());
		if (pICmdTarget)
		{
			CCommandSource^ p=gcnew CCommandSource;
			m_CmdSource = p;		
			pICmdTarget->Initialize(p);
		}
	}
	return fSucceeded;
}

void CWinFormsView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	IView ^pIView = dynamic_cast<IView^>(GetControl());
	if (pIView != nullptr)
	{
		pIView->OnUpdate();
	}
	__super::OnUpdate(pSender, lHint, pHint);
}

void CWinFormsView::OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView)
{
	IView ^pIView = dynamic_cast<IView^>(GetControl());
			
	if (pIView != nullptr)
	{
		pIView->OnActivateView(bActivate ? true : false);
	}
	__super::OnActivateView(bActivate, pActivateView,
		pDeactiveView);
}	

BOOL CWinFormsView::PreTranslateMessage(MSG* pMsg)
{
	ASSERT(pMsg != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);

	// allow tooltip messages to be filtered
	if (__super::PreTranslateMessage(pMsg))
		return TRUE;

	// don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
		return FALSE;

	// since 'IsDialogMessage' will eat frame window accelerators,
	//   we call all frame windows' PreTranslateMessage first
	pFrameWnd = GetParentFrame();   // start with first parent frame
	while (pFrameWnd != NULL)
	{
		// allow owner & frames to translate before IsDialogMessage does
		if (pFrameWnd->PreTranslateMessage(pMsg))
			return TRUE;

		// try parent frames until there are no parent frames
		pFrameWnd = pFrameWnd->GetParentFrame();
	}

	// don't call IsDialogMessage if form is empty
	if (::GetWindow(m_hWnd, GW_CHILD) == NULL)
		return FALSE;

	// filter both messages to dialog and from children
	return PreTranslateInput(pMsg);
}

BOOL CWinFormsView::PreCreateWindow(CREATESTRUCT& cs)
{
	BOOL bRet = __super::PreCreateWindow(cs);
	cs.style |= WS_CLIPCHILDREN;
	return bRet;
}

void CWinFormsView::OnSize(UINT nType, int cx, int cy)
{		
	__super::OnSize(nType, cx, cy);

	//When view size changes, adjust the WinForms control (which is child of the view) 
	//size to occupy the entire client area of the view.
	CRect rcView;
	GetClientRect(&rcView);
	System::Drawing::Size size(rcView.Width(), rcView.Height());

	if (GetControl()!=nullptr)
	{
		GetControl()->Size = size;
	}

}

		} //MFC
	} //VisualC
} //Microsoft