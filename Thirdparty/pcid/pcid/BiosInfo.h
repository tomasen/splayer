#pragma once
//#include "include.h"
#include "SMBiosTable.h"

/*
string m_strManufacturer;
string m_strProductName;
string m_strVersion;
string m_strSerialNumber;
string m_strSKUNumber;
string m_strFamily;
*/
class CBiosInfo
{
public:
	CBiosInfo(void);
public:
	~CBiosInfo(void);
public:
	bool InitBiosInfo();
	void CloseBiosInfo();
	string GetSystemManufacturer();
	string GetSystemProductName();
	string GetSystemVersion();
	string GetSystemSerialNumber();
	string GetSystemSKUNumber();
	string GetSystemFamily();

	string GetSystemECType();
	string GetSystemECManufacturer();
	string GetSystemECVersion();
	string GetSystemECSerialNumber();
	string GetSystemECAssetTagNmuber();
private:
	string GetECType( unsigned char cType );
private:
	string m_strSystemManufacturer;
	string m_strSystemProductName;
	string m_strSystemVersion;
	string m_strSystemSerialNumber;
	string m_strSystemSKUNumber;
	string m_strSystemFamily;

	string m_strSystemEnclosureChassisType;
	string m_strSystemEnclosureChassisManufacturer;
	string m_strSystemEnclosureChassisVersion;
	string m_strSystemEnclosureChassisSerialNumber;
	string m_strSystemEnclosureChassisAssetTagNumber;

};
