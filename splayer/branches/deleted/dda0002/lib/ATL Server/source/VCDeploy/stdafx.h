// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

//Windows 2000 and higher
#define _WIN32_WINNT 0x0500
#define WINVER 0x0500

#include <stdio.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include <atlbase.h>

// for adsi objects
#include <iads.h>
#include <Adshlp.h>

#include <msxml.h>
#include <atlstr.h>

#include <iwamreg.h>
#include <atlpath.h>
#include <atlcoll.h>
#include <atlpath.h>
#include <atlsafe.h>
#include <atlctl.h>
#include <atltime.h>
#include <atlfile.h>
#include <limits.h>
#include <atlsecurity.h>
