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
#include "afxmenuhash.h"
#include "afxglobals.h"
#include "afxcommandmanager.h"
#include "afxvisualmanager.h"
#include "afxtoolbarbutton.h"
#include "afxtoolbar.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxframewndex.h"
#include "afxtoolbarmenubutton.h"
#include "afxcustomizebutton.h"
#include "afxkeyboardmanager.h"

IMPLEMENT_SERIAL(CMFCToolBarButton, CObject, VERSIONABLE_SCHEMA | 1)

CLIPFORMAT CMFCToolBarButton::m_cFormat = 0;
CString CMFCToolBarButton::m_strClipboardFormatName;
BOOL CMFCToolBarButton::m_bWrapText = TRUE;

static const int nTextMargin = 3;
static const int nSeparatorWidth = 8;
static const CString strDummyAmpSeq = _T("\001\001");

CList<UINT, UINT> CMFCToolBarButton::m_lstProtectedCommands;
BOOL CMFCToolBarButton::m_bUpdateImages = TRUE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCToolBarButton::CMFCToolBarButton()
{
	Initialize();
}

CMFCToolBarButton::CMFCToolBarButton(UINT uiID, int iImage, LPCTSTR lpszText, BOOL bUserButton, BOOL bLocked)
{
	Initialize();

	m_bLocked = bLocked;

	m_nID = uiID;
	m_bUserButton = bUserButton;
	SetImage(iImage);

	m_strText = (lpszText == NULL) ? _T("") : lpszText;

	if (m_nID != 0 && !m_bLocked)
	{
		if (m_bUserButton)
		{
			if (m_iUserImage != -1)
			{
				afxCommandManager->SetCmdImage(m_nID, m_iUserImage, TRUE);
			}
			else
			{
				m_iUserImage = afxCommandManager->GetCmdImage(m_nID, TRUE);
			}
		}
		else
		{
			if (m_iImage != -1)
			{
				afxCommandManager->SetCmdImage(m_nID, m_iImage, FALSE);
			}
			else
			{
				m_iImage = afxCommandManager->GetCmdImage(m_nID, FALSE);
			}
		}
	}
}

void CMFCToolBarButton::Initialize()
{
	m_nID = 0;
	m_nStyle = TBBS_BUTTON;
	m_iImage = -1;
	m_iUserImage = -1;
	m_bUserButton = FALSE;
	m_bDragFromCollection = FALSE;
	m_bText = FALSE;
	m_bImage = TRUE;
	m_bWrap = FALSE;
	m_bWholeText = TRUE;
	m_bLocked = FALSE;
	m_bIsHidden = FALSE;
	m_bTextBelow = FALSE;
	m_dwdItemData = 0;

	m_rect.SetRectEmpty();
	m_sizeText = CSize(0, 0);
	m_bDisableFill = FALSE;
	m_bExtraSize = FALSE;
	m_bHorz = TRUE;
	m_bVisible = TRUE;
	m_pWndParent = NULL;
}

CMFCToolBarButton::~CMFCToolBarButton()
{
}

void CMFCToolBarButton::CopyFrom(const CMFCToolBarButton& src)
{
	m_nID = src.m_nID;
	m_bLocked = src.m_bLocked;
	m_bUserButton = src.m_bUserButton;
	m_nStyle = src.m_nStyle;
	SetImage(src.m_bUserButton ? src.m_iUserImage : src.m_iImage);
	m_strText = src.m_strText;
	m_bText = src.m_bText;
	m_bImage = src.m_bImage;
	m_bWrap = src.m_bWrap;
	m_strTextCustom = src.m_strTextCustom;
	m_bVisible = src.m_bVisible;
	m_dwdItemData = src.m_dwdItemData;

	m_bDragFromCollection = FALSE;
}

void CMFCToolBarButton::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);

	if (ar.IsLoading())
	{
		int iImage;

		ar >> m_nID;
		ar >> m_nStyle;
		ar >> iImage;
		ar >> m_strText;
		ar >> m_bUserButton;
		ar >> m_bDragFromCollection;
		ar >> m_bText;
		ar >> m_bImage;
		ar >> m_bVisible;

		SetImage(iImage);
	}
	else
	{
		ar << m_nID;
		ar << m_nStyle;
		ar << GetImage();
		ar << m_strText;
		ar << m_bUserButton;
		ar << m_bDragFromCollection;
		ar << m_bText;
		ar << m_bImage;
		ar << m_bVisible;
	}
}

CLIPFORMAT __stdcall CMFCToolBarButton::GetClipboardFormat()
{
	if (m_cFormat == 0) // Not registered yet
	{
		CString strFormat = m_strClipboardFormatName;

		if (strFormat.IsEmpty())
		{
			strFormat.Format(_T("ToolbarButton%p"), static_cast<void *>(AfxGetMainWnd()));
			// Format should be unique per application
		}

		m_cFormat = (CLIPFORMAT)::RegisterClipboardFormat(strFormat);
		ENSURE(m_cFormat != NULL);
	}

	return m_cFormat;
}

CMFCToolBarButton* __stdcall CMFCToolBarButton::CreateFromOleData(COleDataObject* pDataObject)
{
	ENSURE(pDataObject != NULL);
	ENSURE(pDataObject->IsDataAvailable(CMFCToolBarButton::m_cFormat));

	CMFCToolBarButton* pButton = NULL;

	try
	{
		// Get file refering to clipboard data:
		CFile* pFile = pDataObject->GetFileData(GetClipboardFormat());
		if (pFile == NULL)
		{
			return FALSE;
		}

		// Connect the file to the archive and read the contents:
		CArchive ar(pFile, CArchive::load);

		// First, read run-time class information:
		CRuntimeClass* pClass = ar.ReadClass();
		ENSURE(pClass != NULL);

		if (pClass != NULL)
		{
			pButton = (CMFCToolBarButton*) pClass->CreateObject();
			ENSURE(pButton != NULL);

			if ((pButton != NULL) && (pButton->IsKindOf(RUNTIME_CLASS(CMFCToolBarButton))))
			{
				pButton->Serialize(ar);
			}
			else if (pButton != NULL)
			{
				delete pButton;
				pButton = NULL;
			}
		}

		ar.Close();
		delete pFile;

		return pButton;
	}
	catch(COleException* pEx)
	{
		TRACE(_T("CMFCToolBarButton::CreateFromOleData. OLE exception: %x\r\n"), pEx->m_sc);
		pEx->Delete();
	}
	catch(CArchiveException* pEx)
	{
		TRACE(_T("CMFCToolBarButton::CreateFromOleData. Archive exception\r\n"));
		pEx->Delete();
	}
	catch(CNotSupportedException *pEx)
	{
		TRACE(_T("CMFCToolBarButton::CreateFromOleData. \"Not Supported\" exception\r\n"));
		pEx->Delete();
	}

	if (pButton != NULL)
	{
		delete pButton;
	}

	return NULL;
}

void CMFCToolBarButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	m_bHorz = bHorz;

	// Fill button interior:
	FillInterior(pDC, rect, bHighlight);

	BOOL bHot = bHighlight;
	CSize sizeImage = (pImages == NULL) ? CSize(0, 0) : pImages->GetImageSize(TRUE);

	CUserTool* pUserTool = NULL;
	if (afxUserToolsManager != NULL && !m_bUserButton)
	{
		pUserTool = afxUserToolsManager->FindTool(m_nID);
	}

	CRect rectInternal = rect;
	CSize sizeExtra = m_bExtraSize ? CMFCVisualManager::GetInstance()->GetButtonExtraBorder() : CSize(0, 0);
	rectInternal.DeflateRect(sizeExtra.cx / 2, sizeExtra.cy / 2);

	int x = rectInternal.left;
	int y = rectInternal.top;

	int iTextLen = 0;

	CString strWithoutAmp = m_strText;
	strWithoutAmp.Replace(_T("&&"), strDummyAmpSeq);
	strWithoutAmp.Remove(_T('&'));
	strWithoutAmp.Replace(strDummyAmpSeq, _T("&"));

	CSize sizeText = pDC->GetTextExtent(strWithoutAmp);

	if (IsDrawText() && !(m_bTextBelow && bHorz))
	{
		int nMargin = IsDrawImage() ? 0 : nTextMargin;
		iTextLen = sizeText.cx + nMargin;
	}

	int dx = 0;
	int dy = 0;

	if (m_bTextBelow && bHorz)
	{
		ASSERT(bHorz);

		dx = rectInternal.Width();
		dy = sizeImage.cy + 2 * nTextMargin;
	}
	else
	{
		dx = bHorz ? rectInternal.Width() - iTextLen : rectInternal.Width();
		dy = bHorz ? rectInternal.Height() : rectInternal.Height() - iTextLen;
	}

	// determine offset of bitmap(centered within button)
	CPoint ptImageOffset;
	ptImageOffset.x = (dx - sizeImage.cx) / 2;
	ptImageOffset.y = (dy - sizeImage.cy) / 2;

	CPoint ptTextOffset(nTextMargin, nTextMargin);

	if (IsDrawText() && !(m_bTextBelow && bHorz))
	{
		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);

		if (bHorz)
		{
			ptImageOffset.x -= nTextMargin;
			ptTextOffset.y = (dy - tm.tmHeight - 1) / 2;
		}
		else
		{
			ptImageOffset.y -= nTextMargin;
			ptTextOffset.x = (dx - tm.tmHeight + 1) / 2;
		}
	}

	CPoint ptImageOffsetInButton(0, 0);
	BOOL bPressed = FALSE;

	BOOL bDrawImageShadow = bHighlight && !bCustomizeMode && !IsDroppedDown() && CMFCVisualManager::GetInstance()->IsShadowHighlightedImage() &&\
		!afxGlobalData.IsHighContrastMode() && ((m_nStyle & TBBS_PRESSED) == 0) && ((m_nStyle & TBBS_CHECKED) == 0) && ((m_nStyle & TBBS_DISABLED) == 0);

	if ((m_nStyle &(TBBS_PRESSED | TBBS_CHECKED)) && !bCustomizeMode &&
		!CMFCVisualManager::GetInstance()->IsShadowHighlightedImage() && CMFCVisualManager::GetInstance()->IsOffsetPressedButton())
	{
		// pressed in or checked
		ptImageOffset.Offset(1, 1);
		bPressed = TRUE;

		ptTextOffset.y ++;

		if (bHorz)
		{
			ptTextOffset.x ++;
		}
		else
		{
			ptTextOffset.x --;
		}
	}

	BOOL bFadeImage = !bHighlight && CMFCVisualManager::GetInstance()->IsFadeInactiveImage();
	BOOL bImageIsReady = FALSE;

	if ((m_nStyle & TBBS_PRESSED) || !(m_nStyle & TBBS_DISABLED) || bCustomizeMode)
	{
		if (IsDrawImage() && pImages != NULL)
		{
			if (pUserTool != NULL)
			{
				pUserTool->DrawToolIcon(pDC, CRect(CPoint(x + ptImageOffset.x, y + ptImageOffset.y), sizeImage));
			}
			else
			{
				CPoint pt = ptImageOffset;

				if (bDrawImageShadow)
				{
					pt.Offset(1, 1);

					pImages->Draw(pDC, x + pt.x, y + pt.y, GetImage(), FALSE, FALSE, FALSE, TRUE);
					pt.Offset(-2, -2);
				}

				pImages->Draw(pDC, x + pt.x, y + pt.y, GetImage(), FALSE, FALSE, FALSE, FALSE, bFadeImage);
			}
		}

		bImageIsReady = TRUE;
	}

	BOOL bDisabled = (bCustomizeMode && !IsEditable()) || (!bCustomizeMode &&(m_nStyle & TBBS_DISABLED));

	if (!bImageIsReady)
	{
		if (IsDrawImage() && pImages != NULL)
		{
			if (pUserTool != NULL)
			{
				pUserTool->DrawToolIcon(pDC, CRect(CPoint(x + ptImageOffset.x, y + ptImageOffset.y), sizeImage));
			}
			else
			{
				if (bDrawImageShadow)
				{
					ptImageOffset.Offset(1, 1);

					pImages->Draw(pDC, x + ptImageOffset.x, y + ptImageOffset.y, GetImage(), FALSE, FALSE, FALSE, TRUE);
					ptImageOffset.Offset(-2, -2);
				}

				pImages->Draw(pDC, x + ptImageOffset.x, y + ptImageOffset.y, GetImage(), FALSE, bDisabled && bGrayDisabledButtons, FALSE, FALSE, bFadeImage);
			}
		}
	}

	if ((m_bTextBelow && bHorz) || IsDrawText())
	{
		// Draw button's text:
		CMFCVisualManager::AFX_BUTTON_STATE state = CMFCVisualManager::ButtonsIsRegular;

		if (bHighlight)
		{
			state = CMFCVisualManager::ButtonsIsHighlighted;
		}
		else if (m_nStyle &(TBBS_PRESSED | TBBS_CHECKED))
		{
			// Pressed in or checked:
			state = CMFCVisualManager::ButtonsIsPressed;
		}

		COLORREF clrText = CMFCVisualManager::GetInstance()->GetToolbarButtonTextColor(this, state);

		pDC->SetTextColor(clrText);
		CString strText = m_strText;
		CRect rectText = rectInternal;
		UINT uiTextFormat = 0;

		if (m_bTextBelow && bHorz)
		{
			ASSERT(bHorz);

			ptTextOffset.y += sizeImage.cy + nTextMargin;
			uiTextFormat = DT_CENTER;

			if (m_bWrapText)
			{
				uiTextFormat |= DT_WORDBREAK;
			}

			rectText.left = (rectInternal.left + rectInternal.right - m_sizeText.cx) / 2 + ptTextOffset.x;
			rectText.right = (rectInternal.left + rectInternal.right + m_sizeText.cx) / 2;
		}
		else
		{
			if (IsDrawImage())
			{
				const int nExtra = CMFCToolBar::IsLargeIcons() ? 2 * nTextMargin : 0;

				if (bHorz)
				{
					ptTextOffset.x += sizeImage.cx + nExtra;
				}
				else
				{
					ptTextOffset.y += sizeImage.cy + nExtra;
				}

				rectText.left = x + ptTextOffset.x + nTextMargin;
			}
			else
			{
				rectText.left = x + nTextMargin + 1;
			}

			uiTextFormat = DT_SINGLELINE;
		}

		if (bHorz)
		{
			rectText.top += ptTextOffset.y;

			if (m_bTextBelow && m_bExtraSize)
			{
				rectText.OffsetRect(0, CMFCVisualManager::GetInstance()->GetButtonExtraBorder().cy / 2);
			}

			pDC->DrawText(strWithoutAmp, &rectText, uiTextFormat);
		}
		else
		{
			rectText = rectInternal;
			rectText.top += ptTextOffset.y;

			rectText.left = rectText.CenterPoint().x - sizeText.cy / 2;
			rectText.right = rectText.left + sizeText.cy;
			rectText.top += max(0, (rectText.Height() - sizeText.cx) / 2);

			rectText.SwapLeftRight();

			uiTextFormat = DT_NOCLIP | DT_SINGLELINE;

			strText.Replace(_T("&&"), strDummyAmpSeq);
			int iAmpIndex = strText.Find(_T('&')); // Find a SINGLE '&'
			strText.Remove(_T('&'));
			strText.Replace(strDummyAmpSeq, _T("&&"));

			if (iAmpIndex >= 0)
			{
				// Calculate underlined character position:
				CRect rectSubText;
				rectSubText.SetRectEmpty();
				CString strSubText = strText.Left(iAmpIndex + 1);

				pDC->DrawText(strSubText, &rectSubText, uiTextFormat | DT_CALCRECT);
				int y1 = rectSubText.right;

				rectSubText.SetRectEmpty();
				strSubText = strText.Left(iAmpIndex);

				pDC->DrawText(strSubText, &rectSubText, uiTextFormat | DT_CALCRECT);
				int y2 = rectSubText.right;

				pDC->DrawText(strWithoutAmp, &rectText, uiTextFormat);

				int xAmp = rect.CenterPoint().x - sizeText.cy / 2;

				CPen* pOldPen = NULL;
				CPen pen(PS_SOLID, 1, pDC->GetTextColor());

				if (pDC->GetTextColor() != 0)
				{
					pOldPen = pDC->SelectObject(&pen);
				}

				pDC->MoveTo(xAmp, rectText.top + y1);
				pDC->LineTo(xAmp, rectText.top + y2);

				if (pOldPen != NULL)
				{
					pDC->SelectObject(pOldPen);
				}
			}
			else
			{
				pDC->DrawText(strWithoutAmp, &rectText, uiTextFormat);
			}
		}
	}

	// Draw button border:
	if (!bCustomizeMode && HaveHotBorder() && bDrawBorder)
	{
		if (m_nStyle &(TBBS_PRESSED | TBBS_CHECKED))
		{
			// Pressed in or checked:
			CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rect, CMFCVisualManager::ButtonsIsPressed);
		}
		else if (bHot && !(m_nStyle & TBBS_DISABLED) && !(m_nStyle &(TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rect, CMFCVisualManager::ButtonsIsHighlighted);
		}
	}
}

SIZE CMFCToolBarButton::OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	ASSERT_VALID(pDC);

	if (!IsVisible())
		return CSize(0,0);

	CSize size = sizeDefault;

	if (m_nStyle & TBBS_SEPARATOR)
	{
		if (bHorz)
		{
			size.cx = m_iImage > 0 ? m_iImage : nSeparatorWidth;
		}
		else
		{
			size.cy = nSeparatorWidth;
		}
	}
	else
	{
		BOOL bHasImage = TRUE;

		if (!IsDrawImage() || GetImage() < 0)
		{
			bHasImage = FALSE;

			CSize sizeExtra = m_bExtraSize ? CMFCVisualManager::GetInstance()->GetButtonExtraBorder() : CSize(0, 0);

			if (bHorz)
			{
				size.cx = sizeExtra.cx;
			}
			else
			{
				size.cy = sizeExtra.cy;
			}
		}

		m_sizeText = CSize(0, 0);

		if (!m_strText.IsEmpty())
		{
			if (m_bTextBelow && bHorz)
			{
				// Try format text that it ocuppies no more tow lines an its
				// width less than 3 images:
				CRect rectText(0, 0, sizeDefault.cx * 3, sizeDefault.cy);

				UINT uiTextFormat = DT_CENTER | DT_CALCRECT;
				if (m_bWrapText)
				{
					uiTextFormat |= DT_WORDBREAK;
				}

				pDC->DrawText( m_strText, rectText, uiTextFormat);
				m_sizeText = rectText.Size();
				m_sizeText.cx += 2 * nTextMargin;

				size.cx = max(size.cx, m_sizeText.cx) + 4 * nTextMargin;
				size.cy += m_sizeText.cy + AFX_CY_BORDER;
			}
			else if (IsDrawText())
			{
				CString strWithoutAmp = m_strText;
				strWithoutAmp.Replace(_T("&&"), strDummyAmpSeq);
				strWithoutAmp.Remove(_T('&'));
				strWithoutAmp.Replace(strDummyAmpSeq, _T("&"));

				int nTextExtra = bHasImage ? 2 * nTextMargin : 3 * nTextMargin;
				int iTextLen = pDC->GetTextExtent(strWithoutAmp).cx + nTextExtra;

				if (bHorz)
				{
					size.cx += iTextLen;
				}
				else
				{
					size.cy += iTextLen;
				}
			}
		}
	}

	return size;
}

BOOL CMFCToolBarButton::PrepareDrag(COleDataSource& srcItem)
{
	if (!CanBeStored())
	{
		return TRUE;
	}

	try
	{
		CSharedFile globFile;
		CArchive ar(&globFile,CArchive::store);

		// Save run-time class information:
		CRuntimeClass* pClass = GetRuntimeClass();
		ENSURE(pClass != NULL);

		ar.WriteClass(pClass);

		// Save button context:
		Serialize(ar);
		ar.Close();

		srcItem.CacheGlobalData(GetClipboardFormat(), globFile.Detach());
	}
	catch(COleException* pEx)
	{
		TRACE(_T("CMFCToolBarButton::PrepareDrag. OLE exception: %x\r\n"), pEx->m_sc);
		pEx->Delete();
		return FALSE;
	}
	catch(CArchiveException* pEx)
	{
		TRACE(_T("CMFCToolBarButton::PrepareDrag. Archive exception\r\n"));
		pEx->Delete();
		return FALSE;
	}

	return TRUE;
}

void CMFCToolBarButton::SetImage(int iImage)
{
	if (m_nStyle & TBBS_SEPARATOR)
	{
		m_iImage = iImage; // Actualy, separator width!
		return;
	}

	if (m_bUserButton)
	{
		m_iUserImage = iImage;
	}
	else
	{
		m_iImage = iImage;
	}

	if (!m_bLocked)
	{
		if (m_nID != 0 && iImage != -1)
		{
			if (m_bUpdateImages || m_bUserButton)
			{
				afxCommandManager->SetCmdImage(m_nID, iImage, m_bUserButton);
			}
		}
		else if (m_nID != 0)
		{
			m_iImage = afxCommandManager->GetCmdImage(m_nID, FALSE);
			m_iUserImage = afxCommandManager->GetCmdImage(m_nID, TRUE);

			if (m_iImage == -1 && !m_bUserButton)
			{
				m_bUserButton = TRUE;
			}
			else if (m_iImage == -1 && m_bUserButton)
			{
				m_bUserButton = FALSE;
			}
		}
	}

	if ((!m_bUserButton && m_iImage < 0) ||
		(m_bUserButton && m_iUserImage < 0))
	{
		m_bImage = FALSE;
		m_bText  = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarButton diagnostics

#ifdef _DEBUG
void CMFCToolBarButton::AssertValid() const
{
	CObject::AssertValid();
}

void CMFCToolBarButton::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	CString strId;
	strId.Format(_T("%x"), m_nID);

	dc << "[" << strId << " " << m_strText << "]";
	dc << "\n";
}

#endif

int CMFCToolBarButton::OnDrawOnCustomizeList(CDC* pDC, const CRect& rect, BOOL bSelected)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	BOOL bText = m_bText;
	m_bText = FALSE;

	int iWidth = 0;

	CMFCToolBarImages* pImages = CMFCToolBar::GetImages();
	if (m_bUserButton)
	{
		pImages = CMFCToolBar::GetUserImages();
	}
	else
	{
		CMFCToolBarImages* pMenuImages = CMFCToolBar::GetMenuImages();
		if (pMenuImages != NULL && pMenuImages->GetCount() == pImages->GetCount())
		{
			pImages = pMenuImages;
		}
	}

	CUserTool* pUserTool = NULL;
	if (afxUserToolsManager != NULL && !m_bUserButton)
	{
		pUserTool = afxUserToolsManager->FindTool(m_nID);
	}

	CSize sizeMenuImage = CMFCToolBar::GetMenuImageSize();

	int nMargin = 3;
	CSize sizeButton = CSize( sizeMenuImage.cx + 2 * nMargin, sizeMenuImage.cy + 2 * nMargin);

	CRect rectFill = rect;

	if (bSelected && !CMFCVisualManager::GetInstance()->IsHighlightWholeMenuItem() && GetImage() >= 0 && pImages != NULL)
	{
		rectFill.left += sizeButton.cx;

		CRect rectLeftBtn = rect;
		rectLeftBtn.right = rectFill.left;

		CMFCVisualManager::GetInstance()->OnFillButtonInterior(pDC, this, rectLeftBtn, CMFCVisualManager::ButtonsIsHighlighted);

		CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectLeftBtn, CMFCVisualManager::ButtonsIsHighlighted);
	}

	COLORREF clrText = CMFCVisualManager::GetInstance()->OnFillCommandsListBackground(pDC, rectFill, bSelected);

	CRect rectText = rect;
	rectText.left += sizeMenuImage.cx + 2 * AFX_IMAGE_MARGIN + 2;

	iWidth = sizeButton.cx;

	// Draw button image:
	if (GetImage() >= 0 && pImages != NULL)
	{
		if (pUserTool != NULL)
		{
			CRect rectImage = rect;
			rectImage.right = rectImage.left + sizeButton.cx;

			pUserTool->DrawToolIcon(pDC, rectImage);
		}
		else
		{
			BOOL bFadeImage = !bSelected && CMFCVisualManager::GetInstance()->IsFadeInactiveImage();
			BOOL bDrawImageShadow = bSelected && CMFCVisualManager::GetInstance()->IsShadowHighlightedImage() && !afxGlobalData.IsHighContrastMode();

			CAfxDrawState ds;
			pImages->PrepareDrawImage(ds, CSize(0, 0), bFadeImage);

			CPoint pt = rect.TopLeft();
			pt.x += nMargin;
			pt.y += nMargin;

			if (bDrawImageShadow)
			{
				pt.Offset(1, 1);

				pImages->Draw(pDC, pt.x, pt.y, GetImage(), FALSE, FALSE, FALSE, TRUE);
				pt.Offset(-2, -2);
			}

			pImages->Draw(pDC, pt.x, pt.y, GetImage(), FALSE, FALSE, FALSE, FALSE, bFadeImage);

			pImages->EndDrawImage(ds);
		}
	}

	// Draw button text:
	if (!m_strText.IsEmpty())
	{
		COLORREF clrTextOld = pDC->SetTextColor(clrText);

		pDC->SetBkMode(TRANSPARENT);
		pDC->DrawText(m_strText, rectText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pDC->SetTextColor(clrTextOld);

		int iTextWidth = min(rectText.Width(), pDC->GetTextExtent(m_strText).cx);
		iWidth += iTextWidth;
	}

	m_bText = bText;
	return iWidth;
}

BOOL CMFCToolBarButton::OnToolHitTest(const CWnd* pWnd, TOOLINFO* pTI)
{
	CFrameWnd* pTopFrame = (pWnd == NULL) ? (CFrameWnd*) AfxGetMainWnd() : AFXGetTopLevelFrame(pWnd);

	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pTopFrame);
	if (pMainFrame != NULL)
	{
		return pMainFrame->OnMenuButtonToolHitTest(this, pTI);
	}
	else // Maybe, SDI frame...
	{
		CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pTopFrame);
		if (pFrame != NULL)
		{
			return pFrame->OnMenuButtonToolHitTest(this, pTI);
		}
		else // Maybe, OLE frame...
		{
			COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, pFrame);
			if (pOleFrame != NULL)
			{
				return pOleFrame->OnMenuButtonToolHitTest(this, pTI);
			}
		}
	}

	CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pTopFrame);
	if (pFrame != NULL)
	{
		return pFrame->OnMenuButtonToolHitTest(this, pTI);
	}

	return FALSE;
}

BOOL CMFCToolBarButton::ExportToMenuButton(CMFCToolBarMenuButton& menuButton) const
{
	// Text may be undefined, bring it from the tooltip :-(
	if (m_strText.IsEmpty() && m_nID != 0)
	{
		CString strMessage;
		int iOffset;

		if (strMessage.LoadString(m_nID) &&
			(iOffset = strMessage.Find(_T('\n'))) != -1)
		{
			menuButton.m_strText = strMessage.Mid(iOffset + 1);
		}
	}

	return TRUE;
}

void __stdcall CMFCToolBarButton::SetProtectedCommands(const CList<UINT, UINT>& lstCmds)
{
	m_lstProtectedCommands.RemoveAll();
	m_lstProtectedCommands.AddTail((CList<UINT,UINT>*) &lstCmds);
}

void __stdcall CMFCToolBarButton::SetClipboardFormatName(LPCTSTR lpszName)
{
	ENSURE(lpszName != NULL);
	ENSURE(m_cFormat == 0);

	m_strClipboardFormatName = lpszName;
}

void CMFCToolBarButton::FillInterior(CDC* pDC, const CRect& rect, BOOL bHighlight)
{
	if (m_bDisableFill)
	{
		return;
	}

	CMFCVisualManager::AFX_BUTTON_STATE state = CMFCVisualManager::ButtonsIsRegular;

	if (!CMFCToolBar::IsCustomizeMode() || CMFCToolBar::IsAltCustomizeMode() || m_bLocked)
	{
		if (bHighlight)
		{
			state = CMFCVisualManager::ButtonsIsHighlighted;
		}
		else if (m_nStyle &(TBBS_PRESSED | TBBS_CHECKED))
		{
			// Pressed in or checked:
			state = CMFCVisualManager::ButtonsIsPressed;
		}
	}

	CMFCVisualManager::GetInstance()->OnFillButtonInterior(pDC, this, rect, state);
}

void CMFCToolBarButton::ResetImageToDefault()
{
	if (m_bUserButton ||(int) m_nID <= 0)
	{
		return;
	}

	if (afxUserToolsManager != NULL && afxUserToolsManager->FindTool(m_nID) != NULL)
	{
		// User tool has its own image
		return;
	}

	BOOL bWasImage = m_bImage;

	int iImage = CMFCToolBar::GetDefaultImage(m_nID);
	if (iImage >= 0)
	{
		SetImage(iImage);
	}
	else if (bWasImage)
	{
		m_bImage = FALSE;
		m_bText = TRUE;

		if (m_strText.IsEmpty())
		{
			CString strMessage;
			int iOffset;

			if (strMessage.LoadString(m_nID) && (iOffset = strMessage.Find(_T('\n'))) != -1)
			{
				m_strText = strMessage.Mid(iOffset + 1);
			}
		}
	}
}

BOOL CMFCToolBarButton::CompareWith(const CMFCToolBarButton& other) const
{
	return m_nID == other.m_nID;
}

void CMFCToolBarButton::OnChangeParentWnd(CWnd* pWndParent)
{
	m_bExtraSize = FALSE;
	m_pWndParent = pWndParent;

	if (pWndParent == NULL)
	{
		return;
	}

	CMFCToolBar* pParentBar = DYNAMIC_DOWNCAST(CMFCToolBar, pWndParent);
	if (pParentBar != NULL && pParentBar->IsButtonExtraSizeAvailable())
	{
		m_bExtraSize = TRUE;
	}
}

BOOL CMFCToolBarButton::IsFirstInGroup() const
{
	ASSERT_VALID(this);

	CMFCToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, m_pWndParent);
	if (pToolBar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pToolBar);

	CMFCCustomizeButton* pCustomizeButton = pToolBar->GetCustomizeButton();
	if (this == pCustomizeButton)
	{
		return FALSE;
	}

	BOOL bIsFirstInGroup = TRUE;

	for (int i = 0; i < pToolBar->GetCount(); i++)
	{
		CMFCToolBarButton* pButton = pToolBar->GetButton(i);
		ASSERT_VALID(pButton);

		if (pButton == this)
		{
			return bIsFirstInGroup;
		}

		if (pButton->IsVisible())
		{
			bIsFirstInGroup = pButton->m_bWrap ||(pButton->m_nStyle & TBBS_SEPARATOR) || pButton->GetHwnd() != NULL;
		}
	}

	return FALSE;
}

BOOL CMFCToolBarButton::IsLastInGroup() const
{
	ASSERT_VALID(this);

	CMFCToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, m_pWndParent);
	if (pToolBar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pToolBar);

	CMFCCustomizeButton* pCustomizeButton = pToolBar->GetCustomizeButton();
	if (this == pCustomizeButton)
	{
		return FALSE;
	}

	BOOL bIsLastInGroup = TRUE;

	int nCount = pToolBar->GetCount();
	if (pCustomizeButton != NULL)
	{
		nCount--;
	}

	for (int i = nCount - 1; i >= 0; i--)
	{
		CMFCToolBarButton* pButton = pToolBar->GetButton(i);
		ASSERT_VALID(pButton);

		if (pButton == this)
		{
			return bIsLastInGroup || pButton->m_bWrap;
		}

		if (pButton->IsVisible())
		{
			bIsLastInGroup = (pButton->m_nStyle & TBBS_SEPARATOR) || pButton->GetHwnd() != NULL;
		}
	}

	return FALSE;
}

BOOL CMFCToolBarButton::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pParent);

	data.Clear();
	CString strText = m_strText;
	if (strText.IsEmpty() && m_nID != 0 && m_nID != (UINT) -1)
	{
		TCHAR szFullText[256];
		CString strTipText;

		if (AfxLoadString(m_nID, szFullText) && AfxExtractSubString(strTipText, szFullText, 1, '\n'))
		{
			strText = strTipText;
		}
	}

	data.m_strAccName = strText;
	data.m_strAccName.Remove(_T('&'));
	data.m_strAccDefAction = _T("Press");

	CFrameWnd* pFrame  = pParent->GetParentFrame();
	if (pFrame != NULL && pFrame->GetSafeHwnd() != NULL)
	{
		CString strDescr;
		pFrame->GetMessageString(m_nID, strDescr);
		data.m_strDescription = strDescr;
	}

	CWnd* pWnd = GetParentWnd();
	CFrameWnd* pParentFrame = pWnd->GetParentFrame();
	CString strLabel;
	if (pParentFrame != NULL && (CKeyboardManager::FindDefaultAccelerator(m_nID, strLabel, pParentFrame, TRUE) ||
		CKeyboardManager::FindDefaultAccelerator(m_nID, strLabel, pParentFrame->GetActiveFrame(), FALSE)))
	{
		data.m_strAccKeys = strLabel;
	}

	data.m_nAccHit = 1;
	data.m_nAccRole = ROLE_SYSTEM_PUSHBUTTON;
	data.m_bAccState = STATE_SYSTEM_FOCUSABLE;
	if (m_nStyle & TBBS_CHECKED)
	{
		data.m_bAccState |= STATE_SYSTEM_CHECKED;
	}

	if (m_nStyle & TBBS_DISABLED)
	{
		data.m_bAccState |= STATE_SYSTEM_UNAVAILABLE;
	}

	if (m_nStyle & TBBS_PRESSED)
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
	}
	else
	{
		data.m_bAccState |= STATE_SYSTEM_HOTTRACKED;
	}

	data.m_rectAccLocation = m_rect;
	pParent->ClientToScreen(&data.m_rectAccLocation);

	return TRUE;
}
