// Please do not remove this comment.
//
// URL Encoding/Decodeing class, supports Unicode and ANSI (MFC TCHAR)
//
// Written by Daniel Cohen Gindi, (danielgindi (at) gmail.com)
//
// If you have any comments or questions, feel free to email me.
//
// You may use this class for any purpose, commercial or personal.

#pragma once

#include "stdafx.h"
#include "main.h"

class CURLEncode
{

  private:
    static LPCTSTR m_lpszUnsafeString;
    static int m_iUnsafeLen;
    static LPCTSTR m_lpszReservedString;
    static int m_iReservedLen;

    static tstring toHex(BYTE val);
    static DWORD toUTF8(TCHAR tc);


  public:
    static tstring Encode(tstring strURL, BOOL bEncodeReserved = FALSE);
};
