/*
*  sqllitepp - test project
*
*/

#if !defined(AFX_STDAFX_H__C76533D6_6242_4BEB_8FD3_C6BE58F07224__INCLUDED_)
#define AFX_STDAFX_H__C76533D6_6242_4BEB_8FD3_C6BE58F07224__INCLUDED_

#pragma once


#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

// VS2005 changed result of OnNCHitTest
#if _MSC_VER >= 1400
#define HITTEST_RET LRESULT
#else
#define HITTEST_RET UINT
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxdlgs.h>

#include <afxdisp.h>
#include <afxole.h>

#include <Shlwapi.h>

#include <atlcoll.h>
#include <atlpath.h>


template <class T = CString, class S = CString>
class CAtlStringMap : public CAtlMap<S, T, CStringElementTraits<S> > {};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

//////////////////////////////////////////////////////////////////////////
//
//  ATL/WTL include files below.
//
//  The definition _WTL_NO_AUTOMATIC_NAMESPACE will disable
//  automatic WTL namespace declaration (using namespace WTL;), and
//  _ATL_NO_AUTOMATIC_NAMESPACE will disable automatic ATL namespace
//  declaration (using namespace ATL;), so that they won't be conflicting
//  with MFC class definitions (which made no use of namespace).
//
//  Additionally, we choose to use WTL::CString, because ATL's CString
//  will automatically use another class name when used with MFC. To do
//  this safely, we first undefine __ATLSTR_H__ then define 
//  _WTL_USE_CSTRING.
//
//  Finally, we choose to undefine __IStream_INTERFACE_DEFINED__ because
//  it's causing trouble when compiling IStream interface functions such as
//  ImageList_Read(Ex) and ImageList_Write(Ex). This will cause WTL to
//  loose these features.
//

#undef __ATLSTR_H__
#undef __IStream_INTERFACE_DEFINED__

#define _WTL_NEW_PAGE_NOTIFY_HANDLERS
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _WTL_NO_AUTOMATIC_NAMESPACE

#define _WTL_USE_CSTRING

#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>

#include <atlwin.h>
#include <atltypes.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atltheme.h>
#include <atlcrack.h>
#include <atlddx.h>

// we'll need the following stl stuff
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include <set>

#endif // !defined(AFX_STDAFX_H__C76533D6_6242_4BEB_8FD3_C6BE58F07224__INCLUDED_)
