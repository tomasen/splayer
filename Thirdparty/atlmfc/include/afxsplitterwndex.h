// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#pragma once

#include "afxcontrolbarutil.h"

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

//
// CSplitterWndEx window

class CSplitterWndEx : public CSplitterWnd
{
// Construction
public:
	CSplitterWndEx();

// Overrides
	virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);

// Implementation
public:
	virtual ~CSplitterWndEx();

protected:
	//{{AFX_MSG(CSplitterWndEx)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
