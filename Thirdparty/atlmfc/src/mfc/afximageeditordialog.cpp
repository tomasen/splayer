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
#include <afxpriv.h>
#include "afxribbonres.h"
#include "afxglobals.h"
#include "afximageeditordialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static void __stdcall Create16ColorsStdPalette(CPalette& pal);

/////////////////////////////////////////////////////////////////////////////
// CMFCImageEditorDialog dialog

#pragma warning(disable : 4355)

CMFCImageEditorDialog::CMFCImageEditorDialog(CBitmap* pBitmap, CWnd* pParent /*=NULL*/, int nBitsPixel /* = -1 */) :
	CDialogEx(CMFCImageEditorDialog::IDD, pParent), m_pBitmap(pBitmap), m_wndLargeDrawArea(this)
{
	ASSERT_VALID(m_pBitmap);

	BITMAP bmp;
	m_pBitmap->GetBitmap(&bmp);

	m_sizeImage = CSize(bmp.bmWidth, bmp.bmHeight);

	m_nBitsPixel = (nBitsPixel == -1) ? bmp.bmBitsPixel : nBitsPixel;
	ASSERT(m_nBitsPixel >= 4); // Monochrome bitmaps are not supported
}

#pragma warning(default : 4355)

void CMFCImageEditorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCImageEditorDialog)
	DDX_Control(pDX, IDC_AFXBARRES_COLORS, m_wndColorPickerLocation);
	DDX_Control(pDX, IDC_AFXBARRES_PALETTE, m_wndPaletteBarLocation);
	DDX_Control(pDX, IDC_AFXBARRES_PREVIEW_AREA, m_wndPreview);
	DDX_Control(pDX, IDC_AFXBARRES_DRAW_AREA, m_wndLargeDrawArea);
	//}}AFX_DATA_MAP
}

//{{AFX_MSG_MAP(CMFCImageEditorDialog)
BEGIN_MESSAGE_MAP(CMFCImageEditorDialog, CDialogEx)
	ON_WM_PAINT()
	ON_COMMAND(ID_AFX_TOOL_CLEAR, &CMFCImageEditorDialog::OnToolClear)
	ON_COMMAND(ID_AFX_TOOL_COPY, &CMFCImageEditorDialog::OnToolCopy)
	ON_COMMAND(ID_AFX_TOOL_PASTE, &CMFCImageEditorDialog::OnToolPaste)
	ON_UPDATE_COMMAND_UI(ID_AFX_TOOL_PASTE, &CMFCImageEditorDialog::OnUpdateToolPaste)
	ON_COMMAND(ID_AFX_TOOL_ELLIPSE, &CMFCImageEditorDialog::OnToolEllipse)
	ON_COMMAND(ID_AFX_TOOL_FILL, &CMFCImageEditorDialog::OnToolFill)
	ON_COMMAND(ID_AFX_TOOL_LINE, &CMFCImageEditorDialog::OnToolLine)
	ON_COMMAND(ID_AFX_TOOL_PEN, &CMFCImageEditorDialog::OnToolPen)
	ON_COMMAND(ID_AFX_TOOL_PICK, &CMFCImageEditorDialog::OnToolPick)
	ON_COMMAND(ID_AFX_TOOL_RECT, &CMFCImageEditorDialog::OnToolRect)
	ON_UPDATE_COMMAND_UI(ID_AFX_TOOL_ELLIPSE, &CMFCImageEditorDialog::OnUpdateToolEllipse)
	ON_UPDATE_COMMAND_UI(ID_AFX_TOOL_FILL, &CMFCImageEditorDialog::OnUpdateToolFill)
	ON_UPDATE_COMMAND_UI(ID_AFX_TOOL_LINE, &CMFCImageEditorDialog::OnUpdateToolLine)
	ON_UPDATE_COMMAND_UI(ID_AFX_TOOL_PEN, &CMFCImageEditorDialog::OnUpdateToolPen)
	ON_UPDATE_COMMAND_UI(ID_AFX_TOOL_PICK, &CMFCImageEditorDialog::OnUpdateToolPick)
	ON_UPDATE_COMMAND_UI(ID_AFX_TOOL_RECT, &CMFCImageEditorDialog::OnUpdateToolRect)
	ON_MESSAGE(WM_KICKIDLE, &CMFCImageEditorDialog::OnKickIdle)
	ON_COMMAND(IDC_AFXBARRES_COLORS, &CMFCImageEditorDialog::OnColors)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCImageEditorDialog message handlers

BOOL CMFCImageEditorDialog::OnInitDialog()
{
	const int iBorderWidth = 10;
	const int iBorderHeight = 5;
	const int iPreviewBorderSize = 4;

	CDialogEx::OnInitDialog();

	if (AfxGetMainWnd() != NULL && (AfxGetMainWnd()->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}

	m_wndLargeDrawArea.SetBitmap(m_pBitmap);

	//------------------------
	// Create the palette bar:
	//------------------------
	{
		CRect rectPaletteBarWnd;
		m_wndPaletteBarLocation.GetWindowRect (&rectPaletteBarWnd);

		CRect rectPaletteBar;
		m_wndPaletteBarLocation.GetClientRect(&rectPaletteBar);
		m_wndPaletteBarLocation.MapWindowPoints(this, &rectPaletteBar);
		rectPaletteBar.DeflateRect(2, 2);

		m_wndPaletteBar.EnableLargeIcons(FALSE);
		m_wndPaletteBar.Create(this);

		const UINT uiToolbarHotID = afxGlobalData.Is32BitIcons() ? IDR_AFXRES_PALETTE32 : 0;

		m_wndPaletteBar.LoadToolBar( IDR_AFXRES_PALETTE, 0, 0, TRUE /* Locked bar */, 0, 0, uiToolbarHotID);
		m_wndPaletteBar.SetPaneStyle(m_wndPaletteBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
		m_wndPaletteBar.SetPaneStyle(m_wndPaletteBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
		m_wndPaletteBar.SetBorders(iBorderWidth, iBorderHeight, iBorderWidth, iBorderHeight);

		const int nButtonWidth = m_wndPaletteBar.GetButtonSize().cx;
		m_wndPaletteBar.WrapToolBar(nButtonWidth * 3);

		const CSize szLayout = m_wndPaletteBar.CalcSize (FALSE);
		rectPaletteBar.bottom = rectPaletteBar.top + szLayout.cy + iBorderHeight * 2;

		m_wndPaletteBar.MoveWindow(rectPaletteBar);

		if (rectPaletteBar.Height () > rectPaletteBarWnd.Height ())
		{
			m_wndPaletteBarLocation.SetWindowPos (NULL, -1, -1, rectPaletteBarWnd.Width (), rectPaletteBar.Height () + iBorderHeight + 2,
													SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}

		m_wndPaletteBar.SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_wndPaletteBar.SetOwner(this);

		// All commands will be routed via this dialog, not via the parent frame:
		m_wndPaletteBar.SetRouteCommandsViaFrame(FALSE);
	}

	// Create color picker:
	{
		CRect rectColorBar;
		m_wndColorPickerLocation.GetClientRect(&rectColorBar);
		m_wndColorPickerLocation.MapWindowPoints(this, &rectColorBar);
		rectColorBar.DeflateRect(2, 2);

		m_wndColorBar.m_bInternal = TRUE;

		int nColumns = 4;

		// If bitmap has 256 or less colors, create 16 colors palette:
		CPalette pal;
		if (m_nBitsPixel <= 8)
		{
			Create16ColorsStdPalette(pal);
		}
		else
		{
			m_wndColorBar.EnableOtherButton(_T("Other"));

			nColumns = 5;
			m_wndColorBar.SetVertMargin(1);
			m_wndColorBar.SetHorzMargin(1);
		}

		m_wndColorBar.CreateControl(this, rectColorBar, IDC_AFXBARRES_COLORS, nColumns, m_nBitsPixel <= 8 ? &pal : NULL);
		m_wndColorBar.SetColor(RGB(0, 0, 0));
	}

	//---------------------
	// Define preview area:
	//---------------------
	m_wndPreview.GetClientRect(&m_rectPreviewImage);
	m_wndPreview.MapWindowPoints(this, &m_rectPreviewImage);

	m_rectPreviewImage.left = (m_rectPreviewImage.left + m_rectPreviewImage.right - m_sizeImage.cx) / 2;
	m_rectPreviewImage.right = m_rectPreviewImage.left + m_sizeImage.cx;

	m_rectPreviewImage.top = (m_rectPreviewImage.top + m_rectPreviewImage.bottom - m_sizeImage.cy) / 2;
	m_rectPreviewImage.bottom = m_rectPreviewImage.top + m_sizeImage.cy;

	m_rectPreviewFrame = m_rectPreviewImage;
	m_rectPreviewFrame.InflateRect(iPreviewBorderSize, iPreviewBorderSize);

	m_wndLargeDrawArea.m_rectParentPreviewArea = m_rectPreviewImage;
	m_wndLargeDrawArea.ModifyStyle(WS_TABSTOP, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMFCImageEditorDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	dc.FillRect(m_rectPreviewFrame, &afxGlobalData.brBtnFace);

	CBitmap* pbmOld = NULL;
	CDC dcMem;

	dcMem.CreateCompatibleDC(&dc);
	pbmOld = dcMem.SelectObject(m_pBitmap);

	dc.BitBlt(m_rectPreviewImage.left, m_rectPreviewImage.top, m_sizeImage.cx, m_sizeImage.cy, &dcMem, 0, 0, SRCCOPY);
	dc.Draw3dRect(&m_rectPreviewFrame, afxGlobalData.clrBtnHilite, afxGlobalData.clrBtnShadow);

	dcMem.SelectObject(pbmOld);
	dcMem.DeleteDC();
}

LRESULT CMFCImageEditorDialog::OnKickIdle(WPARAM, LPARAM)
{
	m_wndPaletteBar.OnUpdateCmdUI((CFrameWnd*) this, TRUE);
	return 0;
}

void CMFCImageEditorDialog::OnColors()
{
	COLORREF color = m_wndColorBar.GetColor();
	if (color == RGB(192, 192, 192))
	{
		color = afxGlobalData.clrBtnFace;
	}

	m_wndLargeDrawArea.SetColor(color);
}

BOOL CMFCImageEditorDialog::OnPickColor(COLORREF color)
{
	m_wndColorBar.SetColor(color);
	m_wndLargeDrawArea.SetColor(color);

	//-----------------------------------------
	// Move to the pen mode(not so good :-(!):
	//-----------------------------------------
	m_wndLargeDrawArea.SetMode(CMFCImagePaintArea::IMAGE_EDIT_MODE_PEN);
	return TRUE;
}

void CMFCImageEditorDialog::OnToolClear()
{
	CWindowDC dc(this);
	CDC 		memDC;

	memDC.CreateCompatibleDC(&dc);

	CBitmap* pOldBitmap = memDC.SelectObject(m_pBitmap);

	CRect rect(0, 0, m_sizeImage.cx, m_sizeImage.cy);
	memDC.FillRect(&rect, &afxGlobalData.brBtnFace);

	memDC.SelectObject(pOldBitmap);

	InvalidateRect(m_rectPreviewImage);
	m_wndLargeDrawArea.Invalidate();
}

void CMFCImageEditorDialog::OnToolCopy()
{
	if (m_pBitmap == NULL)
	{
		return;
	}

	try
	{
		CWindowDC dc(this);

		//----------------------
		// Create a bitmap copy:
		//----------------------
		CDC memDCDest;
		memDCDest.CreateCompatibleDC(NULL);

		CDC memDCSrc;
		memDCSrc.CreateCompatibleDC(NULL);

		CBitmap bitmapCopy;
		if (!bitmapCopy.CreateCompatibleBitmap(&dc, m_sizeImage.cx, m_sizeImage.cy))
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			return;
		}

		CBitmap* pOldBitmapDest = memDCDest.SelectObject(&bitmapCopy);
		CBitmap* pOldBitmapSrc = memDCSrc.SelectObject(m_pBitmap);

		memDCDest.BitBlt(0, 0, m_sizeImage.cx, m_sizeImage.cy, &memDCSrc, 0, 0, SRCCOPY);

		memDCDest.SelectObject(pOldBitmapDest);
		memDCSrc.SelectObject(pOldBitmapSrc);

		if (!OpenClipboard())
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			return;
		}

		if (!::EmptyClipboard())
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			::CloseClipboard();
			return;
		}


		HANDLE hclipData = ::SetClipboardData(CF_BITMAP, bitmapCopy.Detach());
		if (hclipData == NULL)
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			TRACE(_T("CMFCImageEditorDialog::Copy() error. Error code = %x\n"), GetLastError());
		}

		::CloseClipboard();
	}
	catch(...)
	{
		AfxMessageBox(IDP_AFXBARRES_INTERLAL_ERROR);
	}
}

void CMFCImageEditorDialog::OnToolPaste()
{
	COleDataObject data;
	if (!data.AttachClipboard())
	{
		AfxMessageBox(IDP_AFXBARRES_CANT_PASTE_BITMAP);
		return;
	}

	if (!data.IsDataAvailable(CF_BITMAP))
	{
		AfxMessageBox(IDP_AFXBARRES_CANT_PASTE_BITMAP);
		return;
	}

	tagSTGMEDIUM dataMedium;
	if (!data.GetData(CF_BITMAP, &dataMedium))
	{
		AfxMessageBox(IDP_AFXBARRES_CANT_PASTE_BITMAP);
		return;
	}

	CBitmap* pBmpClip = CBitmap::FromHandle(dataMedium.hBitmap);
	if (pBmpClip == NULL)
	{
		AfxMessageBox(IDP_AFXBARRES_CANT_PASTE_BITMAP);
		return;
	}

	BITMAP bmp;
	pBmpClip->GetBitmap(&bmp);

	CDC memDCDst;
	CDC memDCSrc;

	memDCSrc.CreateCompatibleDC(NULL);
	memDCDst.CreateCompatibleDC(NULL);

	CBitmap* pSrcOldBitmap = memDCSrc.SelectObject(pBmpClip);
	if (pSrcOldBitmap == NULL)
	{
		AfxMessageBox(IDP_AFXBARRES_CANT_PASTE_BITMAP);
		return;
	}

	CBitmap* pDstOldBitmap = memDCDst.SelectObject(m_pBitmap);
	if (pDstOldBitmap == NULL)
	{
		AfxMessageBox(IDP_AFXBARRES_CANT_PASTE_BITMAP);

		memDCSrc.SelectObject(pSrcOldBitmap);
		return;
	}

	memDCDst.FillRect(CRect(0, 0, m_sizeImage.cx, m_sizeImage.cy), &afxGlobalData.brBtnFace);

	int x = max(0, (m_sizeImage.cx - bmp.bmWidth) / 2);
	int y = max(0, (m_sizeImage.cy - bmp.bmHeight) / 2);

	int cx = min((m_sizeImage.cx - x), (bmp.bmWidth));
	int cy = min((m_sizeImage.cy - y), (bmp.bmHeight));

	if (cx > 0 && cy > 0)
	{
		CMFCToolBarImages::TransparentBlt(memDCDst.GetSafeHdc(), x, y, cx, cy, &memDCSrc, 0, 0, RGB(192, 192, 192));
	}

	memDCDst.SelectObject(pDstOldBitmap);
	memDCSrc.SelectObject(pSrcOldBitmap);

	InvalidateRect(m_rectPreviewImage);
	m_wndLargeDrawArea.Invalidate();
}

void CMFCImageEditorDialog::OnUpdateToolPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_BITMAP));
}

void CMFCImageEditorDialog::OnToolEllipse()
{
	SetMode(CMFCImagePaintArea::IMAGE_EDIT_MODE_ELLIPSE);
}

void CMFCImageEditorDialog::OnToolFill()
{
	SetMode(CMFCImagePaintArea::IMAGE_EDIT_MODE_FILL);
}

void CMFCImageEditorDialog::OnToolLine()
{
	SetMode(CMFCImagePaintArea::IMAGE_EDIT_MODE_LINE);
}

void CMFCImageEditorDialog::OnToolPen()
{
	SetMode(CMFCImagePaintArea::IMAGE_EDIT_MODE_PEN);
}

void CMFCImageEditorDialog::OnToolPick()
{
	SetMode(CMFCImagePaintArea::IMAGE_EDIT_MODE_COLOR);
}

void CMFCImageEditorDialog::OnToolRect()
{
	SetMode(CMFCImagePaintArea::IMAGE_EDIT_MODE_RECT);
}

void CMFCImageEditorDialog::OnUpdateToolEllipse(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetMode() == CMFCImagePaintArea::IMAGE_EDIT_MODE_ELLIPSE);
}

void CMFCImageEditorDialog::OnUpdateToolFill(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetMode() == CMFCImagePaintArea::IMAGE_EDIT_MODE_FILL);
}

void CMFCImageEditorDialog::OnUpdateToolLine(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetMode() == CMFCImagePaintArea::IMAGE_EDIT_MODE_LINE);
}

void CMFCImageEditorDialog::OnUpdateToolPen(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetMode() == CMFCImagePaintArea::IMAGE_EDIT_MODE_PEN);
}

void CMFCImageEditorDialog::OnUpdateToolPick(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetMode() == CMFCImagePaintArea::IMAGE_EDIT_MODE_COLOR);
}

void CMFCImageEditorDialog::OnUpdateToolRect(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetMode() == CMFCImagePaintArea::IMAGE_EDIT_MODE_RECT);
}

void __stdcall Create16ColorsStdPalette(CPalette& pal)
{
	const int nStdColorCount = 20;
	CPalette* pPalDefault = CPalette::FromHandle((HPALETTE) ::GetStockObject(DEFAULT_PALETTE));
	if (pPalDefault == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	const int nColors = 16;
	UINT nSize = sizeof(LOGPALETTE) +(sizeof(PALETTEENTRY) * nColors);
	LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

	pLP->palVersion = 0x300;
	pLP->palNumEntries = (USHORT) nColors;

	pal.CreatePalette(pLP);

	delete[] pLP;

	PALETTEENTRY palEntry;
	int iDest = 0;

	for (int i = 0; i < nStdColorCount; i++)
	{
		if (i < 8 || i >= 12)
		{
			pPalDefault->GetPaletteEntries(i, 1, &palEntry);
			pal.SetPaletteEntries(iDest++, 1, &palEntry);
		}
	}
}



