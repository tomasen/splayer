// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

#include "stdafx.h"

#undef ATLINLINE
#define ATLINLINE
#ifdef _DEBUG
#include <atlbase.inl>
#endif

// C4740 - flow in or out of inline asm code suppresses global optimization
#pragma warning( disable: 4740 )

namespace  ATL
{

/////////////////////////////////////////////////////////////////////////////
// ComStdThunk

#ifdef _M_IX86
extern "C"
void __declspec(naked) __stdcall CComStdCallThunkHelper()
{
	__asm
	{
		mov eax, [esp+4];	// get pThunk
		mov edx, [eax+4];	// get the pThunk->pThis
		mov [esp+4], edx;	// replace pThunk with pThis
		mov eax, [eax+8];	// get pThunk->pfn
		jmp eax;			// jump pfn
	};
}

#endif

}
