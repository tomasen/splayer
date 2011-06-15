#pragma once

#include "LazyInstance.h"
#include <string>

class FilePath:
  public LazyInstanceImpl<FilePath>
{
public:
  wchar_t kSeparators[2];
  FilePath() {kSeparators[0] = L'\\'; kSeparators[1] = L'/';}
  bool IsSeparator(wchar_t character);
  std::wstring DirName(const wchar_t* path);
};
