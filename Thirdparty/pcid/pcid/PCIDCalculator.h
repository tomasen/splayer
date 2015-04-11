#pragma once

#define PCID_DISK (0x00000001)
#define PCID_MAC  (0x00000002)
#define PCID_CPU  (0x00000004)
#define PCID_BIOS (0x00000008)
#define PCID_ALL  (PCID_DISK | PCID_MAC | PCID_CPU | PCID_BIOS)

class CPCIDCalculator
{
public:
	CPCIDCalculator(void);
public:
	~CPCIDCalculator(void);
public:
	int  CalculatePCID( char buf[16], char flag );
	int  GetPCBrand( char * buf, int buflen );
	int  GetPCModel( char * buf, int buflen );
	int  GetPCMac( char * buf, int buflen );
};

/*
By wangrui 2011.3.25
在驱动之家的代码基础上增加一个普通函数.方便其他人使用
*/

extern "C" int GetPCID(char buf[16], char flag);
extern "C" int GetFullPCID(char buf[16]);