#ifndef FILEASSOC_WIN_H
#define FILEASSOC_WIN_H

//////////////////////////////////////////////////////////////////////////
//
//  FileAssoc manages file association on Windows
//
class FileAssoc
{
public:
  typedef enum 
  {
    AP_VIDEO=0,
    AP_MUSIC,
    AP_AUDIOCD,
    AP_DVDMOVIE, 
    AP_SVCDMOVIE, 
    AP_VCDMOVIE, 
    AP_BDMOVIE, 
    AP_DVDAUDIO, 
    AP_CAPTURECAMERA
  }autoplay_t;

  static bool IsExtRegistered(const wchar_t* ext);
  static std::wstring GetFileIcon(const wchar_t* ext);
  static bool IsAutoPlayRegistered(autoplay_t ap);
  static void RegisterPlayer(int action_id);
};

#endif // FILEASSOC_WIN_H