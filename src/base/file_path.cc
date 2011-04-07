#include "file_path.h"


std::auto_ptr<FilePath> FilePath::m_instance;


// libgen's dirname and basename aren't guaranteed to be thread-safe and aren't
// guaranteed to not modify their input strings, and in fact are implemented
// differently in this regard on different platforms.  Don't use them, but
// adhere to their behavior.
std::wstring FilePath::DirName(const wchar_t* path)
{
  std::wstring new_path(path);


  std::wstring::size_type last_separator =
    new_path.find_last_of(kSeparators, std::wstring::npos,
    sizeof(kSeparators) - 1);
  if (last_separator != std::wstring::npos) {
    // path_ is somewhere else, trim the basename.
    new_path.resize(last_separator+1);
  }

  return new_path;
}

bool FilePath::IsSeparator(wchar_t character) {
  for (size_t i = 0; i < sizeof(kSeparators) - 1; ++i) {
    if (character == kSeparators[i]) {
      return true;
    }
  }

  return false;
}
