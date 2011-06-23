#ifndef SUBTITLESTYLE_H
#define SUBTITLESTYLE_H

//////////////////////////////////////////////////////////////////////////
//
//  SubtitleStyle provides mapping of internal style index to actual
//  subtitle rendering settings accepted by external subtitle filters.
//  It also provides Windows GDI based painting capability to draw
//  mock up rendering for dialog display purposes.
//

typedef enum _FontName {
  None, SimHei, SimSun, KaiTi
}FontName;

struct StyleParam
{
  StyleParam():_fontname(None), fontname(L""), fontsize(0), fontcolor(0), strokesize(0)
    , strokecolor(0), shadowsize(0), shadowcolor(0)
  {

  }

  StyleParam(FontName _fn, std::wstring fn, int fs, int fc, int strokes, int strokec,
    int shadows, int shadowc):_fontname(_fn), fontname(fn), fontsize(fs)
    ,fontcolor(fc), strokesize(strokes), strokecolor(strokec), shadowsize(shadows)
    ,shadowcolor(shadowc)
  {

  }

  FontName       _fontname;
  std::wstring   fontname;
  int            fontsize;
  int            fontcolor;
  int            strokesize;
  int            strokecolor;
  int            shadowsize;
  int            shadowcolor;
//   int            pos_vert;
//   int            pos_horz;
};

class DrawSubtitle
{
public:
  DrawSubtitle();

  void SetFont(StyleParam sp);
  void SetSampleText(std::wstring text);
#ifdef _WINDOWS_
  void Paint(HDC dc, WTL::CRect rc);
#endif

private:
  StyleParam m_styleparam;
  std::wstring m_sptext;
  WTL::CBitmap m_bmp;
};

#endif // SUBTITLESTYLE_H