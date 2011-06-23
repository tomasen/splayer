#ifndef _REGISTRY_UTILS_H_
#define _REGISTRY_UTILS_H_

/*

Examples :
----------

RegisterSourceFilterPattern("0,4,,54544131", CLSID_AsyncReader,
							MEDIATYPE_Stream, MEDIASUBTYPE_TTA1_Stream);

RegisterSourceFilterExtension(".tta", CLSID_AsyncReader,
							  MEDIATYPE_Stream, MEDIASUBTYPE_TTA1_Stream);

RegisterWMPExtension("*.tta", "True Audio Files (*.tta)",
					 "True Audio Files", "audio");


UnRegisterWMPExtension("*.tta");
UnRegisterSourceFilterExtension(".tta");
UnRegisterSourceFilterPattern(MEDIATYPE_Stream, MEDIASUBTYPE_TTA1_Stream);

*/

HRESULT GUID2String(TCHAR *DstString, const GUID SrcGuid);

void RegisterSourceFilterExtension(const char* Extension,
								   const GUID SourceFilterGUID,
								   const GUID MediaType,
								   const GUID Subtype);

void UnRegisterSourceFilterExtension(const char* Extension);

void RegisterSourceFilterPattern(const char* Pattern,
								 const GUID SourceFilterGUID,
								 const GUID MajorType,
								 const GUID Subtype);

void UnRegisterSourceFilterPattern(const GUID MajorType,
								   const GUID Subtype);								 

void RegisterWMPExtension(const char* Extension,
						  const char* Description,
						  const char* MUIDescription,
						  const char* PerceivedType);

void UnRegisterWMPExtension(const char* Extension);

#endif // _REGISTRY_UTILS_H_
