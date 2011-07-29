#pragma once

#include "LazyInstance.h"
#include <threadhelper.h>
#include "NetworkControlerImpl.h"
#include "..\Utils\Monitor\factory.h"

class Ed2kController:
  public NetworkControlerImpl,
  public ThreadHelperImpl<Ed2kController>,
  public LazyInstanceImpl<Ed2kController>
{
public:
  enum ED2K_EXIST_ENUM
  {
    ED2K_NOTEXIST_NO_UPLOAD,  // ed2k is exist but not need to be upload
    ED2K_NOT_EXIST,        // ed2k is not exist and need to be upload
    ED2K_EXIST,            // ed2k is already exist, don't upload
    ED2K_SERVER_ERROR      // an error occur when check the status of an ed2k
  };

public:
  Ed2kController();

  // add media path to calculate later
  void AddMediaFile(const std::wstring &path);

  // asyn calculate and upload ed2k
  void _Thread();

  // is ed2k already exist on server
  ED2K_EXIST_ENUM IsEd2kExist(const std::wstring &file);

protected:
  bool UploadEd2k(const std::wstring &file, const std::wstring &link);

private:
  CriticalSection m_cs;
  std::list<std::wstring> m_lsMediaFiles;
  MT::factory::monitor_ptr m_monitor;
};