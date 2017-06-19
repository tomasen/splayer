// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

/////////////////////////////////////////////////////////////////////////////
// Fusion: Dlls that have WinSxS assemblies and several versions can be loaded
// into the same process, and called by mfc, need special wrappers to allow the
// same mfc dll to call the correct version, stored in the module state of the
// user exe/dll.

#ifndef __AFXCOMCTL32_H__
#define __AFXCOMCTL32_H__

#pragma once

#pragma warning(disable: 4127)  // conditional expression constant

/////////////////////////////////////////////////////////////////////////////
// (WinSxS/Manifest) API.



enum eActCtxResult { ActCtxFailed, ActCtxSucceeded, ActCtxNoFusion };
#if (_WIN32_WINNT >= 0x0500) || (_WIN32_FUSION >= 0x0100) || ISOLATION_AWARE_ENABLED

HANDLE AFXAPI AfxCreateActCtxW(PCACTCTXW pActCtx);
void AFXAPI AfxReleaseActCtx(HANDLE hActCtx);
BOOL AFXAPI AfxActivateActCtx(HANDLE hActCtx, ULONG_PTR *lpCookie);
BOOL AFXAPI AfxDeactivateActCtx(DWORD dwFlags, ULONG_PTR ulCookie);
#else
HANDLE AFXAPI AfxCreateActCtxW(void *pActCtx);
void AFXAPI AfxReleaseActCtx(HANDLE hActCtx);
BOOL AFXAPI AfxActivateActCtx(HANDLE hActCtx, ULONG_PTR *lpCookie);
BOOL AFXAPI AfxDeactivateActCtx(DWORD dwFlags, ULONG_PTR ulCookie);
#endif
BOOL AFXAPI AfxGetAmbientActCtx();
void AFXAPI AfxSetAmbientActCtx(BOOL bSet);
eActCtxResult AFXAPI AfxActivateActCtxWrapper(HANDLE hActCtx, ULONG_PTR *lpCookie);
/////////////////////////////////////////////////////////////////////////////

#pragma push_macro("AFX_ISOLATIONAWARE_COMMON_ACTIVATE")
#pragma push_macro("AFX_ISOLATIONAWARE_FUNC_ACTIVATE")
#pragma push_macro("AFX_ISOLATIONAWARE_FUNC_DEACTIVATE")
#pragma push_macro("AFX_ISOLATIONAWARE_FUNC")
#pragma push_macro("AFX_ISOLATIONAWARE_STATICLINK_FUNC")
#pragma push_macro("AFX_ISOLATIONAWARE_STATICLINK_PROC")
#pragma push_macro("AFX_ISOLATIONAWARE_PROC")

#define AFX_ISOLATIONAWARE_COMMON_ACTIVATE() \
		ULONG_PTR ulActCtxCookie = 0;\
		eActCtxResult eActResult = AfxActivateActCtxWrapper(AfxGetModuleState()->m_hActCtx, &ulActCtxCookie);\

#define AFX_ISOLATIONAWARE_FUNC_ACTIVATE(type, failure_retval) \
		AFX_ISOLATIONAWARE_COMMON_ACTIVATE() \
		type result=(failure_retval);\
		if (eActResult==ActCtxFailed)\
		{\
			return result;\
		}\
		__try {

#define AFX_ISOLATIONAWARE_FUNC_DEACTIVATE(failure_retval) \
}\
		__finally\
		{\
			if (eActResult!=ActCtxNoFusion)\
			{\
				const BOOL fPreserveLastError = (result == (failure_retval) );\
				const DWORD dwLastError = fPreserveLastError ? GetLastError() : NO_ERROR;\
				AfxDeactivateActCtx(0,ulActCtxCookie);\
				if (fPreserveLastError)\
				{\
					SetLastError(dwLastError);\
				}\
			}\
		}\
		return result;

#define AFX_ISOLATIONAWARE_STATICLINK_FUNC(type, name, params, args, failure_retval) \
	inline type AfxCtx##name##params \
	{ \
		AFX_ISOLATIONAWARE_FUNC_ACTIVATE(type, failure_retval)\
		result=name##args; \
		AFX_ISOLATIONAWARE_FUNC_DEACTIVATE(failure_retval)\
	}

#define AFX_ISOLATIONAWARE_PROC_ACTIVATE() \
		AFX_ISOLATIONAWARE_COMMON_ACTIVATE() \
		if (eActResult==ActCtxFailed)\
		{\
			return;\
		}\
		__try {

#define AFX_ISOLATIONAWARE_PROC_DEACTIVATE() \
}\
		__finally\
		{\
			if (eActResult!=ActCtxNoFusion)\
			{\
				AfxDeactivateActCtx(0,ulActCtxCookie);\
			}\
		}		

#define AFX_ISOLATIONAWARE_STATICLINK_PROC(name, params, args) \
	inline void AfxCtx##name##params \
	{ \
		AFX_ISOLATIONAWARE_PROC_ACTIVATE() \
		name##args; \
		AFX_ISOLATIONAWARE_PROC_DEACTIVATE() \
	}
	

#define AFX_PROC_PTR_TYPE(type, name_with_postfix, params) \
	struct name_with_postfix \
	{ \
		typedef type (WINAPI *Ptr)##params; \
		Ptr p; \
		name_with_postfix() : p(NULL) {} \
		void operator=(Ptr q) { p = q; } \
		Ptr operator->() { return p; } \
		operator Ptr() { return p; } \
		bool operator!() const { return !p; } \
	}

#define AFX_ISOLATIONAWARE_FUNC(type, name, params, args, failure_retval) \
	AFX_PROC_PTR_TYPE(type, name##_Type, params) m__##name; \
	\
	name##_Type GetProcAddress_##name() \
	{ \
		if (!m__##name) \
		{ \
			m__##name = (name##_Type::Ptr) ::GetProcAddress(GetModuleHandle(), #name); \
		} \
		return m__##name; \
	} \
	\
	type _##name##params \
	{ \
			AFX_ISOLATIONAWARE_FUNC_ACTIVATE(type, failure_retval)\
			GetProcAddress_##name(); \
			ENSURE(m__##name != NULL); \
			result=m__##name##args; \
			AFX_ISOLATIONAWARE_FUNC_DEACTIVATE(failure_retval)\
	}

#define AFX_ISOLATIONAWARE_PROC(name, params, args) \
	AFX_PROC_PTR_TYPE(void, name##_Type, params) m__##name; \
	\
	name##_Type GetProcAddress_##name() \
	{ \
		if (!m__##name) \
		{ \
			m__##name = (name##_Type::Ptr) ::GetProcAddress(GetModuleHandle(), #name); \
		} \
		return m__##name; \
	} \
	\
	void _##name##params \
	{ \
			AFX_ISOLATIONAWARE_PROC_ACTIVATE() \
			GetProcAddress_##name(); \
			ENSURE(m__##name != NULL); \
			m__##name##args; \
			AFX_ISOLATIONAWARE_PROC_DEACTIVATE() \
	}


#define AFX_COMCTL32_IF_EXISTS(proc) (afxComCtlWrapper->GetProcAddress_##proc() != NULL)

#if defined(_UNICODE)
#define AFX_COMCTL32_IF_EXISTS2(proc) (afxComCtlWrapper->GetProcAddress_##proc##W() != NULL)
#else
#define AFX_COMCTL32_IF_EXISTS2(proc) (afxComCtlWrapper->GetProcAddress_##proc##A() != NULL)
#endif

/////////////////////////////////////////////////////////////////////////////
// Base class for all dll wrappers
//
class CDllIsolationWrapperBase : public CNoTrackObject
{
public:
	HMODULE m_hModule;
	bool m_bFreeLib;
protected:
	CString m_strModuleName;
public:
	HMODULE GetModuleHandle()
	{
		if (m_hModule == NULL)
		{
			m_hModule = ::GetModuleHandle(m_strModuleName.GetString());
			if (m_hModule == NULL)
			{
				m_hModule = ::LoadLibrary(m_strModuleName.GetString());
				m_bFreeLib = m_hModule != NULL;
			}
		}
		return m_hModule;
	}

public:
	CDllIsolationWrapperBase()
	{
		CommonConstruct();
	}
	CDllIsolationWrapperBase(const CString& strModuleName) 
	: m_strModuleName(strModuleName)
	{
		CommonConstruct();
	}

	void CommonConstruct()
	{
		m_hModule  = NULL;
		m_bFreeLib = false;
	}
	virtual ~CDllIsolationWrapperBase()
	{ 
		m_bFreeLib && ::FreeLibrary(m_hModule);
	}

};

class CComCtlWrapper : public CDllIsolationWrapperBase
{
public:
	CComCtlWrapper() 
	: CDllIsolationWrapperBase(_T("comctl32.dll"))
	{
	}

public:
	AFX_ISOLATIONAWARE_PROC(InitCommonControls, (), ())
	AFX_ISOLATIONAWARE_FUNC(BOOL, InitCommonControlsEx, (LPINITCOMMONCONTROLSEX unnamed1), (unnamed1), FALSE)
	AFX_ISOLATIONAWARE_FUNC(HIMAGELIST, ImageList_Create, (int cx,int cy,UINT flags,int cInitial,int cGrow), (cx,cy,flags,cInitial,cGrow), NULL)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_Destroy, (HIMAGELIST himl), (himl), FALSE)
	AFX_ISOLATIONAWARE_FUNC(int, ImageList_GetImageCount, (HIMAGELIST himl), (himl), 0)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_SetImageCount, (HIMAGELIST himl,UINT uNewCount), (himl,uNewCount), FALSE)
	AFX_ISOLATIONAWARE_FUNC(int, ImageList_Add, (HIMAGELIST himl,HBITMAP hbmImage,HBITMAP hbmMask), (himl,hbmImage,hbmMask), -1)
	AFX_ISOLATIONAWARE_FUNC(int, ImageList_ReplaceIcon, (HIMAGELIST himl,int i,HICON hicon), (himl,i,hicon),-1)
	AFX_ISOLATIONAWARE_FUNC(COLORREF, ImageList_SetBkColor, (HIMAGELIST himl,COLORREF clrBk), (himl,clrBk),RGB(0,0,0))
	AFX_ISOLATIONAWARE_FUNC(COLORREF, ImageList_GetBkColor, (HIMAGELIST himl), (himl),RGB(0,0,0))
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_SetOverlayImage, (HIMAGELIST himl,int iImage,int iOverlay), (himl,iImage,iOverlay),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_Draw, (HIMAGELIST himl,int i,HDC hdcDst,int x,int y,UINT fStyle), (himl,i,hdcDst,x,y,fStyle),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_Replace, (HIMAGELIST himl,int i,HBITMAP hbmImage,HBITMAP hbmMask), (himl,i,hbmImage,hbmMask),FALSE)
	AFX_ISOLATIONAWARE_FUNC(int, ImageList_AddMasked, (HIMAGELIST himl,HBITMAP hbmImage,COLORREF crMask), (himl,hbmImage,crMask),-1)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_DrawEx, (HIMAGELIST himl,int i,HDC hdcDst,int x,int y,int dx,int dy,COLORREF rgbBk,COLORREF rgbFg,UINT fStyle), (himl,i,hdcDst,x,y,dx,dy,rgbBk,rgbFg,fStyle),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_DrawIndirect, (IMAGELISTDRAWPARAMS*pimldp), (pimldp),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_Remove, (HIMAGELIST himl,int i), (himl,i),FALSE)
	AFX_ISOLATIONAWARE_FUNC(HICON, ImageList_GetIcon, (HIMAGELIST himl,int i,UINT flags), (himl,i,flags),NULL)
	AFX_ISOLATIONAWARE_FUNC(HIMAGELIST, ImageList_LoadImageA, (HINSTANCE hi,LPCSTR lpbmp,int cx,int cGrow,COLORREF crMask,UINT uType,UINT uFlags), (hi,lpbmp,cx,cGrow,crMask,uType,uFlags),NULL)
	AFX_ISOLATIONAWARE_FUNC(HIMAGELIST, ImageList_LoadImageW, (HINSTANCE hi,LPCWSTR lpbmp,int cx,int cGrow,COLORREF crMask,UINT uType,UINT uFlags), (hi,lpbmp,cx,cGrow,crMask,uType,uFlags),NULL)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_Copy, (HIMAGELIST himlDst,int iDst,HIMAGELIST himlSrc,int iSrc,UINT uFlags), (himlDst,iDst,himlSrc,iSrc,uFlags),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_BeginDrag, (HIMAGELIST himlTrack,int iTrack,int dxHotspot,int dyHotspot), (himlTrack,iTrack,dxHotspot,dyHotspot),FALSE)
	AFX_ISOLATIONAWARE_PROC(ImageList_EndDrag, (), ())
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_DragEnter, (HWND hwndLock,int x,int y), (hwndLock,x,y),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_DragLeave, (HWND hwndLock), (hwndLock),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_DragMove, (int x,int y), (x,y),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_SetDragCursorImage, (HIMAGELIST himlDrag,int iDrag,int dxHotspot,int dyHotspot), (himlDrag,iDrag,dxHotspot,dyHotspot),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_DragShowNolock, (BOOL fShow), (fShow),FALSE)
	AFX_ISOLATIONAWARE_FUNC(HIMAGELIST, ImageList_GetDragImage, (POINT*ppt,POINT*pptHotspot), (ppt,pptHotspot),NULL)
	AFX_ISOLATIONAWARE_FUNC(HIMAGELIST, ImageList_Read, (IStream *pstm), (pstm),NULL)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_Write, (HIMAGELIST himl,IStream *pstm), (himl,pstm),FALSE)
	
	AFX_ISOLATIONAWARE_FUNC(HRESULT, ImageList_ReadEx, (DWORD dwFlags,IStream *pstm,REFIID riid,PVOID*ppv), (dwFlags,pstm,riid,ppv),S_OK)
	AFX_ISOLATIONAWARE_FUNC(HRESULT, ImageList_WriteEx, (HIMAGELIST himl,DWORD dwFlags,IStream *pstm), (himl,dwFlags,pstm),S_OK)
	
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_GetIconSize, (HIMAGELIST himl,int*cx,int*cy), (himl,cx,cy),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_SetIconSize, (HIMAGELIST himl,int cx,int cy), (himl,cx,cy),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, ImageList_GetImageInfo, (HIMAGELIST himl,int i,IMAGEINFO*pImageInfo), (himl,i,pImageInfo),FALSE)
	AFX_ISOLATIONAWARE_FUNC(HIMAGELIST, ImageList_Merge, (HIMAGELIST himl1,int i1,HIMAGELIST himl2,int i2,int dx,int dy), (himl1,i1,himl2,i2,dx,dy),NULL)
	AFX_ISOLATIONAWARE_FUNC(HIMAGELIST, ImageList_Duplicate, (HIMAGELIST himl), (himl),NULL)
	AFX_ISOLATIONAWARE_FUNC(HWND, CreateToolbarEx, (HWND hwnd,DWORD ws,UINT wID,int nBitmaps,HINSTANCE hBMInst,UINT_PTR wBMID,LPCTBBUTTON lpButtons,int iNumButtons,int dxButton,int dyButton,int dxBitmap,int dyBitmap,UINT uStructSize), (hwnd,ws,wID,nBitmaps,hBMInst,wBMID,lpButtons,iNumButtons,dxButton,dyButton,dxBitmap,dyBitmap,uStructSize),NULL)
	AFX_ISOLATIONAWARE_FUNC(HBITMAP, CreateMappedBitmap, (HINSTANCE hInstance,INT_PTR idBitmap,UINT wFlags,LPCOLORMAP lpColorMap,int iNumMaps), (hInstance,idBitmap,wFlags,lpColorMap,iNumMaps),NULL)
	AFX_ISOLATIONAWARE_PROC(DrawStatusTextA, (HDC hDC,LPRECT lprc,LPCSTR pszText,UINT uFlags), (hDC,lprc,pszText,uFlags))
	AFX_ISOLATIONAWARE_PROC(DrawStatusTextW, (HDC hDC,LPRECT lprc,LPCWSTR pszText,UINT uFlags), (hDC,lprc,pszText,uFlags))
	AFX_ISOLATIONAWARE_FUNC(HWND, CreateStatusWindowA, (long style,LPCSTR lpszText,HWND hwndParent,UINT wID), (style,lpszText,hwndParent,wID),NULL)
	AFX_ISOLATIONAWARE_FUNC(HWND, CreateStatusWindowW, (long style,LPCWSTR lpszText,HWND hwndParent,UINT wID), (style,lpszText,hwndParent,wID),NULL)
	AFX_ISOLATIONAWARE_PROC(MenuHelp, (UINT uMsg,WPARAM wParam,LPARAM lParam,HMENU hMainMenu,HINSTANCE hInst,HWND hwndStatus,UINT*lpwIDs), (uMsg,wParam,lParam,hMainMenu,hInst,hwndStatus,lpwIDs))
	AFX_ISOLATIONAWARE_FUNC(BOOL, ShowHideMenuCtl, (HWND hWnd,UINT_PTR uFlags,LPINT lpInfo), (hWnd,uFlags,lpInfo),FALSE)
	AFX_ISOLATIONAWARE_PROC(GetEffectiveClientRect, (HWND hWnd,LPRECT lprc,LPINT lpInfo), (hWnd,lprc,lpInfo))
	AFX_ISOLATIONAWARE_FUNC(BOOL, MakeDragList, (HWND hLB), (hLB),FALSE)
	AFX_ISOLATIONAWARE_PROC(DrawInsert, (HWND handParent,HWND hLB,int nItem), (handParent,hLB,nItem))
	AFX_ISOLATIONAWARE_FUNC(int, LBItemFromPt, (HWND hLB,POINT pt,BOOL bAutoScroll), (hLB,pt,bAutoScroll),-1)
	AFX_ISOLATIONAWARE_FUNC(HWND, CreateUpDownControl, (DWORD dwStyle,int x,int y,int cx,int cy,HWND hParent,int nID,HINSTANCE hInst,HWND hBuddy,int nUpper,int nLower,int nPos), (dwStyle,x,y,cx,cy,hParent,nID,hInst,hBuddy,nUpper,nLower,nPos),NULL)
	AFX_ISOLATIONAWARE_PROC(InitMUILanguage, (LANGID uiLang), (uiLang))
	AFX_ISOLATIONAWARE_FUNC(LANGID, GetMUILanguage, (), (),MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
	AFX_ISOLATIONAWARE_FUNC(HDSA, DSA_Create, (int cbItem,int cItemGrow), (cbItem,cItemGrow),NULL)
	AFX_ISOLATIONAWARE_FUNC(BOOL, DSA_Destroy, (HDSA hdsa), (hdsa),FALSE)
	AFX_ISOLATIONAWARE_PROC(DSA_DestroyCallback, (HDSA hdsa,PFNDSAENUMCALLBACK pfnCB,void*pData), (hdsa,pfnCB,pData))
	AFX_ISOLATIONAWARE_FUNC(PVOID, DSA_GetItemPtr, (HDSA hdsa,int i), (hdsa,i),NULL)
	AFX_ISOLATIONAWARE_FUNC(int, DSA_InsertItem, (HDSA hdsa,int i,void*pitem), (hdsa,i,pitem),-1)
	AFX_ISOLATIONAWARE_FUNC(HDPA, DPA_Create, (int cItemGrow), (cItemGrow),NULL)
	AFX_ISOLATIONAWARE_FUNC(BOOL, DPA_Destroy, (HDPA hdpa), (hdpa),FALSE)
	AFX_ISOLATIONAWARE_FUNC(PVOID, DPA_DeletePtr, (HDPA hdpa,int i), (hdpa,i),NULL)
	AFX_ISOLATIONAWARE_FUNC(BOOL, DPA_DeleteAllPtrs, (HDPA hdpa), (hdpa),FALSE)
	AFX_ISOLATIONAWARE_PROC(DPA_EnumCallback, (HDPA hdpa,PFNDPAENUMCALLBACK pfnCB,void*pData), (hdpa,pfnCB,pData))
	AFX_ISOLATIONAWARE_PROC(DPA_DestroyCallback, (HDPA hdpa,PFNDPAENUMCALLBACK pfnCB,void*pData), (hdpa,pfnCB,pData))
	AFX_ISOLATIONAWARE_FUNC(BOOL, DPA_SetPtr, (HDPA hdpa,int i,void*p), (hdpa,i,p),FALSE)
	AFX_ISOLATIONAWARE_FUNC(int, DPA_InsertPtr, (HDPA hdpa,int i,void*p), (hdpa,i,p),-1)
	AFX_ISOLATIONAWARE_FUNC(PVOID, DPA_GetPtr, (HDPA hdpa,INT_PTR i), (hdpa,i),NULL)
	AFX_ISOLATIONAWARE_FUNC(BOOL, DPA_Sort, (HDPA hdpa,PFNDPACOMPARE pfnCompare,LPARAM lParam), (hdpa,pfnCompare,lParam),FALSE)
	AFX_ISOLATIONAWARE_FUNC(int, DPA_Search, (HDPA hdpa,void*pFind,int iStart,PFNDPACOMPARE pfnCompare,LPARAM lParam,UINT options), (hdpa,pFind,iStart,pfnCompare,lParam,options),-1)
	AFX_ISOLATIONAWARE_FUNC(BOOL, Str_SetPtrW, (LPWSTR*ppsz,LPCWSTR psz), (ppsz,psz),FALSE)
	
	AFX_ISOLATIONAWARE_FUNC(BOOL, _TrackMouseEvent, (LPTRACKMOUSEEVENT lpEventTrack), (lpEventTrack),FALSE)
	
	AFX_ISOLATIONAWARE_FUNC(BOOL, FlatSB_EnableScrollBar, (HWND unnamed1,int unnamed2,UINT unnamed3), (unnamed1,unnamed2,unnamed3),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, FlatSB_ShowScrollBar, (HWND unnamed1,int code,BOOL unnamed2), (unnamed1,code,unnamed2),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, FlatSB_GetScrollRange, (HWND unnamed1,int code,LPINT unnamed2,LPINT unnamed3), (unnamed1,code,unnamed2,unnamed3),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, FlatSB_GetScrollInfo, (HWND unnamed1,int code,LPSCROLLINFO unnamed2), (unnamed1,code,unnamed2),FALSE)
	AFX_ISOLATIONAWARE_FUNC(int, FlatSB_GetScrollPos, (HWND unnamed1,int code), (unnamed1,code),0)
	AFX_ISOLATIONAWARE_FUNC(BOOL, FlatSB_GetScrollProp, (HWND unnamed1,int propIndex,LPINT unnamed2), (unnamed1,propIndex,unnamed2),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, FlatSB_GetScrollPropPtr, (HWND unnamed1,int propIndex,PINT_PTR unnamed2), (unnamed1,propIndex,unnamed2),FALSE)
	AFX_ISOLATIONAWARE_FUNC(int, FlatSB_SetScrollPos, (HWND unnamed1,int code,int pos,BOOL fRedraw), (unnamed1,code,pos,fRedraw),0)
	AFX_ISOLATIONAWARE_FUNC(int, FlatSB_SetScrollInfo, (HWND unnamed1,int code,LPSCROLLINFO unnamed2,BOOL fRedraw), (unnamed1,code,unnamed2,fRedraw),0)
	AFX_ISOLATIONAWARE_FUNC(int, FlatSB_SetScrollRange, (HWND unnamed1,int code,int min,int max,BOOL fRedraw), (unnamed1,code,min,max,fRedraw),0)
	AFX_ISOLATIONAWARE_FUNC(BOOL, FlatSB_SetScrollProp, (HWND unnamed1,UINT index,INT_PTR newValue,BOOL unnamed2), (unnamed1,index,newValue,unnamed2),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, InitializeFlatSB, (HWND unnamed1), (unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(HRESULT, UninitializeFlatSB, (HWND unnamed1), (unnamed1),S_OK)

	typedef LRESULT (CALLBACK *SUBCLASSPROC)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	AFX_ISOLATIONAWARE_FUNC(BOOL, SetWindowSubclass, (HWND hWnd,SUBCLASSPROC pfnSubclass,UINT_PTR uIdSubclass,DWORD_PTR dwRefData), (hWnd,pfnSubclass,uIdSubclass,dwRefData),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, GetWindowSubclass, (HWND hWnd,SUBCLASSPROC pfnSubclass,UINT_PTR uIdSubclass,DWORD_PTR*pdwRefData), (hWnd,pfnSubclass,uIdSubclass,pdwRefData),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL, RemoveWindowSubclass, (HWND hWnd,SUBCLASSPROC pfnSubclass,UINT_PTR uIdSubclass), (hWnd,pfnSubclass,uIdSubclass),FALSE)
	
	AFX_ISOLATIONAWARE_FUNC(LRESULT, DefSubclassProc, (HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam), (hWnd,uMsg,wParam,lParam),0)
	AFX_ISOLATIONAWARE_FUNC(int, DrawShadowText, (HDC hdc,LPCWSTR pszText,UINT cch,RECT*prc,DWORD dwFlags,COLORREF crText,COLORREF crShadow,int ixOffset,int iyOffset), (hdc,pszText,cch,prc,dwFlags,crText,crShadow,ixOffset,iyOffset),-1)
	
	AFX_ISOLATIONAWARE_FUNC(HPROPSHEETPAGE, CreatePropertySheetPageA, (LPCPROPSHEETPAGEA constPropSheetPagePointer), (constPropSheetPagePointer),NULL)
	AFX_ISOLATIONAWARE_FUNC(HPROPSHEETPAGE, CreatePropertySheetPageW, (LPCPROPSHEETPAGEW constPropSheetPagePointer), (constPropSheetPagePointer),NULL)
	AFX_ISOLATIONAWARE_FUNC(BOOL, DestroyPropertySheetPage, (HPROPSHEETPAGE unnamed1), (unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(INT_PTR, PropertySheetA, (LPCPROPSHEETHEADERA unnamed1), (unnamed1),-1)
	AFX_ISOLATIONAWARE_FUNC(INT_PTR, PropertySheetW, (LPCPROPSHEETHEADERW unnamed1), (unnamed1),-1)
};
/////////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
#define AfxCreateStatusWindow AfxCreateStatusWindowW
#define AfxDrawStatusText AfxDrawStatusTextW
#define AfxImageList_LoadImage AfxImageList_LoadImageW
#define AfxCreatePropertySheetPage AfxCreatePropertySheetPageW
#define AfxPropertySheet AfxPropertySheetW
#else
#define AfxCreateStatusWindow AfxCreateStatusWindowA
#define AfxDrawStatusText AfxDrawStatusTextA
#define AfxImageList_LoadImage AfxImageList_LoadImageA
#define AfxCreatePropertySheetPage AfxCreatePropertySheetPageA
#define AfxPropertySheet AfxPropertySheetA
#endif

#define AfxImageList_RemoveAll(himl) AfxImageList_Remove(himl, -1)
#define AfxImageList_ExtractIcon(hi, himl, i) AfxImageList_GetIcon(himl, i, 0)
#define AfxImageList_LoadBitmap(hi, lpbmp, cx, cGrow, crMask) AfxImageList_LoadImage(hi, lpbmp, cx, cGrow, crMask, IMAGE_BITMAP, 0)
#define AfxImageList_AddIcon(himl, hicon) AfxImageList_ReplaceIcon(himl, -1, hicon)

////////////////////// commdlg.h //////////////////////////////////////////
class CCommDlgWrapper : public CDllIsolationWrapperBase
{
public:
	CCommDlgWrapper() 
	: CDllIsolationWrapperBase(_T("comdlg32.dll"))
	{
	}
public:
	AFX_ISOLATIONAWARE_FUNC(BOOL,GetOpenFileNameA,(LPOPENFILENAMEA unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL,GetOpenFileNameW,(LPOPENFILENAMEW unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL,GetSaveFileNameA,(LPOPENFILENAMEA unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL,GetSaveFileNameW,(LPOPENFILENAMEW unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(short,GetFileTitleA,(LPCSTR unnamed1,LPSTR unnamed2,WORD unnamed3),(unnamed1,unnamed2,unnamed3),-1)
	AFX_ISOLATIONAWARE_FUNC(short ,GetFileTitleW,(LPCWSTR unnamed1,LPWSTR unnamed2,WORD unnamed3),(unnamed1,unnamed2,unnamed3),-1)
	AFX_ISOLATIONAWARE_FUNC(BOOL ,ChooseColorA,(LPCHOOSECOLORA unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL ,ChooseColorW,(LPCHOOSECOLORW unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(HWND ,FindTextA,(LPFINDREPLACEA unnamed1),(unnamed1),NULL)
	AFX_ISOLATIONAWARE_FUNC(HWND ,FindTextW,(LPFINDREPLACEW unnamed1),(unnamed1),NULL)
	AFX_ISOLATIONAWARE_FUNC(HWND ,ReplaceTextA,(LPFINDREPLACEA unnamed1),(unnamed1),NULL)
	AFX_ISOLATIONAWARE_FUNC(HWND ,ReplaceTextW,(LPFINDREPLACEW unnamed1),(unnamed1),NULL)
	AFX_ISOLATIONAWARE_FUNC(BOOL ,ChooseFontA,(LPCHOOSEFONTA unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL ,ChooseFontW,(LPCHOOSEFONTW unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL ,PrintDlgA,(LPPRINTDLGA unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL ,PrintDlgW,(LPPRINTDLGW unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(DWORD ,CommDlgExtendedError,(void),(),0)
	AFX_ISOLATIONAWARE_FUNC(BOOL ,PageSetupDlgA,(LPPAGESETUPDLGA unnamed1),(unnamed1),FALSE)
	AFX_ISOLATIONAWARE_FUNC(BOOL ,PageSetupDlgW,(LPPAGESETUPDLGW unnamed1),(unnamed1),FALSE)
//These 2 must be the last in struct, because MFC always build them and the user may not 
//define WINVER >= 0x0500, so code in user module (inline funcs) will miscalculate the offsets. 
#if defined(STDMETHOD) && (WINVER >= 0x0500)
	AFX_ISOLATIONAWARE_FUNC(HRESULT ,PrintDlgExA,(LPPRINTDLGEXA unnamed1),(unnamed1),E_FAIL)
	AFX_ISOLATIONAWARE_FUNC(HRESULT ,PrintDlgExW,(LPPRINTDLGEXW unnamed1),(unnamed1),E_FAIL)
#endif /* defined(STDMETHOD) && (WINVER >= 0x0500) */
};
#ifdef _UNICODE
#define AfxCtxGetOpenFileName AfxCtxGetOpenFileNameW
#define AfxCtxGetSaveFileName AfxCtxGetSaveFileNameW
#define AfxCtxGetFileTitle	  AfxCtxGetFileTitleW
#define AfxCtxChooseColor	  AfxCtxChooseColorW
#define AfxCtxFindText		  AfxCtxFindTextW
#define AfxCtxReplaceText	  AfxCtxReplaceTextW
#define AfxCtxChooseFont	  AfxCtxChooseFontW
#define AfxCtxPrintDlg		  AfxCtxPrintDlgW
#define AfxCtxCommDlgExtendedError AfxCtxCommDlgExtendedErrorW
#define AfxCtxPageSetupDlg	  AfxCtxPageSetupDlgW
#define AfxCtxPrintDlgEx	  AfxCtxPrintDlgExW
#else // ANSI
#define AfxCtxGetOpenFileName AfxCtxGetOpenFileNameA
#define AfxCtxGetSaveFileName AfxCtxGetSaveFileNameA
#define AfxCtxGetFileTitle	  AfxCtxGetFileTitleA
#define AfxCtxChooseColor	  AfxCtxChooseColorA
#define AfxCtxFindText		  AfxCtxFindTextA
#define AfxCtxReplaceText	  AfxCtxReplaceTextA
#define AfxCtxChooseFont	  AfxCtxChooseFontA
#define AfxCtxPrintDlg		  AfxCtxPrintDlgA
#define AfxCtxCommDlgExtendedError AfxCtxCommDlgExtendedErrorA
#define AfxCtxPageSetupDlg	  AfxCtxPageSetupDlgA
#define AfxCtxPrintDlgEx	  AfxCtxPrintDlgExA
#endif


////////////////////// WinUser.inl //////////////////////////////////////////
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(ATOM ,RegisterClassA,(const WNDCLASSA*lpWndClass),(lpWndClass),0)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(ATOM ,RegisterClassW,(const WNDCLASSW*lpWndClass),(lpWndClass),0)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(BOOL ,UnregisterClassA,(LPCSTR lpClassName,HINSTANCE hInstance),(lpClassName,hInstance),FALSE)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(BOOL ,UnregisterClassW,(LPCWSTR lpClassName,HINSTANCE hInstance),(lpClassName,hInstance),FALSE)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(BOOL ,GetClassInfoA,(HINSTANCE hInstance,LPCSTR lpClassName,LPWNDCLASSA lpWndClass),(hInstance,lpClassName,lpWndClass),FALSE)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(BOOL ,GetClassInfoW,(HINSTANCE hInstance,LPCWSTR lpClassName,LPWNDCLASSW lpWndClass),(hInstance,lpClassName,lpWndClass),FALSE)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(ATOM ,RegisterClassExA,(const WNDCLASSEXA*unnamed1),(unnamed1),0)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(ATOM ,RegisterClassExW,(const WNDCLASSEXW*unnamed1),(unnamed1),0)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(BOOL ,GetClassInfoExA,(HINSTANCE unnamed1,LPCSTR unnamed2,LPWNDCLASSEXA unnamed3),(unnamed1,unnamed2,unnamed3),FALSE)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(BOOL ,GetClassInfoExW,(HINSTANCE unnamed1,LPCWSTR unnamed2,LPWNDCLASSEXW unnamed3),(unnamed1,unnamed2,unnamed3),FALSE)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(HWND ,CreateWindowExA,(DWORD dwExStyle,LPCSTR lpClassName,LPCSTR lpWindowName,DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam),(dwExStyle,lpClassName,lpWindowName,dwStyle,X,Y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam),NULL)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(HWND ,CreateWindowExW,(DWORD dwExStyle,LPCWSTR lpClassName,LPCWSTR lpWindowName,DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam),(dwExStyle,lpClassName,lpWindowName,dwStyle,X,Y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam),NULL)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(HWND ,CreateDialogParamA,(HINSTANCE hInstance,LPCSTR lpTemplateName,HWND hWndParent,DLGPROC lpDialogFunc,LPARAM dwInitParam),(hInstance,lpTemplateName,hWndParent,lpDialogFunc,dwInitParam),NULL)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(HWND ,CreateDialogParamW,(HINSTANCE hInstance,LPCWSTR lpTemplateName,HWND hWndParent,DLGPROC lpDialogFunc,LPARAM dwInitParam),(hInstance,lpTemplateName,hWndParent,lpDialogFunc,dwInitParam),NULL)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(HWND ,CreateDialogIndirectParamA,(HINSTANCE hInstance,LPCDLGTEMPLATEA lpTemplate,HWND hWndParent,DLGPROC lpDialogFunc,LPARAM dwInitParam),(hInstance,lpTemplate,hWndParent,lpDialogFunc,dwInitParam),NULL)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(HWND ,CreateDialogIndirectParamW,(HINSTANCE hInstance,LPCDLGTEMPLATEW lpTemplate,HWND hWndParent,DLGPROC lpDialogFunc,LPARAM dwInitParam),(hInstance,lpTemplate,hWndParent,lpDialogFunc,dwInitParam),NULL)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(INT_PTR ,DialogBoxParamA,(HINSTANCE hInstance,LPCSTR lpTemplateName,HWND hWndParent,DLGPROC lpDialogFunc,LPARAM dwInitParam),(hInstance,lpTemplateName,hWndParent,lpDialogFunc,dwInitParam),-1)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(INT_PTR ,DialogBoxParamW,(HINSTANCE hInstance,LPCWSTR lpTemplateName,HWND hWndParent,DLGPROC lpDialogFunc,LPARAM dwInitParam),(hInstance,lpTemplateName,hWndParent,lpDialogFunc,dwInitParam),-1)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(INT_PTR ,DialogBoxIndirectParamA,(HINSTANCE hInstance,LPCDLGTEMPLATEA hDialogTemplate,HWND hWndParent,DLGPROC lpDialogFunc,LPARAM dwInitParam),(hInstance,hDialogTemplate,hWndParent,lpDialogFunc,dwInitParam),-1)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(INT_PTR ,DialogBoxIndirectParamW,(HINSTANCE hInstance,LPCDLGTEMPLATEW hDialogTemplate,HWND hWndParent,DLGPROC lpDialogFunc,LPARAM dwInitParam),(hInstance,hDialogTemplate,hWndParent,lpDialogFunc,dwInitParam),-1)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(int ,MessageBoxA,(HWND hWnd,LPCSTR lpText,LPCSTR lpCaption,UINT uType),(hWnd,lpText,lpCaption,uType),0)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(int ,MessageBoxW,(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCaption,UINT uType),(hWnd,lpText,lpCaption,uType),0)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(int ,MessageBoxExA,(HWND hWnd,LPCSTR lpText,LPCSTR lpCaption,UINT uType,WORD wLanguageId),(hWnd,lpText,lpCaption,uType,wLanguageId),0)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(int ,MessageBoxExW,(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCaption,UINT uType,WORD wLanguageId),(hWnd,lpText,lpCaption,uType,wLanguageId),0)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(int ,MessageBoxIndirectA,(const MSGBOXPARAMSA*unnamed1),(unnamed1),0)
 AFX_ISOLATIONAWARE_STATICLINK_FUNC(int ,MessageBoxIndirectW,(const MSGBOXPARAMSW*unnamed1),(unnamed1),0)

#ifdef _UNICODE
#define AfxCtxRegisterClass   AfxCtxRegisterClassW
#define AfxCtxUnregisterClass AfxCtxUnregisterClassW
#define AfxCtxGetClassInfo   AfxCtxGetClassInfoW
#define AfxCtxRegisterClassEx AfxCtxRegisterClassExW
#define AfxCtxGetClassInfoEx AfxCtxGetClassInfoExW
#define AfxCtxCreateWindowEx AfxCtxCreateWindowExW
#define AfxCtxCreateDialogParam AfxCtxCreateDialogParamW
#define AfxCtxCreateDialogIndirectParam AfxCtxCreateDialogIndirectParamW
#define AfxCtxDialogBoxParam AfxCtxDialogBoxParamW
#define AfxCtxDialogBoxIndirectParam AfxCtxDialogBoxIndirectParamW
#define AfxCtxMessageBox AfxCtxMessageBoxW
#define AfxCtxMessageBoxEx AfxCtxMessageBoxExW
#define AfxCtxMessageBoxIndirect AfxCtxMessageBoxIndirectW
#else // ANSI
#define AfxCtxRegisterClass   AfxCtxRegisterClassA
#define AfxCtxUnregisterClass AfxCtxUnregisterClassA
#define AfxCtxGetClassInfo   AfxCtxGetClassInfoA
#define AfxCtxRegisterClassEx AfxCtxRegisterClassExA
#define AfxCtxGetClassInfoEx AfxCtxGetClassInfoExA
#define AfxCtxCreateWindowEx AfxCtxCreateWindowExA
#define AfxCtxCreateDialogParam AfxCtxCreateDialogParamA
#define AfxCtxCreateDialogIndirectParam AfxCtxCreateDialogIndirectParamA
#define AfxCtxDialogBoxParam AfxCtxDialogBoxParamA
#define AfxCtxDialogBoxIndirectParam AfxCtxDialogBoxIndirectParamA
#define AfxCtxMessageBox AfxCtxMessageBoxA
#define AfxCtxMessageBoxEx AfxCtxMessageBoxExA
#define AfxCtxMessageBoxIndirect AfxCtxMessageBoxIndirectA
#endif

////////////////////// WinBase.inl //////////////////////////////////////////
//Only the funcs that actually change in winbase.inl context are in this list.
AFX_ISOLATIONAWARE_STATICLINK_FUNC(HMODULE,LoadLibraryA,(LPCSTR lpLibFileName),(lpLibFileName),NULL)
AFX_ISOLATIONAWARE_STATICLINK_FUNC(HMODULE,LoadLibraryW,(LPCWSTR lpLibFileName),(lpLibFileName),NULL)
AFX_ISOLATIONAWARE_STATICLINK_FUNC(HMODULE,LoadLibraryExA,(LPCSTR lpLibFileName,HANDLE hFile,DWORD dwFlags),(lpLibFileName,hFile,dwFlags),NULL)
AFX_ISOLATIONAWARE_STATICLINK_FUNC(HMODULE,LoadLibraryExW,(LPCWSTR lpLibFileName,HANDLE hFile,DWORD dwFlags),(lpLibFileName,hFile,dwFlags),NULL)

#ifdef _UNICODE
#define AfxCtxLoadLibrary AfxCtxLoadLibraryW
#define AfxCtxLoadLibraryEx AfxCtxLoadLibraryExW
#else // ANSI
#define AfxCtxLoadLibrary AfxCtxLoadLibraryA
#define AfxCtxLoadLibraryEx AfxCtxLoadLibraryExA
#endif
///////////////////////// ShellApi.h ////////////////////////////////////////

class CShellWrapper : public CDllIsolationWrapperBase
{
public:
	CShellWrapper() 
	: CDllIsolationWrapperBase(_T("shell32.dll"))
	{
	}
public:
	AFX_ISOLATIONAWARE_FUNC(BOOL,InitNetworkAddressControl, (void), (),FALSE)
};

/////////////////////////////////////////////////////////////////////////////

#pragma pop_macro("AFX_ISOLATIONAWARE_FUNC")
#pragma pop_macro("AFX_ISOLATIONAWARE_PROC")
#pragma pop_macro("AFX_ISOLATIONAWARE_STATICLINK_FUNC")
#pragma pop_macro("AFX_ISOLATIONAWARE_STATICLINK_PROC")
#pragma pop_macro("AFX_ISOLATIONAWARE_FUNC_DEACTIVATE")
#pragma pop_macro("AFX_ISOLATIONAWARE_FUNC_ACTIVATE")
#pragma pop_macro("AFX_ISOLATIONAWARE_COMMON_ACTIVATE")

/////////////////////////////////////////////////////////////////////////////

#endif // __AFXCOMCTL32_H__
