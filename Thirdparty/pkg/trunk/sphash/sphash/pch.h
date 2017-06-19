#ifndef PCH_H
#define PCH_H

#if defined(WIN32)

#ifndef WINVER                  // Specifies that the minimum required platform is Windows Vista.
#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410   // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <io.h>
#endif // defined(WIN32)

#include <vector>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <sstream>
#include <iomanip>
#include <iostream>

#ifdef _MAC_

#define _DARWIN_FEATURE_64_BIT_INODE

#define UCHAR unsigned char
#define __int64 __int64_t
#define MAX_PATH 1024

#include <CoreFoundation/CoreFoundation.h>

class Utf8
{
public:
  Utf8(const wchar_t* wsz): m_utf8(NULL), m_wstring(NULL)
  {
    // OS X uses 32-bit wchar
    const int bytes = wcslen(wsz) * sizeof(wchar_t);
    m_wstring = new wchar_t[bytes+1];
    wcslcpy(m_wstring, wsz, bytes+1);
    // kCFStringEncodingUTF32BE for PowerPC
    CFStringEncoding encoding = kCFStringEncodingUTF32LE;
    
    CFStringRef str = CFStringCreateWithBytesNoCopy(NULL, 
                                                    (const UInt8*)wsz, bytes, 
                                                    encoding, false, 
                                                    kCFAllocatorNull
                                                    );
    
    const int bytesUtf8 = CFStringGetMaximumSizeOfFileSystemRepresentation(str);
    m_utf8 = new char[bytesUtf8];
    CFStringGetFileSystemRepresentation(str, m_utf8, bytesUtf8);
    CFRelease(str);
  }   
  Utf8(const char* sz): m_utf8(NULL), m_wstring(NULL)
  {
    // OS X uses 32-bit wchar
    const int bytes = strlen(sz) * sizeof(char);
    // kCFStringEncodingUTF32BE for PowerPC
    CFStringEncoding encoding = kCFStringEncodingUTF32LE;
    
    CFStringRef cfstring = CFStringCreateWithBytesNoCopy(NULL, 
                                                         (const UInt8*)sz, bytes, 
                                                         kCFStringEncodingUTF8, false, 
                                                         kCFAllocatorNull
                                                         );
    
    const int bytesUtf8 = CFStringGetMaximumSizeOfFileSystemRepresentation(cfstring);
    m_utf8 = new char[bytesUtf8];
    strlcpy(m_utf8, sz, bytesUtf8);
    
    CFIndex length = CFStringGetLength(cfstring);
    if (length == 0)
      return;
    
    CFRange whole_string = CFRangeMake(0, length);
    CFIndex out_size;
    CFIndex converted = CFStringGetBytes(cfstring,
                                         whole_string,
                                         encoding,
                                         0,      // lossByte
                                         false,  // isExternalRepresentation
                                         NULL,   // buffer
                                         0,      // maxBufLen
                                         &out_size);
    if (converted == 0 || out_size == 0)
      return;
    
    // out_size is the number of UInt8-sized units needed in the destination.
    // A buffer allocated as UInt8 units might not be properly aligned to
    // contain elements of StringType::value_type.  Use a container for the
    // proper value_type, and convert out_size by figuring the number of
    // value_type elements per UInt8.  Leave room for a NUL terminator.
    const int elements = out_size * sizeof(UInt8) / sizeof(wchar_t) + 1;
    
    m_wstring = new wchar_t[elements];
    converted = CFStringGetBytes(cfstring,
                                 whole_string,
                                 encoding,
                                 0,      // lossByte
                                 false,  // isExternalRepresentation
                                 reinterpret_cast<UInt8*>(&m_wstring[0]),
                                 out_size,
                                 NULL);  // usedBufLen
    if (converted == 0)
      return;
    
    m_wstring[elements - 1] = '\0';
    
  }   
  
  ~Utf8() 
  { 
    if (m_utf8)
      delete[] m_utf8;
    if (m_wstring)
      delete[] m_wstring;
  }
  
public:
  operator const char*() const { return m_utf8?m_utf8:"";}
  operator const wchar_t*() const { return m_wstring?m_wstring:L""; }
private:
  char* m_utf8;
  wchar_t* m_wstring;
};

#endif // _MAC_

#endif // PCH_H