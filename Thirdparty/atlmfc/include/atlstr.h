// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.
#ifndef __ATLSTR_H__
#define __ATLSTR_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#include <atlbase.h>

#include <winnls.h>
#include <limits.h>

#include <cstringt.h>


#pragma pack(push,_ATL_PACKING)

 
namespace ATL
{

class CAtlStringMgr :
	public IAtlStringMgr
{
public:
	CAtlStringMgr( _In_opt_ IAtlMemMgr* pMemMgr = NULL ) throw() :
		m_pMemMgr( pMemMgr )
	{
		m_nil.SetManager( this );
	}
	virtual ~CAtlStringMgr() throw()
	{
	}

	void SetMemoryManager( __reserved IAtlMemMgr* pMemMgr ) throw()
	{
		ATLASSUME( m_pMemMgr == NULL );
		m_pMemMgr = pMemMgr;
	}

// IAtlStringMgr
public:
	_Ret_opt_bytecap_x_(sizeof(CStringData) + nChars*nCharSize) virtual CStringData* Allocate( _In_ int nChars, _In_ int nCharSize ) throw()
	{
		size_t nTotalSize;
		CStringData* pData;
		size_t nDataBytes;

		nChars = AtlAlignUp( nChars + 1, 8 );  // Prevent excessive reallocation.  The heap will usually round up anyway.

		if(	FAILED(::ATL::AtlMultiply(&nDataBytes, static_cast<size_t>(nChars), static_cast<size_t>(nCharSize))) ||
			FAILED(::ATL::AtlAdd(&nTotalSize, static_cast<size_t>(sizeof( CStringData )), nDataBytes)))
		{
			return NULL;
		}
		pData = static_cast< CStringData* >( m_pMemMgr->Allocate( nTotalSize ) );
		if( pData == NULL )
		{
			return( NULL );
		}
		pData->pStringMgr = this;
		pData->nRefs = 1;
		pData->nAllocLength = nChars - 1;
		pData->nDataLength = 0;

		return( pData );
	}
	virtual void Free( _Inout_ CStringData* pData ) throw()
	{
		ATLASSERT( pData->pStringMgr == this );
		m_pMemMgr->Free( pData );
	}
	_Ret_opt_bytecap_x_(sizeof(CStringData) + nChars*nCharSize) virtual CStringData* Reallocate( _Inout_ CStringData* pData, _In_ int nChars, _In_ int nCharSize ) throw()
	{
		CStringData* pNewData;
		ULONG nTotalSize;
		ULONG nDataBytes;

		ATLASSERT( pData->pStringMgr == this );
		nChars = AtlAlignUp( nChars+1, 8 );  // Prevent excessive reallocation.  The heap will usually round up anyway.

		if(	FAILED(::ATL::AtlMultiply(&nDataBytes, static_cast<ULONG>(nChars), static_cast<ULONG>(nCharSize))) ||
			FAILED(::ATL::AtlAdd(&nTotalSize, static_cast<ULONG>(sizeof( CStringData )), nDataBytes)))
		{
			return NULL;
		}
		pNewData = static_cast< CStringData* >( m_pMemMgr->Reallocate( pData, nTotalSize ) );
		if( pNewData == NULL )
		{
			return NULL;
		}
		pNewData->nAllocLength = nChars - 1;

		return pNewData;
	}
	virtual CStringData* GetNilString() throw()
	{
		m_nil.AddRef();
		return &m_nil;
	}
	virtual IAtlStringMgr* Clone() throw()
	{
		return this;
	}

protected:
	IAtlMemMgr* m_pMemMgr;
	CNilStringData m_nil;
};

extern CAtlStringMgr g_strmgr;

template <class ChTraits>
inline typename ChTraits::PCXSTR strstrT(typename ChTraits::PCXSTR pStr,typename ChTraits::PCXSTR pCharSet);

template< typename _CharType = char >
class ChTraitsOS :
	public ChTraitsBase< _CharType >
{
public:
	static int tclen(_In_z_ LPCSTR p) throw()
	{
		ATLASSERT(p != NULL);
		LPCSTR pnext = CharNext(p);
		return ((pnext-p)>1) ? 2 : 1;
	}
	static LPCSTR strchr(_In_z_ LPCSTR p, _In_ char ch) throw()
	{
		return AtlstrchrT(p,ch);
	}
	static LPCSTR strchr_db(_In_z_ LPCSTR p, _In_ char ch1, _In_ char ch2) throw()
	{
		ATLASSERT(p != NULL);
		while (*p != 0)
		{
			if (*p == ch1 && *(p+1) == ch2)
			{
				return p;
			}
			p = CharNext(p);
		}
		return NULL;
	}
	static LPCSTR strrchr(_In_z_ LPCSTR p, _In_ char ch) throw()
	{		
		ATLASSERT(p != NULL);
		const _CharType* pch = NULL;
		while (*p != 0)
		{
			if (*p == ch)
			{
				pch = p;
			}
			p = CharNext(p);
		}
		//for strrchr(buff,'\0')
		if (*p == ch) 
		{
			pch = p;
		}
		return const_cast< _CharType* >( pch );
	}
	static _CharType* _strrev(_Inout_ _CharType* psz) throw()
	{
		// Optimize NULL, zero-length, and single-char case.
		if ((psz == NULL) || (psz[0] == '\0') || (psz[1] == '\0'))
			return psz;

		_CharType* p = psz;
		// first go through and reverse the bytes in MBCS chars
		while (*p != _T('\0')) 
		{
			if (IsDBCSLeadByte(*p++))
			{
				if (*p)
				{
					_CharType c = *p;
					*p=*(p-1);
					*(p-1)=c;
					p++;

				} else
				{
				        //  second byte is EOS
                        //There is nothing really satisfying to do here. We have a string
                        //that ends in leadbyte,'\0'. Reversing this would lead to the leadbyte
                        //becoming falsely attached to the character before it:
                        //(XL0 -> LX0, X has suddenly become a trailbyte)

                        //So what we choose to do is assert and purge the dud byte from within the
                        //string.
                    
                    ATLASSERT(FALSE && _T("Bad MBCS string passed into __strev"));

                    // String has at least moved once already, so this is safe 
                    ATLASSERT(p>psz);

                    // move back one to point at the dud leadbyte
                    --p;

                    // now truncate the string one byte earlier 
                    *p='\0';

				}
			}

			
		} //end while

		_CharType* q = psz;
		--p;
		while (q < p)
		{
			_CharType t = *q;
			*q = *p;
			*p = t;
			q++;
			p--;
		}
		return psz;
	}
	static LPCSTR strstr(_In_z_ LPCSTR pStr, _In_z_ LPCSTR pCharSet) throw()
	{		
		return strstrT< ChTraitsOS<XCHAR> >(pStr,pCharSet);
	}
	static int strspn(_In_ const _CharType* pStr, _In_ const _CharType* pCharSet) throw()
	{
		ATLASSERT(pStr != NULL);
		int nRet = 0;
		_CharType* p = const_cast<_CharType*>(pStr);
		while (*p != 0)
		{
			_CharType* pNext = CharNext(p);
			if(pNext > p + 1)
			{
				if(strchr_db(pCharSet, *p, *(p+1)) == NULL)
					break;
				nRet += 2;
			}
			else
			{
				if(strchr(pCharSet, *p) == NULL)
					break;
				nRet++;
			}
			p = pNext;
		}
		return nRet;
	}
	static int strcspn(_In_ const _CharType* pStr, _In_ const _CharType* pCharSet) throw()
	{
		ATLASSERT(pStr != NULL);
		int nRet = 0;
		_CharType* p = const_cast<_CharType*>(pStr);
		while (*p != 0)
		{
			_CharType* pNext = CharNext(p);
			if(pNext > p + 1)
			{
				if(strchr_db(pCharSet, *p, *(p+1)) != NULL)
					break;
				nRet += 2;
			}
			else
			{
				if(strchr(pCharSet, *p) != NULL)
					break;
				nRet++;
			}
			p = pNext;
		}
		return nRet;
	}
	static LPCSTR strpbrk(_In_z_ LPCSTR p, _In_z_ LPCSTR lpszCharSet) throw()
	{
		int nRet=0;
		nRet=strcspn(p,lpszCharSet);
		if (p[nRet]){
		p+=nRet;
		return p;
		}
     return NULL;
	}

	static _CharType* CharNext(_In_ const _CharType* p) throw()
	{
		return AtlCharNext(p);
	}

	static int IsDigit(_In_ _CharType ch) throw()
	{
		WORD type;
		GetStringTypeExA(GetThreadLocale(), CT_CTYPE1, &ch, 1, &type);
		return (type & C1_DIGIT) == C1_DIGIT;
	}

	static int IsSpace(_In_ _CharType ch) throw()
	{
		WORD type;
		GetStringTypeExA(GetThreadLocale(), CT_CTYPE1, &ch, 1, &type);
		return (type & C1_SPACE) == C1_SPACE;
	}

	static int StringCompare(_In_ const _CharType* pstrOne,
		_In_ const _CharType* pstrOther) throw()
	{
		return lstrcmpA((LPCSTR) pstrOne, (LPCSTR) pstrOther);
	}

	static int StringCompareIgnore(_In_ const _CharType* pstrOne,
		_In_ const _CharType* pstrOther) throw()
	{
		return lstrcmpiA((LPCSTR) pstrOne, (LPCSTR) pstrOther);
	}

	static int StringCollate(_In_ const _CharType* pstrOne,
		_In_ const _CharType* pstrOther) throw()
	{
		int nRet = CompareStringA(GetThreadLocale(), 0, (LPCSTR)pstrOne, -1, 
			(LPCSTR)pstrOther, -1);
		ATLASSERT(nRet != 0);
		return nRet-2;  // Convert to strcmp convention.  This really is documented.
	}

	static int StringCollateIgnore(_In_ const _CharType* pstrOne,
		_In_ const _CharType* pstrOther) throw()
	{
		int nRet = CompareStringA(GetThreadLocale(), NORM_IGNORECASE, (LPCSTR)pstrOne, -1, 
			(LPCSTR)pstrOther, -1);
		ATLASSERT(nRet != 0);
		return nRet-2;  // Convert to strcmp convention.  This really is documented.
	}

	static LPCSTR StringFindString(_In_z_ LPCSTR pstrBlock,
		_In_z_ LPCSTR pstrMatch) throw()
	{
		return strstr(pstrBlock, pstrMatch);
	}

	static LPSTR StringFindString(_In_z_ LPSTR pszBlock, _In_z_ LPCSTR pszMatch) throw()
	{
		return( const_cast< LPSTR >( StringFindString( const_cast< LPCSTR >( pszBlock ), pszMatch ) ) );
	}

	static LPCSTR StringFindChar(_In_z_ LPCSTR pszBlock,
		_In_ char chMatch) throw()
	{
		return strchr(pszBlock, chMatch);
	}

	static LPCSTR StringFindCharRev(_In_z_ LPCSTR psz, _In_ char ch) throw()
	{
		return strrchr(psz, ch);
	}

	static LPCSTR StringScanSet(_In_z_ LPCSTR pszBlock,
		_In_z_ LPCSTR pszMatch) throw()
	{
		return strpbrk(pszBlock, pszMatch);
	}

	static int StringSpanIncluding(_In_ const _CharType* pstrBlock,
		_In_ const _CharType* pstrSet) throw()
	{
		return strspn(pstrBlock, pstrSet);
	}

	static int StringSpanExcluding(_In_ const _CharType* pstrBlock,
		_In_ const _CharType* pstrSet) throw()
	{
		return strcspn(pstrBlock, pstrSet);
	}

	_ATL_INSECURE_DEPRECATE("ChTraitsOS::StringUppercase must be passed a buffer size")
	static _CharType* StringUppercase(_Inout_ _CharType* psz) throw()
	{
		return CharUpperA( psz );
	}

	_ATL_INSECURE_DEPRECATE("ChTraitsOS::StringLowercase must be passed a buffer size")
	static _CharType* StringLowercase(_Inout_ _CharType* psz) throw()
	{
		return CharLowerA( psz );
	}

	static _CharType* StringUppercase(_Inout_cap_(size) _CharType* psz, _In_ size_t size) throw()
	{
		if(size>UINT_MAX)
		{
			// API only allows DWORD size
			AtlThrow(E_INVALIDARG);
		}
		DWORD dwSize=static_cast<DWORD>(size);
		CharUpperBuffA( psz, dwSize );
		return psz;
	}

	static _CharType* StringLowercase(_Inout_cap_(size) _CharType* psz, size_t size) throw()
	{
		if(size>UINT_MAX)
		{
			// API only allows DWORD size
			AtlThrow(E_INVALIDARG);
		}
		DWORD dwSize=static_cast<DWORD>(size);
		CharLowerBuffA( psz, dwSize );
		return psz;
	}



	static _CharType* StringReverse(_Inout_ _CharType* psz) throw()
	{
		return _strrev( psz );
	}

	static int GetFormattedLength(_In_ _Printf_format_string_ const _CharType* pszFormat, va_list args)
	{
		_CharType szBuffer[1028];
		int nLength = 0;

		SetLastError(ERROR_SUCCESS);
#pragma warning(push)
#pragma warning(disable:4996)
		// wvsprintf always truncates the output to 1024 character plus the '\0'.
		// Note that we are using wvsprintf only in the MIN_CRT case; wvsprintf is
		// an insecure function and should be avoided. Here the use of wvsprintf
		// is safe and the only way to get a string formatted without using the CRT.
		nLength = wvsprintfA(szBuffer, pszFormat, args);
#pragma warning(pop)
		ATLENSURE(GetLastError() == ERROR_SUCCESS);
		ATLASSERT(nLength >= 0);
		ATLASSERT(nLength <= 1024);

		return nLength;
	}

	_ATL_INSECURE_DEPRECATE("CSimpleStringT::Format must be passed a buffer size")
	static int Format(_Out_ _CharType* pszBuffer, _In_ _Printf_format_string_ const _CharType* pszFormat,
		va_list args) throw()
	{
#pragma warning(push)
#pragma warning(disable:4996)
		return wvsprintfA(pszBuffer, pszFormat, args);
#pragma warning(pop)
	}
	static int Format(_Out_cap_post_count_(nlength, return) _CharType*  pszBuffer, _In_ size_t nlength, _In_ _Printf_format_string_ const _CharType* pszFormat, va_list args )
	{
		_CharType buffSafe[1030]; //wvsprintf output is max 1024.
		int nCharsWritten = 0;
		
		SetLastError(ERROR_SUCCESS);
#pragma warning(push)
#pragma warning(disable:4996)
		// wvsprintf always truncates the output to 1024 character plus the '\0'.
		// Note that we are using wvsprintf only in the MIN_CRT case; wvsprintf is
		// an insecure function and should be avoided. Here the use of wvsprintf
		// is safe and the only way to get a string formatted without using the CRT.
		nCharsWritten = wvsprintfA(buffSafe, pszFormat, args);
#pragma warning(pop)
		ATLENSURE(GetLastError() == ERROR_SUCCESS);
		ATLENSURE(nCharsWritten <= 1024);
		//nlength should have room for nCharsWritten + NULL
		ATLENSURE_THROW((size_t)nCharsWritten < nlength ,E_INVALIDARG);
		Checked::strcpy_s(pszBuffer,nlength,buffSafe);
		return nCharsWritten;		
	}

	static int GetBaseTypeLength(_In_z_ const char* pszSrc) throw()
	{
		// Returns required buffer length in XCHARs
		return lstrlenA(pszSrc);
	}

	static int GetBaseTypeLength(_In_z_ const char* pszSrc, _In_ int nLength) throw()
	{
		(void)pszSrc;
		// Returns required buffer length in XCHARs
		return nLength;
	}

	static int GetBaseTypeLength(_In_z_ const wchar_t* pszSrc) throw()
	{
		// Returns required buffer length in XCHARs
		return ::WideCharToMultiByte(_AtlGetConversionACP(), 0, pszSrc, -1, NULL, 0, NULL, NULL)-1;
	}

	static int GetBaseTypeLength(_In_count_(nLength) const wchar_t* pszSrc, _In_ int nLength) throw()
	{		
		// Returns required buffer length in XCHARs
		return ::WideCharToMultiByte(_AtlGetConversionACP(), 0, pszSrc, nLength, NULL, 0, NULL, NULL);
	}

	static void ConvertToBaseType(_Out_cap_(nDestLength) _CharType* pszDest, _In_ int nDestLength,
		_In_count_(nSrcLength) const char* pszSrc, _In_ int nSrcLength = -1) throw()
	{			
		if (nSrcLength == -1) { nSrcLength=1 + GetBaseTypeLength(pszSrc); }
		// nLen is in chars
		Checked::memcpy_s(pszDest, nDestLength*sizeof(_CharType), 
			pszSrc, nSrcLength*sizeof(_CharType));
	}

	static void ConvertToBaseType(_Out_cap_(nDestLength) _CharType* pszDest, _In_ int nDestLength,
		_In_count_(nSrcLength) const wchar_t* pszSrc, _In_ int nSrcLength = -1) throw()
	{		
		// nLen is in XCHARs
		::WideCharToMultiByte(_AtlGetConversionACP(), 0, pszSrc, nSrcLength, pszDest, nDestLength, NULL, NULL);
	}

	_ATL_INSECURE_DEPRECATE("ChTraitsOS::ConvertToOem must be passed a buffer size")
	static void ConvertToOem(_Inout_ _CharType* pstrString) throw()
	{
		BOOL fSuccess=::CharToOemA(pstrString, pstrString);
		// old version can't report error
		ATLASSERT(fSuccess);
	}

	_ATL_INSECURE_DEPRECATE("ChTraitsOS::ConvertToAnsi must be passed a buffer size")
	static void ConvertToAnsi(_Inout_ _CharType* pstrString) throw()
	{
		BOOL fSuccess=::OemToCharA(pstrString, pstrString);
		// old version can't report error
		ATLASSERT(fSuccess);
	}

	static void ConvertToOem(_Out_cap_(size) _CharType* pstrString, _In_ size_t size)
	{
		if(size>UINT_MAX)
		{
			// API only allows DWORD size
			AtlThrow(E_INVALIDARG);
		}
		DWORD dwSize=static_cast<DWORD>(size);
		BOOL fSuccess=::CharToOemBuffA(pstrString, pstrString, dwSize);
		if(!fSuccess)
		{
			AtlThrowLastWin32();
		}
	}

	static void ConvertToAnsi(_Out_cap_(size) _CharType* pstrString, _In_ size_t size)
	{
		if(size>UINT_MAX)
		{
			// API only allows DWORD size
			AtlThrow(E_INVALIDARG);
		}
		DWORD dwSize=static_cast<DWORD>(size);
		BOOL fSuccess=::OemToCharBuffA(pstrString, pstrString, dwSize);
		if(!fSuccess)
		{
			AtlThrowLastWin32();
		}
	}

	static void FloodCharacters(_In_ _CharType ch, _In_ int nLength, _Out_bytecapcount_(nLength) _CharType* pstr) throw()
	{
		// nLength is in XCHARs
		memset(pstr, ch, nLength);
	}

	static BSTR AllocSysString(_In_count_(nDataLength) const _CharType* pchData, _In_ int nDataLength) throw()
	{
		int nLen = MultiByteToWideChar(_AtlGetConversionACP(), 0, pchData, nDataLength,
			NULL, NULL);
		BSTR bstr = ::SysAllocStringLen(NULL, nLen);
		if (bstr != NULL)
		{
			MultiByteToWideChar(_AtlGetConversionACP(), 0, pchData, nDataLength,
				bstr, nLen);
		}

		return bstr;
	}

	static BOOL ReAllocSysString(_In_bytecount_(nDataLength) const _CharType* pchData, _Inout_ BSTR* pbstr,
		_In_ int nDataLength) throw()
	{
		int nLen = MultiByteToWideChar(_AtlGetConversionACP(), 0, pchData,
			nDataLength, NULL, NULL);
		BOOL bSuccess =::SysReAllocStringLen(pbstr, NULL, nLen);
		if (bSuccess)
		{
			MultiByteToWideChar(_AtlGetConversionACP(), 0, pchData, nDataLength,
				*pbstr, nLen);
		}

		return bSuccess;
	}

	static DWORD FormatMessage(_In_ DWORD dwFlags, _In_ LPCVOID lpSource,
		_In_ DWORD dwMessageID, _In_ DWORD dwLanguageID, _Out_cap_(nSize) char* pstrBuffer,
		_In_ DWORD nSize, va_list* pArguments) throw()
	{
		return ::FormatMessageA(dwFlags, lpSource, dwMessageID, dwLanguageID,
				pstrBuffer, nSize, pArguments);
	}

	static int SafeStringLen(_In_opt_ const char* psz) throw()
	{
		// returns length in bytes
		return (psz != NULL) ? lstrlenA(psz) : 0;
	}

	static int SafeStringLen(_In_opt_ const wchar_t* psz) throw()
	{
		// returns length in wchar_ts
		return (psz != NULL) ? lstrlenW(psz) : 0;
	}

	static int GetCharLen(const wchar_t*) throw()
	{
		// returns char length
		return 1;
	}
	static int GetCharLen(_In_z_ const char* psz) throw()
	{
		const char* p = ::CharNextA(psz);
		return (p - psz);
	}

	static DWORD GetEnvironmentVariable(_In_ const _CharType* pstrVar,
		_Out_opt_cap_(dwSize) _CharType* pstrBuffer, _In_ DWORD dwSize) throw()
	{
		return ::GetEnvironmentVariableA(pstrVar, pstrBuffer, dwSize);
	}
};

// specialization for wchar_t
template<>
class ChTraitsOS< wchar_t > :
	public ChTraitsBase< wchar_t >
{
protected:
	static int CompareStringW(_In_ LCID lcid, _In_ DWORD dwFlags, 
		_In_count_(nLength1) LPCWSTR pszString1, _In_ int nLength1, _In_count_(nLength2) LPCWSTR pszString2, _In_ int nLength2)
	{
		return ::CompareStringW(lcid, dwFlags, pszString1, nLength1, 
			pszString2, nLength2);
	}
	static BOOL GetStringTypeExW(_In_ LCID lcid, _In_ DWORD dwInfoType, _In_count_(nLength) LPCWSTR pszSrc,
		_In_ int nLength, _Out_ LPWORD pwCharType)
	{
		return ::GetStringTypeExW(lcid, dwInfoType, pszSrc, nLength, pwCharType);
	}
	static int lstrcmpiW(_In_z_ LPCWSTR psz1, _In_z_ LPCWSTR psz2)
	{
		return ::lstrcmpiW(psz1, psz2);
	}
	static LPWSTR CharLowerW(_Inout_ LPWSTR psz)
	{
		return ::CharLowerW(psz);
	}
	static LPWSTR CharUpperW(_Inout_ LPWSTR psz)
	{
		return ::CharUpperW(psz);
	}
	static DWORD _GetEnvironmentVariableW(_In_z_ LPCWSTR pszName, _Out_opt_cap_(nSize) LPWSTR pszBuffer, _In_ DWORD nSize)
	{
		return ::GetEnvironmentVariableW(pszName, pszBuffer, nSize);
	}

public:
	static int tclen(const wchar_t*) throw()
	{
		return 1;
	}
	static LPCWSTR strchr(_In_z_ LPCWSTR p, _In_ wchar_t ch) throw()
	{
		return AtlstrchrT(p,ch);
	}
	static LPCWSTR strrchr(_In_z_ LPCWSTR p, _In_ wchar_t ch) throw()
	{		
		const wchar_t* pch = p+lstrlenW(p);
		while ((pch != p) && (*pch != ch))
		{
			pch--;
		}
		if (*pch == ch)
		{
			return pch;
		}
		else
		{
			return NULL;
		}
	}
	static wchar_t* _strrev(_Inout_z_ wchar_t* psz) throw()
	{
		// Optimize NULL, zero-length, and single-char case.
		#pragma warning(suppress:6385)
		if ((psz == NULL) || (psz[0] == L'\0') || (psz[1] == L'\0'))
			return psz;

		wchar_t* p = psz+(lstrlenW( psz )-1);
		wchar_t* q = psz;
		while(q < p)
		{
			wchar_t t = *q;
			*q = *p;
			*p = t;
			q++;
			p--;
		}
		return psz;
	}
	static LPCWSTR strstr(_In_z_ LPCWSTR pStr, _In_z_ LPCWSTR pCharSet) throw()
	{
		return strstrT< ChTraitsOS<XCHAR> >(pStr,pCharSet);				
	}
	static int strspn(_In_z_ const wchar_t* psz, _In_z_ const wchar_t* pszCharSet) throw()
	{
		int nRet = 0;
		const wchar_t* p = psz;
		while (*p != 0)
		{
			if(strchr(pszCharSet, *p) == NULL)
				break;
			nRet++;
			p++;
		}
		return nRet;
	}
	static int strcspn(_In_z_ const wchar_t* psz, _In_z_ const wchar_t* pszCharSet) throw()
	{
		int nRet = 0;
		const wchar_t* p = psz;
		while (*p != 0)
		{
			if(strchr(pszCharSet, *p) != NULL)
				break;
			nRet++;
			p++;
		}
		return nRet;
	}
	static LPCWSTR strpbrk(_In_z_ LPCWSTR psz, _In_z_ LPCWSTR pszCharSet) throw()
	{
		const wchar_t* p = psz;
		while (*p != 0)
		{
			if (strchr(pszCharSet, *p) != NULL)
				return p;
			p++;
		}
		return NULL;
	}

	static wchar_t* CharNext(_In_z_ const wchar_t* p) throw()
	{
		return AtlCharNext(p);
	}

	static int IsDigit(_In_ wchar_t ch) throw()
	{
		WORD type;
		GetStringTypeExW(0, CT_CTYPE1, &ch, 1, &type);
		return (type & C1_DIGIT) == C1_DIGIT;
	}

	static int IsSpace(_In_ wchar_t ch) throw()
	{
		WORD type;
		GetStringTypeExW(0, CT_CTYPE1, &ch, 1, &type);
		return (type & C1_SPACE) == C1_SPACE;
	}


	static int StringCompare(_In_z_ const wchar_t* pstrOne,
		_In_z_ const wchar_t* pstrOther) throw()
	{
		return wcscmp(pstrOne, pstrOther);
	}

	static int StringCompareIgnore(_In_z_ const wchar_t* pstrOne,
		_In_z_ const wchar_t* pstrOther) throw()
	{
		return lstrcmpiW(pstrOne, pstrOther);
	}

	static int StringCollate(_In_z_ const wchar_t* pstrOne,
		_In_z_ const wchar_t* pstrOther) throw()
	{ 
		int nRet;

		nRet = CompareStringW(GetThreadLocale(), 0, pstrOne, -1, pstrOther, -1);
		ATLASSERT(nRet != 0);
		return nRet-2;  // Convert to strcmp convention.  This really is documented.
	}

	static int StringCollateIgnore(_In_z_ const wchar_t* pstrOne,
		_In_z_ const wchar_t* pstrOther) throw()
	{
		int nRet = CompareStringW(GetThreadLocale(), NORM_IGNORECASE, pstrOne, -1, pstrOther, -1);
		ATLASSERT(nRet != 0);
		return nRet-2;  // Convert to strcmp convention.  This really is documented.
	}

	static LPCWSTR StringFindString(_In_z_ LPCWSTR pstrBlock,
		_In_z_ LPCWSTR pstrMatch) throw()
	{
		return strstr(pstrBlock, pstrMatch);
	}

	static LPWSTR StringFindString( _In_z_ LPWSTR pszBlock, _In_z_ LPCWSTR pszMatch ) throw()
	{
		return( const_cast< LPWSTR >( StringFindString( const_cast< LPCWSTR >( pszBlock ), pszMatch ) ) );
	}

	static LPCWSTR StringFindChar(_In_z_ LPCWSTR pstrBlock,
		_In_ wchar_t pstrMatch) throw()
	{
		return strchr(pstrBlock, pstrMatch);
	}

	static LPCWSTR StringFindCharRev(_In_z_ LPCWSTR pstr, _In_ wchar_t ch) throw()
	{
		return strrchr(pstr, ch);
	}

	static LPCWSTR StringScanSet(_In_z_ LPCWSTR pszBlock,
		_In_z_ LPCWSTR pszMatch) throw()
	{
		return strpbrk(pszBlock, pszMatch);
	}

	static int StringSpanIncluding(_In_z_ const wchar_t* pszBlock,
		_In_z_ const wchar_t* pszSet) throw()
	{
		return strspn(pszBlock, pszSet);
	}

	static int StringSpanExcluding(_In_z_ const wchar_t* pszBlock,
		_In_z_ const wchar_t* pszSet) throw()
	{
		return strcspn(pszBlock, pszSet);
	}

	_ATL_INSECURE_DEPRECATE("ChTraitsOS::StringUppercase must be passed a buffer size")
	static wchar_t* StringUppercase(_Inout_ wchar_t* psz) throw()
	{
		CharUpperW(psz);
		return psz;
	}

	_ATL_INSECURE_DEPRECATE("ChTraitsOS::StringUppercase must be passed a buffer size")
	static wchar_t* StringLowercase(_Inout_ wchar_t* psz) throw()
	{
		CharLowerW(psz);
		return psz;
	}

	static wchar_t* StringUppercase(_Inout_cap_(size) wchar_t* psz, _In_ size_t size) throw()
	{
		if(size>(UINT_MAX/sizeof(wchar_t)))
		{
			// API only allows DWORD size
			AtlThrow(E_INVALIDARG);
		}
		DWORD dwSize=static_cast<DWORD>(size);
		CharUpperBuffW(psz, dwSize);
		return psz;
	}

	static wchar_t* StringLowercase(_Inout_cap_(size) wchar_t* psz, _In_ size_t size) throw()
	{
		if(size>(UINT_MAX/sizeof(wchar_t)))
		{
			// API only allows DWORD size
			AtlThrow(E_INVALIDARG);
		}
		DWORD dwSize=static_cast<DWORD>(size);
		CharLowerBuffW(psz, dwSize);
		return psz;
	}

	static wchar_t* StringReverse(_Inout_z_ wchar_t* psz) throw()
	{
		return _strrev(psz);
	}

#ifdef _UNICODE
	static int GetFormattedLength(_In_ _Printf_format_string_ const wchar_t* pszFormat, va_list args)
	{
		wchar_t szBuffer[1028];
		int nLength = 0;

		SetLastError(ERROR_SUCCESS);
#pragma warning(push)
#pragma warning(disable:4996)
		// wvsprintf always truncates the output to 1024 character plus the '\0'.
		// Note that we are using wvsprintf only in the MIN_CRT case; wvsprintf is
		// an insecure function and should be avoided. Here the use of wvsprintf
		// is safe and the only way to get a string formatted without using the CRT.
		nLength = wvsprintfW(szBuffer, pszFormat, args);
#pragma warning(pop)
		ATLENSURE(GetLastError() == ERROR_SUCCESS);
		ATLASSERT(nLength >= 0);
		ATLASSERT(nLength <= 1024);

		return nLength;
	}

	_ATL_INSECURE_DEPRECATE("ChTraitsOS::Format must be passed a buffer size")
	static int Format(_Out_ wchar_t* pszBuffer, _In_ _Printf_format_string_ const wchar_t* pszFormat,
		va_list args) throw()
	{
#pragma warning(push)
#pragma warning(disable:4996)
		return wvsprintfW(pszBuffer, pszFormat, args);
#pragma warning(pop)
	}

	static int Format(_Out_cap_(nLength) wchar_t* pszBuffer, _In_ size_t nLength, _In_ _Printf_format_string_ const wchar_t* pszFormat, va_list args )
	{
		wchar_t buffSafe[1028];
		int nCharsWritten = 0;

		SetLastError(ERROR_SUCCESS);
#pragma warning(push)
#pragma warning(disable:4996)
		// wvsprintf always truncates the output to 1024 character plus the '\0'.
		// Note that we are using wvsprintf only in the MIN_CRT case; wvsprintf is
		// an insecure function and should be avoided. Here the use of wvsprintf
		// is safe and the only way to get a string formatted without using the CRT.
		nCharsWritten = wvsprintfW(buffSafe, pszFormat, args);
#pragma warning(pop)
		ATLENSURE(GetLastError() == ERROR_SUCCESS);
		ATLENSURE(nCharsWritten <= 1024);
		//nlength should have room for nCharsWritten + NULL
		ATLENSURE_THROW((size_t)nCharsWritten < nLength ,E_INVALIDARG);
		ATLENSURE_THROW(wcslen(buffSafe) < nLength ,E_INVALIDARG);
#pragma warning(push)
#pragma warning(disable:6386)
		/* prefast noise 497597 */
		Checked::wcscpy_s(pszBuffer,nLength,buffSafe);
#pragma warning(pop)
		return nCharsWritten;
	}
#endif

	static int GetBaseTypeLength(_In_z_ const char* pszSrc) throw()
	{
		// Returns required buffer size in wchar_ts
		return ::MultiByteToWideChar(_AtlGetConversionACP(), 0, pszSrc, -1, NULL, 0)-1;
	}

	static int GetBaseTypeLength(_In_count_(nLength) const char* pszSrc, _In_ int nLength) throw()
	{
		// Returns required buffer size in wchar_ts
		return ::MultiByteToWideChar(_AtlGetConversionACP(), 0, pszSrc, nLength, NULL, 0);
	}

	static int GetBaseTypeLength(_In_z_ const wchar_t* pszSrc) throw()
	{
		// Returns required buffer size in wchar_ts
		return lstrlenW(pszSrc);
	}

	static int GetBaseTypeLength(_In_count_(nLength) const wchar_t* pszSrc, _In_ int nLength) throw()
	{
		(void)pszSrc;
		// Returns required buffer size in wchar_ts
		return nLength;
	}

	static void ConvertToBaseType(_Out_cap_(nDestLength) wchar_t* pszDest, int nDestLength,
		_In_count_(nSrcLength) const char* pszSrc, _In_ int nSrcLength = -1) throw()
	{
		// nLen is in wchar_ts
		::MultiByteToWideChar(_AtlGetConversionACP(), 0, pszSrc, nSrcLength, pszDest, nDestLength);
	}

	static void ConvertToBaseType(_Out_cap_(nDestLength) wchar_t* pszDest, _In_ int nDestLength,
		_In_count_(nSrcLength) const wchar_t* pszSrc, int nSrcLength = -1) throw()
	{
		if (nSrcLength == -1) { nSrcLength=1 + GetBaseTypeLength(pszSrc); }
		// nLen is in wchar_ts
		Checked::wmemcpy_s(pszDest, nDestLength, pszSrc, nSrcLength);
	}

	// this conversion on Unicode strings makes no sense
	/*
	static void ConvertToOem(wchar_t*)
	{
		ATLASSERT(FALSE);
	}
	*/

	// this conversion on Unicode strings makes no sense
	/*
	static void ConvertToAnsi(wchar_t*)
	{
		ATLASSERT(FALSE);
	}
	*/

	static void FloodCharacters(_In_ wchar_t ch, _In_ int nLength, _Out_capcount_(nLength) wchar_t* pstr) throw()
	{
		// nLength is in XCHARs
		for (int i = 0; i < nLength; i++)
			pstr[i] = ch;
	}

	static BSTR AllocSysString(_In_count_(nDataLength) const wchar_t* pchData, _In_ int nDataLength) throw()
	{
		BSTR bstr = ::SysAllocStringLen(pchData, nDataLength);
		return bstr;
	}

	static BOOL ReAllocSysString(_In_count_(nDataLength) const wchar_t* pchData, _Inout_ BSTR* pbstr,
		_In_ int nDataLength) throw()
	{
		return ::SysReAllocStringLen(pbstr, pchData, nDataLength);
	}

#ifdef _UNICODE
	static DWORD FormatMessage(_In_ DWORD dwFlags, _In_ LPCVOID lpSource,
		_In_ DWORD dwMessageID, _In_ DWORD dwLanguageID, _Out_cap_(nSize) wchar_t* pstrBuffer,
		_In_ DWORD nSize, va_list* pArguments) throw()
	{
		return ::FormatMessageW(dwFlags, lpSource, dwMessageID, dwLanguageID,
				pstrBuffer, nSize, pArguments);
	}
#endif
	static int SafeStringLen(_In_opt_ const char* psz) throw()
	{
		// returns length in bytes
		return (psz != NULL) ? lstrlenA(psz) : 0;
	}

	static int SafeStringLen(_In_opt_ const wchar_t* psz) throw()
	{
		// returns length in wchar_ts
		return (psz != NULL) ? lstrlenW(psz) : 0;
	}

	static int GetCharLen(const wchar_t*) throw()
	{
		// returns char length
		return 1;
	}
	static int GetCharLen(_In_z_ const char* psz) throw()
	{
		LPCSTR p = ::CharNextA( psz );
		return int( p-psz );
	}

	static DWORD GetEnvironmentVariable(_In_z_ const wchar_t* pstrVar,
		_Out_opt_cap_(dwSize) wchar_t* pstrBuffer, _In_ DWORD dwSize) throw()
	{
		return ::GetEnvironmentVariableW(pstrVar, pstrBuffer, dwSize);
	}
};

template <class ChTraits>
inline typename ChTraits::PCXSTR strstrT(_In_ typename ChTraits::PCXSTR pStr,_In_ typename ChTraits::PCXSTR pCharSet)
{
	ATLASSERT(pStr != NULL);
	size_t nCharSetLen = ChTraits::GetBaseTypeLength(pCharSet);
	if (nCharSetLen == 0)
		return pStr;
	//lstrlenA returns length in bytes, not chars.
	size_t nStrLen = ChTraits::GetBaseTypeLength(pStr);
	ChTraits::PCXSTR pStrEnd=pStr + nStrLen;
	const ChTraits::XCHAR* pMatch;
	const ChTraits::XCHAR* pStart = pStr;
	while ((pMatch = ChTraits::strchr(pStart, *pCharSet)) != NULL)
	{
		size_t nCharsLeftInStr=pStrEnd - pMatch;
		if (nCharsLeftInStr < nCharSetLen)
		{
			break;
		}
		if (memcmp(pMatch, pCharSet, nCharSetLen*sizeof(ChTraits::XCHAR)) == 0)
		{
			return pMatch;
		}
		pStart = ChTraits::CharNext(pMatch);
	}

	return NULL;
}

template< typename _BaseType = char, class StringIterator = ChTraitsOS< _BaseType > >
class StrTraitATL : public StringIterator
{
public:
	static HINSTANCE FindStringResourceInstance(_In_ UINT nID) throw()
	{
		return( AtlFindStringResourceInstance( nID ) );
	}

	static IAtlStringMgr* GetDefaultManager() throw()
	{
		return( &g_strmgr );
	}
};

#ifndef _ATL_CSTRING_NO_CRT
typedef CStringT< wchar_t, StrTraitATL< wchar_t, ChTraitsCRT< wchar_t > > > CAtlStringW;
typedef CStringT< char, StrTraitATL< char, ChTraitsCRT< char > > > CAtlStringA;
typedef CStringT< TCHAR, StrTraitATL< TCHAR, ChTraitsCRT< TCHAR > > > CAtlString;
#else  // _ATL_CSTRING_NO_CRT
typedef CStringT< wchar_t, StrTraitATL< wchar_t > > CAtlStringW;
typedef CStringT< char, StrTraitATL< char > > CAtlStringA;
typedef CStringT< TCHAR, StrTraitATL< TCHAR > > CAtlString;
#endif  // _ATL_CSTRING_NO_CRT

#ifndef _AFX
typedef CAtlStringW CStringW;
typedef CAtlStringA CStringA;
typedef CAtlString CString;
#endif

template< typename T >
class CElementTraits;

template<>
class CElementTraits< ATL::CAtlStringA > :
	public CStringElementTraits< ATL::CAtlStringA >
{
};

template<>
class CElementTraits< ATL::CAtlStringW > :
	public CStringElementTraits< ATL::CAtlStringW >
{
};

}; //namespace ATL

#pragma pack(pop)
#endif // __ATLSTR_H__

/////////////////////////////////////////////////////////////////////////////
