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
  typedef struct _STYLEPARAM{
    wchar_t fontname[128];
    int fontsize;
    int fontcolor;      // color is 0x00bbggrr
    int strokesize;
    int strokecolor;    // color is 0x00bbggrr
    int shadowoffset;
    int shadowcolor;    // color is 0x00bbggrr
    int pos_vert;
    int pos_horz;
  }STYLEPARAM;
  static bool GetStyleParams(int index, STYLEPARAM** param_refout);
  static int GetStyleCount();
#ifdef _WINDOWS_
  static void Paint(HDC dc, RECT* rc, int index, bool selected = false, bool secondary = false);
#endif
};

#endif // SUBTITLESTYLE_H