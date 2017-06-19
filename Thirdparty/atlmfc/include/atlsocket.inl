// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

#include <wtypes.h>
#include <atlconv.h>
#include <tchar.h>

namespace ATL
{
////////////////////////////////////////////////////////////////////////////
// CSocketAddr implmenetation. 
////////////////////////////////////////////////////////////////////////////

inline CSocketAddr::CSocketAddr() throw()
{
	m_pAddrs = NULL;
}

inline CSocketAddr::~CSocketAddr() throw()
{
	if (m_pAddrs != NULL)
	{
		FreeAddrInfo(m_pAddrs);
		m_pAddrs = NULL;
	}
}

inline int CSocketAddr::FindAddr(
	LPCTSTR szHost,
	LPCTSTR szPortOrServiceName,
	int flags,
	int addr_family,
	int sock_type,
	int ai_proto
) throw()
{
	if (m_pAddrs)
	{
		FreeAddrInfo(m_pAddrs);
		m_pAddrs = NULL;
	}

	ADDRINFOT hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_flags = flags;
	hints.ai_family = addr_family;
	hints.ai_socktype = sock_type;
	hints.ai_protocol = ai_proto;
#if _WIN32_WINNT < 0x0502
#ifdef _UNICODE
	USES_CONVERSION_EX;
	const char * pszHost = W2CA_EX(szHost, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	const char * pszPortOrServiceName = W2CA_EX(szPortOrServiceName, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
	if(pszHost == NULL || pszPortOrServiceName == NULL )
	{
		WSASetLastError(WSA_NOT_ENOUGH_MEMORY);
		return SOCKET_ERROR;
	}
#else
	const char * pszHost = szHost;
	const char * pszPortOrServiceName = szPortOrServiceName;
#endif
	return ::GetAddrInfo(pszHost, pszPortOrServiceName, &hints, &m_pAddrs);
#else
	return ::GetAddrInfo(szHost, szPortOrServiceName, &hints, &m_pAddrs);
#endif	
}

inline int CSocketAddr::FindAddr(
	LPCTSTR szHost,
	int nPortNo,
	int flags,
	int addr_family,
	int sock_type,
	int ai_proto
) throw()
{
	// convert port number to string
	TCHAR szPort[12];
	if(::_itot_s(nPortNo, szPort, _countof(szPort), 10))
	{
		WSASetLastError(WSA_NOT_ENOUGH_MEMORY);
		return SOCKET_ERROR;
	}

	return FindAddr(szHost, szPort, flags, addr_family, sock_type, ai_proto);
} 

inline int CSocketAddr::FindINET4Addr
(
	LPCTSTR szHost,
	int nPortNo,
	int flags /* = 0 */,
	int sock_type /* = SOCK_STREAM */
) throw()
{
	// convert port number to string
	TCHAR szPort[12];
	if(::_itot_s(nPortNo, szPort, _countof(szPort), 10))
	{
		WSASetLastError(WSA_NOT_ENOUGH_MEMORY);
		return SOCKET_ERROR;
	}
	return FindAddr(szHost, szPort, flags, PF_INET, sock_type, IPPROTO_IP);
}

inline int CSocketAddr::FindINET6Addr
(
	LPCTSTR szHost,
	int nPortNo,
	int flags /* = 0 */,
	int sock_type /* = SOCK_STREAM */
) throw()
{
	// convert port number to string
	TCHAR szPort[12];
	if(::_itot_s(nPortNo, szPort, _countof(szPort), 10))
	{
		WSASetLastError(WSA_NOT_ENOUGH_MEMORY);
		return SOCKET_ERROR;
	}

	// TEMP (alecont): With the new PSDK, IPPROTO_IPV6 is available only if _WIN32_WINNT >= 0x0501
	return FindAddr(szHost, szPort, flags, PF_INET6, sock_type, /*IPPROTO_IPV6*/ 41);
}

inline ADDRINFOT* const CSocketAddr::GetAddrInfoList() const
{
	return m_pAddrs;
}

inline ADDRINFOT* const CSocketAddr::GetAddrInfo(int nIndex /* = 0 */) const
{
	if (!m_pAddrs)
		return NULL;
	ADDRINFOT *pAI = m_pAddrs;
	while (nIndex > 0 && pAI != NULL)
	{
		pAI = pAI->ai_next;
		nIndex --;
	}
	return pAI;
}


}; // namespace ATL