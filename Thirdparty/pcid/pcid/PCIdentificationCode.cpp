#include "StdAfx.h"
#include "PCIdentificationCode.h"
#include <IPTypes.h>
#include <intrin.h>
#include <Iphlpapi.h>
#include "md5.h"

#pragma comment( lib, "Iphlpapi.lib" )

CPCIdentificationCode::CPCIdentificationCode(void)
{
}

CPCIdentificationCode::~CPCIdentificationCode(void)
{
}

int  CPCIdentificationCode::GetIdentificationCode( char * buf, int len )
{
	char tmpBuf[61];
	memset( tmpBuf, 0, 61 );
	OSVERSIONINFO VersionInfo;     
	ZeroMemory( &VersionInfo, sizeof(VersionInfo) );     
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);     
	GetVersionEx( &VersionInfo );     
	switch( VersionInfo.dwPlatformId )
	{     
	case VER_PLATFORM_WIN32s: 
		memset( tmpBuf, 1, 60 );
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		HD9X( tmpBuf, 61 );
		break;
	case VER_PLATFORM_WIN32_NT: 
		HDNT( tmpBuf, 61 );
		break;
	default:
		memset( tmpBuf, 1, 60 );
		break;
	}
// 	assert( len >= 16 );
// 	MD5 md5( tmpBuf, 60 );
// 	int  cpu[4]={0,0,0,0};
// 	GetCPUInfo( cpu );
// 	md5.update( cpu, sizeof(cpu) );

	memcpy_s( buf, len, tmpBuf, 60 );
	return 60;
}
void CPCIdentificationCode::HDNT( char * buf, int len )
{
	GETVERSIONOUTPARAMS  vers;   
	ZeroMemory( &vers, sizeof(vers) );        
	for( DWORD j = 0; j < 4; j++ )
	{     
		char hd[80];
		sprintf_s( hd, 80, "\\\\.\\PhysicalDrive%d", j );     
		HANDLE hPD = CreateFileA( hd, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 );     
		if( !hPD )
		{     
			continue;     
		}
		DWORD i;  
		if( !DeviceIoControl( hPD, DFP_GET_VERSION, 0, 0, &vers, sizeof(vers), &i, 0 ) )
		{     
			CloseHandle( hPD );     
			continue;     
		}     
		//If IDE identify command not supported, fails     
		if( !(vers.fCapabilities&1) )
		{     
			//cout<<"Error: IDE identify command not supported.";     
			CloseHandle( hPD );     
			return;     
		}     
		//Identify the IDE drives 
		SENDCMDINPARAMS      in;     
		SENDCMDOUTPARAMS     out; 
		ZeroMemory( &in, sizeof(in) );     
		ZeroMemory( &out, sizeof(out) );     
		if( j&1 )
		{     
			in.irDriveRegs.bDriveHeadReg = 0xb0;     
		}
		else
		{     
			in.irDriveRegs.bDriveHeadReg = 0xa0;     
		}     
		if( vers.fCapabilities&(16>>j) )
		{     
			//We don't detect a  ATAPI device.     
			//cout<<"Drive "<<(int)(j+1)<<" is a ATAPI device, we don't detect it"<<endl;     
			continue;
		}
		else
		{     
			in.irDriveRegs.bCommandReg = 0xec;
		}     
		in.bDriveNumber = j;
		in.irDriveRegs.bSectorCountReg  = 1;
		in.irDriveRegs.bSectorNumberReg = 1;
		in.cBufferSize = 512;
		if( !DeviceIoControl( hPD, DFP_RECEIVE_DRIVE_DATA, &in, sizeof( in ), &out, sizeof( out ), &i, 0 ) )
		{
			CloseHandle( hPD );
			return;
		}
		PIDSECTOR phdinfo = (PIDSECTOR)out.bBuffer;

		if( len < 60 )
		{
			return;
		}
		memcpy( buf, phdinfo->sModelNumber, 40 ); 
		memcpy( &buf[40], phdinfo->sSerialNumber, 20 );
		buf[60] = 0;
		// 		cout<<endl<<"Ó²ÅÌID:"<<s<<endl;
		// 		memcpy( s, phdinfo->sFirmwareRev, 8 );     
		// 		s[8]=0;     
		// 		ChangeByteOrder( s, 8 );     
		// 		cout<<"ÐÞ¶©ºÅ:"<<s<<endl;     
		// 		memcpy(s,phdinfo->sSerialNumber,20);     
		// 		s[20]=0;     
		// 		ChangeByteOrder( s, 20 );     
		// 		cout<< "ÐòÁÐºÅ:" << s << endl;     
		// 		cout<< "ÈÝÁ¿:" << phdinfo->ulTotalAddressableSectors/2/1024 << "M" << endl << endl;     
		CloseHandle( hPD );     
	}   
}
void CPCIdentificationCode::HD9X(  char * buf, int len )
{
	GETVERSIONOUTPARAMS  vers;
	ZeroMemory( &vers, sizeof(vers) );     
	HANDLE h = CreateFileA( "\\\\.\\Smartvsd", 0, 0, 0, CREATE_NEW, 0, 0 );     
	if( !h )
	{     
		return;
	}     

	DWORD i;
	if( !DeviceIoControl( h, DFP_GET_VERSION, 0, 0, &vers, sizeof(vers), &i, 0 ) )
	{   
		CloseHandle(h);     
		return;     
	}     
	//If IDE identify command not supported, fails     
	if( !(vers.fCapabilities&1) )
	{    
		CloseHandle(h);     
		return;     
	}      
	//Identify the IDE drives     
	for( DWORD j = 0; j < 4; j++ )
	{     
		SENDCMDINPARAMS      in;     
		SENDCMDOUTPARAMS     out;
		ZeroMemory( &in, sizeof(in) );     
		ZeroMemory( &out, sizeof(out) );     
		if( j&1 )
		{     
			in.irDriveRegs.bDriveHeadReg = 0xb0;     
		}
		else
		{     
			in.irDriveRegs.bDriveHeadReg = 0xa0;     
		}     
		if( vers.fCapabilities&(16>>j) )
		{     
			//We don't detect a ATAPI device.       
			continue;     
		}
		else
		{     
			in.irDriveRegs.bCommandReg = 0xec;     
		}     
		in.bDriveNumber = j;     
		in.irDriveRegs.bSectorCountReg  = 1;     
		in.irDriveRegs.bSectorNumberReg = 1;     
		in.cBufferSize = 512;     
		if( !DeviceIoControl( h, DFP_RECEIVE_DRIVE_DATA, &in, sizeof(in), &out, sizeof(out), &i, 0 ) )
		{
//			cout<<"DeviceIoControl   failed:DFP_RECEIVE_DRIVE_DATA"<<endl;
			CloseHandle(h);
			return;
		}
		PIDSECTOR phdinfo=(PIDSECTOR)out.bBuffer;
		if( len < 60 )
		{
			return;
		}
		memcpy( buf, phdinfo->sModelNumber, 40 ); 
		memcpy( &buf[40], phdinfo->sSerialNumber, 20 );
		buf[60] = 0;

// 		memcpy(s,phdinfo->sModelNumber,40);
// 		s[40]=0; 
// 		ChangeByteOrder(s,40);
// 		cout<<endl<<"Module   Number:"<<s<<endl;  
// 		memcpy(s,phdinfo->sFirmwareRev,8); 
// 		s[8]=0;
// 		ChangeByteOrder(s,8);
// 		cout<<"\tFirmware   rev:"<<s<<endl;
// 		memcpy(s,phdinfo->sSerialNumber,20);
// 		s[20]=0;
// 		ChangeByteOrder(s,20);
// 		cout<<"\tSerial   Number:"<<s<<endl;
// 		cout<<"\tCapacity:"<<phdinfo->ulTotalAddressableSectors/2/1024<<"M"<<endl<<endl;
	}
	//Close   handle   before   quit
	CloseHandle(h);
}
void CPCIdentificationCode::ChangeByteOrder( PCHAR szString, USHORT uscStrSize )
{  
	CHAR temp;     
	for( USHORT i = 0; i < uscStrSize; i += 2 )     
	{     
		temp          = szString[i];     
		szString[i]   = szString[i+1];     
		szString[i+1] = temp;     
	}   
}
int CPCIdentificationCode::GetMac( char * buf, int len )
{
	UINT uErrorCode = 0;
	IP_ADAPTER_INFO iai;
	ULONG uSize = 0;
	DWORD dwResult = GetAdaptersInfo( &iai, &uSize );
	if( dwResult == ERROR_BUFFER_OVERFLOW )
	{
		IP_ADAPTER_INFO* piai = ( IP_ADAPTER_INFO* )new char[uSize];
		if( piai != NULL )
		{
			dwResult = GetAdaptersInfo( piai, &uSize );
			if( ERROR_SUCCESS == dwResult )
			{
				IP_ADAPTER_INFO* piai2 = piai;
				DWORD uSystemInfoLen = 0;
				while( piai2 != NULL && ( uSystemInfoLen + piai2->AddressLength ) < len )
				{
					CopyMemory( buf + uSystemInfoLen, piai2->Address, piai2->AddressLength );
					uSystemInfoLen += piai2->AddressLength;
					piai2 = piai2->Next;
				}
				delete piai;
				return uSystemInfoLen;
			}
		}
		delete piai;
	}
	return 0;
}
void CPCIdentificationCode::GetCPUInfo( int arr[4] )
{
	__cpuid( arr,  0);
}