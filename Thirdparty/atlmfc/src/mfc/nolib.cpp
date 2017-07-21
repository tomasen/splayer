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

/////////////////////////////////////////////////////////////////////////////
// global data

// The following symbol used to force inclusion of this module
#ifdef _X86_
extern "C" { int _afxForceEXCLUDE; }
#else
extern "C" { int __afxForceEXCLUDE; }
#endif

// Win32 library excludes
#ifndef _AFXDLL
	#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER "d.lib")
	#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER "d.lib")
	#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER ".lib")
	#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER ".lib")
	#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER "ud.lib")
	#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER "ud.lib")
	#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER "u.lib")
	#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER "u.lib")
	#ifndef _UNICODE
		#pragma comment(linker, "/disallowlib:uafxcwd.lib")
		#pragma comment(linker, "/disallowlib:uafxcw.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:nafxcw.lib")
		#else
			#pragma comment(linker, "/disallowlib:nafxcwd.lib")
		#endif
	#else
		#pragma comment(linker, "/disallowlib:nafxcwd.lib")
		#pragma comment(linker, "/disallowlib:nafxcw.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:uafxcw.lib")
		#else
			#pragma comment(linker, "/disallowlib:uafxcwd.lib")
		#endif
	#endif
#else
	#pragma comment(linker, "/disallowlib:nafxcwd.lib")
	#pragma comment(linker, "/disallowlib:nafxcw.lib")
	#pragma comment(linker, "/disallowlib:uafxcwd.lib")
	#pragma comment(linker, "/disallowlib:uafxcw.lib")
	#ifndef _UNICODE
		#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER "ud.lib")
		#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER "ud.lib")
		#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER "u.lib")
		#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER "u.lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER ".lib")
			#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER ".lib")
		#else
			#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER "d.lib")
			#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER "d.lib")
		#endif
	#else
		#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER "d.lib")
		#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER "d.lib")
		#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER ".lib")
		#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER ".lib")
		#ifdef _DEBUG
			#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER "u.lib")
			#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER "u.lib")
		#else
			#pragma comment(linker, "/disallowlib:mfc" _MFC_FILENAME_VER "ud.lib")
			#pragma comment(linker, "/disallowlib:mfcs" _MFC_FILENAME_VER "ud.lib")
		#endif
	#endif
#endif

/////////////////////////////////////////////////////////////////////////////
