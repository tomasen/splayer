// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.
#ifndef __ATLSECURITY_H__
#define __ATLSECURITY_H__

#pragma once

#include <sddl.h>
#include <userenv.h>
#include <aclapi.h>
#include <atlcoll.h>
#include <atlstr.h>

#pragma warning(push)
#pragma warning(disable: 4625) // copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(disable: 4626) // assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning(disable: 4571) //catch(...) blocks compiled with /EHs do NOT catch or re-throw Structured Exceptions
#ifndef _CPPUNWIND
#pragma warning (disable : 4702)	// unreachable code
#endif


#pragma pack(push,_ATL_PACKING)
namespace ATL
{
#pragma comment(lib, "userenv.lib")

// **************************************************************
// CSid

class CSid
{

public:
	CSid() throw();

	explicit CSid(LPCTSTR pszAccountName, LPCTSTR pszSystem = NULL) throw(...);
	explicit CSid(const SID *pSid, LPCTSTR pszSystem = NULL) throw(...);
	CSid(const SID_IDENTIFIER_AUTHORITY &IdentifierAuthority, BYTE nSubAuthorityCount, ...) throw(...);
	virtual ~CSid() throw();

	CSid(const CSid &rhs) throw(...);
	CSid &operator=(const CSid &rhs) throw(...);

	CSid(const SID &rhs) throw(...);
	CSid &operator=(const SID &rhs) throw(...);

	typedef CAtlArray<CSid> CSidArray;

	bool LoadAccount(LPCTSTR pszAccountName, LPCTSTR pszSystem = NULL) throw(...);
	bool LoadAccount(const SID *pSid, LPCTSTR pszSystem = NULL) throw(...);

	LPCTSTR AccountName() const throw(...);
	LPCTSTR Domain() const throw(...);
	LPCTSTR Sid() const throw(...);

	const SID *GetPSID() const throw(...);
	operator const SID *() const throw(...);
	SID_NAME_USE SidNameUse() const throw();

	UINT GetLength() const throw();

	// SID functions
	bool EqualPrefix(const CSid &rhs) const throw();
	bool EqualPrefix(const SID &rhs) const throw();

	const SID_IDENTIFIER_AUTHORITY *GetPSID_IDENTIFIER_AUTHORITY() const throw();
	DWORD GetSubAuthority(DWORD nSubAuthority) const throw();
	UCHAR GetSubAuthorityCount() const throw();
	bool IsValid() const throw();

private:
	void Copy(const SID &rhs) throw(...);
	void Clear() throw();
	void GetAccountNameAndDomain() const throw(...);
	SID* _GetPSID() const throw();

	BYTE m_buffer[SECURITY_MAX_SID_SIZE];
	bool m_bValid; // true if the CSid has been given a value

	mutable SID_NAME_USE m_sidnameuse;
	mutable CString m_strAccountName;
	mutable CString m_strDomain;
	mutable CString m_strSid;

	CString m_strSystem;
};

bool operator==(const CSid &lhs, const CSid &rhs) throw();
bool operator!=(const CSid &lhs, const CSid &rhs) throw();

// sort operations are provided to allow CSids to be put into
// sorted stl collections (stl::[multi]map, stl::[multi]set)
bool operator<(const CSid &lhs, const CSid &rhs) throw();
bool operator>(const CSid &lhs, const CSid &rhs) throw();
bool operator<=(const CSid &lhs, const CSid &rhs) throw();
bool operator>=(const CSid &lhs, const CSid &rhs) throw();

// **************************************************************
// Well-known sids

namespace Sids
{
// Universal
CSid Null() throw(...);
CSid World() throw(...);
CSid Local() throw(...);
CSid CreatorOwner() throw(...);
CSid CreatorGroup() throw(...);
CSid CreatorOwnerServer() throw(...);
CSid CreatorGroupServer() throw(...);

// NT Authority
CSid Dialup() throw(...);
CSid Network() throw(...);
CSid Batch() throw(...);
CSid Interactive() throw(...);
CSid Service() throw(...);
CSid AnonymousLogon() throw(...);
CSid Proxy() throw(...);
CSid ServerLogon() throw(...);
CSid Self() throw(...);
CSid AuthenticatedUser() throw(...);
CSid RestrictedCode() throw(...);
CSid TerminalServer() throw(...);
CSid System() throw(...);
CSid NetworkService() throw (...);

// NT Authority\BUILTIN
CSid Admins() throw(...);
CSid Users() throw(...);
CSid Guests() throw(...);
CSid PowerUsers() throw(...);
CSid AccountOps() throw(...);
CSid SystemOps() throw(...);
CSid PrintOps() throw(...);
CSid BackupOps() throw(...);
CSid Replicator() throw(...);
CSid RasServers() throw(...);
CSid PreW2KAccess() throw(...);
} // namespace Sids

//***************************************
// CAcl
//		CAce
//
//		CDacl
//			CAccessAce
//
//		CSacl
//			CAuditAce
//***************************************

// **************************************************************
// CAcl

class CAcl
{
public:
	CAcl() throw();
	virtual ~CAcl() throw();

	CAcl(const CAcl &rhs) throw(...);
	CAcl &operator=(const CAcl &rhs) throw(...);

	typedef CAtlArray<ACCESS_MASK> CAccessMaskArray;
	typedef CAtlArray<BYTE> CAceTypeArray;
	typedef CAtlArray<BYTE> CAceFlagArray;

	void GetAclEntries(
		CSid::CSidArray *pSids,
		CAccessMaskArray *pAccessMasks = NULL,
		CAceTypeArray *pAceTypes = NULL,
		CAceFlagArray *pAceFlags = NULL) const throw(...);
	void GetAclEntry(
		UINT nIndex,
		CSid* pSid,
		ACCESS_MASK* pMask = NULL,
		BYTE* pType = NULL,
		BYTE* pFlags = NULL,
		GUID* pObjectType = NULL,
		GUID* pInheritedObjectType = NULL) const throw(...);

	bool RemoveAces(const CSid &rSid) throw(...);

	virtual UINT GetAceCount() const throw() = 0;
	virtual void RemoveAllAces() throw() = 0;
	virtual void RemoveAce(UINT nIndex) = 0;

	const ACL *GetPACL() const throw(...);
	operator const ACL *() const throw(...);
	UINT GetLength() const throw(...);

	void SetNull() throw();
	void SetEmpty() throw();
	bool IsNull() const throw();
	bool IsEmpty() const throw();

private:
	mutable ACL *m_pAcl;
	bool m_bNull;

protected:
	void Dirty() throw();

	class CAce
	{
	public:
		CAce(const CSid &rSid, ACCESS_MASK accessmask, BYTE aceflags) throw(...);
		virtual ~CAce() throw();

		CAce(const CAce &rhs) throw(...);
		CAce &operator=(const CAce &rhs) throw(...);

		virtual void *GetACE() const throw(...) = 0;
		virtual UINT GetLength() const throw() = 0;
		virtual BYTE AceType() const throw() = 0;
		virtual bool IsObjectAce() const throw();
		virtual GUID ObjectType() const throw();
		virtual GUID InheritedObjectType() const throw();

		ACCESS_MASK AccessMask() const throw();
		BYTE AceFlags() const throw();
		const CSid &Sid() const throw();

		void AddAccess(ACCESS_MASK accessmask) throw();

	protected:
		CSid m_sid;
		ACCESS_MASK m_dwAccessMask;
		BYTE m_aceflags;
		mutable void *m_pAce;
	};

	virtual const CAce *GetAce(UINT nIndex) const = 0;
	virtual void PrepareAcesForACL() const throw();

	DWORD m_dwAclRevision;
};

// ************************************************
// CDacl

class CDacl : public CAcl
{
public:
	CDacl() throw();
	virtual ~CDacl() throw();

	CDacl(const CDacl &rhs) throw(...);
	CDacl &operator=(const CDacl &rhs) throw(...);

	CDacl(const ACL &rhs) throw(...);
	CDacl &operator=(const ACL &rhs) throw(...);

	bool AddAllowedAce(const CSid &rSid, ACCESS_MASK accessmask, BYTE aceflags = 0) throw(...);
	bool AddDeniedAce(const CSid &rSid, ACCESS_MASK accessmask, BYTE aceflags = 0) throw(...);
#if(_WIN32_WINNT >= 0x0500)
	bool AddAllowedAce(
		const CSid &rSid,
		ACCESS_MASK accessmask,
		BYTE aceflags,
		const GUID *pObjectType,
		const GUID *pInheritedObjectType) throw(...);
	bool AddDeniedAce(
		const CSid &rSid,
		ACCESS_MASK accessmask,
		BYTE aceflags,
		const GUID *pObjectType,
		const GUID *pInheritedObjectType) throw(...);
#endif
	void RemoveAllAces() throw();
	void RemoveAce(UINT nIndex);

	UINT GetAceCount() const throw();

private:
	void Copy(const CDacl &rhs) throw(...);
	void Copy(const ACL &rhs) throw(...);

	class CAccessAce : public CAcl::CAce
	{
	public:
		CAccessAce(const CSid &rSid, ACCESS_MASK accessmask, BYTE aceflags, bool bAllowAccess) throw(...);
		virtual ~CAccessAce() throw();

		void *GetACE() const throw(...);
		UINT GetLength() const throw();
		BYTE AceType() const throw();

		bool Allow() const throw();
		bool Inherited() const throw();

		static int Order(const CAccessAce &lhs, const CAccessAce &rhs) throw();

	protected:
		bool m_bAllow;
	};

#if(_WIN32_WINNT >= 0x0500)
	class CAccessObjectAce : public CAccessAce
	{
	public:
		CAccessObjectAce(
			const CSid &rSid,
			ACCESS_MASK accessmask,
			BYTE aceflags,
			bool bAllowAccess,
			const GUID *pObjectType,
			const GUID *pInheritedObjectType) throw(...);
		virtual ~CAccessObjectAce() throw();

		CAccessObjectAce(const CAccessObjectAce &rhs) throw(...);
		CAccessObjectAce &operator=(const CAccessObjectAce &rhs) throw(...);

		void *GetACE() const throw(...);
		UINT GetLength() const throw();
		BYTE AceType() const throw();
		bool IsObjectAce() const throw();
		virtual GUID ObjectType() const throw();
		virtual GUID InheritedObjectType() const throw();

	protected:
		GUID *m_pObjectType;
		GUID *m_pInheritedObjectType;
	};

#endif
	const CAcl::CAce *GetAce(UINT nIndex) const;

	void PrepareAcesForACL() const throw();

	mutable CAutoPtrArray<CAccessAce> m_acl;
};

//******************************************
// CSacl

class CSacl : public CAcl
{
public:
	CSacl() throw();
	virtual ~CSacl() throw();

	CSacl(const CSacl &rhs) throw(...);
	CSacl &operator=(const CSacl &rhs) throw(...);

	CSacl(const ACL &rhs) throw(...);
	CSacl &operator=(const ACL &rhs) throw(...);

	bool AddAuditAce(
		const CSid &rSid,
		ACCESS_MASK accessmask,
		bool bSuccess,
		bool bFailure,
		BYTE aceflags = 0) throw(...);
#if(_WIN32_WINNT >= 0x0500)
	bool AddAuditAce(
		const CSid &rSid,
		ACCESS_MASK accessmask,
		bool bSuccess,
		bool bFailure,
		BYTE aceflags,
		const GUID *pObjectType,
		const GUID *pInheritedObjectType) throw(...);
#endif
	void RemoveAllAces() throw();
	void RemoveAce(UINT nIndex);

	UINT GetAceCount() const throw();

private:
	void Copy(const CSacl &rhs) throw(...);
	void Copy(const ACL &rhs) throw(...);

	class CAuditAce : public CAcl::CAce
	{
	public:
		CAuditAce(
			const CSid &rSid,
			ACCESS_MASK accessmask,
			BYTE aceflags,
			bool bAuditSuccess,
			bool bAuditFailure) throw(...);
		virtual ~CAuditAce() throw();

		void *GetACE() const throw(...);
		UINT GetLength() const throw();
		BYTE AceType() const throw();
	protected:
		bool m_bSuccess;
		bool m_bFailure;
	};

#if(_WIN32_WINNT >= 0x0500)
	class CAuditObjectAce : public CAuditAce
	{
	public:
		CAuditObjectAce(
			const CSid &rSid,
			ACCESS_MASK accessmask,
			BYTE aceflags,
			bool bAuditSuccess,
			bool bAuditFailure,
			const GUID *pObjectType,
			const GUID *pInheritedObjectType) throw(...);
		virtual ~CAuditObjectAce() throw();

		CAuditObjectAce(const CAuditObjectAce &rhs) throw(...);
		CAuditObjectAce &operator=(const CAuditObjectAce &rhs) throw(...);

		void *GetACE() const throw(...);
		UINT GetLength() const throw();
		BYTE AceType() const throw();
		bool IsObjectAce() const throw();
		virtual GUID ObjectType() const throw();
		virtual GUID InheritedObjectType() const throw();

	protected:
		GUID *m_pObjectType;
		GUID *m_pInheritedObjectType;
	};
#endif
	const CAce *GetAce(UINT nIndex) const;

	CAutoPtrArray<CAuditAce> m_acl;
};

//******************************************
// CSecurityDesc

class CSecurityDesc
{
public:
	CSecurityDesc() throw();
	virtual ~CSecurityDesc() throw();

	CSecurityDesc(const CSecurityDesc &rhs) throw(...);
	CSecurityDesc &operator=(const CSecurityDesc &rhs) throw(...);

	CSecurityDesc(const SECURITY_DESCRIPTOR &rhs) throw(...);
	CSecurityDesc &operator=(const SECURITY_DESCRIPTOR &rhs) throw(...);

#if(_WIN32_WINNT >= 0x0500)
	bool FromString(LPCTSTR pstr) throw(...);
	bool ToString(
		CString *pstr,
		SECURITY_INFORMATION si =
			OWNER_SECURITY_INFORMATION |
			GROUP_SECURITY_INFORMATION |
			DACL_SECURITY_INFORMATION |
			SACL_SECURITY_INFORMATION) const throw(...);
#endif

	void SetOwner(const CSid &sid, bool bDefaulted = false) throw(...);
	void SetGroup(const CSid &sid, bool bDefaulted = false) throw(...);
	void SetDacl(const CDacl &Dacl, bool bDefaulted = false) throw(...);
	void SetDacl(bool bPresent, bool bDefaulted = false) throw(...);
	void SetSacl(const CSacl &Sacl, bool bDefaulted = false) throw(...);
	bool GetOwner(CSid *pSid, bool *pbDefaulted = NULL) const throw(...);
	bool GetGroup(CSid *pSid, bool *pbDefaulted = NULL) const throw(...);
	bool GetDacl(CDacl *pDacl, bool *pbPresent = NULL, bool *pbDefaulted = NULL) const throw(...);
	bool GetSacl(CSacl *pSacl, bool *pbPresent = NULL, bool *pbDefaulted = NULL) const throw(...);

	bool IsDaclDefaulted() const throw();
	bool IsDaclPresent() const throw();
	bool IsGroupDefaulted() const throw();
	bool IsOwnerDefaulted() const throw();
	bool IsSaclDefaulted() const throw();
	bool IsSaclPresent() const throw();
	bool IsSelfRelative() const throw();

	// Only meaningful on Win2k or better
	bool IsDaclAutoInherited() const throw();
	bool IsDaclProtected() const throw();
	bool IsSaclAutoInherited() const throw();
	bool IsSaclProtected() const throw();

	const SECURITY_DESCRIPTOR *GetPSECURITY_DESCRIPTOR() const throw();
	operator const SECURITY_DESCRIPTOR *() const throw();

	void GetSECURITY_DESCRIPTOR(SECURITY_DESCRIPTOR *pSD, LPDWORD lpdwBufferLength) throw(...);

	UINT GetLength() throw();

	bool GetControl(SECURITY_DESCRIPTOR_CONTROL *psdc) const throw();
#if(_WIN32_WINNT >= 0x0500)
	bool SetControl(
		SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,
		SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet) throw();
#endif

	void MakeSelfRelative() throw(...);
	void MakeAbsolute() throw(...);

protected:
	virtual void Clear() throw();
	void AllocateAndInitializeSecurityDescriptor() throw(...);
	void Init(const SECURITY_DESCRIPTOR &rhs) throw(...);

	SECURITY_DESCRIPTOR *m_pSecurityDescriptor;
};

// **************************************************************
// CSecurityAttributes

class CSecurityAttributes : public SECURITY_ATTRIBUTES
{
public:
	CSecurityAttributes() throw();
	explicit CSecurityAttributes(const CSecurityDesc &rSecurityDescriptor, bool bInheritHandle = false) throw(...);

	void Set(const CSecurityDesc &rSecurityDescriptor, bool bInheritHandle = false) throw(...);

protected:
	CSecurityDesc m_SecurityDescriptor;
};

template<>
class CElementTraits< LUID > :
	public CElementTraitsBase< LUID >
{
public:
	typedef const LUID& INARGTYPE;
	typedef LUID& OUTARGTYPE;

	static ULONG Hash( INARGTYPE luid ) throw()
	{
		return luid.HighPart ^ luid.LowPart;
	}

	static BOOL CompareElements( INARGTYPE element1, INARGTYPE element2 ) throw()
	{
		return element1.HighPart == element2.HighPart && element1.LowPart == element2.LowPart;
	}

	static int CompareElementsOrdered( INARGTYPE element1, INARGTYPE element2 ) throw()
	{
		_LARGE_INTEGER li1, li2;
		li1.LowPart = element1.LowPart;
		li1.HighPart = element1.HighPart;
		li2.LowPart = element2.LowPart;
		li2.HighPart = element2.HighPart;

		if( li1.QuadPart > li2.QuadPart )
			return( 1 );
		else if( li1.QuadPart < li2.QuadPart )
			return( -1 );

		return( 0 );
	}
};

typedef CAtlArray<LUID> CLUIDArray;

//******************************************************
// CTokenPrivileges

class CTokenPrivileges
{
public:
	CTokenPrivileges() throw();
	virtual ~CTokenPrivileges() throw();

	CTokenPrivileges(const CTokenPrivileges &rhs) throw(...);
	CTokenPrivileges &operator=(const CTokenPrivileges &rhs) throw(...);

	CTokenPrivileges(const TOKEN_PRIVILEGES &rPrivileges) throw(...);
	CTokenPrivileges &operator=(const TOKEN_PRIVILEGES &rPrivileges) throw(...);

	void Add(const TOKEN_PRIVILEGES &rPrivileges) throw(...);
	bool Add(LPCTSTR pszPrivilege, bool bEnable) throw(...);

	typedef CAtlArray<CString> CNames;
	typedef CAtlArray<DWORD> CAttributes;

	bool LookupPrivilege(LPCTSTR pszPrivilege, DWORD *pdwAttributes = NULL) const throw(...);
	void GetNamesAndAttributes(CNames *pNames, CAttributes *pAttributes = NULL) const throw(...);
	void GetDisplayNames(CNames *pDisplayNames) const throw(...);
	void GetLuidsAndAttributes(CLUIDArray *pPrivileges, CAttributes *pAttributes = NULL) const throw(...);

	bool Delete(LPCTSTR pszPrivilege) throw();
	void DeleteAll() throw();

	UINT GetCount() const throw();
	UINT GetLength() const throw();

	const TOKEN_PRIVILEGES *GetPTOKEN_PRIVILEGES() const throw(...);
	operator const TOKEN_PRIVILEGES *() const throw(...);

private:
	typedef CAtlMap<LUID, DWORD> Map;
	Map m_mapTokenPrivileges;
	mutable TOKEN_PRIVILEGES *m_pTokenPrivileges;
	bool m_bDirty;

	void AddPrivileges(const TOKEN_PRIVILEGES &rPrivileges) throw(...);
};

//******************************************************
// CTokenGroups

class CTokenGroups
{
public:
	CTokenGroups() throw();
	virtual ~CTokenGroups() throw();

	CTokenGroups(const CTokenGroups &rhs) throw(...);
	CTokenGroups &operator=(const CTokenGroups &rhs) throw(...);

	CTokenGroups(const TOKEN_GROUPS &rhs) throw(...);
	CTokenGroups &operator=(const TOKEN_GROUPS &rhs) throw(...);

	void Add(const TOKEN_GROUPS &rTokenGroups) throw(...);
	void Add(const CSid &rSid, DWORD dwAttributes) throw(...);

	bool LookupSid(const CSid &rSid, DWORD *pdwAttributes = NULL) const throw();
	void GetSidsAndAttributes(
		CSid::CSidArray *pSids,
		CAtlArray<DWORD> *pAttributes = NULL) const throw(...);

	bool Delete(const CSid &rSid) throw();
	void DeleteAll() throw();

	UINT GetCount() const throw();
	UINT GetLength() const throw();

	const TOKEN_GROUPS *GetPTOKEN_GROUPS() const throw(...);
	operator const TOKEN_GROUPS *() const throw(...);

private:
	class CTGElementTraits : 
		public CElementTraitsBase< CSid >
	{
	public:
		static UINT Hash(const CSid &sid) throw()
		{
			return sid.GetSubAuthority(sid.GetSubAuthorityCount() - 1);
		}

		static bool CompareElements( INARGTYPE element1, INARGTYPE element2 ) throw()
		{
			return( element1 == element2 );
		}
	};

	typedef CAtlMap<CSid, DWORD, CTGElementTraits> Map;
	Map m_mapTokenGroups;
	mutable TOKEN_GROUPS *m_pTokenGroups;
	mutable bool m_bDirty;

	void AddTokenGroups(const TOKEN_GROUPS &rTokenGroups) throw(...);
};

// *************************************
// CAccessToken

class CAccessToken
{
public:
	CAccessToken() throw();
	virtual ~CAccessToken() throw();

	void Attach(HANDLE hToken) throw();
	HANDLE Detach() throw();
	HANDLE GetHandle() const throw();
	HKEY HKeyCurrentUser() const throw();

	// Privileges
	bool EnablePrivilege(LPCTSTR pszPrivilege, CTokenPrivileges *pPreviousState = NULL,bool* pbErrNotAllAssigned=NULL) throw(...);
	bool EnablePrivileges(
		const CAtlArray<LPCTSTR> &rPrivileges,
		CTokenPrivileges *pPreviousState = NULL,bool* pbErrNotAllAssigned=NULL) throw(...);
	bool DisablePrivilege(LPCTSTR pszPrivilege, CTokenPrivileges *pPreviousState = NULL,bool* pbErrNotAllAssigned=NULL) throw(...);
	bool DisablePrivileges(
		const CAtlArray<LPCTSTR> &rPrivileges,
		CTokenPrivileges *pPreviousState = NULL,bool* pbErrNotAllAssigned=NULL) throw(...);
	bool EnableDisablePrivileges(
		const CTokenPrivileges &rPrivilenges,
		CTokenPrivileges *pPreviousState = NULL,bool* pbErrNotAllAssigned=NULL) throw(...);
	bool PrivilegeCheck(PPRIVILEGE_SET RequiredPrivileges, bool *pbResult) const throw();

	bool GetLogonSid(CSid *pSid) const throw(...);
	bool GetTokenId(LUID *pluid) const throw(...);
	bool GetLogonSessionId(LUID *pluid) const throw(...);

	bool CheckTokenMembership(const CSid &rSid, bool *pbIsMember) const throw(...);
#if(_WIN32_WINNT >= 0x0500)
	bool IsTokenRestricted() const throw();
#endif

	// Token Information
protected:
	void InfoTypeToRetType(CSid *pRet, const TOKEN_USER &rWork) const throw(...)
	{
		ATLENSURE(pRet);
		*pRet = *static_cast<SID *>(rWork.User.Sid);
	}

	void InfoTypeToRetType(CTokenGroups *pRet, const TOKEN_GROUPS &rWork) const throw(...)
	{
		ATLENSURE(pRet);
		*pRet = rWork;
	}

	void InfoTypeToRetType(CTokenPrivileges *pRet, const TOKEN_PRIVILEGES &rWork) const throw(...)
	{
		ATLENSURE(pRet);
		*pRet = rWork;
	}

	void InfoTypeToRetType(CSid *pRet, const TOKEN_OWNER &rWork) const throw(...)
	{
		ATLENSURE(pRet);
		*pRet = *static_cast<SID *>(rWork.Owner);
	}

	void InfoTypeToRetType(CSid *pRet, const TOKEN_PRIMARY_GROUP &rWork) const throw(...)
	{
		ATLENSURE(pRet);
		*pRet = *static_cast<SID *>(rWork.PrimaryGroup);
	}

	void InfoTypeToRetType(CDacl *pRet, const TOKEN_DEFAULT_DACL &rWork) const throw(...)
	{
		ATLENSURE(pRet);
		*pRet = *rWork.DefaultDacl;
	}

	template<typename RET_T, typename INFO_T>
	bool GetInfoConvert(RET_T *pRet, TOKEN_INFORMATION_CLASS TokenClass, INFO_T *pWork = NULL) const throw(...)
	{
		ATLASSERT(pRet);
		if(!pRet)
			return false;

		DWORD dwLen;
		::GetTokenInformation(m_hToken, TokenClass, NULL, 0, &dwLen);
		if(::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			return false;

		USES_ATL_SAFE_ALLOCA;
		pWork = static_cast<INFO_T *>(_ATL_SAFE_ALLOCA(dwLen, _ATL_SAFE_ALLOCA_DEF_THRESHOLD));
		if (pWork == NULL)
			return false;
		if(!::GetTokenInformation(m_hToken, TokenClass, pWork, dwLen, &dwLen))
			return false;

		InfoTypeToRetType(pRet, *pWork);
		return true;
	}

	template<typename RET_T>
	bool GetInfo(RET_T *pRet, TOKEN_INFORMATION_CLASS TokenClass) const throw(...)
	{
		ATLASSERT(pRet);
		if(!pRet)
			return false;

		DWORD dwLen;
		if(!::GetTokenInformation(m_hToken, TokenClass, pRet, sizeof(RET_T), &dwLen))
			return false;
		return true;
	}

public:
	bool GetDefaultDacl(CDacl *pDacl) const throw(...);
	bool GetGroups(CTokenGroups *pGroups) const throw(...);
	bool GetImpersonationLevel(SECURITY_IMPERSONATION_LEVEL *pImpersonationLevel) const throw(...);
	bool GetOwner(CSid *pSid) const throw(...);
	bool GetPrimaryGroup(CSid *pSid) const throw(...);
	bool GetPrivileges(CTokenPrivileges *pPrivileges) const throw(...);
	bool GetTerminalServicesSessionId(DWORD *pdwSessionId) const throw(...);
	bool GetSource(TOKEN_SOURCE *pSource) const throw(...);
	bool GetStatistics(TOKEN_STATISTICS *pStatistics) const throw(...);
	bool GetType(TOKEN_TYPE *pType) const throw(...);
	bool GetUser(CSid *pSid) const throw(...);

	bool SetOwner(const CSid &rSid) throw(...);
	bool SetPrimaryGroup(const CSid &rSid) throw(...);
	bool SetDefaultDacl(const CDacl &rDacl) throw(...);

	bool CreateImpersonationToken(
		CAccessToken *pImp,
		SECURITY_IMPERSONATION_LEVEL sil = SecurityImpersonation) const throw(...);
	bool CreatePrimaryToken(
		CAccessToken *pPri,
		DWORD dwDesiredAccess = MAXIMUM_ALLOWED,
		const CSecurityAttributes *pTokenAttributes = NULL) const throw(...);

#if(_WIN32_WINNT >= 0x0500)
	bool CreateRestrictedToken(
		CAccessToken *pRestrictedToken,
		const CTokenGroups &SidsToDisable,
		const CTokenGroups &SidsToRestrict, 
		const CTokenPrivileges &PrivilegesToDelete = CTokenPrivileges()) const throw(...);
#endif

	// Token API type functions
	bool GetProcessToken(DWORD dwDesiredAccess, HANDLE hProcess = NULL) throw();
	bool GetThreadToken(DWORD dwDesiredAccess, HANDLE hThread = NULL, bool bOpenAsSelf = true) throw();
	bool GetEffectiveToken(DWORD dwDesiredAccess) throw();

	bool OpenThreadToken(
		DWORD dwDesiredAccess,
		bool bImpersonate = false,
		bool bOpenAsSelf = true,
		SECURITY_IMPERSONATION_LEVEL sil = SecurityImpersonation) throw(...);

#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) 
	bool OpenCOMClientToken(
		DWORD dwDesiredAccess,
		bool bImpersonate = false,
		bool bOpenAsSelf = true) throw(...);
#endif //(_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) 

	bool OpenNamedPipeClientToken(
		HANDLE hPipe,
		DWORD dwDesiredAccess,
		bool bImpersonate = false,
		bool bOpenAsSelf = true) throw(...);
	bool OpenRPCClientToken(
		RPC_BINDING_HANDLE BindingHandle,
		DWORD dwDesiredAccess,
		bool bImpersonate = false,
		bool bOpenAsSelf = true) throw(...);

	bool ImpersonateLoggedOnUser() const throw(...);
	bool Impersonate(HANDLE hThread = NULL) const throw(...);
	bool Revert(HANDLE hThread = NULL) const throw();

	bool LoadUserProfile() throw(...);
	HANDLE GetProfile() const throw();

	// Must hold Tcb privilege
	bool LogonUser(
		LPCTSTR pszUserName,
		LPCTSTR pszDomain,
		LPCTSTR pszPassword,
		DWORD dwLogonType = LOGON32_LOGON_INTERACTIVE,
		DWORD dwLogonProvider = LOGON32_PROVIDER_DEFAULT) throw();

	// Must hold AssignPrimaryToken (unless restricted token) and
	// IncreaseQuota privileges
	bool CreateProcessAsUser(
		_In_opt_z_ LPCTSTR pApplicationName,
		_In_opt_z_ LPTSTR pCommandLine,
		_In_ LPPROCESS_INFORMATION pProcessInformation,
		_In_ LPSTARTUPINFO pStartupInfo,
		_In_ DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS,
		_In_ bool bLoadProfile = false,
		_In_opt_ const CSecurityAttributes *pProcessAttributes = NULL,
		_In_opt_ const CSecurityAttributes *pThreadAttributes = NULL,
		_In_ bool bInherit = false,
		_In_opt_z_ LPCTSTR pCurrentDirectory = NULL) throw();

protected:
	bool EnableDisablePrivileges(
		const CAtlArray<LPCTSTR> &rPrivileges,
		bool bEnable,
		CTokenPrivileges *pPreviousState,bool* pbErrNotAllAssigned=NULL) throw(...);
	bool CheckImpersonation() const throw();

	bool RevertToLevel(SECURITY_IMPERSONATION_LEVEL *pSil) const throw();

	virtual void Clear() throw();

	HANDLE m_hToken, m_hProfile;

private:
	CAccessToken(const CAccessToken &rhs) throw(...);
	CAccessToken &operator=(const CAccessToken &rhs) throw(...);

	class CRevert
	{
	public:
		virtual bool Revert() throw() = 0;
	};

	class CRevertToSelf : public CRevert
	{
	public:
		bool Revert() throw()
		{
			return 0 != ::RevertToSelf();
		}
	};

#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) 
	class CCoRevertToSelf : public CRevert
	{
	public:
		bool Revert() throw()
		{
			return SUCCEEDED(::CoRevertToSelf());
		}
	};
#endif //(_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) 

	class CRpcRevertToSelfEx : public CRevert
	{
	public:
		CRpcRevertToSelfEx(RPC_BINDING_HANDLE BindingHandle) throw() :
			m_hBinding(BindingHandle)
		{
		}
		bool Revert() throw()
		{
			return RPC_S_OK == ::RpcRevertToSelfEx(m_hBinding);
		}

	private:
		RPC_BINDING_HANDLE m_hBinding;
	};
	mutable CRevert *m_pRevert;
};

//*******************************************
// CAutoRevertImpersonation

class CAutoRevertImpersonation
{
public:
	CAutoRevertImpersonation(const CAccessToken* pAT) throw();
	~CAutoRevertImpersonation() throw();

	void Attach(const CAccessToken* pAT) throw();
	const CAccessToken* Detach() throw();

	const CAccessToken* GetAccessToken() throw();

private:
	const CAccessToken* m_pAT;

	CAutoRevertImpersonation(const CAutoRevertImpersonation &rhs) throw(...);
	CAutoRevertImpersonation &operator=(const CAutoRevertImpersonation &rhs) throw(...);
};

//*******************************************
// CPrivateObjectSecurityDesc

class CPrivateObjectSecurityDesc : public CSecurityDesc
{
public:
	CPrivateObjectSecurityDesc() throw();
	~CPrivateObjectSecurityDesc() throw();

	bool Create(
		const CSecurityDesc *pParent,
		const CSecurityDesc *pCreator,
		bool bIsDirectoryObject,
		const CAccessToken &Token,
		PGENERIC_MAPPING GenericMapping) throw();

#if(_WIN32_WINNT >= 0x0500)
	bool Create(
		const CSecurityDesc *pParent,
		const CSecurityDesc *pCreator,
		GUID *ObjectType,
		bool bIsContainerObject,
		ULONG AutoInheritFlags,
		const CAccessToken &Token,
		PGENERIC_MAPPING GenericMapping) throw();
#endif

	bool Get(SECURITY_INFORMATION si, CSecurityDesc *pResult) const throw();
	bool Set(
		SECURITY_INFORMATION si,
		const CSecurityDesc &Modification,
		PGENERIC_MAPPING GenericMapping,
		const CAccessToken &Token) throw();

#if(_WIN32_WINNT >= 0x0500)
	bool Set(
		SECURITY_INFORMATION si,
		const CSecurityDesc &Modification,
		ULONG AutoInheritFlags,
		PGENERIC_MAPPING GenericMapping,
		const CAccessToken &Token) throw();

	bool ConvertToAutoInherit(
		const CSecurityDesc *pParent,
		GUID *ObjectType,
		bool bIsDirectoryObject,
		PGENERIC_MAPPING GenericMapping) throw();
#endif

protected:
	void Clear() throw();

private:
	bool m_bPrivate;

	CPrivateObjectSecurityDesc(const CPrivateObjectSecurityDesc &rhs) throw(...);
	CPrivateObjectSecurityDesc &operator=(const CPrivateObjectSecurityDesc &rhs) throw(...);
};

//*******************************************
// Global Functions

inline bool AtlGetSecurityDescriptor(
	LPCTSTR pszObjectName,
	SE_OBJECT_TYPE ObjectType,
	CSecurityDesc *pSecurityDescriptor,
	SECURITY_INFORMATION requestedInfo =
		OWNER_SECURITY_INFORMATION |
		GROUP_SECURITY_INFORMATION |
		DACL_SECURITY_INFORMATION |
		SACL_SECURITY_INFORMATION,
	bool bRequestNeededPrivileges = true) throw(...);

inline bool AtlGetSecurityDescriptor(
	HANDLE hObject,
	SE_OBJECT_TYPE ObjectType,
	CSecurityDesc *pSecurityDescriptor,
	SECURITY_INFORMATION requestedInfo =
		OWNER_SECURITY_INFORMATION |
		GROUP_SECURITY_INFORMATION |
		DACL_SECURITY_INFORMATION |
		SACL_SECURITY_INFORMATION,
	bool bRequestNeededPrivileges = true) throw(...);

inline bool AtlGetOwnerSid(
	HANDLE hObject,
	SE_OBJECT_TYPE ObjectType,
	CSid *pSid) throw(...);

inline bool AtlSetOwnerSid(
	HANDLE hObject,
	SE_OBJECT_TYPE ObjectType,
	const CSid &rSid) throw(...);

inline bool AtlGetOwnerSid(
	LPCTSTR pszObjectName,
	SE_OBJECT_TYPE ObjectType,
	CSid *pSid) throw(...);

inline bool AtlSetOwnerSid(
	LPCTSTR pszObjectName,
	SE_OBJECT_TYPE ObjectType,
	const CSid &rSid) throw(...);

inline bool AtlGetGroupSid(
	HANDLE hObject,
	SE_OBJECT_TYPE ObjectType,
	CSid *pSid) throw(...);

inline bool AtlSetGroupSid(
	HANDLE hObject,
	SE_OBJECT_TYPE ObjectType,
	const CSid &rSid) throw(...);

inline bool AtlGetGroupSid(
	LPCTSTR pszObjectName,
	SE_OBJECT_TYPE ObjectType,
	CSid *pSid) throw(...);

inline bool AtlSetGroupSid(
	LPCTSTR pszObjectName,
	SE_OBJECT_TYPE ObjectType,
	const CSid &rSid) throw(...);

inline bool AtlGetDacl(
	HANDLE hObject,
	SE_OBJECT_TYPE ObjectType,
	CDacl *pDacl) throw(...);

inline bool AtlSetDacl(
	HANDLE hObject,
	SE_OBJECT_TYPE ObjectType,
	const CDacl &rDacl,
	DWORD dwInheritanceFlowControl = 0) throw(...);

inline bool AtlGetDacl(
	LPCTSTR pszObjectName,
	SE_OBJECT_TYPE ObjectType,
	CDacl *pDacl) throw(...);

inline bool AtlSetDacl(
	LPCTSTR pszObjectName,
	SE_OBJECT_TYPE ObjectType,
	const CDacl &rDacl,
	DWORD dwInheritanceFlowControl = 0) throw(...);

inline bool AtlGetSacl(
	HANDLE hObject,
	SE_OBJECT_TYPE ObjectType,
	CSacl *pSacl,
	bool bRequestNeededPrivileges = true) throw(...);

inline bool AtlSetSacl(
	HANDLE hObject,
	SE_OBJECT_TYPE ObjectType,
	const CSacl &rSacl,
	DWORD dwInheritanceFlowControl = 0,
	bool bRequestNeededPrivileges = true) throw(...);

inline bool AtlGetSacl(
	LPCTSTR pszObjectName,
	SE_OBJECT_TYPE ObjectType,
	CSacl *pSacl,
	bool bRequestNeededPrivileges = true) throw(...);

inline bool AtlSetSacl(
	LPCTSTR pszObjectName,
	SE_OBJECT_TYPE ObjectType,
	const CSacl &rSacl,
	DWORD dwInheritanceFlowControl = 0,
	bool bRequestNeededPrivileges = true) throw(...);

} // namespace ATL
 

#pragma warning(pop)

#include <atlsecurity.inl>
#pragma pack(pop)
#endif // __ATLSECURITY_H__
