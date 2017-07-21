// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Helper class to initialize a user dll compiler managed (_M_CEE defined).
// Used in afxdisp.h (check the pragma include).
// This helper code is added to the mfcmXX* import library.

// Note: This file may not contain any inclusion of or reference to any WIN32 or AFX types
// lest we get metadata mismatches when the user sets WIN32_WINNT differently.

/* redeclare here to avoid including any afx or windows headers */
#ifndef AFXAPI
#define AFXAPI __stdcall
#endif

#ifndef WINAPI
#define WINAPI __stdcall
#endif

#ifndef WINBASEAPI
#define WINBASEAPI __declspec(dllimport)
#endif

#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE HMODULE;

#define DLL_PROCESS_ATTACH   1    
#define DLL_PROCESS_DETACH   0    

typedef int BOOL;

extern "C"
{
	BOOL WINAPI RawDllMain(HINSTANCE hInstance, unsigned int dwReason, void *lpReserved);
}

class PostRawDllMain
{
public:
	PostRawDllMain()
	{
		RawDllMain(0, DLL_PROCESS_ATTACH, 0);
	}

	~PostRawDllMain()
	{
		RawDllMain(0, DLL_PROCESS_DETACH, 0);
	}
};

#pragma warning(push)
#pragma warning(disable:4378)
#pragma const_seg(".CRTMA$XCB")
#pragma init_seg(".CRTMP$XCB")
#pragma warning(pop)

const PostRawDllMain postRawDllMain;
