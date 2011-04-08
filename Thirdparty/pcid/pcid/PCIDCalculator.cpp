#include "StdAfx.h"
#include "PCIDCalculator.h"
#include "PCIdentificationCode.h"
#include "BiosInfo.h"
#include <intrin.h>

#include <shellapi.h>

#include "MyMacAddress.h"

#include <Nb30.h>
#pragma comment(lib, "Netapi32.lib")
typedef  struct  _ASTAT_
{
	ADAPTER_STATUS adapt;
	NAME_BUFFER    NameBuff [ 30 ];
}ASTAT,  *  PASTAT;
ASTAT Adapter;





CPCIDCalculator::CPCIDCalculator(void)
{
}

CPCIDCalculator::~CPCIDCalculator(void)
{
}

int CPCIDCalculator::CalculatePCID( char buf[16], char flag )
{

//test

	/*int nresult;
	LANA_ENUM   lenum;
	NCB m_ncb;
	ZeroMemory(&m_ncb, sizeof(m_ncb));
	m_ncb.ncb_command = NCBENUM;
	m_ncb.ncb_buffer = (UCHAR*)&lenum;
	m_ncb.ncb_length = sizeof(m_ncb);
	nresult = Netbios(&m_ncb);
	for(int i = 0; i < lenum.length; i++)
	{
		ZeroMemory(&m_ncb, sizeof(m_ncb));
		m_ncb.ncb_command = NCBRESET;
		m_ncb.ncb_lana_num = lenum.lana[i];
		nresult = Netbios(&m_ncb);
		ZeroMemory(&m_ncb, sizeof(m_ncb));
		m_ncb.ncb_command = NCBASTAT;
		m_ncb.ncb_lana_num = lenum.lana[i];
		strcpy((char *)m_ncb.ncb_callname, "* ");
		m_ncb.ncb_buffer = (UCHAR*)&Adapter;
		m_ncb.ncb_length = sizeof(Adapter);
		nresult = Netbios(&m_ncb);
		if(nresult == NRC_GOODRET)
		{
			char MacAddres[MAX_PATH] = {0};
			sprintf(MacAddres, "%d is:%02x-%02x-%02x-%02x-%02x-%02x", lenum.lana[i], Adapter.adapt.adapter_address[0], Adapter.adapt.adapter_address[1],
				Adapter.adapt.adapter_address[2], Adapter.adapt.adapter_address[3], Adapter.adapt.adapter_address[4], Adapter.adapt.adapter_address[5]);
			MessageBoxA(NULL, MacAddres, "", 0);
		}
	}*/

    MyMacAddress myAdress;
    if( myAdress.WMI_GetMacAddress(1, 1) > 0)
	{
		char MacAddres[MAX_PATH] = {0};
		//myAdress.GetMacReallyValue(MyMacAddress::MACAddress, MacAddres);
		//MessageBoxA(NULL, MacAddres, "", 0);
		myAdress.GetMacReallyValue(MyMacAddress::PermanentAddress, MacAddres);
		//MessageBoxA(NULL, MacAddres, "", 0);
	}
//test


	char tembuf[1024];
	int  total = 1024;
	int  used  = 0;

	ZeroMemory(tembuf, sizeof(tembuf));
	if( flag&PCID_DISK )
	{
		CPCIdentificationCode ide;
		used += ide.GetIdentificationCode( &tembuf[used], total-used );
	}
	if( flag&PCID_MAC )
	{
		MyMacAddress myAdress;
        if( myAdress.WMI_GetMacAddress(1, 1) > 0)
		{
            memcpy(&tembuf[used], MyMacAddress::PermanentAddress, MACADDRESS_BYTELEN);
            used += MACADDRESS_BYTELEN;
		}
		//CPCIdentificationCode ide;
		//used += ide.GetMac( &tembuf[used], total-used );
	}
	if( flag&PCID_CPU )
	{
		int arr[4];
		ZeroMemory( arr, sizeof(arr) );
		__cpuid( arr, 0 );
		memcpy_s( &tembuf[used], total-used, arr, sizeof(arr));
		used += sizeof(arr);

		ZeroMemory( arr, sizeof( arr ) );
		__cpuid( arr, 1 );
		//  防止数组第二位开头出错，出现 不同结果
		memcpy_s( &tembuf[used], total-used, arr, 4);
		memcpy_s( &tembuf[used + 4], total-used, arr + 2, sizeof(arr) - 8);
		//memcpy_s( &tembuf[used], total-used, arr, sizeof(arr));
		used += (sizeof(arr) - 4);

		ZeroMemory( arr, sizeof( arr ));
		__cpuid( arr, 3 );
		memcpy_s( &tembuf[used], total-used, arr, sizeof(arr) );
		used += sizeof(arr);

		ZeroMemory( arr, sizeof(arr) );
		__cpuid( arr, 4 );
		memcpy_s( &tembuf[used], total-used, arr, sizeof(arr) );
		used += sizeof(arr);
	}
	if( flag&PCID_BIOS )
	{
		CBiosInfo bios;
		if( bios.InitBiosInfo() )
		{
			string str = bios.GetSystemECAssetTagNmuber();
			str += bios.GetSystemECManufacturer();
			str += bios.GetSystemECSerialNumber();
			str += bios.GetSystemECType();
			str += bios.GetSystemECVersion();
			str += bios.GetSystemFamily();
			str += bios.GetSystemManufacturer();
			str += bios.GetSystemProductName();
			str += bios.GetSystemSerialNumber();
			str += bios.GetSystemSKUNumber();
			str += bios.GetSystemVersion();
			memcpy_s( &tembuf[used], total-used, str.c_str(), str.length() );
			used += str.length();
		}
	}
	unsigned long md5value[4] = {0};
	MD5(tembuf, used, md5value);
	//MD5 md5( tembuf, used );
	//md5.digest()
	memcpy_s( buf, 16, md5value, 16 );
	return true;

			//test

		//	char Title[128] = "ECAssetTagNmuber: ";
		//	FILE* fl = fopen("./test.txt", "w");
		//	if(fl)
		//	{
  //             strcat(Title, bios.GetSystemECAssetTagNmuber().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "ECManufacturer: ");
		//	   strcat(Title, bios.GetSystemECManufacturer().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "ECSerialNumber: ");
		//	   strcat(Title, bios.GetSystemECSerialNumber().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "ECType: ");
		//	   strcat(Title, bios.GetSystemECType().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "ECVersion: ");
		//	   strcat(Title, bios.GetSystemECVersion().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "SystemFamily: ");
		//	   strcat(Title, bios.GetSystemFamily().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "Manufacturer: ");
		//	   strcat(Title, bios.GetSystemManufacturer().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "ProductName: ");
		//	   strcat(Title, bios.GetSystemProductName().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "SerialNumber: ");
		//	   strcat(Title, bios.GetSystemSerialNumber().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "SKUNumber: ");
		//	   strcat(Title, bios.GetSystemSKUNumber().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "Version: ");
		//	   strcat(Title, bios.GetSystemVersion().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);

		//	   strcat(Title, "ESKUNumber: ");
		//	   strcat(Title, bios.GetSystemSKUNumber().c_str());
		//	   fwrite(Title, sizeof(char), 128, fl);
		//	   fwrite("\r\n", sizeof(char), 2, fl);
		//	   ZeroMemory(Title, 128);


		//	   //strcat(Title, "PCMac");
		//	   char Buf[128];
		//	   int nReslut = GetPCMac( Buf, 128 );
		//	   sprintf_s(Title, 16, "%d PCMac: ", nReslut);
		//	   memcpy(&Title[16], Buf, 112);
  //             //strcat(Title, Buf);
		//	   fwrite(Title, sizeof(char), 128, fl);
  //             fwrite("\r\n", sizeof(char), 2, fl);
		//	   fclose(fl);

		//	}
		//	ShellExecuteA(NULL, "open", "./test.txt", NULL, NULL, SW_SHOW);
		//}
}

int  CPCIDCalculator::GetPCBrand( char * buf, int buflen )
{
	CBiosInfo bios;
	if( bios.InitBiosInfo() )
	{
		string str = bios.GetSystemManufacturer();
		if( buflen > str.length() )
		{
			strcat_s( buf, buflen, str.c_str() );
			return str.length();
		}
		else
		{
			return -1;
		}
	}
	return -2;
}
int  CPCIDCalculator::GetPCModel( char * buf, int buflen )
{
	CBiosInfo bios;
	if( bios.InitBiosInfo() )
	{
		string str = bios.GetSystemProductName();
		if( buflen > str.length() )
		{
			strcat_s( buf, buflen, str.c_str() );
			return str.length();
		}
		else
		{
			return -1;
		}
	}
	return -2;
}
int  CPCIDCalculator::GetPCMac( char * buf, int buflen )
{
	CPCIdentificationCode ide;
	return ide.GetMac( buf, buflen );
}



int __declspec(dllexport) GetPCID(char buf[16], char flag)
{
    CPCIDCalculator  pcid;
	return pcid.CalculatePCID(buf, flag);
}

int __declspec(dllexport) GetFullPCID(char buf[16])
{
	CPCIDCalculator  pcid;
	return pcid.CalculatePCID(buf, PCID_DISK | PCID_MAC | PCID_CPU | PCID_BIOS);
}