#pragma once

#include <threadhelper.h>
#include "..\Model\MediaTreeModel.h"

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
  //void AddInfoToDetectPath();

private:
  MediaTreeModel m_treeModel;
};