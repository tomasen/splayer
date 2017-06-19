// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Inlines for AFXCOMCTL32.H

#pragma once


/////////////////////////////////////////////////////////////////////////////
// Fusion: Macros to create global functions from wrapper methods (comctl32,commdlg)
// of dynamic wrappers.

#pragma push_macro("COMCTL_AFXFUNC")
#pragma push_macro("COMCTL_AFXPROC")

#define COMCTL_AFXFUNC(type, name, params, args) \
	inline type WINAPI Afx##name##params \
	{ \
		return afxComCtlWrapper->_##name##args; \
	}

#define COMCTL_AFXPROC(name, params, args) \
	inline void WINAPI Afx##name##params \
	{ \
		afxComCtlWrapper->_##name##args; \
	}

/////////////////////////////////////////////////////////////////////////////

COMCTL_AFXPROC(InitCommonControls, (), ())
COMCTL_AFXFUNC(BOOL, InitCommonControlsEx, (LPINITCOMMONCONTROLSEX unnamed1), (unnamed1))
COMCTL_AFXFUNC(HIMAGELIST, ImageList_Create, (int cx,int cy,UINT flags,int cInitial,int cGrow), (cx,cy,flags,cInitial,cGrow))
COMCTL_AFXFUNC(BOOL, ImageList_Destroy, (HIMAGELIST himl), (himl))
COMCTL_AFXFUNC(int, ImageList_GetImageCount, (HIMAGELIST himl), (himl))
COMCTL_AFXFUNC(BOOL, ImageList_SetImageCount, (HIMAGELIST himl,UINT uNewCount), (himl,uNewCount))
COMCTL_AFXFUNC(int, ImageList_Add, (HIMAGELIST himl,HBITMAP hbmImage,HBITMAP hbmMask), (himl,hbmImage,hbmMask))
COMCTL_AFXFUNC(int, ImageList_ReplaceIcon, (HIMAGELIST himl,int i,HICON hicon), (himl,i,hicon))
COMCTL_AFXFUNC(COLORREF, ImageList_SetBkColor, (HIMAGELIST himl,COLORREF clrBk), (himl,clrBk))
COMCTL_AFXFUNC(COLORREF, ImageList_GetBkColor, (HIMAGELIST himl), (himl))
COMCTL_AFXFUNC(BOOL, ImageList_SetOverlayImage, (HIMAGELIST himl,int iImage,int iOverlay), (himl,iImage,iOverlay))
COMCTL_AFXFUNC(BOOL, ImageList_Draw, (HIMAGELIST himl,int i,HDC hdcDst,int x,int y,UINT fStyle), (himl,i,hdcDst,x,y,fStyle))
COMCTL_AFXFUNC(BOOL, ImageList_Replace, (HIMAGELIST himl,int i,HBITMAP hbmImage,HBITMAP hbmMask), (himl,i,hbmImage,hbmMask))
COMCTL_AFXFUNC(int, ImageList_AddMasked, (HIMAGELIST himl,HBITMAP hbmImage,COLORREF crMask), (himl,hbmImage,crMask))
COMCTL_AFXFUNC(BOOL, ImageList_DrawEx, (HIMAGELIST himl,int i,HDC hdcDst,int x,int y,int dx,int dy,COLORREF rgbBk,COLORREF rgbFg,UINT fStyle), (himl,i,hdcDst,x,y,dx,dy,rgbBk,rgbFg,fStyle))
COMCTL_AFXFUNC(BOOL, ImageList_DrawIndirect, (IMAGELISTDRAWPARAMS*pimldp), (pimldp))
COMCTL_AFXFUNC(BOOL, ImageList_Remove, (HIMAGELIST himl,int i), (himl,i))
COMCTL_AFXFUNC(HICON, ImageList_GetIcon, (HIMAGELIST himl,int i,UINT flags), (himl,i,flags))
COMCTL_AFXFUNC(HIMAGELIST, ImageList_LoadImageA, (HINSTANCE hi,LPCSTR lpbmp,int cx,int cGrow,COLORREF crMask,UINT uType,UINT uFlags), (hi,lpbmp,cx,cGrow,crMask,uType,uFlags))
COMCTL_AFXFUNC(HIMAGELIST, ImageList_LoadImageW, (HINSTANCE hi,LPCWSTR lpbmp,int cx,int cGrow,COLORREF crMask,UINT uType,UINT uFlags), (hi,lpbmp,cx,cGrow,crMask,uType,uFlags))
COMCTL_AFXFUNC(BOOL, ImageList_Copy, (HIMAGELIST himlDst,int iDst,HIMAGELIST himlSrc,int iSrc,UINT uFlags), (himlDst,iDst,himlSrc,iSrc,uFlags))
COMCTL_AFXFUNC(BOOL, ImageList_BeginDrag, (HIMAGELIST himlTrack,int iTrack,int dxHotspot,int dyHotspot), (himlTrack,iTrack,dxHotspot,dyHotspot))
COMCTL_AFXPROC(ImageList_EndDrag, (), ())
COMCTL_AFXFUNC(BOOL, ImageList_DragEnter, (HWND hwndLock,int x,int y), (hwndLock,x,y))
COMCTL_AFXFUNC(BOOL, ImageList_DragLeave, (HWND hwndLock), (hwndLock))
COMCTL_AFXFUNC(BOOL, ImageList_DragMove, (int x,int y), (x,y))
COMCTL_AFXFUNC(BOOL, ImageList_SetDragCursorImage, (HIMAGELIST himlDrag,int iDrag,int dxHotspot,int dyHotspot), (himlDrag,iDrag,dxHotspot,dyHotspot))
COMCTL_AFXFUNC(BOOL, ImageList_DragShowNolock, (BOOL fShow), (fShow))
COMCTL_AFXFUNC(HIMAGELIST, ImageList_GetDragImage, (POINT*ppt,POINT*pptHotspot), (ppt,pptHotspot))
COMCTL_AFXFUNC(HIMAGELIST, ImageList_Read, (IStream *pstm), (pstm))
COMCTL_AFXFUNC(BOOL, ImageList_Write, (HIMAGELIST himl,IStream *pstm), (himl,pstm))

#if (_WIN32_WINNT >= 0x0501)
COMCTL_AFXFUNC(HRESULT, ImageList_ReadEx, (DWORD dwFlags,IStream *pstm,REFIID riid,PVOID*ppv), (dwFlags,pstm,riid,ppv))
COMCTL_AFXFUNC(HRESULT, ImageList_WriteEx, (HIMAGELIST himl,DWORD dwFlags,IStream *pstm), (himl,dwFlags,pstm))
#endif /* (_WIN32_WINNT >= 0x0501) */

COMCTL_AFXFUNC(BOOL, ImageList_GetIconSize, (HIMAGELIST himl,int*cx,int*cy), (himl,cx,cy))
COMCTL_AFXFUNC(BOOL, ImageList_SetIconSize, (HIMAGELIST himl,int cx,int cy), (himl,cx,cy))
COMCTL_AFXFUNC(BOOL, ImageList_GetImageInfo, (HIMAGELIST himl,int i,IMAGEINFO*pImageInfo), (himl,i,pImageInfo))
COMCTL_AFXFUNC(HIMAGELIST, ImageList_Merge, (HIMAGELIST himl1,int i1,HIMAGELIST himl2,int i2,int dx,int dy), (himl1,i1,himl2,i2,dx,dy))
COMCTL_AFXFUNC(HIMAGELIST, ImageList_Duplicate, (HIMAGELIST himl), (himl))
COMCTL_AFXFUNC(HWND, CreateToolbarEx, (HWND hwnd,DWORD ws,UINT wID,int nBitmaps,HINSTANCE hBMInst,UINT_PTR wBMID,LPCTBBUTTON lpButtons,int iNumButtons,int dxButton,int dyButton,int dxBitmap,int dyBitmap,UINT uStructSize), (hwnd,ws,wID,nBitmaps,hBMInst,wBMID,lpButtons,iNumButtons,dxButton,dyButton,dxBitmap,dyBitmap,uStructSize))
COMCTL_AFXFUNC(HBITMAP, CreateMappedBitmap, (HINSTANCE hInstance,INT_PTR idBitmap,UINT wFlags,LPCOLORMAP lpColorMap,int iNumMaps), (hInstance,idBitmap,wFlags,lpColorMap,iNumMaps))
COMCTL_AFXPROC(DrawStatusTextA, (HDC hDC,LPRECT lprc,LPCSTR pszText,UINT uFlags), (hDC,lprc,pszText,uFlags))
COMCTL_AFXPROC(DrawStatusTextW, (HDC hDC,LPRECT lprc,LPCWSTR pszText,UINT uFlags), (hDC,lprc,pszText,uFlags))
COMCTL_AFXFUNC(HWND, CreateStatusWindowA, (long style,LPCSTR lpszText,HWND hwndParent,UINT wID), (style,lpszText,hwndParent,wID))
COMCTL_AFXFUNC(HWND, CreateStatusWindowW, (long style,LPCWSTR lpszText,HWND hwndParent,UINT wID), (style,lpszText,hwndParent,wID))
COMCTL_AFXPROC(MenuHelp, (UINT uMsg,WPARAM wParam,LPARAM lParam,HMENU hMainMenu,HINSTANCE hInst,HWND hwndStatus,UINT*lpwIDs), (uMsg,wParam,lParam,hMainMenu,hInst,hwndStatus,lpwIDs))
COMCTL_AFXFUNC(BOOL, ShowHideMenuCtl, (HWND hWnd,UINT_PTR uFlags,LPINT lpInfo), (hWnd,uFlags,lpInfo))
COMCTL_AFXPROC(GetEffectiveClientRect, (HWND hWnd,LPRECT lprc,LPINT lpInfo), (hWnd,lprc,lpInfo))
COMCTL_AFXFUNC(BOOL, MakeDragList, (HWND hLB), (hLB))
COMCTL_AFXPROC(DrawInsert, (HWND handParent,HWND hLB,int nItem), (handParent,hLB,nItem))
COMCTL_AFXFUNC(int, LBItemFromPt, (HWND hLB,POINT pt,BOOL bAutoScroll), (hLB,pt,bAutoScroll))
COMCTL_AFXFUNC(HWND, CreateUpDownControl, (DWORD dwStyle,int x,int y,int cx,int cy,HWND hParent,int nID,HINSTANCE hInst,HWND hBuddy,int nUpper,int nLower,int nPos), (dwStyle,x,y,cx,cy,hParent,nID,hInst,hBuddy,nUpper,nLower,nPos))
COMCTL_AFXPROC(InitMUILanguage, (LANGID uiLang), (uiLang))
COMCTL_AFXFUNC(LANGID, GetMUILanguage, (), ())
COMCTL_AFXFUNC(HDSA, DSA_Create, (int cbItem,int cItemGrow), (cbItem,cItemGrow))
COMCTL_AFXFUNC(BOOL, DSA_Destroy, (HDSA hdsa), (hdsa))
COMCTL_AFXPROC(DSA_DestroyCallback, (HDSA hdsa,PFNDSAENUMCALLBACK pfnCB,void*pData), (hdsa,pfnCB,pData))
COMCTL_AFXFUNC(PVOID, DSA_GetItemPtr, (HDSA hdsa,int i), (hdsa,i))
COMCTL_AFXFUNC(int, DSA_InsertItem, (HDSA hdsa,int i,void*pitem), (hdsa,i,pitem))
COMCTL_AFXFUNC(HDPA, DPA_Create, (int cItemGrow), (cItemGrow))
COMCTL_AFXFUNC(BOOL, DPA_Destroy, (HDPA hdpa), (hdpa))
COMCTL_AFXFUNC(PVOID, DPA_DeletePtr, (HDPA hdpa,int i), (hdpa,i))
COMCTL_AFXFUNC(BOOL, DPA_DeleteAllPtrs, (HDPA hdpa), (hdpa))
COMCTL_AFXPROC(DPA_EnumCallback, (HDPA hdpa,PFNDPAENUMCALLBACK pfnCB,void*pData), (hdpa,pfnCB,pData))
COMCTL_AFXPROC(DPA_DestroyCallback, (HDPA hdpa,PFNDPAENUMCALLBACK pfnCB,void*pData), (hdpa,pfnCB,pData))
COMCTL_AFXFUNC(BOOL, DPA_SetPtr, (HDPA hdpa,int i,void*p), (hdpa,i,p))
COMCTL_AFXFUNC(int, DPA_InsertPtr, (HDPA hdpa,int i,void*p), (hdpa,i,p))
COMCTL_AFXFUNC(PVOID, DPA_GetPtr, (HDPA hdpa,INT_PTR i), (hdpa,i))
COMCTL_AFXFUNC(BOOL, DPA_Sort, (HDPA hdpa,PFNDPACOMPARE pfnCompare,LPARAM lParam), (hdpa,pfnCompare,lParam))
COMCTL_AFXFUNC(int, DPA_Search, (HDPA hdpa,void*pFind,int iStart,PFNDPACOMPARE pfnCompare,LPARAM lParam,UINT options), (hdpa,pFind,iStart,pfnCompare,lParam,options))
COMCTL_AFXFUNC(BOOL, Str_SetPtrW, (__deref_inout_opt LPWSTR*ppsz,LPCWSTR psz), (ppsz,psz))

#if !defined(NOTRACKMOUSEEVENT)
COMCTL_AFXFUNC(BOOL, _TrackMouseEvent, (LPTRACKMOUSEEVENT lpEventTrack), (lpEventTrack))
#endif /* !defined(NOTRACKMOUSEEVENT) */

COMCTL_AFXFUNC(BOOL, FlatSB_EnableScrollBar, (HWND unnamed1,int unnamed2,UINT unnamed3), (unnamed1,unnamed2,unnamed3))
COMCTL_AFXFUNC(BOOL, FlatSB_ShowScrollBar, (HWND unnamed1,int code,BOOL unnamed2), (unnamed1,code,unnamed2))
COMCTL_AFXFUNC(BOOL, FlatSB_GetScrollRange, (HWND unnamed1,int code,LPINT unnamed2,LPINT unnamed3), (unnamed1,code,unnamed2,unnamed3))
COMCTL_AFXFUNC(BOOL, FlatSB_GetScrollInfo, (HWND unnamed1,int code,LPSCROLLINFO unnamed2), (unnamed1,code,unnamed2))
COMCTL_AFXFUNC(int, FlatSB_GetScrollPos, (HWND unnamed1,int code), (unnamed1,code))
COMCTL_AFXFUNC(BOOL, FlatSB_GetScrollProp, (HWND unnamed1,int propIndex,LPINT unnamed2), (unnamed1,propIndex,unnamed2))

#ifdef _WIN64
COMCTL_AFXFUNC(BOOL, FlatSB_GetScrollPropPtr, (HWND unnamed1,int propIndex,PINT_PTR unnamed2), (unnamed1,propIndex,unnamed2))
#else
#define AfxFlatSB_GetScrollPropPtr  AfxFlatSB_GetScrollProp
#endif

COMCTL_AFXFUNC(int, FlatSB_SetScrollPos, (HWND unnamed1,int code,int pos,BOOL fRedraw), (unnamed1,code,pos,fRedraw))
COMCTL_AFXFUNC(int, FlatSB_SetScrollInfo, (HWND unnamed1,int code,LPSCROLLINFO unnamed2,BOOL fRedraw), (unnamed1,code,unnamed2,fRedraw))
COMCTL_AFXFUNC(int, FlatSB_SetScrollRange, (HWND unnamed1,int code,int min,int max,BOOL fRedraw), (unnamed1,code,min,max,fRedraw))
COMCTL_AFXFUNC(BOOL, FlatSB_SetScrollProp, (HWND unnamed1,UINT index,INT_PTR newValue,BOOL unnamed2), (unnamed1,index,newValue,unnamed2))
COMCTL_AFXFUNC(BOOL, InitializeFlatSB, (HWND unnamed1), (unnamed1))
COMCTL_AFXFUNC(HRESULT, UninitializeFlatSB, (HWND unnamed1), (unnamed1))

#if _WIN32_WINNT >= 0x501
COMCTL_AFXFUNC(BOOL, SetWindowSubclass, (HWND hWnd,SUBCLASSPROC pfnSubclass,UINT_PTR uIdSubclass,DWORD_PTR dwRefData), (hWnd,pfnSubclass,uIdSubclass,dwRefData))
COMCTL_AFXFUNC(BOOL, GetWindowSubclass, (HWND hWnd,SUBCLASSPROC pfnSubclass,UINT_PTR uIdSubclass,DWORD_PTR*pdwRefData), (hWnd,pfnSubclass,uIdSubclass,pdwRefData))
COMCTL_AFXFUNC(BOOL, RemoveWindowSubclass, (HWND hWnd,SUBCLASSPROC pfnSubclass,UINT_PTR uIdSubclass), (hWnd,pfnSubclass,uIdSubclass))
#endif /* _WIN32_WINNT >= 0x501 */

COMCTL_AFXFUNC(LRESULT, DefSubclassProc, (HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam), (hWnd,uMsg,wParam,lParam))
COMCTL_AFXFUNC(int, DrawShadowText, (HDC hdc,LPCWSTR pszText,UINT cch,RECT*prc,DWORD dwFlags,COLORREF crText,COLORREF crShadow,int ixOffset,int iyOffset), (hdc,pszText,cch,prc,dwFlags,crText,crShadow,ixOffset,iyOffset))

COMCTL_AFXFUNC(HPROPSHEETPAGE, CreatePropertySheetPageA, (LPCPROPSHEETPAGEA constPropSheetPagePointer), (constPropSheetPagePointer))
COMCTL_AFXFUNC(HPROPSHEETPAGE, CreatePropertySheetPageW, (LPCPROPSHEETPAGEW constPropSheetPagePointer), (constPropSheetPagePointer))
COMCTL_AFXFUNC(BOOL, DestroyPropertySheetPage, (HPROPSHEETPAGE unnamed1), (unnamed1))
COMCTL_AFXFUNC(INT_PTR, PropertySheetA, (LPCPROPSHEETHEADERA unnamed1), (unnamed1))
COMCTL_AFXFUNC(INT_PTR, PropertySheetW, (LPCPROPSHEETHEADERW unnamed1), (unnamed1))

/////////////////////////////////////////////////////////////////////////////

#pragma pop_macro("COMCTL_AFXFUNC")
#pragma pop_macro("COMCTL_AFXPROC")

//////////////////// Commdlg.h /////////////////////////////////////////////////////////
#pragma push_macro("COMMDLG_AFXCTXFUNC")
#define COMMDLG_AFXCTXFUNC(type, name, params, args) \
	inline type WINAPI AfxCtx##name##params \
	{ \
		return afxCommDlgWrapper->_##name##args; \
	}

COMMDLG_AFXCTXFUNC(BOOL,GetOpenFileNameA,(LPOPENFILENAMEA unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(BOOL,GetOpenFileNameW,(LPOPENFILENAMEW unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(BOOL,GetSaveFileNameA,(LPOPENFILENAMEA unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(BOOL,GetSaveFileNameW,(LPOPENFILENAMEW unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(short,GetFileTitleA,(LPCSTR unnamed1,LPSTR unnamed2,WORD unnamed3),(unnamed1,unnamed2,unnamed3))
COMMDLG_AFXCTXFUNC(short ,GetFileTitleW,(LPCWSTR unnamed1,LPWSTR unnamed2,WORD unnamed3),(unnamed1,unnamed2,unnamed3))
COMMDLG_AFXCTXFUNC(BOOL ,ChooseColorA,(LPCHOOSECOLORA unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(BOOL ,ChooseColorW,(LPCHOOSECOLORW unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(HWND ,FindTextA,(LPFINDREPLACEA unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(HWND ,FindTextW,(LPFINDREPLACEW unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(HWND ,ReplaceTextA,(LPFINDREPLACEA unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(HWND ,ReplaceTextW,(LPFINDREPLACEW unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(BOOL ,ChooseFontA,(LPCHOOSEFONTA unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(BOOL ,ChooseFontW,(LPCHOOSEFONTW unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(BOOL ,PrintDlgA,(LPPRINTDLGA unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(BOOL ,PrintDlgW,(LPPRINTDLGW unnamed1),(unnamed1))
#if defined(STDMETHOD) && (WINVER >= 0x0500)
COMMDLG_AFXCTXFUNC(HRESULT ,PrintDlgExA,(LPPRINTDLGEXA unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(HRESULT ,PrintDlgExW,(LPPRINTDLGEXW unnamed1),(unnamed1))
#endif /* defined(STDMETHOD) && (WINVER >= 0x0500) */
COMMDLG_AFXCTXFUNC(DWORD ,CommDlgExtendedError,(void),())
COMMDLG_AFXCTXFUNC(BOOL ,PageSetupDlgA,(LPPAGESETUPDLGA unnamed1),(unnamed1))
COMMDLG_AFXCTXFUNC(BOOL ,PageSetupDlgW,(LPPAGESETUPDLGW unnamed1),(unnamed1))

/////////////////////////////////////////////////////////////////////////////

#pragma pop_macro("COMMDLG_AFXCTXFUNC")

//////////////////////// ShellApi.h /////////////////////////////////////////
#pragma push_macro("SHELL_AFXCTXFUNC")

#define SHELL_AFXCTXFUNC(type, name, params, args) \
	inline type WINAPI AfxCtx##name##params \
	{ \
		return afxShellWrapper->_##name##args; \
	}

SHELL_AFXCTXFUNC(BOOL ,InitNetworkAddressControl,(void),())

/////////////////////////////////////////////////////////////////////////////

#pragma pop_macro("SHELL_AFXCTXFUNC")

/////////////////////////////////////////////////////////////////////////////
