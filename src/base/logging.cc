
#include "logging.h"
#include "Strings.h"
#include "file_path.h"
#include <Windows.h>
#include <fstream>

std::auto_ptr<LogController> LogController::m_instance;

LogController::LogController(void):
m_PerfFrequency(0)
{
  QueryPerformanceFrequency ((LARGE_INTEGER*)&m_PerfFrequency);
  wchar_t buff[MAX_PATH];
  GetModuleFileName(NULL, buff, MAX_PATH);
  log_path = FilePath::GetInstance()->DirName(buff) + L"SVPDebug.log";
  _wremove(log_path.c_str());
}

LogController::~LogController(void)
{

}

void LogController::Log(const wchar_t* fmt, ...)
{
  wchar_t buf[2048];
  va_list args;
  va_start(args, fmt);
  vswprintf_s(buf,2048,fmt,args);
  LogLine(buf);
  va_end(args);
}

void LogController::Log(const char* fmt, ...)
{
  char buf[2048];
  va_list args;
  va_start(args, fmt);
  vsprintf_s(buf,2048, fmt,args);
  LogLine(buf);
  va_end(args);
}

__int64 LogController::GetPerfCounter()
{
  __int64 i64Ticks100ns;
  QueryPerformanceCounter ((LARGE_INTEGER*)&i64Ticks100ns);
  return __int64((double(i64Ticks100ns) * 10000000) / double(m_PerfFrequency) + 0.5);
}

void LogController::LogLine(const wchar_t* line)
{
  if (wcslen(line) <= 0)
    return;

  __int64 logTick = GetPerfCounter();
  wchar_t buff[MAX_PATH];

  try
  {
    AutoCSLock autocslock(m_cs);
#ifdef USE_STL_FILE_STREAM

    // doesn't working, crash at destuct
    // TODO: some cache for logging
    std::ofstream logfile (log_path.c_str(), std::ofstream::binary|std::ofstream::ate|std::ofstream::app);
    if (logfile.bad())
      return;

    // get size of file
    if (0 ==logfile.tellp())
      logfile.write((char*)"\xff\xfe" , 2);

    swprintf_s(buff, MAX_PATH, L"[%f]", (double)logTick/10000);

    logfile.write((char*)buff, wcslen(buff)*2);
    logfile.write((char*)line, wcslen(line)*2);

    if (line[wcslen(line)-1] != L'\n')
      logfile.write((char*)L"\r\n", 4);

#else
    FILE *logfile;
    if (_wfopen_s(&logfile, log_path.c_str(), L"ab") != 0)
      return;
    if (_ftelli64(logfile) == 0)
      _fwrite_nolock((void*)"\xff\xfe" , sizeof(char), 2, logfile);

    swprintf_s(buff, MAX_PATH, L"[%f]", (double)logTick/10000);

    _fwrite_nolock((void*)buff , sizeof(wchar_t), wcslen(buff), logfile);
    _fwrite_nolock((void*)line , sizeof(wchar_t), wcslen(line), logfile);
    _fwrite_nolock((void*)L"\r\n" , sizeof(wchar_t), 2, logfile);

    fclose( logfile );
#endif

  }
  catch (...) {}
}

void LogController::LogLine(const char* line)
{
  LogLine(Strings::StringToWString(line).c_str());
}
