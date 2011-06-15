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
  lf.lfHeight   = m_styleparam.fontsize*5/2;
  lf.lfQuality  = ANTIALIASED_QUALITY;
  lf.lfCharSet  = DEFAULT_CHARSET;
  wcscpy_s(lf.lfFaceName, 32, m_styleparam.fontname.c_str());

  WTL::CFont font;
  font.CreateFontIndirect(&lf);

  // assign device context font
  HFONT old_font = (HFONT)SelectObject(dc, font);

  // determine vertical and horizontal center of the |rc|
  //WTL::CRect rc_textraw(0, 0, bmp_width, bmp_height);
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
    rc_textraw.OffsetRect(m_styleparam.shadowsize*3, m_styleparam.shadowsize*3);
    // blend the shadow lighter a bit
    int color = (0x00FFFFFF + m_styleparam.shadowcolor)/2;
    //int color = m_styleparam.shadowcolor;
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
    rc_textraw.OffsetRect(-m_styleparam.shadowsize*3, -m_styleparam.shadowsize*3);
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
    else
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