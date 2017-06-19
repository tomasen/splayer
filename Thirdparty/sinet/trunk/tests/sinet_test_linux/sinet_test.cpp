// 2011.3.2 Soleo Shao：
// In linux, it seems that using wprintf and printf is impossible. So I recommend you to use wprintf all the time.
#include "pch.h"
#include "../../sinet/sinet.h"
#include "../../sinet/pool_impl.h"
#include <time.h>
#include <fstream>

#include <string>
#include <cwchar>
#include <clocale>
#include <cstdlib>
#include <cstdio>
#include <iostream>
  
using namespace sinet;

#if defined(_WINDOWS_)
#include <Shlwapi.h>
#define CLOCKS_PER_SECOND CLOCKS_PER_SEC
#define _SLEEP(secs) ::Sleep(secs*1000);

#elif defined(_MAC_) || defined(__linux__)
#define CLOCKS_PER_SECOND 10000
#define _SLEEP(secs) sleep(secs);

#endif

#define SINET_VALIDATE_INT(test_name, condition, variable) \
  { \
    if (##condition)\
      wprintf (L"test [%S] is [OK]\n", test_name);\
    else\
      wprintf (L"test [%S] [FAILED], condition requires (" #condition ") and its value is: %d\n", test_name, variable);\
    \
  }

#define SINET_VALIDATE_INT64(test_name, condition, variable) \
{ \
  if (##condition)\
  wprintf (L"test [%S] is [OK]\n", test_name);\
    else\
    wprintf (L"test [%S] [FAILED], condition requires (" #condition ") and its value is: %lld\n", test_name, variable);\
    \
}

#if defined(_WINDOWS_)
#define SINET_APPPATH(apppath) \
  wchar_t tmppath[256];\
  GetModuleFileName(NULL, tmppath, 256);\
  PathRemoveFileSpecW(tmppath);\
  apppath = tmppath;
#else
#define SINET_APPPATH(apppath) \
  apppath = L"/tmp" ;
#endif

  /* ################### test macros ################### */
#define TEST_ENTER(testcase) \
{ \
  wprintf(L"\n--------------------------------------------\n");\
  wprintf(#testcase L"\n\n");\
}

#define TEST_THREAD_FEED(pool, task, req, secs, apply) \
{\
  wprintf(L"response feed:\n");\
  while (pool->is_running_or_queued(task))\
  {\
    if (apply(task, req))\
    {\
       pool->cancel(task);\
       break;\
    }\
    _SLEEP(secs);\
  }\
  wprintf(L"\n");\
}

#define TEST_RESULT(testcase, condition, variable) \
{ \
  if (condition)\
  wprintf (L"\ntest [%S] is [PASSED]\n", testcase);\
    else\
    wprintf (L"\ntest [%S] [FAILED], condition requires (" #condition ") and its value is: %d\n", testcase, variable);\
    \
}

std::wstring Utf8StringToWString(const std::string& s)
{
#ifdef WIN32  
  wchar_t* wch;
  UINT bytes = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
  wch  = new wchar_t[bytes];
  if(wch)
    bytes = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wch, bytes);
  std::wstring str = wch;
  delete[] wch;
  return str;
#endif
#ifdef _MAC_
  return std::wstring((const wchar_t*)Utf8(s.c_str()));
#endif
#ifdef __linux__
  std::wstring wcs;
  utf8_wchar(s,wcs);
  //printf("%s",s.c_str());
  //wprintf(L"%S",wcs.c_str());
  return wcs;
#endif
}

void testcase_poolcreation()
{
  TEST_ENTER(L"testcase_poolcreation");

  int num_pools = 1000;
  wprintf(L"start timing for %d pool creation ...\n", num_pools);
  size_t clock_begin = clock();
  std::vector<refptr<pool> > pools(num_pools);
  for (std::vector<refptr<pool> >::iterator it = pools.begin(); it != pools.end(); it++)
    *it = pool::create_instance();
  size_t clock_end = clock();

  size_t num_clocks = clock_end - clock_begin;
  TEST_RESULT(L"testcase_poolcreation", num_clocks < 500, num_clocks);

  //printf("sleeping 20 seconds to observe CPU utilization ...\n");
  //_SLEEP(20);
}

clock_t last_clock;
int testcase_canceldownload_feed(refptr<task> task, refptr<request> req)
{
  if (!req->get_response_size())
    wprintf(L"response: %d \n", req->get_retrieved_size());

  else
  {
  // CLOCKS_PER_SEC is defined as 1000000 on Mac OS X 64 bit
  // but when compiling a 32 bit program, it looks this value 
  // should be something around 10000
  // Windows don't have this problem, the definition is 1000
    size_t kbps = 0;
    if (req->get_response_size() != req->get_retrieved_size() &&
      clock() - last_clock > CLOCKS_PER_SECOND)
    {
      kbps = (req->get_response_size()-req->get_retrieved_size())/1024/((clock()-last_clock)/CLOCKS_PER_SECOND);
      last_clock = clock();
      wprintf(L"response: %d (%d KB/s)\n", req->get_retrieved_size(), kbps);
    }
  }

  if (req->get_retrieved_size() > 3000000)
  {
    wprintf(L"canceling operation ... \n");
    return 1;
  }
  return 0;
}
void testcase_canceldownload(refptr<pool> pool, refptr<task> task, refptr<request> req)
{
  TEST_ENTER(L"testcase_canceldownload");

  req->set_request_method(REQ_GET);
  req->set_request_outmode(REQ_OUTFILE);
  SINET_APPPATH(std::wstring file);
  #ifdef WIN32
  std::wstring filepath = file + L"\\canceldownload.exe";
  #else
  std::wstring filepath = file + L"canceldownload.exe";
  #endif
  req->set_outfile(filepath.c_str());
  task->append_request(req);
  pool->execute(task);
  last_clock = clock();
  TEST_THREAD_FEED(pool, task, req, 1, testcase_canceldownload_feed);
  // must call the request close_outfile function after use REQ_OUTFILE mode
//   req->close_outfile();
  wprintf(L"task complete, size: %d\n", req->get_response_size());

  TEST_RESULT(L"testcase_canceldownload", req->get_response_size() > 0, req->get_response_size());
}
/*
  Test HTTP GET method

  test case
    1°¢the request url use chinese with utf8
    2°¢response body

  validate:
    client-side raw data compare response body
    returns PASSED on success, otherwise FAILED
 */
#define TEST_RESPONSEDATA    L"123456abcdefg\x4E2D\x6587\x5B57\x7B26"
#define TEST_RESPONSELENGTH  wcslen(TEST_RESPONSEDATA)

int testcase_httpget_feed(refptr<task> task, refptr<request> req)
{
  wprintf(L"response: %d \n", req->get_retrieved_size());
  return 0;
}
enum httpget_t
{
  hg_getchar,
  hg_getch
};
void testcase_httpget(refptr<pool> pool, refptr<task> task, refptr<request> req, httpget_t gt = hg_getchar)
{
  TEST_ENTER(L"testcase_httpget");
  req->set_request_method(REQ_GET);
  task->append_request(req);
  pool->execute(task);
  TEST_THREAD_FEED(pool, task, req, 1, testcase_httpget_feed);

  si_buffer buff = req->get_response_buffer();
  buff.push_back(0);
  void* buff2 = malloc(buff.size());
  memcpy(buff2, &buff[0], buff.size());
  std::string buffstr((char*)buff2);
  free(buff2);
 //wprintf(L"encode before: %s\n", buffstr.c_str());
  std::wstring data = Utf8StringToWString(buffstr);
  
  if (gt == hg_getch)
  {
    std::wcout<<data<<"\n";
    wprintf(L"response data: %S, length: %d\n", data.c_str(), data.length());
    TEST_RESULT(L"testcase_httpget", (req->get_response_errcode() == 0), req->get_response_errcode());
    return;
  }

  std::wstring raw = TEST_RESPONSEDATA;
  wprintf(L"raw data: %S, length: %d\n", raw.c_str(), TEST_RESPONSELENGTH);

  wprintf(L"response data: %S, length: %d\n", data.c_str(), data.length());

  int cmpval = wmemcmp(data.c_str(), TEST_RESPONSEDATA, TEST_RESPONSELENGTH);

  TEST_RESULT(L"testcase_httpget", (cmpval == 0), cmpval);
}

/*
  Test HTTP POST method

  test case:
    1. the form value use chinese with utf8
    2. upload file by form
    3. use buffer instead file as upload
    4. above mentioned can mix

  validate:
    client-side post form, the values:"\x4E2D\x6587\x5B57\x7B26"(÷–Œƒ◊÷∑˚),abcdef,123456, or two mix upload
    server-side raw data compare post data and the file size
    returns PASSED on success, otherwise FAILED
 */
enum fupload_t{
  form_null,
  form_all,
  form_buffer,
  form_file
};
#define UPLOADEFILE L"testfile.jpg"
int testcase_httpform_feed(refptr<task> task, refptr<request> req)
{
  wprintf(L"response: %d \n", req->get_retrieved_size());
  return 0;
}
void testcase_httpform(refptr<pool> pool, refptr<task> task, refptr<request> req, fupload_t ft = form_null)
{
  TEST_ENTER(L"testcase_httpform");
  req->set_request_method(REQ_POST);

  refptr<postdata> postdata = postdata::create_instance();
  refptr<postdataelem> elem1 = postdataelem::create_instance();
  elem1->set_name(L"a1");
  elem1->setto_text(L"\x4E2D\x6587\x5B57\x7B26");
  postdata->add_elem(elem1);

  refptr<postdataelem> elem2 = postdataelem::create_instance();
  elem2->set_name(L"a2");
  elem2->setto_text(L"abcdef");
  postdata->add_elem(elem2);

  refptr<postdataelem> elem3 = postdataelem::create_instance();
  elem3->set_name(L"a3");
  elem3->setto_text(L"123456");
  postdata->add_elem(elem3);

  SINET_APPPATH(std::wstring file);
#ifdef __linux__
  std::wstring filepath = file + L"/" + UPLOADEFILE;
#else
  std::wstring filepath = file + L"\\" + UPLOADEFILE;
#endif

  if (ft == form_buffer || ft == form_all)
  {
    std::ifstream fs;
#ifdef _MAC_
    fs.open(Utf8(filepath.c_str()), std::ios::binary|std::ios::in);
#elif WIN32
    fs.open(filepath.c_str(), std::ios::binary|std::ios::in);
#elif __linux__
    fs.open((wchar_utf8(filepath)).c_str(), std::ios::binary|std::ios::in);
#endif
    if (!fs)
    {
      wprintf(L"dose not open file.(%S)\n", filepath.c_str());
      return;
    }
    fs.seekg(0, std::ios::end);
    size_t filesize = fs.tellg();
    void* buffer = malloc(filesize);
    fs.seekg(0, std::ios::beg);
    fs.read((char*)buffer, filesize);
    fs.close();

    refptr<postdataelem> elem4 = postdataelem::create_instance();
    elem4->set_name(L"form_buffer");
    elem4->setto_text(L"testbuffer.jpg");
    elem4->setto_buffer(buffer, filesize);
    postdata->add_elem(elem4);
    free(buffer);
  }
  if (ft == form_file || ft == form_all)
  {
    refptr<postdataelem> elem5 = postdataelem::create_instance();
    elem5->set_name(L"form_file");
    elem5->setto_file(filepath.c_str());
    postdata->add_elem(elem5);
  }

  req->set_postdata(postdata);

  task->append_request(req);
  pool->execute(task);
  TEST_THREAD_FEED(pool, task, req, 1, testcase_httpform_feed);

  si_buffer buff = req->get_response_buffer();
  buff.push_back(0);
  void* buff2 = malloc(buff.size());
  memcpy(buff2, &buff[0], buff.size());
  std::string buffstr((char*)buff2);
  free(buff2);

  std::wstring data = Utf8StringToWString(buffstr);
  wprintf(L"%S\n", data.c_str());

  TEST_RESULT(L"testcase_httpform", (req->get_response_errcode() == 0), req->get_response_errcode());
}

/*
  Test HTTP request header

  test case:
    1. construct http header User-Agent or Accept or Cookie etc.

  validate:
    1. server-side returns response code 200 and empty body
    2. client-side checks the response header and see if it contains the original sent header
    returns PASSED on success, otherwise FAILED
 */
int testcase_httpheader_feed(refptr<task> task, refptr<request> req)
{
  wprintf(L"response: %d \n", req->get_retrieved_size());
  return 0;
}
void testcase_httpheader(refptr<pool> pool, refptr<task> task, refptr<request> req, si_stringmap header)
{
  TEST_ENTER(L"testcase_httpheader");

  req->set_request_method(REQ_GET);
  req->set_request_header(header);
  
  task->append_request(req);

  pool->execute(task);
  TEST_THREAD_FEED(pool, task, req, 1, testcase_httpheader_feed);

  si_stringmap newheader = req->get_response_header();
  bool testok = true;
  for (si_stringmap::iterator it = header.begin(); it != header.end(); it++)
  {
    std::wstring statstr;
    if (newheader.find(it->first) != newheader.end())
      statstr = L"ok";
    else
    {
      statstr = L"fail";
      testok = false;
    }
    wprintf(L"header %S : %S  [%S]\n", it->first.c_str(), it->second.c_str(), statstr.c_str());
  }

  TEST_RESULT(L"testcase_httpheader", (testok == true), testok);
}

int main(int argc, char* argv[])
{
#if defined(__linux__)
  setlocale(LC_ALL,"");
#else
  setlocale(LC_ALL, "chs");
#endif
  
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  task->use_config(cfg);

  // test http get
  refptr<request> req1 = request::create_instance();
  req1->set_request_url(L"http://webpj.com:8080/misc/test_sinet.php?act=get");
  testcase_httpget(pool, task, req1);

  // test http request url of chinese with utf8
  refptr<request> req2 = request::create_instance();
  req2->set_request_url(L"http://webpj.com:8080/misc/test_sinet.php?act=str&p=中文");
  testcase_httpget(pool, task, req2, hg_getch);

  // test http post
  refptr<request> req3 = request::create_instance();
  req3->set_request_url(L"http://webpj.com:8080/misc/test_sinet.php?act=form");
  testcase_httpform(pool, task, req3);

  // test http post + upload buffer
  refptr<request> req4 = request::create_instance();
  req4->set_request_url(L"http://webpj.com:8080/misc/test_sinet.php?act=form");
  testcase_httpform(pool, task, req4, form_buffer);

  // test http post + upload file
  refptr<request> req5 = request::create_instance();
  req5->set_request_url(L"http://webpj.com:8080/misc/test_sinet.php?act=form");
  testcase_httpform(pool, task, req5, form_file);

  // test http post + upload buffer + upload file
  refptr<request> req6 = request::create_instance();
  req6->set_request_url(L"http://webpj.com:8080/misc/test_sinet.php?act=form");
  testcase_httpform(pool, task, req6, form_all);

  // test http header
  refptr<request> req7 = request::create_instance();
  req7->set_request_url(L"http://webpj.com:8080/misc/test_sinet.php?act=header");
  
  si_stringmap header;
  header[L"Cookie"] = L"a1=1234567; a2=cookie; a3=%E8%BF%99%E6%98%AFcookie; a4=%E6%B5%8B%E8%AF%95%E6%B5%8B%E8%AF%95";
  header[L"User-Agent"] = L"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.6) Gecko/20100625 Firefox/3.6.6";
  header[L"Accept"] = L"text/html";
  testcase_httpheader(pool, task, req7, header);

  // test create pool
  testcase_poolcreation();

  // test cancel download
  refptr<request> req0 = request::create_instance();
  //req0->set_request_url(L"http://dl.baofeng.com/storm3/Storm2012-3.10.09.05.exe");
  req0->set_request_url(L"http://dl_dir.qq.com/qqfile/qq/QQ2010/QQ2010sp2_Installer.exe");
  testcase_canceldownload(pool, task, req0);

  return 0;
}
