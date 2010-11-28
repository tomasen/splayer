#ifndef LOGCONTROLLER_H
#define LOGCONTROLLER_H

#include "LazyInstance.h"
#include "CriticalSection.h"
#include <string>

class LogController:
  public LazyInstanceImpl<LogController>
{
public:
  LogController(void);
  void Log(const wchar_t* fmt, ...);
  void Log(const char* fmt, ...);
private:
  void LogLine(const wchar_t* line);
  void LogLine(const char* line);
  __int64 m_PerfFrequency;
  __int64 GetPerfCounter();
  std::wstring log_path;
  CriticalSection m_cs;
};


#define Logging(...) LogController::GetInstance()->Log(__VA_ARGS__)
#endif // LOGCONTROLLER_H