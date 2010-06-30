#include "stdafx.h"
#include "main.h"
#include "utf8.h"

#include "..\..\src\svplib\svplib.h"
#include "..\..\src\svplib\SVPToolBox.h"
using namespace util::utf8;
using namespace pfc;

#define TRACK SVP_LogMsg5
#define TRACK2 SVP_LogMsg5
#define OUTPUT_STR_FUNC SVP_LogMsg6
#define OUTPUT_STR_FUNC_FORMAT  SVP_LogMsg6

window_config::window_config()
:  cfg_timeout(14000)
 , cfg_proxy_setting(INTERNET_OPEN_TYPE_DIRECT)
, cfg_check_assume_charset(false)
 , cfg_assume_charset(CP_ACP)
 , cfg_assume_charset_sel(0)

{
   
}


char format_hex_char(unsigned int p_val)
{
    //PFC_ASSERT(p_val < 16);
    return (p_val < 10) ? p_val + '0' : p_val - 10 + 'A';
}
static char m_buffer[17];
char* pfc_format_hex(t_uint64 p_val,unsigned int p_width)
{
    if (p_width > 16) p_width = 16;
    else if (p_width == 0) p_width = 1;
    char temp[16];
    unsigned int n;
    for(n=0;n<16;n++)
    {
        temp[15-n] = format_hex_char((unsigned)(p_val & 0xF));
        p_val >>= 4;
    }

    for(n=0;n<16 && temp[n] == '0';n++) {}

    if (n > 16 - p_width) n = 16 - p_width;

    char * out = m_buffer;
    for(;n<16;n++)
        *(out++) = temp[n];
    *out = 0;
    return m_buffer;
}


bool ReadInternetTextFromUrl(tstring& output, tstring userAgent, tstring url, DWORD timeout, window_config *wcfg)
{
    HINTERNET hOpen = NULL;
    HINTERNET hConnect = NULL;

    do
    {
        hOpen = InternetOpen(userAgent.c_str(), wcfg->cfg_proxy_setting, (INTERNET_OPEN_TYPE_PROXY == wcfg->cfg_proxy_setting) ? wcfg->cfg_proxy_address.c_str() : NULL, NULL, 0);

        if (!hOpen)
        {
            TRACK2(L"InternetOpen failed.");
            OUTPUT_STR_FUNC("InternetOpen failed.");
            break;
        }

        DWORD dwTimeout = timeout;
        InternetSetOption(hOpen, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(DWORD));
        InternetSetOption(hOpen, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT, &timeout, sizeof(DWORD));
        InternetSetOption(hOpen, INTERNET_OPTION_CONTROL_SEND_TIMEOUT, &timeout, sizeof(DWORD));

        if (wcfg->cfg_proxy_setting == INTERNET_OPEN_TYPE_PROXY)
        {
            tstring username = wcfg->cfg_proxy_username;
            tstring password = wcfg->cfg_proxy_password;

            InternetSetOption(hConnect, INTERNET_OPTION_PROXY_USERNAME, (LPVOID) username.c_str(), username.length());
            InternetSetOption(hConnect, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID) password.c_str(), password.length());
        }


        tchar szHead[] = _T("Accept: */*\r\n\r\n");
        hConnect = InternetOpenUrl(hOpen, url.c_str(), szHead, _countof(szHead) - 1, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);

        if (!hConnect)
        {
            TRACK2(L"InternetOpenUrl failed.");
            OUTPUT_STR_FUNC("InternetOpenUrl failed.");
            break;
        }

        tchar szTemp[ 128 ];
        DWORD szTempSize = _countof(szTemp);

        if (!HttpQueryInfo(hConnect, HTTP_QUERY_STATUS_TEXT, szTemp, &szTempSize, NULL))
        {
            TRACK(L"HttpQueryInfo(STATUS_TEXT) failed.");
            OUTPUT_STR_FUNC("HttpQueryInfo(STATUS_TEXT) failed.");
            break;
        }

        if (_tcsicmp(szTemp, _T("OK")) != 0)
        {
            TRACK(L"HttpQueryInfo(STATUS_TEXT) failed: not OK.%s" , szTemp);
            //OUTPUT_STR_FUNC("HttpQueryInfo(STATUS_TEXT) failed: not OK.");
            break;
        }

        szTemp[ 0 ] = 0;
        DWORD dwSize = 0;
        DWORD dwDwSize = sizeof(DWORD);

        if (!HttpQueryInfoA(hConnect, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH, &dwSize, &dwDwSize, NULL))
        {
            //TRACK(L"HttpQueryInfo(LENGTH) failed.");
            //OUTPUT_STR_FUNC("HttpQueryInfo(LENGTH) failed.");
            dwSize = 100 * 1000; // 100kb
            //break;
        }


        // dont't accept data > 50MB
        if (dwSize > (50 * 1000000))
        {
            break;
        }

        char *buffer = NULL;

        try
        {
            buffer = new char[ dwSize + 2 ];
        }
        catch (...)
        {
            break;
        }

        DWORD TotalBytesRead = 0;
        DWORD NumberOfBytesRead = 0;

        while (TotalBytesRead < dwSize)
        {
            if (!InternetReadFile(hConnect, &buffer[ TotalBytesRead ], dwSize - TotalBytesRead, &NumberOfBytesRead))
            {
                TRACK2(L"InternetReadFile failed.");
                OUTPUT_STR_FUNC("InternetReadFile failed.");
                delete[] buffer;
                goto endreadinternettextfromurl;
            }

            TotalBytesRead += NumberOfBytesRead;

            if (NumberOfBytesRead == 0)
                break;
        }

        buffer[ dwSize ] = 0;
        buffer[ dwSize + 1 ] = 0;

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hOpen);

        if (is_utf16le_sign((BYTE*) buffer, TotalBytesRead))     // little endian unicode
        {
            assign_os_from_utf16(output, (wchar *) (buffer + UTF16_SIGN_SIZE));
        }
        else if (is_utf16be_sign((BYTE*) buffer, TotalBytesRead))     // big endian unicode
        {
            // Convert to little endian
            wchar * src = (wchar *) (buffer + UTF16_SIGN_SIZE);

            for (int len = (TotalBytesRead - UTF16_SIGN_SIZE) / 2; len > 0; --len)
            {
                src[ len ] = pfc::byteswap_t(src[ len ]);
            }

            assign_os_from_utf16(output, (wchar *) (buffer + UTF16_SIGN_SIZE));
        }
        else if (is_utf8_sign((BYTE*) buffer, TotalBytesRead))     // utf8
        {
            assign_os_from_utf8(output, (char *) (buffer + UTF8_SIGN_SIZE));
        }
        else if (is_text_utf8((BYTE*) buffer, TotalBytesRead))     // utf8 no bom
        {
            assign_os_from_utf8(output, buffer);
        }
        else // ansi
        {
            assign_os_from_ansi(output, buffer, -1, wcfg->cfg_check_assume_charset ? wcfg->cfg_assume_charset : 0);
        }

        delete[] buffer;

        return true;

    }
    while (false);

endreadinternettextfromurl:
    if (hConnect != NULL)
        InternetCloseHandle(hConnect);

    if (hOpen != NULL)
        InternetCloseHandle(hOpen);

    return false;
}


bool ReadInternetText(tstring& output, tstring userAgent, tstring host, INTERNET_PORT port, tstring request, tstring postdata, bool isPostRequest, DWORD timeout, window_config *wcfg)
{
    HINTERNET hOpen = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;

    do
    {
        hOpen = InternetOpen(userAgent.c_str(), wcfg->cfg_proxy_setting, (INTERNET_OPEN_TYPE_PROXY == wcfg->cfg_proxy_setting) ? wcfg->cfg_proxy_address.c_str() : NULL, NULL, 0);

        if (!hOpen)
        {
            TRACK2(L"InternetOpen failed.");
            OUTPUT_STR_FUNC("InternetOpen failed.");
            break;
        }

        DWORD dwTimeout = timeout;
        InternetSetOption(hOpen, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(DWORD));
        InternetSetOption(hOpen, INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT, &timeout, sizeof(DWORD));
        InternetSetOption(hOpen, INTERNET_OPTION_CONTROL_SEND_TIMEOUT, &timeout, sizeof(DWORD));

        hConnect = InternetConnect(hOpen, host.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

        if (!hConnect)
        {
            TRACK2(L"InternetConnect failed.");
            OUTPUT_STR_FUNC("InternetConnect failed.");
            break;
        }

        LPTSTR AcceptTypes[ 2 ] = { _T("*/*"), NULL};

        if (isPostRequest)
            hRequest = HttpOpenRequest(hConnect, _T("POST"), request.c_str(), _T("HTTP/1.1"), NULL, (LPCTSTR *) AcceptTypes, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD, 0);
        else
            hRequest = HttpOpenRequest(hConnect, _T("GET"), request.c_str(), _T("HTTP/1.1"), NULL, (LPCTSTR *) AcceptTypes, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD, 0);

        if (!hRequest)
        {
            TRACK2(L"HttpOpenRequest failed.");
            OUTPUT_STR_FUNC("HttpOpenRequest failed.");
            break;
        }

        if (wcfg->cfg_proxy_setting == INTERNET_OPEN_TYPE_PROXY)
        {
            tstring username = wcfg->cfg_proxy_username;
            tstring password = wcfg->cfg_proxy_password;

            InternetSetOption(hRequest, INTERNET_OPTION_PROXY_USERNAME, (LPVOID) username.c_str(), username.length());
            InternetSetOption(hRequest, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID) password.c_str(), password.length());
        }


        CSVPToolBox svpTool;
        int szLen = 0;
        char* szPost8 = svpTool.CStringToUTF8( CString(postdata.c_str()), &szLen);
        
        if (!HttpSendRequest(hRequest, NULL, 0, (void*)szPost8 , strlen(szPost8)))
        {
            LPVOID lpMsgBuf;
            
            DWORD dw = GetLastError(); 

            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL );

            // Display the error message and exit the process

            SVP_LogMsg5(L"%s failed with error %d: %s", __FUNCTION__ , dw, lpMsgBuf); 
            
            LocalFree(lpMsgBuf);

            TRACK2(L"HttpSendRequest failed.");
            OUTPUT_STR_FUNC_FORMAT("HttpSendRequest failed. %s", t2a(request.c_str()));
            if(szLen > 0)
                free(szPost8);
            break;
        }
        if(szLen > 0)
            free(szPost8);

        tchar szTemp[ 128 ];
        DWORD szTempSize = _countof(szTemp);

        if (!HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_TEXT, szTemp, &szTempSize, NULL))
        {
            TRACK(L"HttpQueryInfo(STATUS_TEXT) failed.");
            OUTPUT_STR_FUNC("HttpQueryInfo(STATUS_TEXT) failed.");
            break;
        }

        if (_tcsicmp(szTemp, _T("OK")) != 0)
        {
            //TRACK(L"HttpQueryInfo(STATUS_TEXT) failed: not OK2 %s %s %s %s", szTemp, host.c_str(), request.c_str(), postdata.c_str());
            //Need client key for this
            OUTPUT_STR_FUNC("HttpQueryInfo(STATUS_TEXT) failed: not OK.");
            break;
        }

        szTemp[ 0 ] = 0;
        DWORD dwSize = 0;
        DWORD dwDwSize = sizeof(DWORD);

        if (!HttpQueryInfoA(hRequest, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH, &dwSize, &dwDwSize, NULL))
        {
            //TRACK(L"HttpQueryInfo(LENGTH) failed.");
            //OUTPUT_STR_FUNC("HttpQueryInfo(LENGTH) failed.");
            dwSize = 100 * 1000; // 100kb
            //break;
        }

        // dont't accept data > 50MB
        if (dwSize > (50 * 1000000))
        {
            break;
        }

        char *buffer = NULL;

        try
        {
            buffer = new char[ dwSize + 2 ];
        }
        catch (...)
        {
            break;
        }

        DWORD TotalBytesRead = 0;
        DWORD NumberOfBytesRead = 0;

        while (TotalBytesRead < dwSize)
        {
            if (!InternetReadFile(hRequest, &buffer[ TotalBytesRead ], dwSize - TotalBytesRead, &NumberOfBytesRead))
            {
                TRACK2(L"InternetReadFile failed.");
                OUTPUT_STR_FUNC_FORMAT("InternetReadFile failed. %s", t2a(request.c_str()));
                delete[] buffer;
                goto end_readinternettext;
            }

            TotalBytesRead += NumberOfBytesRead;

            if (NumberOfBytesRead == 0)
                break;
        }

        buffer[ dwSize ] = 0;
        buffer[ dwSize + 1 ] = 0;

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hOpen);

        if (is_utf16le_sign((BYTE*) buffer, TotalBytesRead))     // little endian unicode
        {
            assign_os_from_utf16(output, (wchar *) (buffer + UTF16_SIGN_SIZE));
        }
        else if (is_utf16be_sign((BYTE*) buffer, TotalBytesRead))     // big endian unicode
        {
            // Convert to little endian
            wchar * src = (wchar *) (buffer + UTF16_SIGN_SIZE);

            for (int len = (TotalBytesRead - UTF16_SIGN_SIZE) / 2; len > 0; --len)
            {
                src[ len ] = pfc::byteswap_t(src[ len ]);
            }

            assign_os_from_utf16(output, (wchar *) (buffer + UTF16_SIGN_SIZE));
        }
        else if (is_utf8_sign((BYTE*) buffer, TotalBytesRead))     // utf8
        {
            assign_os_from_utf8(output, (char *) (buffer + UTF8_SIGN_SIZE));
        }
        else if (is_text_utf8((BYTE*) buffer, TotalBytesRead))     // utf8 no bom
        {
            assign_os_from_utf8(output, buffer);
        }
        else // ansi
        {
            assign_os_from_ansi(output, buffer, -1, wcfg->cfg_check_assume_charset ? wcfg->cfg_assume_charset : 0);
        }

        delete[] buffer;

        return true;

    }
    while (false);

end_readinternettext:
    if (hRequest != NULL)
        InternetCloseHandle(hRequest);

    if (hConnect != NULL)
        InternetCloseHandle(hConnect);

    if (hOpen != NULL)
        InternetCloseHandle(hOpen);

    return false;
}

// crashes
tstring ReplaceString(const tstring &stringSearchString, const tstring &stringReplaceString, tstring stringStringToReplace)
{
    tstring::size_type pos = stringStringToReplace.find(stringSearchString, 0);
    int intLengthSearch = stringSearchString.length();
    int intLengthReplacment = stringReplaceString.length();

    while (tstring::npos != pos)
    {
        stringStringToReplace.replace(pos, intLengthSearch, stringReplaceString);
        pos = stringStringToReplace.find(stringSearchString, pos + intLengthReplacment);
    }

    return stringStringToReplace;
}