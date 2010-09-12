// Please do not remove this comment.
//
// URL Encoding/Decodeing class, supports Unicode and ANSI (MFC TCHAR)
//
// Written by Daniel Cohen Gindi, (danielgindi (at) gmail.com)
//
// If you have any comments or questions, feel free to email me.
//
// You may use this class for any purpose, commercial or personal.

#include "stdafx.h"
#include "URLEncode.h"
#include "main.h"
#include "utf8.h"
using namespace util::utf8;

// Put the percent (%) sign first, so it won't overwrite the converted Hex'es
LPCTSTR CURLEncode::m_lpszUnsafeString = _T(" \"<>#{}|\\^~[]`");
int CURLEncode::m_iUnsafeLen = (int) _tcslen(m_lpszUnsafeString);
LPCTSTR CURLEncode::m_lpszReservedString = _T("$&+,./:;=?@-_!*()'");
int CURLEncode::m_iReservedLen = (int) _tcslen(m_lpszReservedString);

// Convert a byte into Hexadecimal CString
tstring CURLEncode::toHex(BYTE val)
{
  tstring csRet = _T("%");
  csRet += u2w(pfc_format_hex(val));
  //csRet.Format(_T("%%%0.2X"), val);
  return csRet;
}

// Convert a Unicode into UTF8
DWORD CURLEncode::toUTF8(TCHAR tc)
{
  WCHAR wc[ 2 ];
  CHAR mb[ 4 ];

  wc[ 0 ] = tc;
  wc[ 1 ] = 0;

  mb[ 0 ] = mb[ 1 ] = mb[ 2 ] = mb[ 3 ] = 0;

  WideCharToMultiByte(CP_UTF8, 0, wc, 1, mb, 4, 0, 0);

  return MAKELONG(MAKEWORD(mb[ 3 ], mb[ 2 ]), MAKEWORD(mb[ 1 ], mb[ 0 ]));
}


// strURL:   URL to encode.
// bEncodeReserved: Encode the reserved characters
//                  for example the ? character, which is used many times
//                  for arguments in URL.
//                  so if we are encoding just a string containing Keywords,
//                  we want to encode the reserved characters.
//                  but if we are encoding a simple URL, we wont.
tstring CURLEncode::Encode(tstring strURL, BOOL bEncodeReserved /*=FALSE*/)
{
  // First encode the % sign, because we are adding lots of it later...
  strURL = ReplaceString(_T("%"), toHex(__toascii(_T('%'))), strURL);

  tstring tmp;

  // Encdoe the reserved characters, if we choose to

  if (bEncodeReserved)
  {
    for (int i = 0; i < m_iReservedLen; i++)
    {
      tmp = m_lpszReservedString[ i ];
      strURL = ReplaceString(tmp, toHex(__toascii(m_lpszReservedString[ i ])), strURL);
    }
  }

  // Encode 'unsafe' characters
  // see: http://www.blooberry.com/indexdot/html/topics/urlencoding.htm
  for (int i = 0; i < m_iUnsafeLen; i++)
  {
    tmp = m_lpszUnsafeString[ i ];
    strURL = ReplaceString(tmp, toHex(__toascii(m_lpszUnsafeString[ i ])), strURL);
  }

  // Encode unprintable characters 0x00-0x1F, and 0x7F
  for (char c = 0x00; c <= 0x1F; c++)
  {
    tmp = c;
    strURL = ReplaceString(tmp, toHex(c), strURL);
  }

  tmp = (char) 0x7F;
  strURL = ReplaceString(tmp, toHex(0x7F), strURL);

  // Now encode all other unsafe characters
  TCHAR tc = 0;
  DWORD dw = 0;

  tstring nc;
  // In this stage we do not want to convert:
  // 1. Characters A-Z, a-z, 0-9, because they are safe.
  // 2. The reserved characteres, we have already dealt with them;
  // 3. The % character...
  tstring strDoNotReplace(m_lpszReservedString);
  strDoNotReplace.append(_T("%"));

  for (tstring::size_type i = 0; i < strURL.length(); i++)
  {
    tc = strURL[ i ];

    if ((tc < _T('a') || tc > _T('z')) &&
         (tc < _T('A') || tc > _T('Z')) &&
         (tc < _T('0') || tc > _T('9')) &&
         strDoNotReplace.find(tc) == tstring::npos)
    {
      dw = toUTF8(tc);

      BYTE hihi = HIBYTE(HIWORD(dw));
      BYTE lohi = LOBYTE(HIWORD(dw));
      BYTE hilo = HIBYTE(LOWORD(dw));
      BYTE lolo = LOBYTE(LOWORD(dw));

      nc = toHex(hihi);

      if (lohi != 0)
        nc.append(toHex(lohi));

      if (hilo != 0)
        nc.append(toHex(hilo));

      if (lolo != 0)
        nc.append(toHex(lolo));

      tmp = tc;

      strURL = ReplaceString(tmp, nc, strURL);

      // We have added 5 extra characters to the length of the string,
      // So we can ignore them.
      i += 5;
    }
  }

  return strURL;
}
