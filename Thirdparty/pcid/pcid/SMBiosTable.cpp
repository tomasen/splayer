#include "stdafx.h"
#include "SMBiosTable.h"
//#include "CommonDefine.h"
#include <Windows.h>
//#include <ntddk.h>
#include <COMDEF.H>    
#include <WBEMIDL.H>

#pragma comment( lib, "wbemuuid.lib" )

CSMBiosTable::CSMBiosTable(void)
{
	ZeroMemory( &m_smbiosbuffer, sizeof(SSMBiosBuffer) );
}

CSMBiosTable::~CSMBiosTable(void)
{
}
void CSMBiosTable::AddStructureType( unsigned char cType )
{
	if( IsStructureType( cType ) )
		return;

	PSMBiosStructureBuffer structBuf = new SSMBiosStructureBuffer;
	ZeroMemory( structBuf, sizeof(SSMBiosStructureBuffer) );
	structBuf->m_cType = cType;
	m_vStructureBuffers.push_back( structBuf );
}


typedef UINT (WINAPI *pfnGetSystemFirmwareTable)(DWORD,DWORD,PVOID,DWORD);
bool CSMBiosTable::InitSMBiosTable()
{
	OSVERSIONINFO   ver = {0};
	ver.dwOSVersionInfoSize = sizeof(ver);
	GetVersionEx(&ver);
	if( ver.dwMajorVersion <= 5 )
	{
		unsigned char * p = NULL;
		if( FetchSMBiosDataInXP32( &p ) )
		{
			//BASE_LOG_OUT(( P2SP_LOG, "buf len = %d\n", m_smbiosbuffer.nLength ));
			ParseSMBiosStructure( p, m_smbiosbuffer.nLength );
			delete []p;
			return true;
		}
		else
		{
			if( FetchSMBiosDataByCom( &p ) )
			{
				ParseSMBiosStructure( p, m_smbiosbuffer.nLength );
				delete []p;
				return true;
			}
			else
				return false;
		}
	}
	pfnGetSystemFirmwareTable pfn = (pfnGetSystemFirmwareTable)GetProcAddress( GetModuleHandleA("kernel32.dll"), "GetSystemFirmwareTable" );
	if( pfn == NULL )
	{
		unsigned char * p = NULL;
		if( FetchSMBiosDataByCom( &p ) )
		{
			ParseSMBiosStructure( p, m_smbiosbuffer.nLength );
			if( p != NULL )
				delete []p;
			return true;
		}
		else
		{
			if( p != NULL )
				delete []p;
			return false;
		}
	}
	else
	{
		DWORD signature = 'R';
		signature = (signature<<8)+'S';
		signature = (signature<<8)+'M';
		signature = (signature<<8)+'B';
		//	memcpy_s( &signature, sizeof(DWORD), signature, sizeof(signature) );
		DWORD size = pfn( signature, 0, NULL, 0 );
		if( size == 0 )
			return false;
		unsigned char * p = new unsigned char[size];
//		unsigned char p[2100];
		size = pfn( signature, 0, p, size );
		if( size == 0 )
		{
			delete []p;
			return false;
		}
		m_smbiosbuffer.nUsed20CallingMethod = p[0];
		m_smbiosbuffer.nSMBIOSMajorVersion  = p[1];
		m_smbiosbuffer.nSMBIOSMinorVersion  = p[2];
		m_smbiosbuffer.nDmiRevision         = p[3];
		m_smbiosbuffer.nLength              = *((DWORD*)&p[4]);

		ParseSMBiosStructure( &p[8], size-8 );

		delete []p;
		return true;
	}
}
bool CSMBiosTable::CloseSMBiosTable()
{
	ClearSMBiosStructureBuffer();
	return false;
}

PSMBiosStructureBuffer CSMBiosTable::GetSMBiosStructureBuffer( unsigned char cType )
{
	size_t size = m_vStructureBuffers.size();
	for( size_t i = 0; i < size; i++ )
	{
		if( m_vStructureBuffers[i] != NULL )
			if( m_vStructureBuffers[i]->m_cType == cType )
				return m_vStructureBuffers[i];
	}
	return NULL;
}
void * CSMBiosTable::MakeSMBiosStructureObj( PSMBiosStructureBuffer struc )
{
	if( struc->m_pStructureBuffer == NULL || struc->m_nStructureBufferLen == 0 )
		return NULL;

	switch( struc->m_cType )
	{
	case 1:
		{
			PSystemInformation sysInfor = new SSystemInformation;
			ZeroMemory( sysInfor, sizeof(SSystemInformation) );
			sysInfor->ParseInfo( struc->m_pStructureBuffer, struc->m_nStructureBufferLen );
			return sysInfor;
		}
	case 3:
		{
			PSystemEnclosureStructure ecInfor = new SSystemEnclosureStructure;
			ZeroMemory( ecInfor, sizeof(SSystemEnclosureStructure) );
			ecInfor->ParseInfo( struc->m_pStructureBuffer, struc->m_nStructureBufferLen );
			return ecInfor;
		}
	case 6:
		{
			PMemoryModuleInformation mdInfor = new SMemoryModuleInformation;
			ZeroMemory( mdInfor, sizeof(SMemoryModuleInformation) );
			mdInfor->ParseInfo( struc->m_pStructureBuffer, struc->m_nStructureBufferLen );
			return mdInfor;
		}
	case 17:
		{
			PMemoryDeviceInformation memInfor = new SMemoryDeviceInformation;
			ZeroMemory( memInfor, sizeof(SMemoryDeviceInformation) );

			memInfor->ParseInfo( struc->m_pStructureBuffer, struc->m_nStructureBufferLen );
			return memInfor;
		}
	default:
		break;
	}
	return NULL;
}
void   CSMBiosTable::ReleaseSMBiosStructureObj( unsigned char cType, void * pObj )
{
	switch( cType )
	{
	case 6:
		{
			PMemoryModuleInformation mdInfor = (PMemoryModuleInformation)pObj;
			delete mdInfor;
			break;
		}
	case 17:
		{
			PMemoryDeviceInformation memInfor = (PMemoryDeviceInformation)pObj;
			delete memInfor;
			break;
		}
	default:
		break;
	}
}
void CSMBiosTable::AddSMBiosStructureBuffer( PSMBiosStructureBuffer struc )
{

}
bool CSMBiosTable::IsStructureType( unsigned char cType )
{
	size_t size = m_vStructureBuffers.size();
	for( size_t i = 0; i < size; i++ )
	{
		if( m_vStructureBuffers[i] != NULL )
			if( cType == m_vStructureBuffers[i]->m_cType )
				return true;
	}
	return false;
}
PSMBiosStructureBuffer CSMBiosTable::GetStructureBuffer( unsigned char cType )
{
	size_t size = m_vStructureBuffers.size();
	for( size_t i = 0; i < size; i++ )
	{
		if( m_vStructureBuffers[i] != NULL )
			if( cType == m_vStructureBuffers[i]->m_cType )
				return m_vStructureBuffers[i];
	}
	return NULL;
}
void CSMBiosTable::ParseSMBiosStructure( unsigned char * pInBuf, int nInBufLen )
{
	int i = 0;
	while( i+1 < nInBufLen )
	{
		unsigned char cType = pInBuf[i];
		unsigned char cLength = pInBuf[i+1];
		if( i+cLength >= nInBufLen )
			break;

//		BASE_LOG_OUT(( P2SP_LOG, " i+cLength = %d  nInBufLen-i-cLength=%d beg\n", i+cLength, nInBufLen-i-cLength ));
		int end = FindStringRegionEnd( &pInBuf[i+cLength], nInBufLen-i-cLength );
//		BASE_LOG_OUT(( P2SP_LOG, " i+cLength = %d  nInBufLen-i-cLength=%d\n", i+cLength, nInBufLen-i-cLength ));
		if( end == -1 )//已经到结尾
		{
//			BASE_LOG_OUT(( P2SP_LOG, "break\n" ));
			break;
		}

		PSMBiosStructureBuffer structBuffer = GetStructureBuffer( cType );
		if( structBuffer != NULL )
		{
			if( structBuffer->m_pStructureBuffer == NULL )//第一个 buffer 对象已经生成 通过其buffer成员 判断是否被使用
			{
				structBuffer->m_pStructureBuffer = new unsigned char[cLength+end];
				memcpy_s( structBuffer->m_pStructureBuffer, cLength+end, &pInBuf[i], cLength+end );
				structBuffer->m_nStructureBufferLen = cLength+end;
			}
			else//接下来的通过成员 m_next 来判断对象是否生成且使用
			{
				while( structBuffer->m_next != NULL )
	    			structBuffer = structBuffer->m_next;

				PSMBiosStructureBuffer tempStructBuffer = new SSMBiosStructureBuffer;
				tempStructBuffer->m_cType = cType;
				tempStructBuffer->m_pStructureBuffer = new unsigned char[cLength+end];
				memcpy_s( tempStructBuffer->m_pStructureBuffer, cLength+end, &pInBuf[i], cLength+end );
				tempStructBuffer->m_nStructureBufferLen = cLength+end;
				structBuffer->m_next = tempStructBuffer;
			}
		}
//		BASE_LOG_OUT(( P2SP_LOG, " i = %d\n", i ));
		i = i + cLength + end;
//		BASE_LOG_OUT(( P2SP_LOG, " i = %d end\n", i ));
	}
//	BASE_LOG_OUT(( P2SP_LOG, "end\n" ));
}
int  CSMBiosTable::FindStringRegionEnd( const unsigned char * pBegin, const unsigned int nLen )
{
	int nCurPos = 0;
	while( nCurPos+1 < nLen )
	{
		if( pBegin[nCurPos] == '\0' && pBegin[nCurPos+1] == '\0' )
		{
			return (nCurPos+2);
		}
		else
		{
			nCurPos++;
		}
	}
	return -1;
}

void CSMBiosTable::ClearSMBiosStructureBuffer()
{
	size_t size = m_vStructureBuffers.size();
	for( size_t i = 0; i < size; i++ )
	{
		PSMBiosStructureBuffer structBuffer = m_vStructureBuffers[i];
		if( structBuffer != NULL )
			delete structBuffer;
	}
	m_vStructureBuffers.clear();
}
bool CSMBiosTable::FetchSMBiosDataByCom( unsigned char ** p )
{
	BOOL bRet = FALSE;   
	HRESULT hres;   

	// Initialize COM.    
	hres =  CoInitializeEx( 0, COINIT_MULTITHREADED );    
	if( FAILED(hres) )   
	{   
		return FALSE;              // Program has failed.    
	}   

	// Obtain the initial locator to Windows Management    
	// on a particular host computer.    
	IWbemLocator *pLoc = 0;   
	hres = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pLoc );   
	if( FAILED(hres) )   
	{   
		CoUninitialize();   
		return FALSE;       // Program has failed.    
	}   

	IWbemServices *pSvc = 0;   

	// Connect to the root\cimv2 namespace with the    z
	// current user and obtain pointer pSvc    
	// to make IWbemServices calls.    
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\WMI"), // WMI namespace    
		NULL,                    // User name    
		NULL,                    // User password    
		0,                       // Locale    
		NULL,                    // Security flags                     
		0,                       // Authority           
		0,                       // Context object    
		&pSvc                    // IWbemServices proxy    
		);                                 

	if( FAILED(hres) )   
	{   
		pLoc->Release();        
		CoUninitialize();   
		return FALSE;                // Program has failed.    
	}   

	// Set the IWbemServices proxy so that impersonation    
	// of the user (client) occurs.    
	hres = CoSetProxyBlanket(   
		pSvc,                         // the proxy to set    
		RPC_C_AUTHN_WINNT,            // authentication service    
		RPC_C_AUTHZ_NONE,             // authorization service    
		NULL,                         // Server principal name    
		RPC_C_AUTHN_LEVEL_CALL,       // authentication level    
		RPC_C_IMP_LEVEL_IMPERSONATE,  // impersonation level    
		NULL,                         // client identity     
		EOAC_NONE                     // proxy capabilities         
		);   

	if( FAILED(hres) )   
	{   
		pSvc->Release();   
		pLoc->Release();        
		CoUninitialize();   
		return FALSE;               // Program has failed.    
	}   

	IEnumWbemClassObject* pEnumerator = NULL;   
	hres = pSvc->CreateInstanceEnum( L"MSSMBios_RawSMBiosTables", 0, NULL, &pEnumerator);
	if( FAILED(hres) )   
	{   
		pSvc->Release();   
		pLoc->Release();        
		CoUninitialize();   
		return FALSE;               // Program has failed.    
	}   
	else   
	{    
		do   
		{   
			IWbemClassObject* pInstance = NULL;   
			ULONG dwCount = NULL;   

			hres = pEnumerator->Next( WBEM_INFINITE, 1, &pInstance, &dwCount);         
			if( SUCCEEDED(hres) )   
			{   
				VARIANT varBIOSData;   
				VariantInit(&varBIOSData);   
				CIMTYPE  type;   

				hres = pInstance->Get(bstr_t("SmbiosMajorVersion"),0,&varBIOSData,&type,NULL);   
				if( FAILED(hres) )   
				{   
					VariantClear(&varBIOSData);   
				}   
				else   
				{   
					m_smbiosbuffer.nSMBIOSMajorVersion = varBIOSData.iVal;   
					VariantInit(&varBIOSData);   
					hres = pInstance->Get( bstr_t("SmbiosMinorVersion"), 0, &varBIOSData, &type, NULL );   
					if( FAILED(hres) )   
					{   
						VariantClear( &varBIOSData );   
					}   
					else   
					{   
						m_smbiosbuffer.nSMBIOSMinorVersion = varBIOSData.iVal;   
						VariantInit(&varBIOSData);   
						hres = pInstance->Get( bstr_t("SMBiosData"), 0, &varBIOSData, &type, NULL );   
						if( SUCCEEDED(hres) )   
						{   
							if( ( VT_UI1 | VT_ARRAY  ) != varBIOSData.vt )   
							{   
							}   
							else   
							{   
								SAFEARRAY           *parray = NULL;   
								parray = V_ARRAY(&varBIOSData);   
								BYTE* pbData = (BYTE*)parray->pvData;   

								m_smbiosbuffer.nLength = parray->rgsabound[0].cElements;   
								(*p) = new unsigned char[m_smbiosbuffer.nLength];   
								memcpy_s( (*p), m_smbiosbuffer.nLength, pbData, m_smbiosbuffer.nLength );
								bRet = TRUE;   
							}   
						}   
						VariantClear( &varBIOSData );   
					}   
				}   
				break;   
			}   

		}while( hres == WBEM_S_NO_ERROR );   
	}   

	// Cleanup    
	// ========    
	pSvc->Release();   
	pLoc->Release();        
	CoUninitialize();   

	return bRet;  
}


typedef struct _UNICODE_STRING
{
	USHORT  Length;         //长度
	USHORT  MaximumLength;  //最大长度
	PWSTR   Buffer;         //缓存指针，访问物理内存时，此处指向UNICODE字符串"\device\physicalmemory"
}UNICODE_STRING,*PUNICODE_STRING;


typedef struct _OBJECT_ATTRIBUTES 
{
	ULONG  Length;                   //长度 18h
	HANDLE RootDirectory;            //  00000000
	PUNICODE_STRING ObjectName;      //指向对象名的指针
	ULONG Attributes;                //对象属性00000040h
	PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR，0
	PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE，0
}OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

typedef DWORD  (__stdcall *ZWOS)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES);
typedef DWORD  (__stdcall *ZWMV)(HANDLE,HANDLE,PVOID,ULONG,ULONG,PLARGE_INTEGER,PSIZE_T,DWORD,ULONG,ULONG);
typedef DWORD  (__stdcall *ZWUMV)(HANDLE,PVOID);
typedef DWORD  (__stdcall *ZWCS)(HANDLE);

#define NT_SUCCESS(Status)            ((NTSTATUS)(Status) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH        ((NTSTATUS)0xC0000004L)
#define STATUS_ACCESS_DENIED ((NTSTATUS)0xC0000022L)

typedef LONG  NTSTATUS;


bool CSMBiosTable::FetchSMBiosDataInXP32( unsigned char ** p )
{
	UNICODE_STRING  struniph;
	struniph.Buffer        = L"\\device\\physicalmemory";
	struniph.Length        = 0x2c;//注意大小是按字节算
	struniph.MaximumLength = 0x2e;//也是字节

	OBJECT_ATTRIBUTES obj_ar;
	obj_ar.Attributes               =64;           //属性
	obj_ar.Length                   =24;           //OBJECT_ATTRIBUTES类型的长度
	obj_ar.ObjectName               =&struniph;    //指向对象的指针
	obj_ar.RootDirectory            =0;
	obj_ar.SecurityDescriptor       =0;
	obj_ar.SecurityQualityOfService =0;

	DWORD   ba       = 0;
	LARGE_INTEGER so;
	so.LowPart       = 0x000f0000;//物理内存的基址，就是f000:0000
	so.HighPart      = 0x00000000;
	SIZE_T ssize     = 0xffff;

//	BASE_LOG_OUT(( P2SP_LOG, "LoadLibraryA\n" ));
	HMODULE hinstLib = LoadLibraryA("ntdll.dll");
	if( hinstLib == NULL )
		return false;

	ZWOS ZWopenS     = (ZWOS)GetProcAddress( hinstLib, "ZwOpenSection" );
	ZWMV ZWmapV      = (ZWMV)GetProcAddress( hinstLib, "ZwMapViewOfSection" );
	ZWUMV ZWunmapV   = (ZWUMV)GetProcAddress( hinstLib, "ZwUnmapViewOfSection" );
	ZWCS  ZWcs       = (ZWCS)GetProcAddress( hinstLib, "ZwClose" );
	if( ZWopenS == NULL || ZWmapV == NULL || ZWunmapV == NULL || ZWcs == NULL )
		return false;
	HANDLE hSection = NULL;
//	BASE_LOG_OUT(( P2SP_LOG, "ZWopenS\n" ));
	NTSTATUS status = ZWopenS( &hSection, 4, &obj_ar );
	if( !NT_SUCCESS(status) )
		return false;
//	BASE_LOG_OUT(( P2SP_LOG, "ZWmapV\n" ));
	status = ZWmapV( hSection, (HANDLE)0xffffffff, &ba, 0, 0xffff, &so, &ssize, 1, 0, 2 );
	if( !NT_SUCCESS(status) )
	{
//		BASE_LOG_OUT(( P2SP_LOG, "ZWcs\n" ));
		ZWcs( hSection );
		return false;
	}
//	BASE_LOG_OUT(( P2SP_LOG, "memcpy_s\n" ));
	*p = new unsigned char[ssize];
	memcpy_s( (*p), ssize, (char*)ba, ssize );
	m_smbiosbuffer.nLength = ssize;
	//do something
//	BASE_LOG_OUT(( P2SP_LOG, "ZWunmapV\n" ));
	ZWunmapV( hSection, (PVOID)ba );
//	BASE_LOG_OUT(( P2SP_LOG, "ZWcs\n" ));
	ZWcs( hSection );
	//BASE_LOG_OUT(( P2SP_LOG, "END\n"));
	return true;
}