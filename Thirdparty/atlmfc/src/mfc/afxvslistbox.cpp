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
#include "afxvslistbox.h"
#include "afxacceleratorkey.h"
#include "afxribbonres.h"
#include "afxglobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int nListId = 1;
static const int nTextMargin = 5;
static const int nBrowseButtonWidth = 20;

/////////////////////////////////////////////////////////////////////////////
// CVSListBoxEditCtrl

CVSListBoxEditCtrl::CVSListBoxEditCtrl()
{
	m_bLocked = FALSE;
	m_pParentList = NULL;
}

CVSListBoxEditCtrl::~CVSListBoxEditCtrl()
{
}

BEGIN_MESSAGE_MAP(CVSListBoxEditCtrl, CMFCEditBrowseCtrl)
	//{{AFX_MSG_MAP(CVSListBoxEditCtrl)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSListBoxEditCtrl message handlers

void CVSListBoxEditCtrl::LockSize(CVSListBoxBase* pParent, BOOL bLock)
{
	m_pParentList = pParent;
	m_bLocked = bLock;
}

void CVSListBoxEditCtrl::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
	if (m_bLocked)
	{
		lpwndpos->flags |= SWP_NOSIZE;
	}

	CEdit::OnWindowPosChanging(lpwndpos);
}

void CVSListBoxEditCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if (!m_bLocked)
	{
		CMFCEditBrowseCtrl::OnNcCalcSize(bCalcValidRects, lpncsp);
	}
}
/////////////////////////////////////////////////////////////////////////////
// CVSListBoxBase

CVSListBoxBase::CVSListBoxBase()
{
	m_sizeButton = CSize(0, 0);
	m_uiStandardBtns = 0;
	m_bNewItem = FALSE;
	m_bIsActualDelete = TRUE;
	m_bBrowseButton = FALSE;
	m_bGrayDisabledButtons = FALSE;
	m_bDefaultCaption = TRUE;
}

CVSListBoxBase::~CVSListBoxBase()
{
	while (!m_lstButtons.IsEmpty())
	{
		delete m_lstButtons.RemoveHead();
	}
}

//{{AFX_MSG_MAP(CVSListBoxBase)
BEGIN_MESSAGE_MAP(CVSListBoxBase, CStatic)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_ENABLE()
	ON_WM_GETDLGCODE()
	ON_MESSAGE(WM_SETFONT, &CVSListBoxBase::OnSetFont)
	ON_MESSAGE(WM_GETFONT, &CVSListBoxBase::OnGetFont)
	ON_MESSAGE(WM_SETTEXT, &CVSListBoxBase::OnSetText)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CVSListBoxBase message handlers
void CVSListBoxBase::PreSubclassWindow()
{
	CStatic::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState->m_pWndInit == NULL)
	{
		Init();
	}
}

int CVSListBoxBase::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;

	Init();
	return 0;
}

void CVSListBoxBase::Init()
{
	ModifyStyle(0, SS_USERITEM);

	if (OnCreateList() == NULL)
	{
		TRACE0("CVSListBoxBase::Init(): Can not create list control\n");
		return;
	}

	AdjustLayout();
}

BOOL CVSListBoxBase::SetStandardButtons(UINT uiBtns)
{
	if (GetSafeHwnd() == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CString strButton;
	if (uiBtns & AFX_VSLISTBOX_BTN_NEW)
	{
		ENSURE(strButton.LoadString(IDS_AFXBARRES_NEW));
		ENSURE(AddButton(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_NEW32 : IDB_AFXBARRES_NEW, strButton, VK_INSERT,0,AFX_VSLISTBOX_BTN_NEW_ID));
	}

	if (uiBtns & AFX_VSLISTBOX_BTN_DELETE)
	{
		ENSURE(strButton.LoadString(IDS_AFXBARRES_DELETE));
		ENSURE(AddButton(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_DELETE32 : IDB_AFXBARRES_DELETE, strButton, VK_DELETE, 0, AFX_VSLISTBOX_BTN_DELETE_ID));
	}

	if (uiBtns & AFX_VSLISTBOX_BTN_UP)
	{
		ENSURE(strButton.LoadString(IDS_AFXBARRES_MOVEUP));
		ENSURE(AddButton(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_UP32 : IDB_AFXBARRES_UP, strButton, VK_UP, FALT, AFX_VSLISTBOX_BTN_UP_ID));
	}

	if (uiBtns & AFX_VSLISTBOX_BTN_DOWN)
	{
		ENSURE(strButton.LoadString(IDS_AFXBARRES_MOVEDN));
		ENSURE(AddButton(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_DOWN32 : IDB_AFXBARRES_DOWN, strButton, VK_DOWN, FALT, AFX_VSLISTBOX_BTN_DOWN_ID));
	}

	m_uiStandardBtns |= uiBtns;
	return TRUE;
}

BOOL CVSListBoxBase::AddButton(UINT uiImageResId, LPCTSTR lpszTooltip/* = NULL*/, WORD wKeyAccelerator/* = 0*/, BYTE fVirt/* = 0*/, UINT uiButtonID/* = 0*/)
{
	if (GetSafeHwnd() == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (uiButtonID != 0)
	{
		ASSERT(GetButtonNum(uiButtonID)==-1); // button with this ID still not added
	}

	CRect rectEmpty;
	rectEmpty.SetRectEmpty();

	CMFCButton* pButton = new CMFCButton();
	if (!pButton->Create(_T(""), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW, rectEmpty, this, (int) m_lstButtons.GetCount() + 2))
	{
		return FALSE;
	}

	pButton->m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	pButton->m_bGrayDisabled = m_bGrayDisabledButtons;
	pButton->m_bDrawFocus = FALSE;

	pButton->SetImage(uiImageResId);
	if (lpszTooltip != NULL)
	{
		CString strTooltip = lpszTooltip;
		if (wKeyAccelerator != 0)
		{
			ACCEL acccel;
			acccel.cmd = 0;
			acccel.fVirt = (BYTE)(fVirt | FVIRTKEY);
			acccel.key = wKeyAccelerator;

			CMFCAcceleratorKey helper(&acccel);
			CString strAccellKey;
			helper.Format(strAccellKey);

			strTooltip += _T("(");
			strTooltip += strAccellKey;
			strTooltip += _T(")");
		}

		pButton->SetTooltip(strTooltip);
	}

	pButton->SizeToContent();
	CRect rectBtn;
	pButton->GetWindowRect(rectBtn);
	CSize sizeButton = rectBtn.Size();

	if (m_lstButtons.IsEmpty())
	{
		m_sizeButton = sizeButton;
	}
	else
	{
		ASSERT(m_sizeButton == sizeButton);
	}

	m_lstButtons.AddTail(pButton);

	if (wKeyAccelerator == 0)
	{
		fVirt = 0;
	}

	DWORD dwKey = (fVirt << 16) | wKeyAccelerator;
	m_lstKeyAccell.AddTail(dwKey);

	if (uiButtonID != 0)
	{
		int iButton = (int) m_lstButtons.GetCount() - 1;
		m_mapButtonIDs.SetAt(iButton, uiButtonID);
	}

	AdjustLayout();
	return TRUE;
}

void CVSListBoxBase::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	dc.FillRect(m_rectCaption, &afxGlobalData.brBtnFace);
	dc.Draw3dRect(m_rectCaption, afxGlobalData.clrBtnShadow, afxGlobalData.clrBtnHilite);

	CRect rectText = m_rectCaption;
	rectText.DeflateRect(nTextMargin, 0);

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(IsWindowEnabled() ? afxGlobalData.clrBtnText : afxGlobalData.clrGrayedText);

	CFont* pOldFont = NULL;

	if (m_font.GetSafeHandle() != NULL)
	{
		pOldFont = dc.SelectObject(&m_font);
	}
	else
	{
		CFont* pParentFont = GetParent()->GetFont();

		if (pParentFont != NULL)
		{
			pOldFont = dc.SelectObject(pParentFont);
			ENSURE(pOldFont != NULL);
		}
	}

	CString strCaption;

	if (m_bDefaultCaption)
	{
		GetWindowText(strCaption);
	}
	else
	{
		strCaption = m_strCaption;
	}

	dc.DrawText(strCaption, rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

	if (pOldFont != NULL)
	{
		dc.SelectObject(pOldFont);
	}
}

void CVSListBoxBase::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CVSListBoxBase::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CWnd* pWndList = CWnd::FromHandle(GetListHwnd());
	if (pWndList == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	m_rectCaption = rectClient;

	CClientDC dc(this);

	CFont* pOldFont = NULL;
	if (m_font.GetSafeHandle() != NULL)
	{
		pOldFont = dc.SelectObject(&m_font);
		pWndList->SetFont(&m_font, FALSE);
	}
	else
	{
		CFont* pParentFont = GetParent()->GetFont();

		if (pParentFont != NULL)
		{
			pWndList->SetFont(pParentFont, FALSE);

			pOldFont = dc.SelectObject(pParentFont);
			ENSURE(pOldFont != NULL);
		}
	}

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	if (pOldFont != NULL)
	{
		dc.SelectObject(pOldFont);
	}

	m_rectCaption.bottom = m_rectCaption.top + max(tm.tmHeight * 4 / 3, m_sizeButton.cy);

	int x = rectClient.right - 1 - m_sizeButton.cx;
	for (POSITION pos = m_lstButtons.GetTailPosition(); pos != NULL;)
	{
		CMFCButton* pButton = m_lstButtons.GetPrev(pos);
		ENSURE(pButton != NULL);

		pButton->MoveWindow(x, rectClient.top + 1, m_sizeButton.cx, m_rectCaption.Height() - 2);
		x -= m_sizeButton.cx;
	}

	pWndList->MoveWindow( rectClient.left, rectClient.top + m_rectCaption.Height(), rectClient.Width(), rectClient.Height() - m_rectCaption.Height());
	OnSizeList();
}

BOOL CVSListBoxBase::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

BOOL CVSListBoxBase::OnCommand(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = (HWND) lParam;

	int iButton = 0;
	for (POSITION pos = m_lstButtons.GetHeadPosition(); pos != NULL; iButton ++)
	{
		CMFCButton* pButton = m_lstButtons.GetNext(pos);
		ENSURE(pButton);

		if (pButton->GetSafeHwnd() == hwnd)
		{
			CWnd* pWndList = CWnd::FromHandle(GetListHwnd());
			if (pWndList == NULL)
			{
				ASSERT(FALSE);
			}
			else
			{
				pWndList->SetFocus();
			}

			OnClickButton(iButton);
			return TRUE;
		}
	}

	return CStatic::OnCommand(wParam, lParam);
}

void CVSListBoxBase::OnSetFocus(CWnd* /*pOldWnd*/)
{
	CWnd* pWndList = CWnd::FromHandle(GetListHwnd());
	if (pWndList == NULL)
	{
		ASSERT(FALSE);
	}
	else
	{
		pWndList->SetFocus();
	}
}

void CVSListBoxBase::OnClickButton(int iButton)
{
	if (m_uiStandardBtns == 0)
	{
		return;
	}

	int iSelItem = GetSelItem();
	UINT uiBtnID = GetButtonID(iButton);

	switch(uiBtnID)
	{
	case AFX_VSLISTBOX_BTN_NEW_ID:
		CreateNewItem();
		return;

	case AFX_VSLISTBOX_BTN_DELETE_ID:
		if (iSelItem >= 0)
		{
			if (OnBeforeRemoveItem(iSelItem))
			{
				RemoveItem(iSelItem);
			}
		}
		break;

	case AFX_VSLISTBOX_BTN_UP_ID:
	case AFX_VSLISTBOX_BTN_DOWN_ID:
		if (iSelItem >= 0)
		{
			BOOL bIsUp = (uiBtnID == AFX_VSLISTBOX_BTN_UP_ID);
			if (bIsUp)
			{
				if (iSelItem == 0)
				{
					return;
				}
			}
			else
			{
				if (iSelItem == GetCount() - 1)
				{
					return;
				}
			}

			// Adjust list control:
			SetRedraw(FALSE);

			CString strLabel = GetItemText(iSelItem);
			DWORD_PTR dwData = GetItemData(iSelItem);

			m_bIsActualDelete = FALSE;
			RemoveItem(iSelItem);
			m_bIsActualDelete = TRUE;

			if (bIsUp)
			{
				iSelItem --;
			}
			else
			{
				iSelItem ++;
			}

			AddItem(strLabel, dwData, iSelItem);
			SelectItem(iSelItem);

			SetRedraw();

			CWnd* pWndList = CWnd::FromHandle(GetListHwnd());
			if (pWndList == NULL)
			{
				ASSERT(FALSE);
			}
			else
			{
				pWndList->Invalidate();
			}

			if (bIsUp)
			{
				OnAfterMoveItemUp(iSelItem);
			}
			else
			{
				OnAfterMoveItemDown(iSelItem);
			}
		}
	}
}

int CVSListBoxBase::GetStdButtonNum(UINT uiStdBtn) const
{
	if ((m_uiStandardBtns & uiStdBtn) == 0)
	{
		return -1;
	}

	switch(uiStdBtn)
	{
	case AFX_VSLISTBOX_BTN_NEW:
		return GetButtonNum(AFX_VSLISTBOX_BTN_NEW_ID);

	case AFX_VSLISTBOX_BTN_DELETE:
		return GetButtonNum(AFX_VSLISTBOX_BTN_DELETE_ID);

	case AFX_VSLISTBOX_BTN_UP:
		return GetButtonNum(AFX_VSLISTBOX_BTN_UP_ID);

	case AFX_VSLISTBOX_BTN_DOWN:
		return GetButtonNum(AFX_VSLISTBOX_BTN_DOWN_ID);
	}

	ASSERT(FALSE);
	return -1;
}

void CVSListBoxBase::CreateNewItem()
{
	int iLastItem = AddItem(_T(""));
	ASSERT(iLastItem >= 0);

	m_bNewItem = TRUE;
	EditItem(iLastItem);
}

void CVSListBoxBase::OnKey(WORD wKey, BYTE fFlags)
{
	int iSelItem = GetSelItem();
	TCHAR cKey = (TCHAR) LOWORD(::MapVirtualKey(wKey, 2));

	if (fFlags == 0 && iSelItem >= 0 && (cKey == _T(' ') || wKey == VK_F2))
	{
		EditItem(iSelItem);
	}
}

LRESULT CVSListBoxBase::OnGetFont(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return(LRESULT) m_font.GetSafeHandle();
}

LRESULT CVSListBoxBase::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = Default();

	CFont* pFont = CFont::FromHandle((HFONT) wParam);
	if (pFont != NULL)
	{
		LOGFONT lf;
		pFont->GetLogFont(&lf);

		m_font.DeleteObject();
		m_font.CreateFontIndirect(&lf);
	}

	if (::IsWindow(GetSafeHwnd()) != NULL)
	{
		AdjustLayout();

		if (lParam != 0)
		{
			Invalidate();
			UpdateWindow();
		}
	}

	return lResult;
}

void CVSListBoxBase::OnEndEditLabel(LPCTSTR lpszLabel)
{
	int iSelItem = GetSelItem();
	if (iSelItem < 0)
	{
		ASSERT(FALSE);
		return;
	}

	CString strLabel = (lpszLabel != NULL) ? lpszLabel : _T("");

	if (!strLabel.IsEmpty())
	{
		SetItemText(iSelItem, strLabel);

		if (m_bNewItem)
		{
			OnAfterAddItem(iSelItem);
		}
		else
		{
			OnAfterRenameItem(iSelItem);
		}
	}
	else
	{
		if (m_bNewItem)
		{
			RemoveItem(iSelItem);
		}
	}

	m_bNewItem = FALSE;
}

BOOL CVSListBoxBase::EnableButton(int iButtonNum, BOOL bEnable/* = TRUE*/)
{
	POSITION pos = m_lstButtons.FindIndex(iButtonNum);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCButton* pButton = m_lstButtons.GetAt(pos);
	ASSERT_VALID(pButton);

	pButton->EnableWindow(bEnable);
	return TRUE;
}

UINT CVSListBoxBase::GetButtonID(int iButtonNum) const
{
	UINT uiID = 0;
	m_mapButtonIDs.Lookup(iButtonNum, uiID);

	return uiID;
}

int CVSListBoxBase::GetButtonNum(UINT uiID) const
{
	for (POSITION pos = m_mapButtonIDs.GetStartPosition(); pos != NULL;)
	{
		int iNum = -1;
		UINT uiButtonID = 0;

		m_mapButtonIDs.GetNextAssoc(pos, iNum, uiButtonID);

		if (uiButtonID == uiID)
		{
			return iNum;
		}
	}

	return -1;
}


void CVSListBoxBase::EnableBrowseButton(BOOL bEnable/* = TRUE*/)
{
	m_bBrowseButton = bEnable;
}

void CVSListBoxBase::SetGrayDisabledButtons(BOOL bOn)
{
	m_bGrayDisabledButtons = bOn;
}

void CVSListBoxBase::OnEnable(BOOL bEnable)
{
	CStatic::OnEnable(bEnable);

	for (POSITION pos = m_lstButtons.GetTailPosition(); pos != NULL;)
	{
		CMFCButton* pButton = m_lstButtons.GetPrev(pos);
		ASSERT_VALID(pButton);

		pButton->m_bGrayDisabled = !bEnable || m_bGrayDisabledButtons;
		pButton->EnableWindow(bEnable);
	}

	CWnd* pWndList = CWnd::FromHandle(GetListHwnd());
	if (pWndList != NULL)
	{
		pWndList->EnableWindow(bEnable);
	}

	RedrawWindow();
}

UINT CVSListBoxBase::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}

LRESULT CVSListBoxBase::OnSetText(WPARAM, LPARAM lParam)
{
	LPCTSTR lpcszTitle = reinterpret_cast<LPCTSTR>(lParam);
	if (lpcszTitle != NULL)
	{
		m_strCaption = lpcszTitle;
		m_bDefaultCaption = FALSE;
	}
	else
	{
		m_bDefaultCaption = TRUE;
		m_strCaption.Empty();
	}

	RedrawWindow();
	return Default();
}

/////////////////////////////////////////////////////////////////////////////
// CVSListBox

CVSListBox::CVSListBox()
{
	m_pWndList = NULL;
	m_ptClick = CPoint(-1, -1);
}

CVSListBox::~CVSListBox()
{
	if (m_pWndList != NULL)
	{
		ASSERT_VALID(m_pWndList);
		delete m_pWndList;
	}
}

BEGIN_MESSAGE_MAP(CVSListBox, CVSListBoxBase)
	//{{AFX_MSG_MAP(CVSListBox)
	ON_NOTIFY(LVN_KEYDOWN, nListId, &CVSListBox::OnKeyDown)
	ON_NOTIFY(NM_DBLCLK, nListId, &CVSListBox::OnDblclkList)
	ON_NOTIFY(LVN_GETDISPINFO, nListId, &CVSListBox::OnGetdispinfo)
	ON_NOTIFY(LVN_ENDLABELEDIT, nListId, &CVSListBox::OnEndLabelEdit)
	ON_NOTIFY(LVN_ITEMCHANGED, nListId, &CVSListBox::OnItemChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVSListBox message handlers

CWnd* CVSListBox::OnCreateList()
{
	if (GetSafeHwnd() == NULL ||
		m_pWndList != NULL)
	{
		return FALSE;
	}

	ASSERT(GetStyle() & WS_CHILD);

	CRect rectEmpty;
	rectEmpty.SetRectEmpty();

	m_pWndList = new CListCtrl;
	m_pWndList->CWnd::CreateEx(WS_EX_CLIENTEDGE, _T("SysListView32"), _T(""), WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL |
		LVS_NOCOLUMNHEADER | LVS_EDITLABELS | LVS_SHOWSELALWAYS, rectEmpty, this, nListId);

	m_pWndList->SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
	m_pWndList->InsertColumn(0, _T(""));

	return m_pWndList;
}

int CVSListBox::AddItem(const CString& strText, DWORD_PTR dwData, int iIndex)
{
	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		return -1;
	}

	ASSERT_VALID(m_pWndList);

	if (iIndex < 0)
	{
		iIndex = m_pWndList->GetItemCount();
	}

	int iItem = m_pWndList->InsertItem(iIndex, strText, I_IMAGECALLBACK);
	m_pWndList->SetItemData(iItem, dwData);

	if (iItem == 0)
	{
		SelectItem(0);
	}

	return iItem;
}

int CVSListBox::GetCount() const
{
	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		return -1;
	}

	return m_pWndList->GetItemCount();
}

CString CVSListBox::GetItemText(int iIndex) const
{
	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		return _T("");
	}

	ASSERT_VALID(m_pWndList);
	return m_pWndList->GetItemText(iIndex, 0);
}

DWORD_PTR CVSListBox::GetItemData(int iIndex) const
{
	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		ASSERT(FALSE);
		return 0;
	}

	ASSERT_VALID(m_pWndList);
	return m_pWndList->GetItemData(iIndex);
}

void CVSListBox::SetItemData(int iIndex, DWORD_PTR dwData)
{
	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(m_pWndList);
	m_pWndList->SetItemData(iIndex, dwData);
}

void CVSListBox::OnKeyDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVKEYDOWN* pKeyDown = (NMLVKEYDOWN*) pNMHDR;
	*pResult = 0;

	if (pKeyDown != NULL)
	{
		BYTE fCurrVirt = 0;

		if (::GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			fCurrVirt |= FCONTROL;
		}

		if (::GetAsyncKeyState(VK_MENU) & 0x8000)
		{
			fCurrVirt |= FALT;
		}

		if (::GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			fCurrVirt |= FSHIFT;
		}

		int iButton = 0;
		for (POSITION pos = m_lstKeyAccell.GetHeadPosition(); pos != NULL; iButton ++)
		{
			DWORD dwKey = m_lstKeyAccell.GetNext(pos);

			if (dwKey != 0 && pKeyDown->wVKey == (dwKey & 0xFFFF))
			{
				//-------------------
				// Check state flags:
				//-------------------
				BYTE fVirt = (BYTE)(dwKey >> 16);
				if (fCurrVirt == fVirt)
				{
					OnClickButton(iButton);
					return;
				}
			}
		}

		OnKey(pKeyDown->wVKey, fCurrVirt);
	}
}

int CVSListBox::GetSelItem() const
{
	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	ASSERT_VALID(m_pWndList);
	return m_pWndList->GetNextItem(-1, LVNI_SELECTED);
}

BOOL CVSListBox::SelectItem(int iItem)
{
	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT_VALID(m_pWndList);

	if (!m_pWndList->SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED))
	{
		return FALSE;
	}

	return m_pWndList->EnsureVisible(iItem, FALSE);
}

void CVSListBox::OnDblclkList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	int iSelItem = GetSelItem();

	if ((m_uiStandardBtns & AFX_VSLISTBOX_BTN_NEW) && iSelItem == -1)
	{
		CreateNewItem();
		return;
	}

	if (iSelItem >= 0)
	{
		EditItem(iSelItem);
	}
}

BOOL CVSListBox::RemoveItem(int iIndex)
{
	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT_VALID(m_pWndList);

	BOOL bIsSelected = (GetSelItem() == iIndex);

	if (!m_pWndList->DeleteItem(iIndex))
	{
		return FALSE;
	}

	if (!bIsSelected || GetCount() == 0)
	{
		return FALSE;
	}

	//-------------------
	// Restore selection:
	//-------------------
	if (iIndex >= GetCount())
	{
		iIndex --;
	}

	SelectItem(iIndex);
	return TRUE;
}

void CVSListBox::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	ENSURE(pNMHDR != NULL);

	LV_ITEM* pItem = &((LV_DISPINFO*)pNMHDR)->item;
	ENSURE(pItem != NULL);

	if (pItem->mask & LVIF_IMAGE)
	{
		pItem->iImage = OnGetImage(pItem);
	}

	*pResult = 0;
}

void CVSListBox::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	ENSURE(pNMHDR != NULL);

	LV_ITEM* pItem = &((LV_DISPINFO*)pNMHDR)->item;
	ENSURE(pItem != NULL);

	OnEndEditLabel(pItem->pszText);

	for (POSITION pos = m_lstButtons.GetTailPosition(); pos != NULL;)
	{
		CMFCButton* pButton = m_lstButtons.GetPrev(pos);
		ENSURE(pButton != NULL);

		pButton->EnableWindow();
	}

	*pResult = 0;
}

void CVSListBox::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	ENSURE(pNMHDR != NULL);

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	ENSURE(pNMListView != NULL);

	if (pNMListView->uChanged == LVIF_STATE && (pNMListView->uOldState & LVIS_SELECTED) != (pNMListView->uNewState & LVIS_SELECTED))
	{
		OnSelectionChanged();
	}

	*pResult = 0;
}

BOOL CVSListBox::EditItem(int iIndex)
{
	m_wndEdit.LockSize(NULL, FALSE);

	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT_VALID(m_pWndList);

	m_pWndList->SetFocus();
	CEdit* pEdit = m_pWndList->EditLabel(iIndex);
	if (pEdit == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pEdit);

	m_wndEdit.SubclassWindow(pEdit->GetSafeHwnd());
	m_wndEdit.EnableBrowseButton(m_bBrowseButton);

	CRect rectItem;
	m_pWndList->GetItemRect(iIndex, rectItem, LVIR_LABEL);

	CRect rectEdit;
	pEdit->GetClientRect(rectEdit);

	pEdit->SetWindowPos(NULL, -1, -1, rectItem.Width(), rectEdit.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	m_wndEdit.LockSize(this);

	for (POSITION pos = m_lstButtons.GetTailPosition(); pos != NULL;)
	{
		CMFCButton* pButton = m_lstButtons.GetPrev(pos);
		ENSURE(pButton != NULL);

		pButton->EnableWindow(FALSE);
	}

	return TRUE;
}

void CVSListBox::OnSizeList()
{
	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(m_pWndList);

	CRect rectClient;
	GetClientRect(rectClient);

	m_pWndList->SetColumnWidth(0, rectClient.Width() - 2 * ::GetSystemMetrics(SM_CXEDGE) - ::GetSystemMetrics(SM_CXVSCROLL));
}

void CVSListBox::SetItemText(int iIndex, const CString& strText)
{
	if (GetSafeHwnd() == NULL || m_pWndList == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(m_pWndList);
	m_pWndList->SetItemText(iIndex, 0, strText);
}

BOOL CVSListBox::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_LBUTTONDOWN && m_pWndList != NULL && m_pWndList->GetEditControl() == NULL && m_pWndList->GetSafeHwnd() == CWnd::GetFocus()->GetSafeHwnd())
	{
		ASSERT_VALID(m_pWndList);

		m_ptClick = CPoint(-1, -1);

		CPoint ptClick = pMsg->pt;
		m_pWndList->ScreenToClient(&ptClick);

		UINT uFlags;
		int iItem = m_pWndList->HitTest(ptClick, &uFlags);
		if (iItem >= 0 &&(uFlags & LVHT_ONITEMLABEL))
		{
			UINT uiMask = LVIS_FOCUSED | LVIS_SELECTED;
			if ((m_pWndList->GetItemState(iItem, uiMask) & uiMask) == uiMask)
			{
				// Secondary click on selected item:
				m_ptClick = ptClick;

				SetCapture();
				return TRUE;
			}
		}
	}
	else if (pMsg->message == WM_LBUTTONUP && m_ptClick != CPoint(-1, -1))
	{
		ASSERT_VALID(m_pWndList);

		ReleaseCapture();

		CPoint ptClick = pMsg->pt;
		m_pWndList->ScreenToClient(&ptClick);

		int iItem = m_pWndList->HitTest(ptClick);

		BOOL bEditItem = iItem >= 0 && (abs(ptClick.x - m_ptClick.x) < ::GetSystemMetrics(SM_CXDRAG) && abs(ptClick.y - m_ptClick.y) < ::GetSystemMetrics(SM_CYDRAG));

		m_ptClick = CPoint(-1, -1);

		if (bEditItem)
		{
			EditItem(iItem);
		}

		return TRUE;
	}

	return CVSListBoxBase::PreTranslateMessage(pMsg);
}



