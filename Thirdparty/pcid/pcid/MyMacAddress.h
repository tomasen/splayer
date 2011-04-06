#pragma once
#include  <windows.h>

#define MACADDRESS_BYTELEN      6
#define MACADDRESS_CHARSLEN     32

class MyMacAddress
{
public:
	MyMacAddress(void);
	~MyMacAddress(void);
public:
	int WMI_GetMacAddress( int iQueryType, int iSize);
	static void GetMacReallyValue(BYTE  MacAddress[MACADDRESS_BYTELEN], char szRealMacAddr[MACADDRESS_CHARSLEN]);

protected:
	static BOOL GetPNPDeviceID(const TCHAR *PNPDeviceID);
public:
	static BYTE  PermanentAddress[MACADDRESS_BYTELEN];
	static BYTE  MACAddress[MACADDRESS_BYTELEN];
};
