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

#include "afxtoolbarbuttoncustomizedialog.h"
#include "afximageeditordialog.h"
#include "afxtoolbarimages.h"
#include "afxtoolbarbutton.h"
#include "afxtoolbar.h"
#include "afxusertoolsmanager.h"
#include "afxusertool.h"
#include "afxglobals.h"
#include "afxtoolbarscustomizedialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CMFCToolBarsCustomizeDialog* g_pWndCustomize;

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarButtonCustomizeDialog dialog

CMFCToolBarButtonCustomizeDialog::CMFCToolBarButtonCustomizeDialog(CMFCToolBarButton* pButton, CMFCToolBarImages* pImages, CWnd* pParent, int iStartImage, BOOL bMenuMode) :
	CDialog(CMFCToolBarButtonCustomizeDialog::IDD, pParent), m_pButton(pButton), m_pImages(pImages), m_iStartImage(iStartImage), m_bMenuMode(bMenuMode), m_pUserTool(NULL)
{
	ASSERT_VALID(m_pButton);

	if (afxUserToolsManager != NULL)
	{
		m_pUserTool = afxUserToolsManager->FindTool(m_pButton->m_nID);
		if (m_pUserTool != NULL)
		{
			ASSERT_VALID(m_pUserTool);
		}
	}

	m_bUserButton = pButton->m_bUserButton || (m_pUserTool == NULL && CMFCToolBar::GetDefaultImage(m_pButton->m_nID) < 0);

	m_iSelImage = pButton->GetImage();
	m_bImage = pButton->m_bImage;

	if (m_bMenuMode && afxCommandManager->IsMenuItemWithoutImage(pButton->m_nID))
	{
		m_bImage = FALSE;
	}

	if (m_bMenuMode || m_pButton->m_bTextBelow)
	{
		m_bText = TRUE;
	}
	else
	{
		m_bText = pButton->m_bText;
	}

	//{{AFX_DATA_INIT(CMFCToolBarButtonCustomizeDialog)
	m_strButtonText = _T("");
	m_strButtonDescr = _T("");
	//}}AFX_DATA_INIT
}

CMFCToolBarButtonCustomizeDialog::~CMFCToolBarButtonCustomizeDialog()
{
	while (!m_Buttons.IsEmpty())
	{
		delete m_Buttons.RemoveHead();
	}
}

void CMFCToolBarButtonCustomizeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCToolBarButtonCustomizeDialog)
	DDX_Control(pDX, IDC_AFXBARRES_DEFAULT_IMAGE, m_wndDefautImageBtn);
	DDX_Control(pDX, IDC_AFXBARRES_USER_IMAGE, m_wndUserImageBtn);
	DDX_Control(pDX, IDC_AFXBARRES_DEFAULT_IMAGE_AREA, m_wndDefaultImageArea);
	DDX_Control(pDX, IDC_AFXBARRES_BUTTON_TEXT, m_wndButtonText);
	DDX_Control(pDX, IDC_AFXBARRES_ADD_IMAGE, m_wndAddImage);
	DDX_Control(pDX, IDC_AFXBARRES_IMAGE_LIST, m_wndButtonList);
	DDX_Control(pDX, IDC_AFXBARRES_EDIT_IMAGE, m_wndEditImage);
	DDX_Text(pDX, IDC_AFXBARRES_BUTTON_TEXT, m_strButtonText);
	DDX_Text(pDX, IDC_AFXBARRES_BUTTON_DESCR, m_strButtonDescr);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCToolBarButtonCustomizeDialog, CDialog)
	//{{AFX_MSG_MAP(CMFCToolBarButtonCustomizeDialog)
	ON_BN_CLICKED(IDC_AFXBARRES_ADD_IMAGE, &CMFCToolBarButtonCustomizeDialog::OnAddImage)
	ON_BN_CLICKED(IDC_AFXBARRES_EDIT_IMAGE, &CMFCToolBarButtonCustomizeDialog::OnEditImage)
	ON_BN_CLICKED(IDC_AFXBARRES_IMAGE_LIST, &CMFCToolBarButtonCustomizeDialog::OnImageList)
	ON_BN_CLICKED(IDC_AFXBARRES_IMAGE, &CMFCToolBarButtonCustomizeDialog::OnImage)
	ON_BN_CLICKED(IDC_AFXBARRES_IMAGE_TEXT, &CMFCToolBarButtonCustomizeDialog::OnImageText)
	ON_BN_CLICKED(IDC_AFXBARRES_TEXT, &CMFCToolBarButtonCustomizeDialog::OnText)
	ON_BN_CLICKED(IDC_AFXBARRES_USER_IMAGE, &CMFCToolBarButtonCustomizeDialog::OnUserImage)
	ON_BN_CLICKED(IDC_AFXBARRES_DEFAULT_IMAGE, &CMFCToolBarButtonCustomizeDialog::OnDefaultImage)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarButtonCustomizeDialog message handlers

void CMFCToolBarButtonCustomizeDialog::OnAddImage()
{
	ENSURE(m_pImages != NULL);
	ASSERT_VALID(m_pImages);

	CSize sizeImage = m_pImages->GetImageSize();

	try
	{
		CClientDC dc(&m_wndButtonList);
		CBitmap bitmap;
		CDC memDC;

		memDC.CreateCompatibleDC(&dc);

		if (!bitmap.CreateCompatibleBitmap(&dc, sizeImage.cx, sizeImage.cy))
		{
			AfxMessageBox(IDP_AFXBARRES_CANNT_CREATE_IMAGE);
			return;
		}

		CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

		CRect rect(0, 0, sizeImage.cx, sizeImage.cy);
		memDC.FillRect(CRect(0, 0, sizeImage.cx, sizeImage.cy), &afxGlobalData.brBtnFace);

		memDC.SelectObject(pOldBitmap);

		BITMAP bmp;
		::GetObject(m_pImages->GetImageWell(), sizeof(BITMAP), (LPVOID)&bmp);

		if (g_pWndCustomize != NULL)
		{
			ASSERT_VALID(g_pWndCustomize);

			if (!g_pWndCustomize->OnEditToolbarMenuImage(this, bitmap, bmp.bmBitsPixel))
			{
				return;
			}
		}
		else
		{
			CMFCImageEditorDialog dlg(&bitmap, this, bmp.bmBitsPixel);
			if (dlg.DoModal() != IDOK)
			{
				return;
			}
		}

		int iImageIndex = m_pImages->AddImage((HBITMAP) bitmap);
		if (iImageIndex < 0)
		{
			AfxMessageBox(IDP_AFXBARRES_CANNT_CREATE_IMAGE);
			return;
		}

		RebuildImageList();
		m_wndButtonList.SelectButton(iImageIndex);
	}
	catch(...)
	{
		AfxMessageBox(IDP_AFXBARRES_INTERLAL_ERROR);
	}
}

void CMFCToolBarButtonCustomizeDialog::OnEditImage()
{
	ENSURE(m_pImages != NULL);
	ENSURE(m_iSelImage >= 0);

	CSize sizeImage = m_pImages->GetImageSize();

	try
	{
		CClientDC dc(&m_wndButtonList);
		CBitmap bitmap;
		CDC memDC;
		memDC.CreateCompatibleDC(&dc);

		if (!bitmap.CreateCompatibleBitmap(&dc, sizeImage.cx, sizeImage.cy))
		{
			return;
		}

		const COLORREF clrGrayStd = RGB(192, 192, 192);

		CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);
		COLORREF clrTransparent = m_pImages->SetTransparentColor(clrGrayStd);

		memDC.FillSolidRect(CRect(0, 0, sizeImage.cx, sizeImage.cy), clrGrayStd);

		CAfxDrawState ds;
		if (!m_pImages->PrepareDrawImage(ds))
		{
			return;
		}

		m_pImages->Draw(&memDC, 0, 0, m_iSelImage);
		m_pImages->EndDrawImage(ds);

		m_pImages->SetTransparentColor(clrTransparent);

		memDC.SelectObject(pOldBitmap);

		BITMAP bmp;
		::GetObject(m_pImages->GetImageWell(), sizeof(BITMAP), (LPVOID)&bmp);

		if (g_pWndCustomize != NULL)
		{
			ASSERT_VALID(g_pWndCustomize);

			if (!g_pWndCustomize->OnEditToolbarMenuImage(this, bitmap, bmp.bmBitsPixel))
			{
				return;
			}
		}
		else
		{
			CMFCImageEditorDialog dlg(&bitmap, this, bmp.bmBitsPixel);
			if (dlg.DoModal() != IDOK)
			{
				return;
			}
		}

		m_pImages->UpdateImage(m_iSelImage, (HBITMAP) bitmap);
		m_wndButtonList.Invalidate();
	}
	catch(...)
	{
		AfxMessageBox(IDP_AFXBARRES_INTERLAL_ERROR);
	}
}

void CMFCToolBarButtonCustomizeDialog::OnImageList()
{
	CMFCToolBarButton* pSelButton = m_wndButtonList.GetSelectedButton();
	m_iSelImage = (pSelButton == NULL) ? -1 : pSelButton->GetImage();

	m_wndEditImage.EnableWindow(m_iSelImage >= 0 && m_pImages != NULL && !m_pImages->IsReadOnly());
}

void CMFCToolBarButtonCustomizeDialog::OnImage()
{
	m_bImage = TRUE;
	m_bText = FALSE;

	EnableControls();
}

void CMFCToolBarButtonCustomizeDialog::OnImageText()
{
	m_bImage = TRUE;
	m_bText = TRUE;

	EnableControls();
}

void CMFCToolBarButtonCustomizeDialog::OnText()
{
	m_bImage = FALSE;
	m_bText = TRUE;

	EnableControls();
}

void CMFCToolBarButtonCustomizeDialog::OnOK()
{
	UpdateData();

	int iImage = m_iSelImage;
	if (!m_bUserButton)
	{
		iImage = m_pUserTool != NULL ? 0 : CMFCToolBar::GetDefaultImage(m_pButton->m_nID);
	}

	if (m_bImage && iImage < 0)
	{
		CString str;
		ENSURE(str.LoadString(IDP_AFXBARRES_IMAGE_IS_REQUIRED));

		AfxMessageBox(str);

		m_wndButtonList.SetFocus();
		return;
	}

	if (m_bText && m_strButtonText.IsEmpty())
	{
		CString str;
		ENSURE(str.LoadString(IDP_AFXBARRES_TEXT_IS_REQUIRED));

		AfxMessageBox(str);

		m_wndButtonText.SetFocus();
		return;
	}

	if (!m_pButton->m_bTextBelow)
	{
		m_pButton->m_bText = m_bText;
	}

	if (m_bMenuMode)
	{
		afxCommandManager->EnableMenuItemImage(m_pButton->m_nID, m_bImage, iImage);
	}
	else
	{
		m_pButton->m_bImage = m_bImage;
	}

	m_pButton->m_bUserButton = m_bUserButton;
	m_pButton->SetImage(iImage);
	m_pButton->m_strText = m_strButtonText;

	if (!m_strAccel.IsEmpty())
	{
		m_pButton->m_strText += _T('\t');
		m_pButton->m_strText += m_strAccel;
	}

	CDialog::OnOK();
}

BOOL CMFCToolBarButtonCustomizeDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWnd* pWndImage = GetDlgItem(IDC_AFXBARRES_IMAGE);
	ENSURE(pWndImage != NULL);

	CWnd* pWndImageText = GetDlgItem(IDC_AFXBARRES_IMAGE_TEXT);
	ENSURE(pWndImageText != NULL);

	if (AfxGetMainWnd() != NULL &&
		(AfxGetMainWnd()->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}

	if (m_pImages != NULL)
	{
		m_wndButtonList.SetImages(m_pImages);
		RebuildImageList();
		m_wndButtonList.SelectButton(m_iSelImage);
	}
	else
	{
		m_wndButtonList.EnableWindow(FALSE);
		m_wndUserImageBtn.EnableWindow(FALSE);

		if (m_iSelImage < 0)
		{
			pWndImage->EnableWindow(FALSE);
			pWndImageText->EnableWindow(FALSE);
		}
	}

	if (m_bUserButton && !m_pButton->IsLocked())
	{
		m_wndUserImageBtn.SetCheck(1);
	}
	else
	{
		m_wndDefautImageBtn.SetCheck(1);
	}

	if (m_bImage)
	{
		if (m_bText)
		{
			CheckDlgButton(IDC_AFXBARRES_IMAGE_TEXT, TRUE);
		}
		else
		{
			CheckDlgButton(IDC_AFXBARRES_IMAGE, TRUE);
		}
	}
	else
	{
		ENSURE(m_bText);
		CheckDlgButton(IDC_AFXBARRES_TEXT, TRUE);
		m_bText = TRUE;

		m_wndButtonList.EnableWindow(FALSE);
	}

	int iTabOffset = m_pButton->m_strText.Find(_T('\t'));
	if (iTabOffset >= 0)
	{
		m_strButtonText = m_pButton->m_strText.Left(iTabOffset);
		m_strAccel = m_pButton->m_strText.Mid(iTabOffset + 1);
	}
	else
	{
		m_strButtonText = m_pButton->m_strText;
	}

	CFrameWnd* pWndFrame = GetParentFrame();
	if (pWndFrame != NULL)
	{
		pWndFrame->GetMessageString(m_pButton->m_nID, m_strButtonDescr);
	}

	if (m_bMenuMode)
	{
		pWndImage->EnableWindow(FALSE);
	}

	if (m_pButton->m_bTextBelow)
	{
		pWndImage->EnableWindow(FALSE);
	}

	m_wndDefaultImageArea.GetClientRect(&m_rectDefaultImage);
	m_wndDefaultImageArea.MapWindowPoints(this, &m_rectDefaultImage);

	CSize sizePreview(16, 16);

	CMFCToolBarImages* pImages = CMFCToolBar::GetImages();
	if (pImages != NULL)
	{
		CSize sizeImage = pImages->GetImageSize();

		sizePreview.cx = min(sizePreview.cx, sizeImage.cx);
		sizePreview.cy = min(sizePreview.cy, sizeImage.cy);
	}

	m_rectDefaultImage.right = m_rectDefaultImage.left + sizePreview.cx;
	m_rectDefaultImage.bottom = m_rectDefaultImage.top + sizePreview.cy;

	EnableControls();
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMFCToolBarButtonCustomizeDialog::RebuildImageList()
{
	m_wndButtonList.RemoveButtons();

	while (!m_Buttons.IsEmpty())
	{
		delete m_Buttons.RemoveHead();
	}

	int iEnd = m_pImages->GetCount() - 1;
	for (int iImage = m_iStartImage; iImage <= iEnd; iImage ++)
	{
		CMFCToolBarButton* pButton = new CMFCToolBarButton;

		pButton->SetImage(iImage);

		m_wndButtonList.AddButton(pButton);
		m_Buttons.AddTail(pButton);
	}

	m_wndButtonList.Invalidate();
}

void CMFCToolBarButtonCustomizeDialog::EnableControls()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pButton);

	BOOL bLocked = m_pButton->IsLocked();

	m_wndButtonText.EnableWindow(m_bText &&(!m_bMenuMode || m_pUserTool == NULL) && !CMFCToolBar::m_bDisableLabelsEdit);

	m_wndButtonList.EnableWindow(m_bImage && m_pImages != NULL && m_bUserButton && !bLocked);
	m_wndAddImage.EnableWindow(m_bImage && m_pImages != NULL && m_bUserButton && !m_pImages->IsReadOnly() && !bLocked);
	m_wndEditImage.EnableWindow(m_bImage && m_pImages != NULL && m_iSelImage >= 0 && m_bUserButton && !m_pImages->IsReadOnly() && !bLocked);

	m_wndUserImageBtn.EnableWindow(m_bImage && m_pImages != NULL && !bLocked);

	m_wndDefautImageBtn.EnableWindow(m_pUserTool != NULL || (m_bImage && CMFCToolBar::GetDefaultImage(m_pButton->m_nID) >= 0) && !bLocked);

	InvalidateRect(&m_rectDefaultImage);
}

void CMFCToolBarButtonCustomizeDialog::OnUserImage()
{
	m_iSelImage = -1;
	m_bUserButton = TRUE;
	m_wndDefautImageBtn.SetCheck(0);
	EnableControls();

	m_wndButtonList.SelectButton(-1);
}

void CMFCToolBarButtonCustomizeDialog::OnDefaultImage()
{
	m_iSelImage = m_pButton->GetImage();
	m_bUserButton = FALSE;
	m_wndUserImageBtn.SetCheck(0);
	EnableControls();
}

void CMFCToolBarButtonCustomizeDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (m_pUserTool != NULL)
	{
		m_pUserTool->DrawToolIcon(&dc, m_rectDefaultImage);
		return;
	}

	if (m_pButton->IsLocked())
	{
		BOOL bText = m_pButton->m_bText;
		BOOL bImage = m_pButton->m_bImage;

		m_pButton->m_bText = FALSE;
		m_pButton->m_bImage = TRUE;

		m_pButton->OnDraw(&dc, m_rectDefaultImage, NULL, TRUE, FALSE, FALSE, FALSE, FALSE);

		m_pButton->m_bText = bText;
		m_pButton->m_bImage = bImage;

		return;
	}

	int iImage = CMFCToolBar::GetDefaultImage(m_pButton->m_nID);
	if (iImage < 0 || !m_bImage)
	{
		return;
	}

	CMFCToolBarImages* pImages = CMFCToolBar::GetImages();
	ENSURE(pImages != NULL);

	CAfxDrawState ds;
	pImages->PrepareDrawImage(ds, m_rectDefaultImage.Size());

	pImages->Draw(&dc, m_rectDefaultImage.left, m_rectDefaultImage.top, iImage);
	pImages->EndDrawImage(ds);
}


