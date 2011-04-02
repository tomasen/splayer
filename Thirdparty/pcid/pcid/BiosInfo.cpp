#include "StdAfx.h"
#include "BiosInfo.h"

CBiosInfo::CBiosInfo(void)
{
}

CBiosInfo::~CBiosInfo(void)
{
}
bool CBiosInfo::InitBiosInfo()
{
	CSMBiosTable biosTable;
	biosTable.AddStructureType( 1 );
	biosTable.AddStructureType( 3 );
	if( !biosTable.InitSMBiosTable() )
		return false;

	PSMBiosStructureBuffer psmbuf = biosTable.GetSMBiosStructureBuffer( 1 );
	if( psmbuf != NULL )
	{
		PSystemInformation systeminfor = (PSystemInformation)biosTable.MakeSMBiosStructureObj( psmbuf );
		if( systeminfor != NULL )
		{
			m_strSystemManufacturer = systeminfor->m_strManufacturer;
			m_strSystemProductName  = systeminfor->m_strProductName;
			m_strSystemSerialNumber = systeminfor->m_strSerialNumber;
			m_strSystemSKUNumber    = systeminfor->m_strSKUNumber;
			m_strSystemVersion      = systeminfor->m_strVersion;
			systeminfor->PrintOut();
			biosTable.ReleaseSMBiosStructureObj( 1, systeminfor );
		}
	}
	//BASE_LOG_OUT(( P2SP_LOG, "GetSMBiosStructureBuffer\n"));
	psmbuf = biosTable.GetSMBiosStructureBuffer( 3 );
	if( psmbuf != NULL )
	{
		PSystemEnclosureStructure systemenclosure = (PSystemEnclosureStructure)biosTable.MakeSMBiosStructureObj( psmbuf );
		if( systemenclosure != NULL )
		{
			m_strSystemEnclosureChassisType = GetECType( systemenclosure->m_cEnclosureType );
			m_strSystemEnclosureChassisManufacturer = systemenclosure->m_strManufacturer;
			m_strSystemEnclosureChassisSerialNumber = systemenclosure->m_strSerialNumber;
			m_strSystemEnclosureChassisVersion      = systemenclosure->m_strVersion;
			m_strSystemEnclosureChassisAssetTagNumber = systemenclosure->m_strAssetTagNumber;
			systemenclosure->PrintOut();
			biosTable.ReleaseSMBiosStructureObj( 3, systemenclosure );
		}
	}
	//BASE_LOG_OUT(( P2SP_LOG, "CloseSMBiosTable\n"));
	biosTable.CloseSMBiosTable();
	return true;
}
void CBiosInfo::CloseBiosInfo()
{

}
string CBiosInfo::GetSystemManufacturer()
{
	return m_strSystemManufacturer.c_str();
}
string CBiosInfo::GetSystemProductName()
{
	return m_strSystemProductName.c_str();
}
string CBiosInfo::GetSystemVersion()
{
	return m_strSystemVersion.c_str();
}
string CBiosInfo::GetSystemSerialNumber()
{
	return m_strSystemSerialNumber.c_str();
}
string CBiosInfo::GetSystemSKUNumber()
{
	return m_strSystemSKUNumber.c_str();
}
string CBiosInfo::GetSystemFamily()
{
	return m_strSystemFamily.c_str();
}

string CBiosInfo::GetSystemECType()
{
	return m_strSystemEnclosureChassisType.c_str();
}
string CBiosInfo::GetSystemECManufacturer()
{
	return m_strSystemEnclosureChassisManufacturer.c_str();
}
string CBiosInfo::GetSystemECVersion()
{
	return m_strSystemEnclosureChassisVersion.c_str();
}
string CBiosInfo::GetSystemECSerialNumber()
{
	return m_strSystemEnclosureChassisSerialNumber.c_str();
}
string CBiosInfo::GetSystemECAssetTagNmuber()
{
	return m_strSystemEnclosureChassisAssetTagNumber.c_str();
}
string CBiosInfo::GetECType( unsigned char cType )
{
	switch( cType )
	{
	case 0x0A:
	case 0x0E:
		return string("NoteBook");
	default:
		return string("Desktop");
	}
}