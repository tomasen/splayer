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

#include "StdAfx.h"
#include "BaseSplitterFile.h"
#include "..\..\..\dsutil\dsutil.h"
#include "..\..\..\svplib\svplib.h"

#define SVP_LogMsg5 __noop
//
// CBaseSplitterFile
//

CBaseSplitterFile::CBaseSplitterFile(IAsyncReader* pAsyncReader, HRESULT& hr, int cachelen, bool fRandomAccess, bool fStreaming)
	: m_pAsyncReader(pAsyncReader)
	, m_fStreaming(false)
	, m_fRandomAccess(false)
	, m_pos(0), m_len(0)
	, m_bitbuff(0), m_bitlen(0)
	, m_cachepos(0), m_cachelen(0)
{
	if(!m_pAsyncReader) {hr = E_UNEXPECTED; return;}

	LONGLONG total = 0, available;
	hr = m_pAsyncReader->Length(&total, &available);

	m_fStreaming = total == 0 && available > 0;
	m_fRandomAccess = total > 0 && total == available;
	m_len = total;
SVP_LogMsg5(L"CBaseSplitterFile::CBaseSplitterFile %d %d %d %d ",FAILED(hr) , fRandomAccess && !m_fRandomAccess  , !fStreaming && m_fStreaming , m_len < 0);
	if(FAILED(hr) || fRandomAccess && !m_fRandomAccess  || !fStreaming && m_fStreaming || m_len < 0)
	{
		hr = E_FAIL;
		return;
	}
SVP_LogMsg5(L"CBaseSplitterFile::CBaseSplitterFile2");
	if(!SetCacheSize(cachelen))
	{
		hr = E_OUTOFMEMORY; 
		return;
	}
SVP_LogMsg5(L"CBaseSplitterFile::CBaseSplitterFile4");
	hr = S_OK;
}

bool CBaseSplitterFile::SetCacheSize(int cachelen)
{
	m_pCache.Free();
	m_cachetotal = 0;
	m_pCache.Allocate((size_t)cachelen);
	if(!m_pCache) return false;
	m_cachetotal = cachelen;
	m_cachelen = 0;
	return true;
}

__int64 CBaseSplitterFile::GetPos()
{
	return m_pos - (m_bitlen>>3);
}

__int64 CBaseSplitterFile::GetAvailable()
{
	LONGLONG total, available = 0;
	m_pAsyncReader->Length(&total, &available);
	return available;
}

__int64 CBaseSplitterFile::GetLength(bool fUpdate)
{
	if(m_fStreaming)
	{
		m_len = GetAvailable();
	}
	else if(fUpdate)
	{
		LONGLONG total = 0, available;
		m_pAsyncReader->Length(&total, &available);
		m_len = total;
	}

	return m_len;
}

void CBaseSplitterFile::Skip(__int64 pos)
{
	Seek(m_pos+pos);
}
void CBaseSplitterFile::Seek(__int64 pos)
{
	__int64 len = GetLength();
	m_pos = min(max(pos, 0), len);
	BitFlush();
}
#if 0

HRESULT CBaseSplitterFile::Read(BYTE* pData, __int64 len)
{
	CheckPointer(m_pAsyncReader, E_NOINTERFACE);

	HRESULT hr = S_OK;

	if(!m_fRandomAccess)
	{
		LONGLONG total = 0, available = -1;
		m_pAsyncReader->Length(&total, &available);
		if(total == available) {m_fRandomAccess = true; OnComplete();}
	}

	if(m_cachetotal == 0 || !m_pCache)
	{
		hr = m_pAsyncReader->SyncRead(m_pos, (long)len, pData);
		m_pos += len;
		return hr;
	}

	BYTE* pCache = m_pCache;

	if(m_cachepos <= m_pos && m_pos < m_cachepos + m_cachelen)
	{
		__int64 minlen = min(len, m_cachelen - (m_pos - m_cachepos));

		memcpy(pData, &pCache[m_pos - m_cachepos], (size_t)minlen);

		len -= minlen;
		m_pos += minlen;
		pData += minlen;
	}

	while(len > m_cachetotal)
	{
		hr = m_pAsyncReader->SyncRead(m_pos, (long)m_cachetotal, pData);
		if(S_OK != hr) return hr;

		len -= m_cachetotal;
		m_pos += m_cachetotal;
		pData += m_cachetotal;
	}

	while(len > 0)
	{
		__int64 tmplen = GetLength();
		__int64 maxlen = min(tmplen - m_pos, m_cachetotal);
		__int64 minlen = min(len, maxlen);
		if(minlen <= 0) return S_FALSE;

		hr = m_pAsyncReader->SyncRead(m_pos, (long)maxlen, pCache);
		if(S_OK != hr) return hr;

		m_cachepos = m_pos;
		m_cachelen = maxlen;

		memcpy(pData, pCache, (size_t)minlen);

		len -= minlen;
		m_pos += minlen;
		pData += minlen;
	}

	return hr;
}
#else
HRESULT CBaseSplitterFile::Read(BYTE* pData, __int64 len)
{
	CheckPointer(m_pAsyncReader, E_NOINTERFACE);

	HRESULT hr = S_OK;

	if(!m_fRandomAccess)
	{
		LONGLONG total = 0, available = -1;
		m_pAsyncReader->Length(&total, &available);
		if(total == available) {m_fRandomAccess = true; OnComplete();}
	}


	if(m_cachetotal == 0 || !m_pCache)
	{
		hr = m_pAsyncReader->SyncRead(m_pos, (long)len, pData);
		m_pos += len;
		return hr;
	}


	BYTE* pCache = m_pCache;

	if(m_cachepos <= m_pos && m_pos < m_cachepos + m_cachelen)
	{
		__int64 minlen = min(len, m_cachelen - (m_pos - m_cachepos));
 		
		if(!pData || len <= 0 || minlen <= 0) {
			SVP_LogMsg5(_T("!pData || len <= 0 || minlen <= 0"));
		    return S_OK; 
		}
		__try{ 
		      memcpy_s(pData, len, &pCache[m_pos - m_cachepos], (size_t)minlen); 
		}__except(EXCEPTION_EXECUTE_HANDLER){SVP_LogMsg5(_T(" memcpy_s(pData, len, &pCache[m_pos - m_cachepos], (size_t)minlen); ")); return S_OK;} 
	/*	if(pData && len > 0){
			__try{
				memcpy_s(pData, len, &pCache[m_pos - m_cachepos], (size_t)minlen);
				//memcpy(pData, &pCache[m_pos - m_cachepos], (size_t)minlen);
			}__except(EXCEPTION_EXECUTE_HANDLER){}
		}
		if(minlen < 0) minlen = 0; */
		len -= minlen;
		m_pos += minlen;
		pData += minlen;
	}

	while(len > m_cachetotal)
	{
		hr = m_pAsyncReader->SyncRead(m_pos, (long)m_cachetotal, pData);
		if(S_OK != hr) return hr;

		len -= m_cachetotal;
		m_pos += m_cachetotal;
		pData += m_cachetotal;
	}
	
	
	while(len > 0)
	{
		__int64 tmplen = GetLength();
		__int64 maxlen = min(tmplen - m_pos, m_cachetotal);
		__int64 minlen = min(len, maxlen);
		if(minlen <= 0) return S_FALSE;

		hr = m_pAsyncReader->SyncRead(m_pos, (long)maxlen, pCache);
		if(S_OK != hr) return hr;

		m_cachepos = m_pos;
		m_cachelen = maxlen;

		if(!pData) {
			SVP_LogMsg5(_T( "Base (!pData)"));
		     return S_OK; 
		}

		__try{		
			memcpy(pData, pCache, (size_t)minlen);
		}__except(EXCEPTION_EXECUTE_HANDLER){ SVP_LogMsg5(_T( "memcpy(pData, pCache, (size_t)minlen)")); return S_OK; }
		/*if(pData && minlen > 0){
			__try{
				memcpy(pData, pCache, (size_t)minlen);
			}__except(EXCEPTION_EXECUTE_HANDLER){}
		}
		if(minlen < 0) minlen = 0; */
		len -= minlen;
		m_pos += minlen;
		pData += minlen;
	}

	return hr;
}
#endif
UINT64 CBaseSplitterFile::BitRead(int nBits, bool fPeek)
{
	ASSERT(nBits >= 0 && nBits <= 64);

	while(m_bitlen < nBits)
	{
		m_bitbuff <<= 8;
		if(S_OK != Read((BYTE*)&m_bitbuff, 1)) {return 0;} // EOF? // ASSERT(0);
		m_bitlen += 8;
	}

	int bitlen = m_bitlen - nBits;

	UINT64 ret = (m_bitbuff >> bitlen) & ((1ui64 << nBits) - 1);

	if(!fPeek)
	{
		m_bitbuff &= ((1ui64 << bitlen) - 1);
		m_bitlen = bitlen;
	}

	return ret;
}

void CBaseSplitterFile::BitByteAlign()
{
	m_bitlen &= ~7;
}

void CBaseSplitterFile::BitFlush()
{
	m_bitlen = 0;
}

HRESULT CBaseSplitterFile::ByteRead(BYTE* pData, __int64 len)
{
    Seek(GetPos());
	return Read(pData, len);
}

UINT64 CBaseSplitterFile::UExpGolombRead()
{
	int n = -1;
	for(BYTE b = 0; !b; n++) b = BitRead(1);
	return (1ui64 << n) - 1 + BitRead(n);
}

INT64 CBaseSplitterFile::SExpGolombRead()
{
	UINT64 k = UExpGolombRead();
	return ((k&1) ? 1 : -1) * ((k + 1) >> 1);
}

HRESULT CBaseSplitterFile::HasMoreData(__int64 len, DWORD ms)
{
	__int64 available = GetLength() - GetPos();

	if(!m_fStreaming)
	{
		return available < 1 ? E_FAIL : S_OK;
	}

	if(available < len)
	{
		if(ms > 0) Sleep(ms);
		return S_FALSE;
	}

	return S_OK;
}

