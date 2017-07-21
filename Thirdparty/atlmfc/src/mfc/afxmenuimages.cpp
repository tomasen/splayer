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
#include "afxmenuimages.h"
#include "afxribbonres.h"
#include "afxglobals.h"

static const COLORREF clrTransparent = RGB(255, 0, 255);
static const int nImageWidth = 9;
static const int nImageHeight = 9;

CMFCToolBarImages CMenuImages::m_ImagesBlack;
CMFCToolBarImages CMenuImages::m_ImagesDkGray;
CMFCToolBarImages CMenuImages::m_ImagesGray;
CMFCToolBarImages CMenuImages::m_ImagesLtGray;
CMFCToolBarImages CMenuImages::m_ImagesWhite;
CMFCToolBarImages CMenuImages::m_ImagesBlack2;

BOOL __stdcall CMenuImages::Initialize()
{
	if (m_ImagesBlack.IsValid())
	{
		return TRUE;
	}

	m_ImagesBlack.SetImageSize(CSize(nImageWidth, nImageHeight));
	if (!m_ImagesBlack.Load(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_MENU_IMAGES24 : IDB_AFXBARRES_MENU_IMAGES))
	{
		TRACE(_T("CMenuImages. Can't load menu images %x\n"), IDB_AFXBARRES_MENU_IMAGES);
		return FALSE;
	}

	if (m_ImagesBlack.IsRTL())
	{
		m_ImagesBlack.Mirror();
	}

	m_ImagesBlack.SetTransparentColor(clrTransparent);

	CreateCopy(m_ImagesGray, RGB(128, 128, 128));
	CreateCopy(m_ImagesDkGray, RGB(72, 72, 72));
	CreateCopy(m_ImagesLtGray, RGB(192, 192, 192));
	CreateCopy(m_ImagesWhite, RGB(255, 255, 255));
	CreateCopy(m_ImagesBlack2, RGB(0, 0, 0));

	return TRUE;
}

void __stdcall CMenuImages::Draw(CDC* pDC, IMAGES_IDS id, const CPoint& ptImage, CMenuImages::IMAGE_STATE state, const CSize& sizeImage/* = CSize(0, 0)*/)
{
	if (!Initialize())
	{
		return;
	}

	CAfxDrawState ds;

	CMFCToolBarImages& images = (state == ImageBlack) ? m_ImagesBlack : (state == ImageGray) ? m_ImagesGray :
		(state == ImageDkGray) ? m_ImagesDkGray : (state == ImageLtGray) ? m_ImagesLtGray : (state == ImageWhite) ? m_ImagesWhite : m_ImagesBlack2;

	images.PrepareDrawImage(ds, sizeImage);
	images.Draw(pDC, ptImage.x, ptImage.y, id);
	images.EndDrawImage(ds);
}

void __stdcall CMenuImages::Draw(CDC* pDC, IMAGES_IDS id, const CRect& rectImage, CMenuImages::IMAGE_STATE state, const CSize& sizeImageDest/* = CSize(0, 0)*/)
{
	const CSize sizeImage = (sizeImageDest == CSize(0, 0)) ? Size() : sizeImageDest;

	CPoint ptImage(rectImage.left +(rectImage.Width() - sizeImage.cx) / 2 +((rectImage.Width() - sizeImage.cx) % 2),
		rectImage.top +(rectImage.Height() - sizeImage.cy) / 2 +((rectImage.Height() - sizeImage.cy) % 2));

	Draw(pDC, id, ptImage, state, sizeImageDest);
}

void __stdcall CMenuImages::CleanUp()
{
	if (m_ImagesBlack.GetCount() > 0)
	{
		m_ImagesBlack.Clear();
		m_ImagesGray.Clear();
		m_ImagesDkGray.Clear();
		m_ImagesLtGray.Clear();
		m_ImagesWhite.Clear();
		m_ImagesBlack2.Clear();
	}
}

void __stdcall CMenuImages::CreateCopy(CMFCToolBarImages& images, COLORREF clr)
{
	m_ImagesBlack.CopyTo(images);
	images.MapTo3dColors(TRUE, RGB(0, 0, 0), clr);
}

void __stdcall CMenuImages::SetColor(CMenuImages::IMAGE_STATE state, COLORREF color)
{
	Initialize();

	CMFCToolBarImages imagesTmp;

	imagesTmp.SetImageSize(m_ImagesBlack.GetImageSize());
	imagesTmp.Load(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_MENU_IMAGES24 : IDB_AFXBARRES_MENU_IMAGES);
	imagesTmp.SetTransparentColor(clrTransparent);

	if (imagesTmp.IsRTL())
	{
		CMFCToolBarImages::MirrorBitmap(imagesTmp.m_hbmImageWell, imagesTmp.GetImageSize().cx);
	}

	CMFCToolBarImages& images = (state == ImageBlack) ? m_ImagesBlack : (state == ImageGray) ? m_ImagesGray :
		(state == ImageDkGray) ? m_ImagesDkGray : (state == ImageLtGray) ? m_ImagesLtGray : (state == ImageWhite) ? m_ImagesWhite : m_ImagesBlack2;

	if (color != (COLORREF)-1)
	{
		imagesTmp.MapTo3dColors(TRUE, RGB(0, 0, 0), color);
	}

	images.Clear();
	imagesTmp.CopyTo(images);
}


