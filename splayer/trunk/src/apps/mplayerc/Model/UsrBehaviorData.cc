#include "stdafx.h"
#include "UsrBehaviorData.h"
#include "session.hpp"
#include "sqlitepp.hpp"
#include "transaction.hpp"
#include "../revision.h"
#include "../Utils/SPlayerGUID.h"
#include "../Controller/PlayerPreference.h"
#include "../Controller/SPlayerDefs.h"
#include "../Controller/UsrBehaviorController.h"
#include <comdef.h>
#include <Wbemidl.h>

#define GET_EVNDATABYWMI(sql, f, var) \
{ \
  hr = ser->ExecQuery(bstr_t("WQL"), bstr_t(sql), \
  WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &enumerator);  \
  if (FAILED(hr)) \
  { \
  loc->Release(); \
  ser->Release(); \
  CoUninitialize(); \
  return; \
  } \
##var = L""; \
  while (enumerator) \
  { \
  enumerator->Next(WBEM_INFINITE, 1, &obj, &result);  \
  if (0 == result) \
  break; \
  obj->Get(f, 0, &vr, 0, 0); \
  std::wstring tmp(vr.bstrVal); \
##var += tmp; \
  VariantClear(&vr); \
  } \
  obj->Release(); \
  enumerator->Release(); \
}

UsrBehaviorData::~UsrBehaviorData()
{
  struct _stat buf;
  int year, weekcount;
  std::wstring uid, format, path;
  wchar_t dbname[MAX_PATH], apppath[MAX_PATH];
  bool setenv = false;

  SPlayerGUID::GenerateGUID(uid);
  GetYearAndWeekcount(year, weekcount);
  ::GetEnvironmentVariable(L"APPDATA", apppath, MAX_PATH);
  path = apppath;
  path += L"\\SPlayer\\ubdata\\";

  if (!::CreateDirectory(path.c_str(), NULL))
  {
    if (GetLastError() == ERROR_PATH_NOT_FOUND)
      return;
  }

  swprintf_s(dbname, MAX_PATH, DATABASE_NAME, uid.c_str(), year, weekcount);
  path += dbname;
  
  sqlitepp::session db;
  if (_wstat(path.c_str(), &buf) != 0)
  {
    setenv = true;

    db.open(path.c_str());
    // create a new db file
    db << "create table usrbhv ("
       << "id integer, data text, time real)";
    // create new environment table
    db << "create table usrenv ("
       << "name text, data text)";
  }
  else
    db.open(path.c_str());

  db << "PRAGMA synchronous=0";
  sqlitepp::transaction ts(db);
  for (std::vector<UsrBehaviorEntry>::iterator it = ubhv_entries.begin();
    it != ubhv_entries.end(); it++)
  {
    db << "insert into usrbhv values(:id, :data, :time)",
      sqlitepp::use((*it).id),
      sqlitepp::use((*it).data),
      sqlitepp::use((*it).time);
  }
  // we need save the local environment data
  if (setenv)
  {
    SetEnvironmentData();
    for (std::vector<UsrEnvEntry>::iterator it = env_entries.begin();
      it != env_entries.end(); it++)
    {
      db << "insert into usrenv values(:name, :data)",
        sqlitepp::use((*it).name),
        sqlitepp::use((*it).data);
    }
  }

  ts.commit();
  db.close();
}

void UsrBehaviorData::AppendBhvEntry(int id, std::wstring data)
{
  SYSTEMTIME st;
  double vt;

  ::GetSystemTime(&st);
  ::SystemTimeToVariantTime(&st, &vt);

  UsrBehaviorEntry ube = {id, data, vt};
  ubhv_entries.push_back(ube);
}

void UsrBehaviorData::AppendEnvEntry(std::wstring name, std::wstring data)
{
  UsrEnvEntry env = {name, data};
  env_entries.push_back(env);
}

void UsrBehaviorData::GetYearAndWeekcount(int& year, int& weekcount)
{
  struct tm tnow;
  time_t t;

  time(&t);
  localtime_s(&tnow, &t);

  year = 1900 + tnow.tm_year;
  weekcount = tnow.tm_yday / 7 + 1;
}

void UsrBehaviorData::SetEnvironmentData()
{
  VARIANT vr;
  ULONG result = 0;
  HRESULT hr = NULL;

  std::wstring var;

  IWbemLocator* loc = NULL;
  IWbemServices* ser = NULL;
  IWbemClassObject* obj = NULL;
  IEnumWbemClassObject* enumerator = NULL;

  hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (FAILED(hr))
    return;

  hr = CoInitializeSecurity(
    NULL, 
    -1,                          // COM authentication
    NULL,                        // Authentication services
    NULL,                        // Reserved
    RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
    RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
    NULL,                        // Authentication info
    EOAC_NONE,                   // Additional capabilities 
    NULL                         // Reserved
    );
  if (FAILED(hr))
  {
    CoUninitialize();
    return;
  }

  hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&loc);
  if (FAILED(hr))
  {
    CoUninitialize();
    return;
  }

  hr = loc->ConnectServer(bstr_t("ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &ser);
  if (FAILED(hr))
  {
    loc->Release();
    CoUninitialize();
    return;
  }

  GET_EVNDATABYWMI(L"SELECT * FROM Win32_Processor", L"Name", var);
  AppendEnvEntry(L"Processor", var);
  GET_EVNDATABYWMI(L"SELECT * FROM Win32_OperatingSystem", L"Caption", var);
  AppendEnvEntry(L"OS", var);
  GET_EVNDATABYWMI(L"SELECT * FROM Win32_BaseBoard", L"Product", var);
  AppendEnvEntry(L"Board", var);
  GET_EVNDATABYWMI(L"SELECT * FROM Win32_VideoController", L"Description", var);
  AppendEnvEntry(L"VideoCard", var);
  GET_EVNDATABYWMI(L"SELECT * FROM Win32_SoundDevice", L"Name", var);
  AppendEnvEntry(L"SoundDevice", var);
  GET_EVNDATABYWMI(L"SELECT * FROM Win32_ComputerSystem", L"Name", var);
  AppendEnvEntry(L"ComputerName", var);
  GET_EVNDATABYWMI(L"SELECT * FROM Win32_CDROMDrive", L"Name", var);
  AppendEnvEntry(L"CDROMDrive", var);
  GET_EVNDATABYWMI(L"SELECT * FROM Win32_PhysicalMemory", L"Capacity", var);
  AppendEnvEntry(L"PhysicalMemory", var);
  GET_EVNDATABYWMI(L"SELECT * FROM Win32_BIOS", L"Name", var);
  AppendEnvEntry(L"BIOS", var);
  GET_EVNDATABYWMI(L"SELECT * FROM Win32_DiskDrive", L"Model", var);
  AppendEnvEntry(L"DiskDrive", var);

  // release
  loc->Release();
  ser->Release();
  CoUninitialize();

  SPlayerGUID::GenerateGUID(var);
  AppendEnvEntry(L"UUID", var);
  AppendEnvEntry(L"SPlayer°æ±¾", SVP_REV_STR);
}