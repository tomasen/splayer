
#define _WIN32_DCOM
#include "stdafx.h"
#include "MyMacAddress.h" 
#include  <comutil.h>  
#include  <Wbemidl.h> 
#include  <tchar.h>
#include <winioctl.h>
#include  <strsafe.h> 
#include  <algorithm>  
#include  <ntddndis.h>  
#include  <atlconv.h>

//#include "E:\WINDDK\3790.1830\inc\ddk\wnet\ndisguid.h"

#pragma comment (lib, "comsuppw.lib")  
#pragma comment (lib, "wbemuuid.lib") 



typedef struct _T_WQL_QUERY{
	CHAR*	szSelect;		// SELECT语句
	WCHAR*	szProperty;// 属性字段  
} T_WQL_QUERY; 

const T_WQL_QUERY szWQLQuery[] = {
	// 包含USB网卡  	
	"SELECT * FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%'))",  
	L"PNPDeviceID",
	// 不包含USB网卡
	"SELECT * FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%')) AND (NOT (PNPDeviceID LIKE 'USB%'))",
	L"PNPDeviceID"}; 

BYTE  MyMacAddress::PermanentAddress[MACADDRESS_BYTELEN];
BYTE MyMacAddress::MACAddress[MACADDRESS_BYTELEN];

MyMacAddress::MyMacAddress(void)
{
	//ZeroMemory(PermanentAddress, sizeof(PermanentAddress));
	//ZeroMemory(MACAddress, sizeof(MACAddress));
}

MyMacAddress::~MyMacAddress(void)
{
}
void MyMacAddress::GetMacReallyValue(BYTE MacAddress[MACADDRESS_BYTELEN], char szRealMacAddr[MACADDRESS_CHARSLEN])
{
	ZeroMemory(szRealMacAddr, MACADDRESS_CHARSLEN * sizeof(char));
	sprintf(szRealMacAddr, "%02x-%02x-%02x-%02x-%02x-%02x", MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5]);
}
int MyMacAddress::WMI_GetMacAddress(int iQueryType, int iSize)
{
	ZeroMemory(PermanentAddress, sizeof(PermanentAddress));
	ZeroMemory(MACAddress, sizeof(MACAddress));
	HRESULT hres;  	int	iTotal = 0;
	if( (iQueryType  < 0) || (iQueryType >= sizeof(szWQLQuery)/sizeof(T_WQL_QUERY)) ) 
	{  		
		return -1;
	}    	
	hres = CoInitializeEx( NULL, COINIT_MULTITHREADED); 
	if( FAILED(hres) )      
	{          
		return -2; 
	}        
	hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if( FAILED(hres))
	{          
		CoUninitialize();
		return -2;      
	}        	
	IWbemLocator *pLoc = NULL;
	hres = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast <LPVOID*>(&pLoc));
	if( FAILED(hres))
	{  		
		CoUninitialize();
		return -2;      
	}
	IWbemServices *pSvc = NULL;
	hres = pLoc->ConnectServer(_bstr_t( L"ROOT\\CIMV2" ), NULL, NULL, NULL, 0, NULL, NULL, &pSvc);
	if( FAILED(hres))      
	{  		
		pLoc->Release();           
		CoUninitialize();          
		return -2;      
	}    	
	// 设置请求代理的安全级别
	hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	if(FAILED(hres))
	{          
		pSvc->Release();
		pLoc->Release(); 
		CoUninitialize();  
		return -2;      
	}       
	IEnumWbemClassObject *pEnumerator = NULL;
	hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t( szWQLQuery[iQueryType].szSelect), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);  	
	if(FAILED(hres)) 
	{          
		pSvc->Release();
		pLoc->Release();  
		CoUninitialize();         
		return -3;      
	}        
	while(pEnumerator)
	{  		
		IWbemClassObject *pclsObj = NULL;
		ULONG uReturn = 0;    	
		if(iTotal >= iSize)  
		{  			
			break;  	
		}           
		pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);            
		if( uReturn == 0 )         
		{              
			break;
		}    		
		VARIANT	vtProperty;  	
		TCHAR szProperty[128]; 
		// 获取网卡范例ID		
		VariantInit( &vtProperty);	  	
		pclsObj->Get( szWQLQuery[iQueryType].szProperty, 0, &vtProperty, NULL, NULL );  
		StringCchCopy( szProperty, sizeof(szProperty)/sizeof(TCHAR), W2T(vtProperty.bstrVal) );  
		VariantClear( &vtProperty );    			
		if( GetPNPDeviceID( szProperty))  	
		{  			
			iTotal++;  		
		}  		  		
		pclsObj->Release();  
	}
	// 释放资源  
	pEnumerator->Release(); 
	pSvc->Release();     
	pLoc->Release();   
	CoUninitialize();     
	return iTotal;  
}

BOOL MyMacAddress::GetPNPDeviceID( const TCHAR *PNPDeviceID)
 {  	
	// GUID_NDIS_LAN_CLASS   ad498944-762f-11d0-8dcb-00c04fc3358c
	 TCHAR	DevicePath[MAX_PATH];
	 HANDLE	hDeviceFile;
	 BOOL	isOK = FALSE;    	// 生成设备路径名
	 ZeroMemory(DevicePath, sizeof(DevicePath));
	 StringCchCopy( DevicePath, MAX_PATH, TEXT("\\\\.\\"));
	 StringCchCat( DevicePath, MAX_PATH, PNPDeviceID );
	 StringCchCat( DevicePath, MAX_PATH, TEXT("#{ad498944-762f-11d0-8dcb-00c04fc3358c}") );
	 std::replace( DevicePath + 4, DevicePath + 4 + _tcslen(PNPDeviceID), TEXT('\\'), TEXT('#') );     	// 获取设备句柄
	 hDeviceFile = CreateFile( DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	 if( hDeviceFile != INVALID_HANDLE_VALUE ){
		 ULONG	dwID;
		 BYTE	ucData[8];
		 DWORD	dwByteRet;
		 dwID = OID_802_3_CURRENT_ADDRESS;
		 isOK = DeviceIoControl( hDeviceFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &dwID, sizeof(dwID), ucData, sizeof(ucData), &dwByteRet, NULL );
		 if( isOK )
		 {	  			
			 memcpy(MACAddress, ucData, dwByteRet);
		 }    
		 dwID = OID_802_3_PERMANENT_ADDRESS;  			
		 isOK = DeviceIoControl( hDeviceFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &dwID, sizeof(dwID), ucData, sizeof(ucData), &dwByteRet, NULL );
		 if( isOK )
		 {	  				
			 memcpy(PermanentAddress, ucData, dwByteRet); 
		 }  	
		 CloseHandle( hDeviceFile );  	
	 }    	
	 return isOK;  
 }