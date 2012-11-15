// ----------------------------------------------------------------------------

#include <streams.h>
#include <initguid.h>
#include <mmreg.h>
#include "RegistryUtils.h"

// ----------------------------------------------------------------------------

HRESULT GUID2String(TCHAR *DstString, const GUID SrcGuid)
{
	OLECHAR CLSIDOLEString[CHARS_IN_GUID];
	HRESULT hr = StringFromGUID2(SrcGuid, CLSIDOLEString, CHARS_IN_GUID);
	if(FAILED(hr))
		return hr;
	wsprintf(DstString, TEXT("%ls"), CLSIDOLEString);
	return S_OK;
}

// ----------------------------------------------------------------------------

void RegisterSourceFilterExtension(const char* Extension,
								   const GUID SourceFilterGUID,
								   const GUID MediaType,
								   const GUID Subtype)
{
	// Identification by extension
	// HKEY_CLASSES_ROOT\Media Type\Extensions\.ext
	
	HKEY Key;
    DWORD Disp;
	TCHAR RegistryKeyName[256];
	TCHAR CLSIDString[CHARS_IN_GUID];
	
	wsprintf(RegistryKeyName, "Media Type\\Extensions\\%s", Extension);
	if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CLASSES_ROOT,
		RegistryKeyName, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_WRITE,
		NULL, &Key, &Disp))
	{
		GUID2String(CLSIDString, SourceFilterGUID);
		RegSetValueEx(Key, "Source Filter", 0, REG_SZ,
			(CONST BYTE *) CLSIDString, strlen(CLSIDString));
		
		if(!IsEqualGUID(MediaType,CLSID_NULL))
		{
			GUID2String(CLSIDString, MediaType);
			RegSetValueEx(Key, "Media Type", 0, REG_SZ,
				(CONST BYTE *) CLSIDString, strlen(CLSIDString));
		}
		
		if(!IsEqualGUID(MediaType,CLSID_NULL))
		{
			GUID2String(CLSIDString, Subtype);
			RegSetValueEx(Key, "Subtype", 0, REG_SZ,
				(CONST BYTE *) CLSIDString, strlen(CLSIDString));
		}
		RegCloseKey(Key);
	}
}

// ----------------------------------------------------------------------------

void UnRegisterSourceFilterExtension(const char* Extension)
{
	TCHAR RegistryKeyName[256];	
	wsprintf(RegistryKeyName, "Media Type\\Extensions\\%s", Extension);
	RegDeleteKey(HKEY_CLASSES_ROOT, RegistryKeyName);	
}

// ----------------------------------------------------------------------------

void RegisterSourceFilterPattern(const char* Pattern,
								 const GUID SourceFilterGUID,
								 const GUID MajorType,
								 const GUID Subtype)
{
    // Identification by content :
    // HKEY_CLASSES_ROOT\MediaType\{major type}\{subtype}
	
	HKEY Key;
    DWORD Disp;
	TCHAR RegistryKeyName[256];
	TCHAR CLSIDString[CHARS_IN_GUID], CLSIDString2[CHARS_IN_GUID];
	
	GUID2String(CLSIDString, MajorType);
	GUID2String(CLSIDString2, Subtype);
	wsprintf(RegistryKeyName, "Media Type\\%s\\%s", CLSIDString, CLSIDString2);
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CLASSES_ROOT,
        RegistryKeyName, 0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_WRITE,
		NULL, &Key, &Disp))
	{
		GUID2String(CLSIDString,SourceFilterGUID);
		RegSetValueEx(Key, "Source Filter", 0, REG_SZ,
			(CONST BYTE *) CLSIDString, strlen(CLSIDString));
		
		// The pattern use the following format : offset,cb,mask,val
		RegSetValueEx(Key, "0", 0, REG_SZ, (CONST BYTE *) Pattern, strlen(Pattern));
		RegCloseKey(Key);
	}
}

// ----------------------------------------------------------------------------

void UnRegisterSourceFilterPattern(const GUID MajorType,
								   const GUID Subtype)
{
	TCHAR RegistryKeyName[256];
	TCHAR CLSIDString[CHARS_IN_GUID], CLSIDString2[CHARS_IN_GUID];
	
	GUID2String(CLSIDString, MajorType);
	GUID2String(CLSIDString2, Subtype);
	wsprintf(RegistryKeyName, "Media Type\\%s\\%s", CLSIDString, CLSIDString2);
	RegDeleteKey(HKEY_CLASSES_ROOT, RegistryKeyName);
}

// ----------------------------------------------------------------------------

void RegisterWMPExtension(const char* Extension, const char* Description,
						  const char* MUIDescription, const char* PerceivedType)
{
	HKEY Key;
	DWORD Disp;
	const char* ExtensionWithoutStar = Extension+1;

	// WMP 6.4
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\MediaPlayer\\Player\\Extensions\\Types", 0,
		KEY_WRITE|KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE, &Key))
	{
		DWORD Index = 0, CurrentIndex = 0, MaxIndex = 0;
		TCHAR KeyName[256];
		DWORD KeyNameMaxLen = 256;
		TCHAR KeyValue[256];
		DWORD KeyValueMaxLen = 256;
		bool AlreadyRegistered = false;

		// Check if our extension is not already here, and get the new index if it's not
		while(ERROR_SUCCESS == RegEnumValue(Key, Index++, KeyName, &KeyNameMaxLen,
			NULL, NULL, (BYTE*)KeyValue, &KeyValueMaxLen))
		{
			_strlwr(KeyValue);
			if(strstr(KeyValue, Extension))
			{
				AlreadyRegistered = true;
				break;
			}
			CurrentIndex = atoi(KeyName);
			if(CurrentIndex > MaxIndex)
				MaxIndex = CurrentIndex;

			KeyNameMaxLen = 256;
			KeyValueMaxLen = 256;
		}

		if(!AlreadyRegistered)
		{
			wsprintf(KeyName,"%d",MaxIndex+1);
			// Add Extension
			RegSetValueEx(Key, KeyName, 0, REG_SZ, (CONST BYTE*)Extension, strlen(Extension));
			RegCloseKey(Key);

			// Add Description
			if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				"SOFTWARE\\Microsoft\\MediaPlayer\\Player\\Extensions\\Descriptions", 0, KEY_WRITE, &Key))
			{
				RegSetValueEx(Key, KeyName, 0, REG_SZ, (CONST BYTE*)Description, strlen(Description));
				RegCloseKey(Key);				
			}

			// Add MUIDescription
			if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				"SOFTWARE\\Microsoft\\MediaPlayer\\Player\\Extensions\\MUIDescriptions", 0, KEY_WRITE, &Key))
			{
				RegSetValueEx(Key, KeyName, 0, REG_SZ, (CONST BYTE*)MUIDescription, strlen(MUIDescription));
				RegCloseKey(Key);				
			}
		} else {
			RegCloseKey(Key);
		}
	}

	// WMP9
	// From "File Name Extension Registry Settings"
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wmplay10/mmp_sdk/filenameextensionregistrysettings.asp

	TCHAR RegistryKeyName[256];
	DWORD dwRuntimeFlag = 0x7;
	DWORD dwPermissionsFlag = 0xf;
	wsprintf(RegistryKeyName,
		"SOFTWARE\\Microsoft\\Multimedia\\WMPlayer\\Extensions\\%s", ExtensionWithoutStar);
	
	if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, RegistryKeyName,
		0, "REG_SZ", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &Key, &Disp))
	{
		RegSetValueEx(Key, "Runtime", 0, REG_DWORD, (BYTE*)&dwRuntimeFlag,
			sizeof(DWORD));
		RegSetValueEx(Key, "Permissions", 0, REG_DWORD, (BYTE*)&dwPermissionsFlag,
			sizeof(DWORD));
		if(PerceivedType)
		{		
			RegSetValueEx(Key, "PerceivedType", 0, REG_SZ, (BYTE*)PerceivedType,
				strlen(PerceivedType));		
		}
		RegCloseKey(Key);
	}
}

// ----------------------------------------------------------------------------

void UnRegisterWMPExtension(const char* Extension)
{
	HKEY Key;
	TCHAR RegistryKeyName[256];
	const char* ExtensionWithoutStar = Extension+1;

	// WMP 6.4
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Microsoft\\MediaPlayer\\Player\\Extensions\\Types", 0,
		KEY_WRITE|KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE, &Key))
	{
		DWORD Index = 0;
		TCHAR KeyName[256];
		DWORD KeyNameMaxLen = 256;
		TCHAR KeyValue[256];
		DWORD KeyValueMaxLen = 256;
		bool AlreadyRegistered = false;
		
		// Check if our extension is already here
		while(ERROR_SUCCESS == RegEnumValue(Key, Index++, KeyName, &KeyNameMaxLen,
			NULL, NULL, (BYTE*)KeyValue, &KeyValueMaxLen))
		{
			_strlwr(KeyValue);
			if(strstr(KeyValue, Extension))
			{
				AlreadyRegistered = true;
				break;
			}
			KeyNameMaxLen = 256;
			KeyValueMaxLen = 256;
		}
		if(AlreadyRegistered)
		{
			// Remove Extension
			RegDeleteValue(Key, KeyName);
			RegCloseKey(Key);

			// Remove Description
			if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				"SOFTWARE\\Microsoft\\MediaPlayer\\Player\\Extensions\\Descriptions", 0, KEY_WRITE, &Key))
			{
				RegDeleteValue(Key, KeyName);
				RegCloseKey(Key);				
			}
			
			// Remove MUIDescription
			if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				"SOFTWARE\\Microsoft\\MediaPlayer\\Player\\Extensions\\MUIDescriptions", 0, KEY_WRITE, &Key))
			{
				RegDeleteValue(Key, KeyName);
				RegCloseKey(Key);				
			}
		}
	}

	// WMP9
	wsprintf(RegistryKeyName,
		"SOFTWARE\\Microsoft\\Multimedia\\WMPlayer\\Extensions\\%s", ExtensionWithoutStar);
	RegDeleteKey(HKEY_LOCAL_MACHINE, RegistryKeyName);
}
