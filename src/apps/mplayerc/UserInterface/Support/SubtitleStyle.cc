#include "stdafx.h"
#include "SubtitleStyle.h"
#include <Strings.h>
#include "../../mplayerc.h"
#include "../../resource.h"
#include "logging.h"

DrawSubtitle::DrawSubtitle():
  m_sptext(L"")
, m_bmp(0)
{

}

  void DrawSubtitle::Paint(HDC dc, WTL::CRect rc)
{
  int bmp_width = rc.Width();
  int bmp_height = rc.Height();
 
  // create font
  WTL::CLogFont lf;
  lf.lfHeight   = m_styleparam.fontsize * 5 / 2;
  lf.lfQuality  = ANTIALIASED_QUALITY;
  lf.lfCharSet  = DEFAULT_CHARSET;
  wcscpy_s(lf.lfFaceName, 32, m_styleparam.fontname.c_str());

  WTL::CFont font;
  font.CreateFontIndirect(&lf);

  // assign device context font
  HFONT old_font = (HFONT)SelectObject(dc, font);

  // determine vertical and horizontal center of the |rc|
  WTL::CRect rc_textraw(rc);
  SetBkMode(dc, TRANSPARENT);
  DrawText(dc, m_sptext.c_str(), -1, &rc_textraw, DT_CALCRECT|DT_EDITCONTROL);
  int offset_y = (bmp_height - rc_textraw.Height())/2;
  int offset_x = ::GetSystemMetrics(SM_CXSMICON);
  rc_textraw.OffsetRect(offset_x, offset_y);
  // now we have |rc_textraw| as the center position of sample text

  // paint logic 1. shadow
  if (m_styleparam.shadowsize > 0)
  {
    rc_textraw.OffsetRect(m_styleparam.shadowsize * 2, m_styleparam.shadowsize * 2);
    // blend the shadow lighter a bit
    int r = GetRValue(m_styleparam.shadowcolor);
    int g = GetGValue(m_styleparam.shadowcolor);
    int b = GetBValue(m_styleparam.shadowcolor);

    COLORREF color = RGB((r + 255) / 2,  (g + 255) / 2, (b + 255) / 2);
    SetTextColor(dc, color);
    // we choose to apply 1 pixel stroke for the shadow to make it look nicer
    // * + = original text dot position
    // * S = stroke position
    // [S] [S] [S]          [1] [5] [2]
    // [S] [+] [S]    =>    [7] [+] [8]
    // [S] [S] [S]          [4] [6] [3]
    int seq_x[] = {-1, 2, 0, -2,  1/*5*/, 0, -1, 2};
    int seq_y[] = {-1, 0, 2,  0, -2/*5*/, 2, -1, 0};
    for (int i = 0; i < sizeof(seq_x)/sizeof(seq_x[0]); i++)
    {
      rc_textraw.OffsetRect(seq_x[i], seq_y[i]);
      DrawText(dc, m_sptext.c_str(), -1, &rc_textraw, DT_EDITCONTROL);
    }
    rc_textraw.OffsetRect(-1, 0);
    rc_textraw.OffsetRect(-m_styleparam.shadowsize * 2, -m_styleparam.shadowsize * 2);
  }

  // paint logic 2. stroke
  if (m_styleparam.strokesize > 0)
  {
    SetTextColor(dc, m_styleparam.strokecolor);
    
    // we only support two kinds of stroke, 1 pixel, and 2 pixels
    // for 1 pixel stroke, simulation strategy is:
    // * + = original text dot position
    // * S = stroke position
    // [S] [S] [S]          [1] [5] [2]
    // [S] [+] [S]    =>    [7] [+] [8]
    // [S] [S] [S]          [4] [6] [3]
    if (m_styleparam.strokesize == 1)
    {
      int seq_x[] = {-1, 2, 0, -2,  1/*5*/, 0, -1, 2};
      int seq_y[] = {-1, 0, 2,  0, -2/*5*/, 2, -1, 0};
      for (int i = 0; i < sizeof(seq_x)/sizeof(seq_x[0]); i++)
      {
        rc_textraw.OffsetRect(seq_x[i], seq_y[i]);
        DrawText(dc, m_sptext.c_str(), -1, &rc_textraw, DT_EDITCONTROL);
      }
      rc_textraw.OffsetRect(-1, 0);
    }
    // for 2 pixels stroke, simulation strategy is:
    // [ ] [S] [ ] [S] [ ]          [ ] [1] [A] [2] [ ]
    // [S] [S] [S] [S] [S]          [7] [6] [5] [4] [3]
    // [ ] [S] [+] [S] [ ]    =>    [D] [9] [+] [8] [C]
    // [S] [S] [S] [S] [S]          [4] [3] [2] [1] [0]
    // [ ] [S] [ ] [S] [ ]          [ ] [6] [B] [5] [ ]
    else if (m_styleparam.strokesize == 2)
    {
      int seq_x[] = {-2, 2, 1, -1, -1, -1, -1, 3, -2, 3/*0*/, -1, -1, -1, -1, 3, -2/*6*/,  1, 0,  2, -4};
      int seq_y[] = {-2, 0, 1,  0,  0,  0,  0, 1,  0, 1/*0*/,  0,  0,  0,  0, 1,  0/*6*/, -4, 4, -2,  0};
      for (int i = 0; i < sizeof(seq_x)/sizeof(seq_x[0]); i++)
      {
        rc_textraw.OffsetRect(seq_x[i], seq_y[i]);
        DrawText(dc, m_sptext.c_str(), -1, &rc_textraw, DT_EDITCONTROL);
      }
      rc_textraw.OffsetRect(2, 0);
    }
    // for 3 pixels stroke simulation strategy is:
    // [ ][S][S][ ][S][S][ ]       [  ][ 1][ 2][41][ 3][ 4][  ]
    // [S][S][S][S][S][S][S]       [11][10][ 9][ 8][ 7][ 6][ 5]
    // [S][S][S][S][S][S][S]       [18][17][16][15][14][13][12]
    // [ ][S][S][+][S][S][ ]   =>  [44][22][21][ +][20][19][43]
    // [S][S][S][S][S][S][S]       [29][28][27][26][25][24][23]
    // [S][S][S][S][S][S][S]       [36][35][34][33][32][31][30]
    // [ ][S][S][ ][S][S][ ]       [  ][40][39][42][38][37][  ]
    else
    {
      int seq_x[] = {-2, 1, 2, 1, 1, -1, -1, -1, -1, -1/*10*/, -1, 6, -1, -1, -1, -1, -1, -1, 5, -1/*20*/, -2, -1, 5,
                     -1, -1, -1, -1, -1, -1, 6/*30*/, -1, -1, -1, -1, -1, -1, 5, -1, -2, -1,  2, 0,  3, -6};
      int seq_y[] = {-3, 0, 0, 0, 1,  0,  0,  0,  0,  0/*10*/,  0, 1,  0,  0,  0,  0,  0,  0, 1,  0/*20*/,  0,  0, 1,
                      0,  0,  0,  0,  0,  0, 1/*30*/,  0,  0,  0,  0,  0,  0, 1,  0,  0,  0, -6, 6, -3,  0};

      if (sizeof(seq_x) / sizeof(seq_x[0]) != 44)
        return;
      if (sizeof(seq_y) / sizeof(seq_y[0]) != 44)
        return;

      for (int i = 0; i < sizeof(seq_x)/sizeof(seq_x[0]); i++)
      {
        rc_textraw.OffsetRect(seq_x[i], seq_y[i]);
        DrawText(dc, m_sptext.c_str(), -1, &rc_textraw, DT_EDITCONTROL);
      }
      rc_textraw.OffsetRect(3, 0);

    }
  }

  // paint logic 3. body
  
  SetTextColor(dc, m_styleparam.fontcolor);
  DrawText(dc, m_sptext.c_str(), -1, &rc_textraw, DT_EDITCONTROL);

  // restore context
  SelectObject(dc, old_font);
}

void DrawSubtitle::SetFont(StyleParam sp)
{
  m_styleparam = sp;
}

void DrawSubtitle::SetSampleText(std::wstring text)
{
  m_sptext = text;
}