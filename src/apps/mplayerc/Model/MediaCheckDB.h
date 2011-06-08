#pragma once

#include <threadhelper.h>
#include <sqlitepp/sqlitepp.hpp>

class MediaCheckDB : public ThreadHelperImpl<MediaCheckDB>
{
public:
  MediaCheckDB();
  ~MediaCheckDB();

public:
  void _Thread();

protected:
  bool ShouldExit();
  void CheckDetectPath();
  void CheckMediaData();
  void AddInfoToDetectPath();
};