#pragma once

#include "stdafx.h"
#include "md5.h"
//#include "IdentificationCode.h"

//#include "CommonDefined.h"

#define   DFP_GET_VERSION   0x00074080     
#define   DFP_SEND_DRIVE_COMMAND   0x0007c084     
#define   DFP_RECEIVE_DRIVE_DATA   0x0007c088     

#pragma pack(push, 1)

typedef struct _GETVERSIONOUTPARAMS{     
	BYTE   bVersion;     //   Binary driver version.     
	BYTE   bRevision;     //   Binary driver revision.     
	BYTE   bReserved;     //   Not used.     
	BYTE   bIDEDeviceMap;   //   Bit map of IDE devices.     
	DWORD  fCapabilities;   //   Bit mask of driver capabilities.     
	DWORD  dwReserved[4];   //   For future use.     
}GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;     
/*WinIOCtl.h中已经有定义
typedef struct _IDEREGS{     
	BYTE   bFeaturesReg;     //Used   for   specifying   SMART   "commands".     
	BYTE   bSectorCountReg;   //IDE   sector   count   register     
	BYTE   bSectorNumberReg;   //IDE   sector   number   register     
	BYTE   bCylLowReg;       //IDE   low   order   cylinder   value     
	BYTE   bCylHighReg;     //IDE   high   order   cylinder   value     
	BYTE   bDriveHeadReg;     //IDE   drive/head   register     
	BYTE   bCommandReg;     //Actual   IDE   command.     
	BYTE   bReserved;       //reserved   for   future   use.     Must   be   zero.     
}   IDEREGS,   *PIDEREGS,   *LPIDEREGS;     

typedef struct _SENDCMDINPARAMS{     
	DWORD   cBufferSize;     //Buffer size in bytes     
	IDEREGS irDriveRegs;     //Structure with drive register values.     
	BYTE    bDriveNumber;     //Physical drive number to send     
	//command to (0,1,2,3).     
	BYTE    bReserved[3];     //Reserved for future expansion.     
	DWORD   dwReserved[4];     //For future use.     
	//BYTE     bBuffer[1];       //Input buffer.     
}SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;     

typedef struct _DRIVERSTATUS{     
	BYTE bDriverError;   //Error code from driver,     
	//or 0 if no error.     
	BYTE bIDEStatus;     //Contents of IDE Error register.     
	//Only valid when bDriverError     
	//is SMART_IDE_ERROR.     
	BYTE bReserved[2];     //Reserved  for future expansion.     
	DWORD dwReserved[2];     //Reserved  for future expansion.     
}   DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;     

typedef struct _SENDCMDOUTPARAMS {     
	DWORD         cBufferSize;     //Size of bBuffer in bytes     
	DRIVERSTATUS   DriverStatus;     //Driver status structure.     
	BYTE       bBuffer[512];       //Buffer of arbitrary length     
	//in which to store the data read from the drive.     
}SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;     
*/
typedef   struct   _IDSECTOR   {     
	USHORT   wGenConfig;     
	USHORT   wNumCyls;     
	USHORT   wReserved;     
	USHORT   wNumHeads;     
	USHORT   wBytesPerTrack;     
	USHORT   wBytesPerSector;     
	USHORT   wSectorsPerTrack;     
	USHORT   wVendorUnique[3];     
	CHAR   sSerialNumber[20];     
	USHORT   wBufferType;     
	USHORT   wBufferSize;     
	USHORT   wECCSize;     
	CHAR   sFirmwareRev[8];     
	CHAR   sModelNumber[40];     
	USHORT   wMoreVendorUnique;     
	USHORT   wDoubleWordIO;     
	USHORT   wCapabilities;     
	USHORT   wReserved1;     
	USHORT   wPIOTiming;     
	USHORT   wDMATiming;     
	USHORT   wBS;     
	USHORT   wNumCurrentCyls;     
	USHORT   wNumCurrentHeads;     
	USHORT   wNumCurrentSectorsPerTrack;     
	ULONG   ulCurrentSectorCapacity;     
	USHORT   wMultSectorStuff;     
	ULONG   ulTotalAddressableSectors;     
	USHORT   wSingleWordDMA;     
	USHORT   wMultiWordDMA;     
	BYTE   bReserved[128];     
}   IDSECTOR,   *PIDSECTOR; 
#pragma pack(pop)



class CIdentificationCode
{
public:
	~CIdentificationCode(){};
};

class CPCIdentificationCode : public CIdentificationCode
{
public:
	CPCIdentificationCode(void);
public:
	~CPCIdentificationCode(void);
public:
	virtual int GetIdentificationCode( char * buf, int len );
	virtual void GetCPUInfo( int arr[4] );
private:
	void HDNT( char * buf, int len );
	void HD9X(  char * buf, int len );
	void ChangeByteOrder( PCHAR szString, USHORT uscStrSize );
public:

	int  GetMac( char * buf, int len );
};
