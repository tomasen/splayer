#ifndef SUBTITLESTYLE_H
#define SUBTITLESTYLE_H

//////////////////////////////////////////////////////////////////////////
//
//  SubtitleStyle provides mapping of internal style index to actual
//  subtitle rendering settings accepted by external subtitle filters.
//  It also provides Windows GDI based painting capability to draw
//  mock up rendering for dialog display purposes.
//
class SubtitleStyle
{
public:
  typedef enum _FontName {
    None, SimHei, SimSun, KaiTi
  }FontName;

  typedef struct _STYLEPARAM{
    FontName  _fontname;
    wchar_t   fontname[128];
    int fontsize;
    int fontcolor;      // color is 0x00bbggrr
    int strokesize;
    int strokecolor;    // color is 0x00bbggrr
    int shadowoffset;
    int shadowcolor;    // color is 0x00bbggrr
    int pos_vert;
    int pos_horz;
  }STYLEPARAM;

  static bool GetStyleParams(int index_main, int index_sec, STYLEPARAM** param_refout);
  static int GetStyleCount(bool secondary = false);
  static int DetectFontType(std::wstring fontname);
#ifdef _WINDOWS_
  static void Paint(HDC dc, RECT* rc, int index_main, int index_sec, bool selected = false);
#endif
};

#endif // SUBTITLESTYLE_H