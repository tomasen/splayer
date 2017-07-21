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
#include <float.h>



/////////////////////////////////////////////////////////////////////////////
// Diagnostic Stream output for floating point numbers

#ifdef _DEBUG
CDumpContext& CDumpContext::operator<<(float f)
{
	char szBuffer[32];
	Checked::gcvt_s(szBuffer, 32, f, FLT_DIG);

	*this << szBuffer;
	return *this;
}

CDumpContext& CDumpContext::operator<<(double d)
{
	char szBuffer[32];
	Checked::gcvt_s(szBuffer, 32, d, DBL_DIG);

	*this << szBuffer;
	return *this;
}
#endif

/////////////////////////////////////////////////////////////////////////////
