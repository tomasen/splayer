#pragma once

#include <string>
#include <vector>
using namespace std;

#define _WIN32_DCOM

//#include "base_log.h"

typedef struct _SystemInformation
{
	unsigned char  m_cType;
	unsigned char  m_cLength;
	unsigned short m_uHandle;
	unsigned char  m_cManufacturer;
	unsigned char  m_cProductName;
	unsigned char  m_cVersion;
	unsigned char  m_cSerialNumber;
	unsigned char  m_uuid[16];
	unsigned char  m_cWakeupType;
	unsigned char  m_cSKUNumber;
	unsigned char  m_cFamily;

	string m_strManufacturer;
	string m_strProductName;
	string m_strVersion;
	string m_strSerialNumber;
	string m_strSKUNumber;
	string m_strFamily;
	_SystemInformation()
	{
		m_cType = 1;
	}
	void ParseStringRegion( string & str, const unsigned char * pBegin, const unsigned int nLen, const unsigned char cIndex )
	{
		unsigned char nIndex  = 0;
		int nCurPos = 0;
		int nBegin  = 0;
		while( nCurPos < nLen )
		{
			if( pBegin[nCurPos] == '\0' )
			{
				nIndex++;
				if( nIndex == cIndex )
				{
					str = string( (char*)(&pBegin[nBegin]), nCurPos-nBegin );
					return;
				}
				else
				{
					nBegin = nCurPos + 1;
				}
			}
			nCurPos++;
		}
	}
	void ParseInfo( unsigned char * pInStructureBuffer, int nInStructureBufferlen )
	{
		if( nInStructureBufferlen < 27 )
			return;
		m_cType      = pInStructureBuffer[0];
		m_cLength    = pInStructureBuffer[1];

		m_uHandle    = pInStructureBuffer[3];
		m_uHandle    = (m_uHandle<<8);
		m_uHandle   += pInStructureBuffer[2];

		m_cManufacturer  = pInStructureBuffer[4];
		m_cProductName   = pInStructureBuffer[5];
		m_cVersion       = pInStructureBuffer[6];
		m_cSerialNumber  = pInStructureBuffer[7];

		memcpy_s( m_uuid, 16, &pInStructureBuffer[8], 16 );
		m_cWakeupType    = pInStructureBuffer[24];
		m_cSKUNumber     = pInStructureBuffer[25];
		m_cFamily        = pInStructureBuffer[26];

		ParseStringRegion( m_strManufacturer, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cManufacturer );
		ParseStringRegion( m_strProductName, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cProductName );
		ParseStringRegion( m_strVersion, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cVersion );
		ParseStringRegion( m_strSerialNumber, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cSerialNumber );
		ParseStringRegion( m_strSKUNumber, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cSKUNumber );
		ParseStringRegion( m_strFamily, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cFamily );
	}
	void PrintOut()
	{
		/*
		BASE_LOG_OUT(( P2SP_LOG, "\nSystem Enclosure:\n" ));
		BASE_LOG_OUT((  P2SP_LOG, "Manufacturer:%s\n", m_strManufacturer.c_str() ));
		BASE_LOG_OUT((  P2SP_LOG, "ProductName:%s\n", m_strProductName.c_str() ));
		BASE_LOG_OUT((  P2SP_LOG, "Version:%s\n", m_strVersion.c_str() ));
		BASE_LOG_OUT((  P2SP_LOG, "SerialNumber:%s\n", m_strSerialNumber.c_str() ));
		BASE_LOG_OUT((  P2SP_LOG, "SKUNumber:%s\n", m_strSKUNumber.c_str() ));
		BASE_LOG_OUT((  P2SP_LOG, "m_strFamily:%s\n", m_strFamily.c_str() ));
		*/
	}
}SSystemInformation, *PSystemInformation;

typedef struct _SystemEnclosureStructure
{
	unsigned char  m_cType;
	unsigned char  m_cLength;
	unsigned short m_uHandle;
	unsigned char  m_cManufacturer;
	unsigned char  m_cEnclosureType;
	unsigned char  m_cVersion;
	unsigned char  m_cSerialNumber;
	unsigned char  m_cAssetTagNumber;

	string m_strManufacturer;
	string m_strVersion;
	string m_strSerialNumber;
	string m_strAssetTagNumber;
	_SystemEnclosureStructure()
	{
		m_cType = 0x03;
	}
	void ParseStringRegion( string & str, const unsigned char * pBegin, const unsigned int nLen, const unsigned char cIndex )
	{
		unsigned char nIndex  = 0;
		int nCurPos = 0;
		int nBegin  = 0;
		while( nCurPos < nLen )
		{
			if( pBegin[nCurPos] == '\0' )
			{
				nIndex++;
				if( nIndex == cIndex )
				{
					str = string( (char*)(&pBegin[nBegin]), nCurPos-nBegin );
					return;
				}
				else
				{
					nBegin = nCurPos + 1;
				}
			}
			nCurPos++;
		}
	}
	void ParseInfo( unsigned char * pInStructureBuffer, int nInStructureBufferlen )
	{
		if( nInStructureBufferlen < 8 )
			return;
		m_cType      = pInStructureBuffer[0];
		m_cLength    = pInStructureBuffer[1];

		m_uHandle    = pInStructureBuffer[3];
		m_uHandle    = (m_uHandle<<8);
		m_uHandle   += pInStructureBuffer[2];

		m_cManufacturer  = pInStructureBuffer[4];
		m_cEnclosureType = pInStructureBuffer[5];
		m_cVersion       = pInStructureBuffer[6];
		m_cSerialNumber  = pInStructureBuffer[7];

		m_cAssetTagNumber= pInStructureBuffer[8];

		ParseStringRegion( m_strManufacturer, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cManufacturer );
		ParseStringRegion( m_strVersion, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cVersion );
		ParseStringRegion( m_strSerialNumber, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cSerialNumber );
		ParseStringRegion( m_strAssetTagNumber, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cAssetTagNumber );
	}
	void PrintOut()
	{/*
		BASE_LOG_OUT(( P2SP_LOG, "\nSystem Enclosure:\n" ));
		BASE_LOG_OUT(( P2SP_LOG, "EnclosureType %d\n", m_cEnclosureType ));
		BASE_LOG_OUT(( P2SP_LOG, "Manufacturer:%s\n", m_strManufacturer.c_str() ));
		BASE_LOG_OUT(( P2SP_LOG, "Version:%s\n", m_strVersion.c_str() ));
		BASE_LOG_OUT(( P2SP_LOG, "SerialNumber:%s\n", m_strSerialNumber.c_str() ));
		BASE_LOG_OUT(( P2SP_LOG, "AssetTagNumber:%s\n", m_strAssetTagNumber.c_str() ));
		*/
	}
}SSystemEnclosureStructure, *PSystemEnclosureStructure;

typedef struct _MemoryModuleInformation
{
	unsigned char  m_cType;
	unsigned char  m_cLength;
	unsigned short m_uHandle;
	unsigned char  m_cSocketDesignation;
	unsigned char  m_cBankConnections;
	unsigned char  m_cCurrentSpeed;
	unsigned short m_uCurrentMemoryType;
	unsigned char  m_cInstalledSize;
	unsigned char  m_cEnabledSize;
	unsigned char  m_cErrorStatus;
	string m_strSocketDesignation;
	_MemoryModuleInformation()
	{
		m_cType = 0x06;
	}
	void ParseStringRegion( string & str, const unsigned char * pBegin, const unsigned int nLen, const unsigned char cIndex )
	{
		unsigned char nIndex  = 0;
		int nCurPos = 0;
		int nBegin  = 0;
		while( nCurPos < nLen )
		{
			if( pBegin[nCurPos] == '\0' )
			{
				nIndex++;
				if( nIndex == cIndex )
				{
					str = string( (char*)(&pBegin[nBegin]), nCurPos-nBegin );
					return;
				}
				else
				{
					nBegin = nCurPos + 1;
				}
			}
			nCurPos++;
		}
	}
	void ParseInfo( unsigned char * pInStructureBuffer, int nInStructureBufferlen )
	{
		if( nInStructureBufferlen <= 11 )
			return;
		m_cType      = pInStructureBuffer[0];
		m_cLength    = pInStructureBuffer[1];

		m_uHandle    = pInStructureBuffer[3];
		m_uHandle    = (m_uHandle<<8);
		m_uHandle   += pInStructureBuffer[2];

		m_cSocketDesignation = pInStructureBuffer[4];
		m_cBankConnections   = pInStructureBuffer[5];
		m_cCurrentSpeed      = pInStructureBuffer[6];

		m_uCurrentMemoryType = pInStructureBuffer[8];
		m_uCurrentMemoryType = (m_uCurrentMemoryType<<8);
		m_uCurrentMemoryType+= pInStructureBuffer[7];

		m_cInstalledSize     = pInStructureBuffer[9];
		m_cEnabledSize       = pInStructureBuffer[10];
		m_cErrorStatus       = pInStructureBuffer[11];

		ParseStringRegion( m_strSocketDesignation, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cSocketDesignation );
	}
}SMemoryModuleInformation, *PMemoryModuleInformation;

typedef struct _BaseBoardInformation 
{
	unsigned char  m_cType;
	unsigned char  m_cLength;
	unsigned short m_uHandle;
	unsigned char  m_cManufacturer;
	unsigned char  m_cProduct;
	unsigned char  m_cVersion;
	unsigned char  m_cSerialNumber;
	unsigned char  m_cAssetTag;
	unsigned char  m_cFeatureTag;
	unsigned char  m_cLocationInChassis;
	unsigned short m_uChassisHandle;
	unsigned char  m_cBoardType;
	unsigned char  m_cNumOfContainObjectHandles;
	unsigned short m_uNContainObjectHandles[1];

	string m_strManufacturer;
	string m_strProduct;
	string m_strVersion;
	string m_strSerialNumber;
	string m_strAssetTag;
	string m_strLocationInChassis;
	_BaseBoardInformation()
	{
		m_cType = 2;
	}
	void ParseStringRegion( string & str, const unsigned char * pBegin, const unsigned int nLen, const unsigned char cIndex )
	{
		unsigned char nIndex  = 0;
		int nCurPos = 0;
		int nBegin  = 0;
		while( nCurPos < nLen )
		{
			if( pBegin[nCurPos] == '\0' )
			{
				nIndex++;
				if( nIndex == cIndex )
				{
					str = string( (char*)(&pBegin[nBegin]), nCurPos-nBegin );
					return;
				}
				else
				{
					nBegin = nCurPos + 1;
				}
			}
			nCurPos++;
		}
	}
	void ParseInfo( unsigned char * pInStructureBuffer, int nInStructureBufferlen )
	{
		m_cType      = pInStructureBuffer[0];
		m_cLength    = pInStructureBuffer[1];
		m_uHandle    = pInStructureBuffer[3];
		m_uHandle    = (m_uHandle<<8);
		m_uHandle   += pInStructureBuffer[2];

		m_cManufacturer  = pInStructureBuffer[4];
		m_cProduct       = pInStructureBuffer[5];
		m_cVersion       = pInStructureBuffer[6];
		m_cSerialNumber  = pInStructureBuffer[7];
		m_cAssetTag      = pInStructureBuffer[8];
		m_cFeatureTag    = pInStructureBuffer[9];
		m_cLocationInChassis = pInStructureBuffer[10];
		
		m_uChassisHandle = pInStructureBuffer[12];
		m_uChassisHandle = (m_uChassisHandle<<8);
		m_uChassisHandle+= pInStructureBuffer[11];

		m_cBoardType = pInStructureBuffer[13];
		m_cNumOfContainObjectHandles = pInStructureBuffer[14];
		
		ParseStringRegion( m_strManufacturer, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cManufacturer );
		ParseStringRegion( m_strProduct, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cProduct );
		ParseStringRegion( m_strVersion, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cVersion );
		ParseStringRegion( m_strSerialNumber, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cSerialNumber );
		ParseStringRegion( m_strAssetTag, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cAssetTag );
		ParseStringRegion( m_strLocationInChassis, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cLocationInChassis );

	}
}SBaseBoardInformation, *PBaseBoardInformation;

typedef struct _MemoryDeviceInformation 
{
	unsigned char  m_cType;
	unsigned char  m_cLength;
	unsigned short m_uHandle;
	unsigned short m_uPhysicalMemoryArrayHandle;
	unsigned short m_uMemoryErrorInformationHandle;
	unsigned short m_uTotalWidth;
	unsigned short m_uDataWidth;
	unsigned short m_uSize;
	unsigned char  m_cFormFactor;
	unsigned char  m_cDeviceSet;
	unsigned char  m_cDeviceLocator;
	unsigned char  m_cBankLocator;
	unsigned char  m_cMemoryType;
	unsigned short m_uTypeDetail;
	unsigned short m_uSpeed;
	unsigned char  m_cManufacturer;//string number
	unsigned char  m_cSerialNumber;//string number
	unsigned char  m_cAssetTag;//string number
	unsigned char  m_cPartNumber;//string number
	unsigned char  m_cAttributes;

	string m_strManufacturer;
	string m_strSerialNumber;
	string m_strAssetTag;
	string m_strPartNumber;

	_MemoryDeviceInformation()
	{
		m_cType = 17;
	}
	void ParseStringRegion( string & str, const unsigned char * pBegin, const unsigned int nLen, const unsigned char cIndex )
	{
		unsigned char nIndex  = 0;
		int nCurPos = 0;
		int nBegin  = 0;
		while( nCurPos < nLen )
		{
			if( pBegin[nCurPos] == '\0' )
			{
				nIndex++;
				if( nIndex == cIndex )
				{
					str = string( (char*)(&pBegin[nBegin]), nCurPos-nBegin );
					return;
				}
				else
				{
					nBegin = nCurPos + 1;
				}
			}
			nCurPos++;
		}
	}
	void ParseInfo( unsigned char * pInStructureBuffer, int nInStructureBufferlen )
	{
		if( nInStructureBufferlen <= 26 )
			return;

		m_cType      = pInStructureBuffer[0];
		m_cLength    = pInStructureBuffer[1];

		m_uHandle    = pInStructureBuffer[3];
		m_uHandle    = (m_uHandle<<8);
		m_uHandle   += pInStructureBuffer[2];

		m_uPhysicalMemoryArrayHandle = pInStructureBuffer[5];
		m_uPhysicalMemoryArrayHandle = (m_uPhysicalMemoryArrayHandle<<8);
		m_uPhysicalMemoryArrayHandle+= pInStructureBuffer[4];

		m_uMemoryErrorInformationHandle = pInStructureBuffer[7];
		m_uMemoryErrorInformationHandle = (m_uMemoryErrorInformationHandle<<8);
		m_uMemoryErrorInformationHandle+= pInStructureBuffer[6];

		m_uTotalWidth  = pInStructureBuffer[9];
		m_uTotalWidth  = (m_uTotalWidth<<8);
		m_uTotalWidth += pInStructureBuffer[8];

		m_uDataWidth   = pInStructureBuffer[11];
		m_uDataWidth   = (m_uDataWidth<<8);
		m_uDataWidth  += pInStructureBuffer[10];

		m_uSize        = pInStructureBuffer[13];
		m_uSize        = (m_uSize<<8);
		m_uSize       += pInStructureBuffer[12];

		m_cFormFactor  = pInStructureBuffer[14];
		m_cDeviceSet   = pInStructureBuffer[15];
		m_cDeviceLocator = pInStructureBuffer[16];
		m_cBankLocator = pInStructureBuffer[17];
		m_cMemoryType  = pInStructureBuffer[18];

		m_uTypeDetail  = pInStructureBuffer[20];
		m_uTypeDetail  = (m_uTypeDetail<<8);
		m_uTypeDetail += pInStructureBuffer[19];

		m_uSpeed       = pInStructureBuffer[22];
		m_uSpeed       = (m_uSpeed<<8);
		m_uSpeed      += pInStructureBuffer[21];

		m_cManufacturer = pInStructureBuffer[23];
		m_cSerialNumber = pInStructureBuffer[24];
		m_cAssetTag     = pInStructureBuffer[25];
		m_cPartNumber   = pInStructureBuffer[26];

		ParseStringRegion( m_strManufacturer, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cManufacturer );
		ParseStringRegion( m_strPartNumber, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cSerialNumber );
		ParseStringRegion( m_strAssetTag, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cAssetTag );
		ParseStringRegion( m_strPartNumber, &pInStructureBuffer[m_cLength], nInStructureBufferlen-m_cLength, m_cPartNumber );
	}
}SMemoryDeviceInformation, *PMemoryDeviceInformation;

typedef struct _SMBiosStructureBuffer
{
	unsigned char m_cType;
	int    m_nStructureBufferLen;
	unsigned char * m_pStructureBuffer;
	_SMBiosStructureBuffer * m_next;
	_SMBiosStructureBuffer()
	{
		m_cType = 0xFF;
		m_nStructureBufferLen = 0;
		m_pStructureBuffer    = NULL;
		m_next                = NULL;
	}
	~_SMBiosStructureBuffer()
	{
		if( m_pStructureBuffer != NULL )
			delete []m_pStructureBuffer;
		if( m_next != NULL )
			delete m_next;
	}
}SSMBiosStructureBuffer, *PSMBiosStructureBuffer;

typedef struct _SMBios_Buffer
{
	unsigned char  nUsed20CallingMethod;
	unsigned char  nSMBIOSMajorVersion;
	unsigned char  nSMBIOSMinorVersion;
	unsigned char  nDmiRevision;
	int            nLength;
}SSMBiosBuffer;

class CSMBiosTable
{
public:
	CSMBiosTable(void);
public:
	~CSMBiosTable(void);
public:
	void AddStructureType( unsigned char cType );
	bool InitSMBiosTable();
	bool CloseSMBiosTable();

	PSMBiosStructureBuffer GetSMBiosStructureBuffer( unsigned char cType );
	void * MakeSMBiosStructureObj( PSMBiosStructureBuffer struc );
	void   ReleaseSMBiosStructureObj( unsigned char cType, void * pObj );
private:
	bool FetchSMBiosDataByCom( unsigned char ** p );
	bool FetchSMBiosDataInXP32( unsigned char ** p );

	void AddSMBiosStructureBuffer( PSMBiosStructureBuffer struc );
	bool IsStructureType( unsigned char cType );
	PSMBiosStructureBuffer GetStructureBuffer( unsigned char cType );
	void ParseSMBiosStructure( unsigned char * pInBuf, int nInBufLen );
	int  FindStringRegionEnd( const unsigned char * pBegin, const unsigned int nLen );

	void ClearSMBiosStructureBuffer();
private:
	SSMBiosBuffer m_smbiosbuffer;
	vector<PSMBiosStructureBuffer> m_vStructureBuffers;
};
