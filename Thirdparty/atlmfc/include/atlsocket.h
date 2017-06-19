// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

#ifndef __ATL_SOCKET__
#define __ATL_SOCKET__

#pragma warning(push)
#pragma warning(disable: 4191) // unsafe conversion from 'functionptr1' to 'functionptr2'

#include <winsock2.h>
#include <mswsock.h>

#pragma warning(push)
#pragma warning(disable : 4127)
#pragma warning(disable : 4706)
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT <= 0x0500)
/* psdk prefast noise */
#pragma warning(disable : 6011)
#endif
#include <ws2tcpip.h>
#pragma warning(pop)

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#if _WIN32_WINNT < 0x0502
#define ADDRINFOT addrinfo
#define GetAddrInfo getaddrinfo
#define FreeAddrInfo freeaddrinfo
#endif


#pragma pack(push,_ATL_PACKING)
namespace ATL
{
	class CSocketAddr;

	////////////////////////////////////////////////////////////////////////
	// class CSocketAddr
	//
	// Description:
	// This class provides an abstraction over an internet address. It provides
	// an IP version agnostic way to look up network addresses for use with
	// Windows sockets API functions and Socket wrappers in libraries
	// The members of this class that are used to look up network addresses
	// use the getaddrinfo Win32 API, which is an IP version agnostic function
	// for retrieving network addresses. This class can find both IPv4 and
	// IPv6 network addresses.
	////////////////////////////////////////////////////////////////////////
	class CSocketAddr
	{
	public:
		// Construction/Destruction
		CSocketAddr() throw();
		virtual ~CSocketAddr() throw();

		// Operations
		int FindAddr
		(
			LPCTSTR szHost, 				// Host name or dotted IP address
			LPCTSTR szPortOrServiceName,	// Port number or name of service on host
			int flags,					// 0 or combination of AI_PASSIVE, AI_CANONNAME or AI_NUMERICHOST
			int addr_family,			// Address family (such as PF_INET)
			int sock_type,				// Socket type (such as SOCK_STREAM)
			int ai_proto 				// Protocol (such as IPPROTO_IP or IPPROTO_IPV6)
		);

		int FindAddr
		(
			LPCTSTR szHost, 				// Host name or dotted IP address
			int nPortNo,				// Port number
			int flags,					// 0 or combination of AI_PASSIVE, AI_CANONNAME or AI_NUMERICHOST
			int addr_family,			// Address family (such as PF_INET)
			int sock_type,				// Socket type (such as SOCK_STREAM)
			int ai_proto 				// Protocol (such as IPPROTO_IP or IPPROTO_IPV6)
		) throw();

		int FindINET4Addr
		(
			LPCTSTR szHost, 				// Host name
			int nPortNo, 				// Port number
			int flags = 0, 				// 0 or combination of AI_PASSIVE, AI_CANONNAME or AI_NUMERICHOST
			int sock_type = SOCK_STREAM // Socket type (such as SOCK_STREAM or SOCK_DGRAM)
		) throw();

		int FindINET6Addr
		(
			LPCTSTR szHost, 				// Host name
			int nPortNo, 				// Port number
			int flags = 0, 				// 0 or combination of AI_PASSIVE, AI_CANONNAME or AI_NUMERICHOST
			int sock_type = SOCK_STREAM // Socket type (such as SOCK_STREAM or SOCK_DGRAM)
		) throw();
		
		ADDRINFOT* const GetAddrInfoList() const;
		ADDRINFOT* const GetAddrInfo(int nIndex = 0) const;

		// Implementation
	private:
		ADDRINFOT *m_pAddrs;
	};
}; // namespace ATL
 


#include <atlsocket.inl>

#pragma pack(pop)
#pragma warning(pop)

#endif __ATL_SOCKET__