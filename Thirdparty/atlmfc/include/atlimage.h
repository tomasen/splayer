// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLIMAGE_H__
#define __ATLIMAGE_H__

#pragma once

#include <atldef.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlsimpcoll.h>
#include <atltypes.h>

#ifndef _ATL_NO_PRAGMA_WARNINGS
#pragma warning (push)
#pragma warning(disable : 4820)	// padding added after member
#endif //!_ATL_NO_PRAGMA_WARNINGS

#pragma warning( push, 3 )
#pragma push_macro("new")
#undef new
#include <gdiplus.h>
#pragma pop_macro("new")
#pragma warning( pop )

#include <shlwapi.h>

#ifndef _ATL_NO_DEFAULT_LIBS
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "gdiplus.lib")
#if WINVER >= 0x0500
#pragma comment(lib, "msimg32.lib")
#endif  // WINVER >= 0x0500
#endif  // !_ATL_NO_DEFAULT_LIBS

#pragma pack(push, _ATL_PACKING)

namespace ATL
{

const int CIMAGE_DC_CACHE_SIZE = 4;

class CImage;

class CImageDC
{
public:
	CImageDC( const CImage& image ) throw( ... );
	~CImageDC() throw();

	operator HDC() const throw();

private:
	const CImage& m_image;
	HDC m_hDC;
};

class CImage
{
private:
	class CDCCache
	{
	public:
		CDCCache() throw();
		~CDCCache() throw();

		HDC GetDC() throw();
		void ReleaseDC( HDC ) throw();

	private:
		HDC m_ahDCs[CIMAGE_DC_CACHE_SIZE];
	};

	class CInitGDIPlus
	{
	public:
		CInitGDIPlus() throw();
		~CInitGDIPlus() throw();

		bool Init() throw();
		void ReleaseGDIPlus() throw();
		void IncreaseCImageCount() throw();
		void DecreaseCImageCount() throw();

	private:
		ULONG_PTR m_dwToken;
		CRITICAL_SECTION m_sect;
		LONG m_nCImageObjects;
	};

public:
	static const DWORD createAlphaChannel = 0x01;

	static const DWORD excludeGIF = 0x01;
	static const DWORD excludeBMP = 0x02;
	static const DWORD excludeEMF = 0x04;
	static const DWORD excludeWMF = 0x08;
	static const DWORD excludeJPEG = 0x10;
	static const DWORD excludePNG = 0x20;
	static const DWORD excludeTIFF = 0x40;
	static const DWORD excludeIcon = 0x80;
	static const DWORD excludeOther = 0x80000000;
	static const DWORD excludeDefaultLoad = 0;
	static const DWORD excludeDefaultSave = excludeIcon|excludeEMF|excludeWMF;
	static const DWORD excludeValid = 0x800000ff;

	enum DIBOrientation
	{
		DIBOR_DEFAULT,
		DIBOR_TOPDOWN,
		DIBOR_BOTTOMUP
	};

public:
	CImage() throw();
	virtual ~CImage() throw();

	operator HBITMAP() const throw();
#if WINVER >= 0x0500
	BOOL AlphaBlend( HDC hDestDC, int xDest, int yDest, BYTE bSrcAlpha = 0xff, 
		BYTE bBlendOp = AC_SRC_OVER ) const throw();
	BOOL AlphaBlend( HDC hDestDC, const POINT& pointDest, BYTE bSrcAlpha = 0xff, 
		BYTE bBlendOp = AC_SRC_OVER ) const throw();
	BOOL AlphaBlend( HDC hDestDC, int xDest, int yDest, int nDestWidth, 
		int nDestHeight, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, 
		BYTE bSrcAlpha = 0xff, BYTE bBlendOp = AC_SRC_OVER ) const throw();
	BOOL AlphaBlend( HDC hDestDC, const RECT& rectDest, const RECT& rectSrc, 
		BYTE bSrcAlpha = 0xff, BYTE bBlendOp = AC_SRC_OVER ) const throw();
#endif  // WINVER >= 0x0500
	void Attach( HBITMAP hBitmap, DIBOrientation eOrientation = DIBOR_DEFAULT ) throw();
	BOOL BitBlt( HDC hDestDC, int xDest, int yDest, DWORD dwROP = SRCCOPY ) const throw();
	BOOL BitBlt( HDC hDestDC, const POINT& pointDest, DWORD dwROP = SRCCOPY ) const throw();
	BOOL BitBlt( HDC hDestDC, int xDest, int yDest, int nDestWidth, 
		int nDestHeight, int xSrc, int ySrc, DWORD dwROP = SRCCOPY ) const throw();
	BOOL BitBlt( HDC hDestDC, const RECT& rectDest, const POINT& pointSrc, 
		DWORD dwROP = SRCCOPY ) const throw();
	BOOL Create( int nWidth, int nHeight, int nBPP, DWORD dwFlags = 0 ) throw();
	BOOL CreateEx( int nWidth, int nHeight, int nBPP, DWORD eCompression, 
		const DWORD* pdwBitmasks = NULL, DWORD dwFlags = 0 ) throw();
	void Destroy() throw();
	HBITMAP Detach() throw();
	BOOL Draw( HDC hDestDC, int xDest, int yDest, int nDestWidth, 
		int nDestHeight, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight ) const throw();
	BOOL Draw( HDC hDestDC, const RECT& rectDest, const RECT& rectSrc ) const throw();
	BOOL Draw( HDC hDestDC, int xDest, int yDest ) const throw();
	BOOL Draw( HDC hDestDC, const POINT& pointDest ) const throw();
	BOOL Draw( HDC hDestDC, int xDest, int yDest, int nDestWidth, 
		int nDestHeight ) const throw();
	BOOL Draw( HDC hDestDC, const RECT& rectDest ) const throw();
	const void* GetBits() const throw();
	void* GetBits() throw();
	int GetBPP() const throw();
	void GetColorTable( UINT iFirstColor, UINT nColors, RGBQUAD* prgbColors ) const throw();
	HDC GetDC() const throw();
	static HRESULT GetExporterFilterString( CSimpleString& strExporters, 
		CSimpleArray< GUID >& aguidFileTypes, LPCTSTR pszAllFilesDescription = NULL, 
		DWORD dwExclude = excludeDefaultSave, TCHAR chSeparator = _T( '|' ) );
	static HRESULT GetImporterFilterString( CSimpleString& strImporters, 
		CSimpleArray< GUID >& aguidFileTypes, LPCTSTR pszAllFilesDescription = NULL, 
		DWORD dwExclude = excludeDefaultLoad, TCHAR chSeparator = _T( '|' ) );
	int GetHeight() const throw();
	int GetMaxColorTableEntries() const throw();
	int GetPitch() const throw();
	const void* GetPixelAddress( int x, int y ) const throw();
	void* GetPixelAddress( int x, int y ) throw();
	COLORREF GetPixel( int x, int y ) const throw();
	LONG GetTransparentColor() const throw();
	int GetWidth() const throw();
	bool IsDIBSection() const throw();
	bool IsIndexed() const throw();
	bool IsNull() const throw();
	HRESULT Load( LPCTSTR pszFileName ) throw();
	HRESULT Load( IStream* pStream ) throw();
	void LoadFromResource( HINSTANCE hInstance, LPCTSTR pszResourceName ) throw();
	void LoadFromResource( HINSTANCE hInstance, UINT nIDResource ) throw();
	BOOL MaskBlt( HDC hDestDC, int xDest, int yDest, int nDestWidth, 
		int nDestHeight, int xSrc, int ySrc, HBITMAP hbmMask, int xMask, 
		int yMask, DWORD dwROP = SRCCOPY ) const throw();
	BOOL MaskBlt( HDC hDestDC, const RECT& rectDest, const POINT& pointSrc, 
		HBITMAP hbmMask, const POINT& pointMask, DWORD dwROP = SRCCOPY ) const throw();
	BOOL MaskBlt( HDC hDestDC, int xDest, int yDest, HBITMAP hbmMask, 
		DWORD dwROP = SRCCOPY ) const throw();
	BOOL MaskBlt( HDC hDestDC, const POINT& pointDest, HBITMAP hbmMask, 
		DWORD dwROP = SRCCOPY ) const throw();
	BOOL PlgBlt( HDC hDestDC, const POINT* pPoints, HBITMAP hbmMask = NULL ) const throw();
	BOOL PlgBlt( HDC hDestDC, const POINT* pPoints, int xSrc, int ySrc, 
		int nSrcWidth, int nSrcHeight, HBITMAP hbmMask = NULL, int xMask = 0, 
		int yMask = 0 ) const throw();
	BOOL PlgBlt( HDC hDestDC, const POINT* pPoints, const RECT& rectSrc, 
		HBITMAP hbmMask = NULL, const POINT& pointMask = CPoint( 0, 0 ) ) const throw();
	void ReleaseDC() const throw();
	HRESULT Save( IStream* pStream, REFGUID guidFileType ) const throw();
	HRESULT Save( LPCTSTR pszFileName, REFGUID guidFileType = GUID_NULL ) const throw();
	void SetColorTable( UINT iFirstColor, UINT nColors, 
		const RGBQUAD* prgbColors ) throw();
	void SetPixel( int x, int y, COLORREF color ) throw();
	void SetPixelIndexed( int x, int y, int iIndex ) throw();
	void SetPixelRGB( int x, int y, BYTE r, BYTE g, BYTE b ) throw();
	LONG SetTransparentColor( LONG iTransparentColor ) throw();
	BOOL StretchBlt( HDC hDestDC, int xDest, int yDest, int nDestWidth, 
		int nDestHeight, DWORD dwROP = SRCCOPY ) const throw();
	BOOL StretchBlt( HDC hDestDC, const RECT& rectDest, DWORD dwROP = SRCCOPY ) const throw();
	BOOL StretchBlt( HDC hDestDC, int xDest, int yDest, int nDestWidth, 
		int nDestHeight, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight,
		DWORD dwROP = SRCCOPY ) const throw();
	BOOL StretchBlt( HDC hDestDC, const RECT& rectDest, const RECT& rectSrc,
		DWORD dwROP = SRCCOPY ) const throw();
#if WINVER >= 0x0500
	BOOL TransparentBlt( HDC hDestDC, int xDest, int yDest, int nDestWidth, 
		int nDestHeight, UINT crTransparent = CLR_INVALID ) const throw();
	BOOL TransparentBlt( HDC hDestDC, const RECT& rectDest, 
		UINT crTransparent = CLR_INVALID ) const throw();
	BOOL TransparentBlt( HDC hDestDC, int xDest, int yDest, int nDestWidth,
		int nDestHeight, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight,
		UINT crTransparent = CLR_INVALID ) const throw();
	BOOL TransparentBlt( HDC hDestDC, const RECT& rectDest, const RECT& rectSrc,
		UINT crTransparent = CLR_INVALID ) const throw();
#endif  // WINVER >= 0x0500

	static BOOL IsTransparencySupported() throw();

private:
	HBITMAP m_hBitmap;
	void* m_pBits;
	int m_nWidth;
	int m_nHeight;
	int m_nPitch;
	int m_nBPP;
	bool m_bIsDIBSection;
	bool m_bHasAlphaChannel;
	LONG m_iTransparentColor;

	static CInitGDIPlus s_initGDIPlus;

public:
	inline static void ReleaseGDIPlus() { s_initGDIPlus.ReleaseGDIPlus(); }


// Implementation
private:
	static CLSID FindCodecForExtension( LPCTSTR pszExtension, const Gdiplus::ImageCodecInfo* pCodecs, UINT nCodecs );
	static CLSID FindCodecForFileType( REFGUID guidFileType, const Gdiplus::ImageCodecInfo* pCodecs, UINT nCodecs );
	static void BuildCodecFilterString( const Gdiplus::ImageCodecInfo* pCodecs, UINT nCodecs, 
		CSimpleString& strFilter, CSimpleArray< GUID >& aguidFileTypes, LPCTSTR pszAllFilesDescription, DWORD dwExclude, TCHAR chSeparator );
	static bool ShouldExcludeFormat( REFGUID guidFileType, DWORD dwExclude ) throw();
	void UpdateBitmapInfo( DIBOrientation eOrientation );
	HRESULT CreateFromGdiplusBitmap( Gdiplus::Bitmap& bmSrc ) throw();

	static bool InitGDIPlus() throw();

	static int ComputePitch( int nWidth, int nBPP )
	{
		return( (((nWidth*nBPP)+31)/32)*4 );
	}
	static void GenerateHalftonePalette( LPRGBQUAD prgbPalette );
	COLORREF GetTransparentRGB() const;

private:
	mutable HDC m_hDC;
	mutable int m_nDCRefCount;
	mutable HBITMAP m_hOldBitmap;

	static CDCCache s_cache;
};

inline CImageDC::CImageDC( const CImage& image ) throw( ... ) :
	m_image( image ),
	m_hDC( image.GetDC() )
{
	if( m_hDC == NULL )
	{
		AtlThrow( E_OUTOFMEMORY );
	}
}

inline CImageDC::~CImageDC() throw()
{
	m_image.ReleaseDC();
}

inline CImageDC::operator HDC() const throw()
{
	return( m_hDC );
}

inline CImage::CInitGDIPlus::CInitGDIPlus() throw() :
	m_dwToken( 0 ), m_nCImageObjects( 0 )
{
	__try
	{
		InitializeCriticalSection(&m_sect);
	}
	__except( STATUS_NO_MEMORY == GetExceptionCode() )
	{
		AtlThrow( E_OUTOFMEMORY );
	}
}

inline CImage::CInitGDIPlus::~CInitGDIPlus() throw()
{
	ReleaseGDIPlus();
	DeleteCriticalSection(&m_sect);
}

inline bool CImage::CInitGDIPlus::Init() throw()
{
	EnterCriticalSection(&m_sect);
	bool fRet = true;
	if( m_dwToken == 0 )
	{
		Gdiplus::GdiplusStartupInput input;
		Gdiplus::GdiplusStartupOutput output;
		Gdiplus::Status status = Gdiplus::GdiplusStartup( &m_dwToken, &input, &output );
		if( status != Gdiplus::Ok )
			fRet = false;
	}
	LeaveCriticalSection(&m_sect);
	return fRet;
}

inline void CImage::CInitGDIPlus::ReleaseGDIPlus() throw()
{
	EnterCriticalSection(&m_sect);
	if( m_dwToken != 0 )
	{
		Gdiplus::GdiplusShutdown( m_dwToken );
	}
	m_dwToken = 0;
	LeaveCriticalSection(&m_sect);
}

inline void CImage::CInitGDIPlus::IncreaseCImageCount() throw()
{
	EnterCriticalSection(&m_sect);
	m_nCImageObjects++;
	LeaveCriticalSection(&m_sect);
}

inline void CImage::CInitGDIPlus::DecreaseCImageCount() throw()
{
	EnterCriticalSection(&m_sect);
	if( --m_nCImageObjects == 0 )
		ReleaseGDIPlus();
	LeaveCriticalSection(&m_sect);
}

inline CImage::CDCCache::CDCCache() throw()
{
	int iDC;

	for( iDC = 0; iDC < CIMAGE_DC_CACHE_SIZE; iDC++ )
	{
		m_ahDCs[iDC] = NULL;
	}
}

inline CImage::CDCCache::~CDCCache() throw()
{
	int iDC;

	for( iDC = 0; iDC < CIMAGE_DC_CACHE_SIZE; iDC++ )
	{
		if( m_ahDCs[iDC] != NULL )
		{
			::DeleteDC( m_ahDCs[iDC] );
		}
	}
}

inline HDC CImage::CDCCache::GetDC() throw()
{
	HDC hDC;

	for( int iDC = 0; iDC < CIMAGE_DC_CACHE_SIZE; iDC++ )
	{
		hDC = static_cast< HDC >( InterlockedExchangePointer( reinterpret_cast< void** >(&m_ahDCs[iDC]), NULL ) );
		if( hDC != NULL )
		{
			return( hDC );
		}
	}

	hDC = ::CreateCompatibleDC( NULL );

	return( hDC );
}

inline void CImage::CDCCache::ReleaseDC( HDC hDC ) throw()
{
	for( int iDC = 0; iDC < CIMAGE_DC_CACHE_SIZE; iDC++ )
	{
		HDC hOldDC;

		hOldDC = static_cast< HDC >( InterlockedExchangePointer( reinterpret_cast< void** >(&m_ahDCs[iDC]), hDC ) );
		if( hOldDC == NULL )
		{
			return;
		}
		else
		{
			hDC = hOldDC;
		}
	}
	if( hDC != NULL )
	{
		::DeleteDC( hDC );
	}
}

inline CImage::CImage() throw() :
	m_hBitmap( NULL ),
	m_pBits( NULL ),
	m_hDC( NULL ),
	m_nDCRefCount( 0 ),
	m_hOldBitmap( NULL ),
	m_nWidth( 0 ),
	m_nHeight( 0 ),
	m_nPitch( 0 ),
	m_nBPP( 0 ),
	m_iTransparentColor( -1 ),
	m_bHasAlphaChannel( false ),
	m_bIsDIBSection( false )
{
	s_initGDIPlus.IncreaseCImageCount();
}

inline CImage::~CImage() throw()
{
	Destroy();
	s_initGDIPlus.DecreaseCImageCount();
}

inline CImage::operator HBITMAP() const throw()
{
	return( m_hBitmap );
}

#if WINVER >= 0x0500
inline BOOL CImage::AlphaBlend( HDC hDestDC, int xDest, int yDest, 
	BYTE bSrcAlpha, BYTE bBlendOp ) const throw()
{
	return( AlphaBlend( hDestDC, xDest, yDest, m_nWidth, m_nHeight, 0, 0, 
		m_nWidth, m_nHeight, bSrcAlpha, bBlendOp ) );
}

inline BOOL CImage::AlphaBlend( HDC hDestDC, const POINT& pointDest, 
   BYTE bSrcAlpha, BYTE bBlendOp ) const throw()
{
	return( AlphaBlend( hDestDC, pointDest.x, pointDest.y, m_nWidth, m_nHeight, 
		0, 0, m_nWidth, m_nHeight, bSrcAlpha, bBlendOp ) );
}

inline BOOL CImage::AlphaBlend( HDC hDestDC, int xDest, int yDest, 
	int nDestWidth, int nDestHeight, int xSrc, int ySrc, int nSrcWidth, 
	int nSrcHeight, BYTE bSrcAlpha, BYTE bBlendOp ) const throw()
{
	BLENDFUNCTION blend;
	BOOL bResult;

	blend.SourceConstantAlpha = bSrcAlpha;
	blend.BlendOp = bBlendOp;
	blend.BlendFlags = 0;
	if( m_bHasAlphaChannel )
	{
		blend.AlphaFormat = AC_SRC_ALPHA;
	}
	else
	{
		blend.AlphaFormat = 0;
	}

	GetDC();

	bResult = ::AlphaBlend( hDestDC, xDest, yDest, nDestWidth, nDestHeight, m_hDC, 
		xSrc, ySrc, nSrcWidth, nSrcHeight, blend );

	ReleaseDC();

	return( bResult );
}

inline BOOL CImage::AlphaBlend( HDC hDestDC, const RECT& rectDest, 
	const RECT& rectSrc, BYTE bSrcAlpha, BYTE bBlendOp ) const throw()
{
	return( AlphaBlend( hDestDC, rectDest.left, rectDest.top, rectDest.right-
		rectDest.left, rectDest.bottom-rectDest.top, rectSrc.left, rectSrc.top, 
		rectSrc.right-rectSrc.left, rectSrc.bottom-rectSrc.top, bSrcAlpha, 
		bBlendOp ) );
}
#endif  // WINVER >= 0x0500

inline void CImage::Attach( HBITMAP hBitmap, DIBOrientation eOrientation ) throw()
{
	ATLASSUME( m_hBitmap == NULL );
	ATLASSERT( hBitmap != NULL );

	m_hBitmap = hBitmap;

	UpdateBitmapInfo( eOrientation );
}

inline BOOL CImage::BitBlt( HDC hDestDC, int xDest, int yDest, DWORD dwROP ) const throw()
{
	return( BitBlt( hDestDC, xDest, yDest, m_nWidth, m_nHeight, 0, 0, dwROP ) );
}

inline BOOL CImage::BitBlt( HDC hDestDC, const POINT& pointDest, DWORD dwROP ) const throw()
{
	return( BitBlt( hDestDC, pointDest.x, pointDest.y, m_nWidth, m_nHeight,
		0, 0, dwROP ) );
}

inline BOOL CImage::BitBlt( HDC hDestDC, int xDest, int yDest, int nDestWidth, 
	int nDestHeight, int xSrc, int ySrc, DWORD dwROP ) const throw()
{
	BOOL bResult;

	ATLASSUME( m_hBitmap != NULL );
	ATLENSURE_RETURN_VAL( hDestDC != NULL, FALSE );

	GetDC();

	bResult = ::BitBlt( hDestDC, xDest, yDest, nDestWidth, nDestHeight, m_hDC, 
		xSrc, ySrc, dwROP );

	ReleaseDC();

	return( bResult );
}

inline BOOL CImage::BitBlt( HDC hDestDC, const RECT& rectDest, 
	const POINT& pointSrc, DWORD dwROP ) const throw()
{
	return( BitBlt( hDestDC, rectDest.left, rectDest.top, rectDest.right-
		rectDest.left, rectDest.bottom-rectDest.top, pointSrc.x, pointSrc.y, 
		dwROP ) );
}

inline BOOL CImage::Create( int nWidth, int nHeight, int nBPP, DWORD dwFlags ) throw()
{
	return( CreateEx( nWidth, nHeight, nBPP, BI_RGB, NULL, dwFlags ) );
}

inline BOOL CImage::CreateEx( int nWidth, int nHeight, int nBPP, DWORD eCompression, 
	const DWORD* pdwBitfields, DWORD dwFlags ) throw()
{
	USES_ATL_SAFE_ALLOCA;
	LPBITMAPINFO pbmi;
	HBITMAP hBitmap;

	ATLASSERT( (eCompression == BI_RGB) || (eCompression == BI_BITFIELDS) );
	if( dwFlags&createAlphaChannel )
	{
		ATLASSERT( (nBPP == 32) && (eCompression == BI_RGB) );
	}
	
	pbmi = (LPBITMAPINFO)_ATL_SAFE_ALLOCA(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	if( pbmi == NULL )
		return FALSE;

	memset( &pbmi->bmiHeader, 0, sizeof( pbmi->bmiHeader ) );
	pbmi->bmiHeader.biSize = sizeof( pbmi->bmiHeader );
	pbmi->bmiHeader.biWidth = nWidth;
	pbmi->bmiHeader.biHeight = nHeight;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = USHORT( nBPP );
	pbmi->bmiHeader.biCompression = eCompression;
	if( nBPP <= 8 )
	{
		ATLASSERT( eCompression == BI_RGB );
#pragma warning(push)
#pragma warning(disable:4068) //Disable unknown pragma warning that prefast pragma causes.
#pragma prefast(push)
#pragma prefast(disable:203, "no buffer overrun here, buffer was alocated properly")	
		memset( pbmi->bmiColors, 0, 256*sizeof( RGBQUAD ) );
#pragma prefast(pop)
#pragma warning(pop)
	}
	
	else 
	{
		if( eCompression == BI_BITFIELDS )
		{
			ATLASSERT( pdwBitfields != NULL );
			Checked::memcpy_s(pbmi->bmiColors, 3*sizeof( DWORD ), pdwBitfields, 3*sizeof( DWORD ));
		}
	}

	hBitmap = ::CreateDIBSection( NULL, pbmi, DIB_RGB_COLORS, &m_pBits, NULL,
		0 );
	if( hBitmap == NULL )
	{
		return( FALSE );
	}

	Attach( hBitmap, (nHeight < 0) ? DIBOR_TOPDOWN : DIBOR_BOTTOMUP );

	if( dwFlags&createAlphaChannel )
	{
		m_bHasAlphaChannel = true;
	}

	return( TRUE );
}

inline void CImage::Destroy() throw()
{
	HBITMAP hBitmap;

	if( m_hBitmap != NULL )
	{
		hBitmap = Detach();
		::DeleteObject( hBitmap );
	}
}

inline HBITMAP CImage::Detach() throw()
{
	HBITMAP hBitmap;

	ATLASSUME( m_hBitmap != NULL );
	ATLASSUME( m_hDC == NULL );

	hBitmap = m_hBitmap;
	m_hBitmap = NULL;
	m_pBits = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nBPP = 0;
	m_nPitch = 0;
	m_iTransparentColor = -1;
	m_bHasAlphaChannel = false;
	m_bIsDIBSection = false;

	return( hBitmap );
}

inline BOOL CImage::Draw( HDC hDestDC, const RECT& rectDest ) const throw()
{
	return( Draw( hDestDC, rectDest.left, rectDest.top, rectDest.right-
		rectDest.left, rectDest.bottom-rectDest.top, 0, 0, m_nWidth, 
		m_nHeight ) );
}

inline BOOL CImage::Draw( HDC hDestDC, int xDest, int yDest, int nDestWidth, int nDestHeight ) const throw()
{
	return( Draw( hDestDC, xDest, yDest, nDestWidth, nDestHeight, 0, 0, m_nWidth, m_nHeight ) );
}

inline BOOL CImage::Draw( HDC hDestDC, const POINT& pointDest ) const throw()
{
	return( Draw( hDestDC, pointDest.x, pointDest.y, m_nWidth, m_nHeight, 0, 0, m_nWidth, m_nHeight ) );
}

inline BOOL CImage::Draw( HDC hDestDC, int xDest, int yDest ) const throw()
{
	return( Draw( hDestDC, xDest, yDest, m_nWidth, m_nHeight, 0, 0, m_nWidth, m_nHeight ) );
}

inline BOOL CImage::Draw( HDC hDestDC, const RECT& rectDest, const RECT& rectSrc ) const throw()
{
	return( Draw( hDestDC, rectDest.left, rectDest.top, rectDest.right-
		rectDest.left, rectDest.bottom-rectDest.top, rectSrc.left, rectSrc.top, 
		rectSrc.right-rectSrc.left, rectSrc.bottom-rectSrc.top ) );
}

inline BOOL CImage::Draw( HDC hDestDC, int xDest, int yDest, int nDestWidth,
	int nDestHeight, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight ) const throw()
{
	BOOL bResult;

	ATLASSUME( m_hBitmap != NULL );
	ATLENSURE_RETURN_VAL( hDestDC != NULL, FALSE );
	ATLASSERT( nDestWidth > 0 );
	ATLASSERT( nDestHeight > 0 );
	ATLASSERT( nSrcWidth > 0 );
	ATLASSERT( nSrcHeight > 0 );

	GetDC();

#if WINVER >= 0x0500
	if( (m_iTransparentColor != -1) && IsTransparencySupported() )
	{
		bResult = ::TransparentBlt( hDestDC, xDest, yDest, nDestWidth, nDestHeight,
			m_hDC, xSrc, ySrc, nSrcWidth, nSrcHeight, GetTransparentRGB() );
	}
	else if( m_bHasAlphaChannel && IsTransparencySupported() )
	{
		BLENDFUNCTION bf;

		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 0xff;
		bf.AlphaFormat = AC_SRC_ALPHA;
		bResult = ::AlphaBlend( hDestDC, xDest, yDest, nDestWidth, nDestHeight, 
			m_hDC, xSrc, ySrc, nSrcWidth, nSrcHeight, bf );
	}
	else
#endif  // WINVER >= 0x0500
	{
		bResult = ::StretchBlt( hDestDC, xDest, yDest, nDestWidth, nDestHeight, 
			m_hDC, xSrc, ySrc, nSrcWidth, nSrcHeight, SRCCOPY );
	}

	ReleaseDC();

	return( bResult );
}

inline const void* CImage::GetBits() const throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( IsDIBSection() );

	return( m_pBits );
}

inline void* CImage::GetBits() throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( IsDIBSection() );

	return( m_pBits );
}

inline int CImage::GetBPP() const throw()
{
	ATLASSUME( m_hBitmap != NULL );

	return( m_nBPP );
}

inline void CImage::GetColorTable( UINT iFirstColor, UINT nColors, 
	RGBQUAD* prgbColors ) const throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSUME( m_pBits != NULL );
	ATLASSERT( IsIndexed() );

	GetDC();

	::GetDIBColorTable( m_hDC, iFirstColor, nColors, prgbColors );

	ReleaseDC();
}

inline HDC CImage::GetDC() const throw()
{
	ATLASSUME( m_hBitmap != NULL );

	m_nDCRefCount++;
	if( m_hDC == NULL )
	{
		m_hDC = s_cache.GetDC();
		m_hOldBitmap = HBITMAP( ::SelectObject( m_hDC, m_hBitmap ) );
	}

	return( m_hDC );
}

inline bool CImage::ShouldExcludeFormat( REFGUID guidFileType, DWORD dwExclude ) throw()
{
	static const GUID* apguidFormats[] =
	{
		&Gdiplus::ImageFormatGIF,
		&Gdiplus::ImageFormatBMP,
		&Gdiplus::ImageFormatEMF,
		&Gdiplus::ImageFormatWMF,
		&Gdiplus::ImageFormatJPEG,
		&Gdiplus::ImageFormatPNG,
		&Gdiplus::ImageFormatTIFF,
		&Gdiplus::ImageFormatIcon,
		NULL
	};

	ATLASSERT( (dwExclude|excludeValid) == excludeValid );
	for( int iFormat = 0; apguidFormats[iFormat] != NULL; iFormat++ )
	{
		if( guidFileType == *apguidFormats[iFormat] )
		{
			return( (dwExclude&(1<<iFormat)) != 0 );
		}
	}

	return( (dwExclude&excludeOther) != 0 );
}

inline void CImage::BuildCodecFilterString( const Gdiplus::ImageCodecInfo* pCodecs, UINT nCodecs,
	CSimpleString& strFilter, CSimpleArray< GUID >& aguidFileTypes, LPCTSTR pszAllFilesDescription, 
	DWORD dwExclude, TCHAR chSeparator )
{
	if( pszAllFilesDescription != NULL )
	{
		aguidFileTypes.Add( GUID_NULL );
	}

	CString strAllExtensions;
	CString strTempFilter;
	for( UINT iCodec = 0; iCodec < nCodecs; iCodec++ )
	{
		const Gdiplus::ImageCodecInfo* pCodec = &pCodecs[iCodec];

		if( !ShouldExcludeFormat( pCodec->FormatID, dwExclude ) )
		{
			CW2CT pwszFilenameExtension( pCodec->FilenameExtension );
			strTempFilter += CW2CT( pCodec->FormatDescription );
			strTempFilter += _T( " (" );
			strTempFilter += pwszFilenameExtension;
			strTempFilter += _T( ")" );
			strTempFilter += chSeparator;
			strTempFilter += pwszFilenameExtension;
			strTempFilter += chSeparator;

			aguidFileTypes.Add( pCodec->FormatID );

			if( !strAllExtensions.IsEmpty() )
			{
				strAllExtensions += _T( ";" );
			}
			strAllExtensions += pwszFilenameExtension;
		}
	}

	if( pszAllFilesDescription != NULL )
	{
		strFilter += pszAllFilesDescription;
		strFilter += chSeparator;
		strFilter += strAllExtensions;
		strFilter += chSeparator;
	}
	strFilter += strTempFilter;

	strFilter += chSeparator;
	if( aguidFileTypes.GetSize() == 0 )
	{
		strFilter += chSeparator;
	}
}

inline HRESULT CImage::GetImporterFilterString( CSimpleString& strImporters, 
	CSimpleArray< GUID >& aguidFileTypes, LPCTSTR pszAllFilesDescription /* = NULL */,
	DWORD dwExclude /* = excludeDefaultLoad */, TCHAR chSeparator /* = '|' */ )
{
	if( !InitGDIPlus() )
	{
		return( E_FAIL );
	}

	UINT nCodecs;
	UINT nSize;
	Gdiplus::Status status;
	Gdiplus::ImageCodecInfo* pCodecs;

	status = Gdiplus::GetImageDecodersSize( &nCodecs, &nSize );
	USES_ATL_SAFE_ALLOCA;
	pCodecs = static_cast< Gdiplus::ImageCodecInfo* >( _ATL_SAFE_ALLOCA(nSize, _ATL_SAFE_ALLOCA_DEF_THRESHOLD) );

	if( pCodecs == NULL )
		return E_OUTOFMEMORY;

	status = Gdiplus::GetImageDecoders( nCodecs, nSize, pCodecs );
	BuildCodecFilterString( pCodecs, nCodecs, strImporters, aguidFileTypes, pszAllFilesDescription, dwExclude, chSeparator );

	return( S_OK );
}

inline HRESULT CImage::GetExporterFilterString( CSimpleString& strExporters, 
	CSimpleArray< GUID >& aguidFileTypes, LPCTSTR pszAllFilesDescription /* = NULL */,
	DWORD dwExclude /* = excludeDefaultSave */, TCHAR chSeparator /* = '|' */ )
{
	if( !InitGDIPlus() )
	{
		return( E_FAIL );
	}

	UINT nCodecs;
	UINT nSize;
	Gdiplus::Status status;
	Gdiplus::ImageCodecInfo* pCodecs;

	status = Gdiplus::GetImageDecodersSize( &nCodecs, &nSize );
	USES_ATL_SAFE_ALLOCA;
	pCodecs = static_cast< Gdiplus::ImageCodecInfo* >( _ATL_SAFE_ALLOCA(nSize, _ATL_SAFE_ALLOCA_DEF_THRESHOLD) );

	if( pCodecs == NULL )
		return E_OUTOFMEMORY;

	status = Gdiplus::GetImageDecoders( nCodecs, nSize, pCodecs );
	BuildCodecFilterString( pCodecs, nCodecs, strExporters, aguidFileTypes, pszAllFilesDescription, dwExclude, chSeparator );

	return( S_OK );
}

inline int CImage::GetHeight() const throw()
{
	ATLASSUME( m_hBitmap != NULL );

	return( m_nHeight );
}

inline int CImage::GetMaxColorTableEntries() const throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( IsDIBSection() );

	if( IsIndexed() )
	{
		return( 1<<m_nBPP );
	}
	else
	{
		return( 0 );
	}
}

inline int CImage::GetPitch() const throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( IsDIBSection() );

	return( m_nPitch );
}

inline COLORREF CImage::GetPixel( int x, int y ) const throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( (x >= 0) && (x < m_nWidth) );
	ATLASSERT( (y >= 0) && (y < m_nHeight) );

	GetDC();

	COLORREF clr = ::GetPixel( m_hDC, x, y );

	ReleaseDC();

	return( clr );
}

inline const void* CImage::GetPixelAddress( int x, int y ) const throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( IsDIBSection() );
	ATLASSERT( (x >= 0) && (x < m_nWidth) );
	ATLASSERT( (y >= 0) && (y < m_nHeight) );

	return( LPBYTE( m_pBits )+(y*m_nPitch)+((x*m_nBPP)/8) );
}

inline void* CImage::GetPixelAddress( int x, int y ) throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( IsDIBSection() );
	ATLASSERT( (x >= 0) && (x < m_nWidth) );
	ATLASSERT( (y >= 0) && (y < m_nHeight) );

	return( LPBYTE( m_pBits )+(y*m_nPitch)+((x*m_nBPP)/8) );
}

inline LONG CImage::GetTransparentColor() const throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( (m_nBPP == 4) || (m_nBPP == 8) );

	return( m_iTransparentColor );
}

inline int CImage::GetWidth() const throw()
{
	ATLASSUME( m_hBitmap != NULL );

	return( m_nWidth );
}

inline bool CImage::IsDIBSection() const throw()
{
	return( m_bIsDIBSection );
}

inline bool CImage::IsIndexed() const throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( IsDIBSection() );

	return( m_nBPP <= 8 );
}

inline bool CImage::IsNull() const throw()
{
	return( m_hBitmap == NULL );
}

inline HRESULT CImage::Load( IStream* pStream ) throw()
{
	if( !InitGDIPlus() )
	{
		return( E_FAIL );
	}

	Gdiplus::Bitmap bmSrc( pStream );
	if( bmSrc.GetLastStatus() != Gdiplus::Ok )
	{
		return( E_FAIL );
	}

	return( CreateFromGdiplusBitmap( bmSrc ) );
}

inline HRESULT CImage::Load( LPCTSTR pszFileName ) throw()
{
	if( !InitGDIPlus() )
	{
		return( E_FAIL );
	}

	Gdiplus::Bitmap bmSrc( (CT2W)pszFileName );
	if( bmSrc.GetLastStatus() != Gdiplus::Ok )
	{
		return( E_FAIL );
	}

	return( CreateFromGdiplusBitmap( bmSrc ) );
}

inline HRESULT CImage::CreateFromGdiplusBitmap( Gdiplus::Bitmap& bmSrc ) throw()
{
	Gdiplus::PixelFormat eSrcPixelFormat = bmSrc.GetPixelFormat();
	UINT nBPP = 32;
	DWORD dwFlags = 0;
	Gdiplus::PixelFormat eDestPixelFormat = PixelFormat32bppRGB;
	if( eSrcPixelFormat&PixelFormatGDI )
	{
		nBPP = Gdiplus::GetPixelFormatSize( eSrcPixelFormat );
		eDestPixelFormat = eSrcPixelFormat;
	}
	if( Gdiplus::IsAlphaPixelFormat( eSrcPixelFormat ) )
	{
		nBPP = 32;
		dwFlags |= createAlphaChannel;
		eDestPixelFormat = PixelFormat32bppARGB;
	}

	BOOL bSuccess = Create( bmSrc.GetWidth(), bmSrc.GetHeight(), nBPP, dwFlags );
	if( !bSuccess )
	{
		return( E_FAIL );
	}
	USES_ATL_SAFE_ALLOCA;
	Gdiplus::ColorPalette* pPalette = NULL;
	if( Gdiplus::IsIndexedPixelFormat( eSrcPixelFormat ) )
	{
		UINT nPaletteSize = bmSrc.GetPaletteSize();
		pPalette = static_cast< Gdiplus::ColorPalette* >( _ATL_SAFE_ALLOCA(nPaletteSize, _ATL_SAFE_ALLOCA_DEF_THRESHOLD) );

		if( pPalette == NULL )
			return E_OUTOFMEMORY;

		bmSrc.GetPalette( pPalette, nPaletteSize );

		RGBQUAD argbPalette[256];
		ATLENSURE_RETURN( (pPalette->Count > 0) && (pPalette->Count <= 256) );
		for( UINT iColor = 0; iColor < pPalette->Count; iColor++ )
		{
			Gdiplus::ARGB color = pPalette->Entries[iColor];
			argbPalette[iColor].rgbRed = (BYTE)( (color>>RED_SHIFT) & 0xff );
			argbPalette[iColor].rgbGreen = (BYTE)( (color>>GREEN_SHIFT) & 0xff );
			argbPalette[iColor].rgbBlue = (BYTE)( (color>>BLUE_SHIFT) & 0xff );
			argbPalette[iColor].rgbReserved = 0;
		}

		SetColorTable( 0, pPalette->Count, argbPalette );
	}

	if( eDestPixelFormat == eSrcPixelFormat )
	{
		// The pixel formats are identical, so just memcpy the rows.
		Gdiplus::BitmapData data;
		Gdiplus::Rect rect( 0, 0, GetWidth(), GetHeight() );
		if(bmSrc.LockBits( &rect, Gdiplus::ImageLockModeRead, eSrcPixelFormat, &data )!=Gdiplus::Ok)
		{
			return E_OUTOFMEMORY;
		}

		size_t nBytesPerRow = AtlAlignUp( nBPP*GetWidth(), 8 )/8;
		BYTE* pbDestRow = static_cast< BYTE* >( GetBits() );
		BYTE* pbSrcRow = static_cast< BYTE* >( data.Scan0 );
		for( int y = 0; y < GetHeight(); y++ )
		{
			Checked::memcpy_s(pbDestRow, nBytesPerRow, pbSrcRow, nBytesPerRow);
			pbDestRow += GetPitch();
			pbSrcRow += data.Stride;
		}

		bmSrc.UnlockBits( &data );
	}
	else
	{
		// Let GDI+ work its magic
		Gdiplus::Bitmap bmDest( GetWidth(), GetHeight(), GetPitch(), eDestPixelFormat, static_cast< BYTE* >( GetBits() ) );
		Gdiplus::Graphics gDest( &bmDest );

		gDest.DrawImage( &bmSrc, 0, 0 );
	}

	return( S_OK );
}

inline void CImage::LoadFromResource( HINSTANCE hInstance, LPCTSTR pszResourceName ) throw()
{
	HBITMAP hBitmap;

	hBitmap = HBITMAP( ::LoadImage( hInstance, pszResourceName, IMAGE_BITMAP, 0, 
		0, LR_CREATEDIBSECTION ) );

	Attach( hBitmap );
}

inline void CImage::LoadFromResource( HINSTANCE hInstance, UINT nIDResource ) throw()
{
	LoadFromResource( hInstance, MAKEINTRESOURCE( nIDResource ) );
}

inline BOOL CImage::MaskBlt( HDC hDestDC, int xDest, int yDest, int nWidth, 
	int nHeight, int xSrc, int ySrc, HBITMAP hbmMask, int xMask, int yMask,
	DWORD dwROP ) const throw()
{
	BOOL bResult;

	ATLASSUME( m_hBitmap != NULL );
	ATLENSURE_RETURN_VAL( hDestDC != NULL, FALSE );

	GetDC();

	bResult = ::MaskBlt( hDestDC, xDest, yDest, nWidth, nHeight, m_hDC, xSrc, 
		ySrc, hbmMask, xMask, yMask, dwROP );

	ReleaseDC();

	return( bResult );
}

inline BOOL CImage::MaskBlt( HDC hDestDC, const RECT& rectDest, 
	const POINT& pointSrc, HBITMAP hbmMask, const POINT& pointMask, 
	DWORD dwROP ) const throw()
{
	return( MaskBlt( hDestDC, rectDest.left, rectDest.top, rectDest.right-
		rectDest.left, rectDest.bottom-rectDest.top, pointSrc.x, pointSrc.y, 
		hbmMask, pointMask.x, pointMask.y, dwROP ) );
}

inline BOOL CImage::MaskBlt( HDC hDestDC, int xDest, int yDest, HBITMAP hbmMask, 
	DWORD dwROP ) const throw()
{
	return( MaskBlt( hDestDC, xDest, yDest, m_nWidth, m_nHeight, 0, 0, hbmMask, 
		0, 0, dwROP ) );
}

inline BOOL CImage::MaskBlt( HDC hDestDC, const POINT& pointDest, HBITMAP hbmMask,
	DWORD dwROP ) const throw()
{
	return( MaskBlt( hDestDC, pointDest.x, pointDest.y, m_nWidth, m_nHeight, 0, 
		0, hbmMask, 0, 0, dwROP ) );
}

inline BOOL CImage::PlgBlt( HDC hDestDC, const POINT* pPoints, int xSrc, 
	int ySrc, int nSrcWidth, int nSrcHeight, HBITMAP hbmMask, int xMask, 
	int yMask ) const throw()
{
	BOOL bResult;

	ATLASSUME( m_hBitmap != NULL );
	ATLENSURE_RETURN_VAL( hDestDC != NULL, FALSE );

	GetDC();

	bResult = ::PlgBlt( hDestDC, pPoints, m_hDC, xSrc, ySrc, nSrcWidth, 
		nSrcHeight, hbmMask, xMask, yMask );

	ReleaseDC();

	return( bResult );
}

inline BOOL CImage::PlgBlt( HDC hDestDC, const POINT* pPoints, 
	const RECT& rectSrc, HBITMAP hbmMask, const POINT& pointMask ) const throw()
{
	return( PlgBlt( hDestDC, pPoints, rectSrc.left, rectSrc.top, rectSrc.right-
		rectSrc.left, rectSrc.bottom-rectSrc.top, hbmMask, pointMask.x, 
		pointMask.y ) );
}

inline BOOL CImage::PlgBlt( HDC hDestDC, const POINT* pPoints, 
	HBITMAP hbmMask ) const throw()
{
	return( PlgBlt( hDestDC, pPoints, 0, 0, m_nWidth, m_nHeight, hbmMask, 0, 
		0 ) );
}

inline void CImage::ReleaseDC() const throw()
{
	HBITMAP hBitmap;

	ATLASSUME( m_hDC != NULL );

	m_nDCRefCount--;
	if( m_nDCRefCount == 0 )
	{
		hBitmap = HBITMAP( ::SelectObject( m_hDC, m_hOldBitmap ) );
		ATLASSERT( hBitmap == m_hBitmap );
		s_cache.ReleaseDC( m_hDC );
		m_hDC = NULL;
	}
}

inline CLSID CImage::FindCodecForExtension( LPCTSTR pszExtension, const Gdiplus::ImageCodecInfo* pCodecs, UINT nCodecs )
{
	CT2CW pszExtensionW( pszExtension );

	for( UINT iCodec = 0; iCodec < nCodecs; iCodec++ )
	{
		CStringW strExtensions( pCodecs[iCodec].FilenameExtension );

		int iStart = 0;
		do
		{
			CStringW strExtension = ::PathFindExtensionW( strExtensions.Tokenize( L";", iStart ) );
			if( iStart != -1 )
			{
				if( strExtension.CompareNoCase( pszExtensionW ) == 0 )
				{
					return( pCodecs[iCodec].Clsid );
				}
			}
		} while( iStart != -1 );
	}

	return( CLSID_NULL );
}

inline CLSID CImage::FindCodecForFileType( REFGUID guidFileType, const Gdiplus::ImageCodecInfo* pCodecs, UINT nCodecs )
{
	for( UINT iCodec = 0; iCodec < nCodecs; iCodec++ )
	{
		if( pCodecs[iCodec].FormatID == guidFileType )
		{
			return( pCodecs[iCodec].Clsid );
		}
	}

	return( CLSID_NULL );
}

inline HRESULT CImage::Save( IStream* pStream, REFGUID guidFileType ) const throw()
{
	if( !InitGDIPlus() )
	{
		return( E_FAIL );
	}

	UINT nEncoders;
	UINT nBytes;
	Gdiplus::Status status;

	status = Gdiplus::GetImageEncodersSize( &nEncoders, &nBytes );
	if( status != Gdiplus::Ok )
	{
		return( E_FAIL );
	}

	USES_ATL_SAFE_ALLOCA;
	Gdiplus::ImageCodecInfo* pEncoders = static_cast< Gdiplus::ImageCodecInfo* >( _ATL_SAFE_ALLOCA(nBytes, _ATL_SAFE_ALLOCA_DEF_THRESHOLD) );

	if( pEncoders == NULL )
		return E_OUTOFMEMORY;

	status = Gdiplus::GetImageEncoders( nEncoders, nBytes, pEncoders );
	if( status != Gdiplus::Ok )
	{
		return( E_FAIL );
	}

	CLSID clsidEncoder = FindCodecForFileType( guidFileType, pEncoders, nEncoders );
	if( clsidEncoder == CLSID_NULL )
	{
		return( E_FAIL );
	}

	if( m_bHasAlphaChannel )
	{
		ATLASSUME( m_nBPP == 32 );
		Gdiplus::Bitmap bm( m_nWidth, m_nHeight, m_nPitch, PixelFormat32bppARGB, static_cast< BYTE* >( m_pBits ) );
		status = bm.Save( pStream, &clsidEncoder, NULL );
		if( status != Gdiplus::Ok )
		{
			return( E_FAIL );
		}
	}
	else
	{
		Gdiplus::Bitmap bm( m_hBitmap, NULL );
		status = bm.Save( pStream, &clsidEncoder, NULL );
		if( status != Gdiplus::Ok )
		{
			return( E_FAIL );
		}
	}

	return( S_OK );
}

inline HRESULT CImage::Save( LPCTSTR pszFileName, REFGUID guidFileType ) const throw()
{
	if( !InitGDIPlus() )
	{
		return( E_FAIL );
	}

	UINT nEncoders;
	UINT nBytes;
	Gdiplus::Status status;

	status = Gdiplus::GetImageEncodersSize( &nEncoders, &nBytes );
	if( status != Gdiplus::Ok )
	{
		return( E_FAIL );
	}

	USES_CONVERSION_EX;
	Gdiplus::ImageCodecInfo* pEncoders = static_cast< Gdiplus::ImageCodecInfo* >( _ATL_SAFE_ALLOCA(nBytes, _ATL_SAFE_ALLOCA_DEF_THRESHOLD) );

	if( pEncoders == NULL )
		return E_OUTOFMEMORY;

	status = Gdiplus::GetImageEncoders( nEncoders, nBytes, pEncoders );
	if( status != Gdiplus::Ok )
	{
		return( E_FAIL );
	}

	CLSID clsidEncoder = CLSID_NULL;
	if( guidFileType == GUID_NULL )
	{
		// Determine clsid from extension
		clsidEncoder = FindCodecForExtension( ::PathFindExtension( pszFileName ), pEncoders, nEncoders );
	}
	else
	{
		// Determine clsid from file type
		clsidEncoder = FindCodecForFileType( guidFileType, pEncoders, nEncoders );
	}
	if( clsidEncoder == CLSID_NULL )
	{
		return( E_FAIL );
	}

	LPCWSTR pwszFileName = T2CW_EX( pszFileName, _ATL_SAFE_ALLOCA_DEF_THRESHOLD );
#ifndef _UNICODE
	if( pwszFileName == NULL )
		return E_OUTOFMEMORY;
#endif // _UNICODE
	if( m_bHasAlphaChannel )
	{
		ATLASSUME( m_nBPP == 32 );
		Gdiplus::Bitmap bm( m_nWidth, m_nHeight, m_nPitch, PixelFormat32bppARGB, static_cast< BYTE* >( m_pBits ) );
		status = bm.Save( pwszFileName, &clsidEncoder, NULL );
		if( status != Gdiplus::Ok )
		{
			return( E_FAIL );
		}
	}
	else
	{
		Gdiplus::Bitmap bm( m_hBitmap, NULL );
		status = bm.Save( pwszFileName, &clsidEncoder, NULL );
		if( status != Gdiplus::Ok )
		{
			return( E_FAIL );
		}
	}

	return( S_OK );
}

inline void CImage::SetColorTable( UINT iFirstColor, UINT nColors, 
	const RGBQUAD* prgbColors ) throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( IsDIBSection() );
	ATLASSERT( IsIndexed() );

	GetDC();

	::SetDIBColorTable( m_hDC, iFirstColor, nColors, prgbColors );

	ReleaseDC();
}

inline void CImage::SetPixel( int x, int y, COLORREF color ) throw()
{
	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( (x >= 0) && (x < m_nWidth) );
	ATLASSERT( (y >= 0) && (y < m_nHeight) );

	GetDC();

	::SetPixel( m_hDC, x, y, color );

	ReleaseDC();
}

inline void CImage::SetPixelIndexed( int x, int y, int iIndex ) throw()
{
	SetPixel( x, y, PALETTEINDEX( iIndex ) );
}

inline void CImage::SetPixelRGB( int x, int y, BYTE r, BYTE g, BYTE b ) throw()
{
	SetPixel( x, y, RGB( r, g, b ) );
}

inline LONG CImage::SetTransparentColor( LONG iTransparentColor ) throw()
{
	LONG iOldTransparentColor;

	ATLASSUME( m_hBitmap != NULL );
	ATLASSERT( (m_nBPP == 4) || (m_nBPP == 8) );
	ATLASSERT( iTransparentColor < GetMaxColorTableEntries() );
	ATLASSERT( iTransparentColor >= -1 );

	iOldTransparentColor = m_iTransparentColor;
	m_iTransparentColor = iTransparentColor;

	return( iOldTransparentColor );
}

inline BOOL CImage::StretchBlt( HDC hDestDC, int xDest, int yDest, 
	int nDestWidth, int nDestHeight, DWORD dwROP ) const throw()
{
	return( StretchBlt( hDestDC, xDest, yDest, nDestWidth, nDestHeight, 0, 0, 
		m_nWidth, m_nHeight, dwROP ) );
}

inline BOOL CImage::StretchBlt( HDC hDestDC, const RECT& rectDest, 
	DWORD dwROP ) const throw()
{
	return( StretchBlt( hDestDC, rectDest.left, rectDest.top, rectDest.right-
		rectDest.left, rectDest.bottom-rectDest.top, 0, 0, m_nWidth, m_nHeight, 
		dwROP ) );
}

inline BOOL CImage::StretchBlt( HDC hDestDC, int xDest, int yDest, 
	int nDestWidth, int nDestHeight, int xSrc, int ySrc, int nSrcWidth, 
	int nSrcHeight, DWORD dwROP ) const throw()
{
	BOOL bResult;

	ATLASSUME( m_hBitmap != NULL );
	ATLENSURE_RETURN_VAL( hDestDC != NULL, FALSE );

	GetDC();

	bResult = ::StretchBlt( hDestDC, xDest, yDest, nDestWidth, nDestHeight, m_hDC,
		xSrc, ySrc, nSrcWidth, nSrcHeight, dwROP );

	ReleaseDC();

	return( bResult );
}

inline BOOL CImage::StretchBlt( HDC hDestDC, const RECT& rectDest, 
	const RECT& rectSrc, DWORD dwROP ) const throw()
{
	return( StretchBlt( hDestDC, rectDest.left, rectDest.top, rectDest.right-
		rectDest.left, rectDest.bottom-rectDest.top, rectSrc.left, rectSrc.top, 
		rectSrc.right-rectSrc.left, rectSrc.bottom-rectSrc.top, dwROP ) );
}

#if WINVER >= 0x0500
inline BOOL CImage::TransparentBlt( HDC hDestDC, int xDest, int yDest, 
	int nDestWidth, int nDestHeight, UINT crTransparent ) const throw()
{
	return( TransparentBlt( hDestDC, xDest, yDest, nDestWidth, nDestHeight, 0, 
		0, m_nWidth, m_nHeight, crTransparent ) );
}

inline BOOL CImage::TransparentBlt( HDC hDestDC, const RECT& rectDest, 
	UINT crTransparent ) const throw()
{
	return( TransparentBlt( hDestDC, rectDest.left, rectDest.top, 
		rectDest.right-rectDest.left, rectDest.bottom-rectDest.top, 
		crTransparent ) );
}

inline BOOL CImage::TransparentBlt( HDC hDestDC, int xDest, int yDest, 
	int nDestWidth, int nDestHeight, int xSrc, int ySrc, int nSrcWidth, 
	int nSrcHeight, UINT crTransparent ) const throw()
{
	BOOL bResult;

	ATLASSUME( m_hBitmap != NULL );
	ATLENSURE_RETURN_VAL( hDestDC != NULL, FALSE );

	GetDC();

	if( crTransparent == CLR_INVALID )
	{
		crTransparent = GetTransparentRGB();
	}

	bResult = ::TransparentBlt( hDestDC, xDest, yDest, nDestWidth, nDestHeight,
		m_hDC, xSrc, ySrc, nSrcWidth, nSrcHeight, crTransparent );

	ReleaseDC();

	return( bResult );
}

inline BOOL CImage::TransparentBlt( HDC hDestDC, const RECT& rectDest, 
	const RECT& rectSrc, UINT crTransparent ) const throw()
{
	return( TransparentBlt( hDestDC, rectDest.left, rectDest.top, 
		rectDest.right-rectDest.left, rectDest.bottom-rectDest.top, rectSrc.left, 
		rectSrc.top, rectSrc.right-rectSrc.left, rectSrc.bottom-rectSrc.top, 
		crTransparent ) );
}
#endif  // WINVER >= 0x0500

inline BOOL CImage::IsTransparencySupported() throw()
{
	return( TRUE );
}

inline void CImage::UpdateBitmapInfo( DIBOrientation eOrientation )
{
	DIBSECTION dibsection;
	int nBytes;

	nBytes = ::GetObject( m_hBitmap, sizeof( DIBSECTION ), &dibsection );
	if( nBytes == sizeof( DIBSECTION ) )
	{
		m_bIsDIBSection = true;
		m_nWidth = dibsection.dsBmih.biWidth;
		m_nHeight = abs( dibsection.dsBmih.biHeight );
		m_nBPP = dibsection.dsBmih.biBitCount;
		m_nPitch = ComputePitch( m_nWidth, m_nBPP );
		m_pBits = dibsection.dsBm.bmBits;
		if( eOrientation == DIBOR_DEFAULT )
		{
			eOrientation = (dibsection.dsBmih.biHeight > 0) ? DIBOR_BOTTOMUP : DIBOR_TOPDOWN;
		}
		if( eOrientation == DIBOR_BOTTOMUP )
		{
			m_pBits = LPBYTE( m_pBits )+((m_nHeight-1)*m_nPitch);
			m_nPitch = -m_nPitch;
		}
	}
	else
	{
		// Non-DIBSection
		ATLASSERT( nBytes == sizeof( BITMAP ) );
		m_bIsDIBSection = false;
		m_nWidth = dibsection.dsBm.bmWidth;
		m_nHeight = dibsection.dsBm.bmHeight;
		m_nBPP = dibsection.dsBm.bmBitsPixel;
		m_nPitch = 0;
		m_pBits = 0;
	}
	m_iTransparentColor = -1;
	m_bHasAlphaChannel = false;
}

inline void CImage::GenerateHalftonePalette( LPRGBQUAD prgbPalette )
{
	int r;
	int g;
	int b;
	int gray;
	LPRGBQUAD prgbEntry;

	prgbEntry = prgbPalette;
	for( r = 0; r < 6; r++ )
	{
		for( g = 0; g < 6; g++ )
		{
			for( b = 0; b < 6; b++ )
			{
				prgbEntry->rgbBlue = BYTE( b*255/5 );
				prgbEntry->rgbGreen = BYTE( g*255/5 );
				prgbEntry->rgbRed = BYTE( r*255/5 );
				prgbEntry->rgbReserved = 0;

				prgbEntry++;
			}
		}
	}

	for( gray = 0; gray < 20; gray++ )
	{
		prgbEntry->rgbBlue = BYTE( gray*255/20 );
		prgbEntry->rgbGreen = BYTE( gray*255/20 );
		prgbEntry->rgbRed = BYTE( gray*255/20 );
		prgbEntry->rgbReserved = 0;

		prgbEntry++;
	}
}

inline COLORREF CImage::GetTransparentRGB() const
{
	RGBQUAD rgb;

	ATLASSUME( m_hDC != NULL );  // Must have a DC
	ATLASSUME( m_iTransparentColor != -1 );

	::GetDIBColorTable( m_hDC, m_iTransparentColor, 1, &rgb );

	return( RGB( rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue ) );
}

inline bool CImage::InitGDIPlus() throw()
{
	bool bSuccess = s_initGDIPlus.Init();
	return( bSuccess );
}

};  // namespace ATL

#pragma pack(pop)

#ifndef _ATL_NO_PRAGMA_WARNINGS
#pragma warning (pop)
#endif

#endif  // __ATLIMAGE_H__
