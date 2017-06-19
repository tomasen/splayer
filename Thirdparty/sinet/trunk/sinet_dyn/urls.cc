#include "pch.h"
#include "sinet_capi.h"
#include <curl/curl.h>
#include "../sinet/strings.h"

using namespace sinet;

SINET_DYN_API int sinet_urlencode(const wchar_t* str_in, wchar_t* str_out, int* length_inout)
{
  int result = 0;
  std::string url_utf8 = strings::wstring_utf8string(str_in);
  CURL *handle = curl_easy_init();
  char *encodedURL = curl_easy_escape(handle, url_utf8.c_str(), url_utf8.length());
  int real_len = strlen(encodedURL);
  if (encodedURL && (*length_inout >= real_len))
  {
    std::wstring encodedURLw = strings::utf8string_wstring(encodedURL);
    memcpy(str_out, encodedURLw.c_str(), real_len * sizeof(wchar_t));
    str_out[real_len] = 0;
    *length_inout = real_len;
    curl_free(encodedURL);
    result = 1;
  }
  curl_easy_cleanup(handle);
  return result;
}

SINET_DYN_API int sinet_urldecode(const wchar_t* str_in, wchar_t* str_out, int* length_inout)
{
  int result = 0;
  CURL *handle = curl_easy_init();
  int real_len;
  std::string encodedURL_utf8 = strings::wstring_utf8string(str_in);
  char *decodedURL = curl_easy_unescape(handle, encodedURL_utf8.c_str(), encodedURL_utf8.length(), &real_len);
  if (decodedURL && (*length_inout >= real_len))
  {
    std::wstring decodedURLw = strings::utf8string_wstring(decodedURL);
    memcpy(str_out, decodedURLw.c_str(), real_len * sizeof(wchar_t));
    str_out[real_len] = 0;
    *length_inout = real_len;
    curl_free(decodedURL);
    result = 1;
  }
  curl_easy_cleanup(handle);
  return result;
}