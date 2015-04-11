/* 
*	Copyright (C) 2003-2006 Gabest
*	http://www.gabest.org
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*   
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*   
*  You should have received a copy of the GNU General Public License
*  along with GNU Make; see the file COPYING.  If not, write to
*  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
*  http://www.gnu.org/copyleft/gpl.html
*
*/

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <atlcoll.h>

// TODO: reference additional headers your program requires here

#include <dshow.h>
#include <streams.h>
#include <dvdmedia.h>

#define GDCLWMVFILTER 1

#include <Wmsdk.h>
#include <comdef.h>


_COM_SMARTPTR_TYPEDEF(IAsyncReader, IID_IAsyncReader);
_COM_SMARTPTR_TYPEDEF(IMediaSample, IID_IMediaSample);
_COM_SMARTPTR_TYPEDEF(IWMReader, IID_IWMReader);
_COM_SMARTPTR_TYPEDEF(IWMReaderAdvanced2, IID_IWMReaderAdvanced2);
_COM_SMARTPTR_TYPEDEF(IWMMediaProps, IID_IWMMediaProps);
_COM_SMARTPTR_TYPEDEF(IWMProfile, IID_IWMProfile);
_COM_SMARTPTR_TYPEDEF(IWMStreamConfig, IID_IWMStreamConfig);
_COM_SMARTPTR_TYPEDEF(INSSBuffer, IID_INSSBuffer);
_COM_SMARTPTR_TYPEDEF(IWMHeaderInfo, IID_IWMHeaderInfo);
_COM_SMARTPTR_TYPEDEF(IPin, IID_IPin);



#include <string>
#include <map>
using namespace std;


#include "smartptr.h"
class DECLSPEC_UUID("6B6D0801-9ADA-11D0-A520-00A0D10129C0") MEDIASUBTYPE_ASF;

