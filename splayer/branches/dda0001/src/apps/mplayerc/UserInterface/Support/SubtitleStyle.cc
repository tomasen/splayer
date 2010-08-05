#include "stdafx.h"
#include "SubtitleStyle.h"
#include "../../Utils/Strings.h"
#include "../../mplayerc.h"
#include "../../resource.h"

// built-in definition for main subtitle styles
static SubtitleStyle::STYLEPARAM g_styleparams[] = 
{
  {L"", 20, 0x00FFFFFF, 1, 0x00333333, 0, 0, 90, 50},
  {L"", 20, 0x00FFFFFF, 2, 0x00333333, 3, 0x00333333, 90, 50},
  {L"", 20, 0x00FFFFFF, 2, 0x00996633, 3, 0x00333333, 90, 50},
#ifdef _WINDOWS_
  {L"SimSun", 20, 0x00FFFFFF, 2, 0x00996633, 3, 0x00333333, 90, 50},
  {L"Kaiti", 20, 0x0086E1FF, 2, 0x0006374A, 3, 0x00333333, 90, 50}
#endif
};

std::vector<std::wstring> g_sampletexts;  // text pieces to paint samples in SubtitleStyle::Paint

int CALLBACK EnumFontProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
  // EnumFontProc is called only when a font is enumerated, so we knew
  // this font is available.
  int* font_count = (int*)lParam;
  (*font_count)++;
  return 0; // stop enum
}

bool SubtitleStyle::GetStyleParams(int index, STYLEPARAM** param_refout)
{
  // initialize fonts first time this method is called
  if (lstrlen(g_styleparams[0].fontname) == 0)
  {
    // this process determines if our preferred fonts are available in system
    // if so, we choose to copy the preferred font name to STYLEPARAM.fontname
#ifdef _WINDOWS_
    WTL::CDC      dc;
    WTL::CLogFont lf;
    int font_count;
    dc.CreateCompatibleDC();

    lf.lfCharSet        = DEFAULT_CHARSET;
    lf.lfPitchAndFamily = 0;

    std::wstring preferred_font = L"SimHei";

    // try "Microsoft YaHei" first
    font_count = 0;
    wcscpy_s(lf.lfFaceName, 32, L"Microsoft YaHei");
    ::EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)EnumFontProc, (LPARAM)&font_count, 0);
    if (font_count > 0)
      preferred_font = L"Microsoft YaHei";

    // if not found, try "WenQuanYi Micro Hei"
    else
    {
      wcscpy_s(lf.lfFaceName, 32, L"WenQuanYi Micro Hei");
      ::EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)EnumFontProc, (LPARAM)&font_count, 0);
      if (font_count > 0)
        preferred_font = L"WenQuanYi Micro Hei";
    }

    wcscpy_s(g_styleparams[0].fontname, 128, preferred_font.c_str());
    wcscpy_s(g_styleparams[1].fontname, 128, preferred_font.c_str());
    wcscpy_s(g_styleparams[2].fontname, 128, preferred_font.c_str());
#endif
  }

  if (index < 0 || index >= sizeof(g_styleparams)/sizeof(g_styleparams[0]))
    return false;

  *param_refout = (STYLEPARAM*)&g_styleparams[index];

  return true;
}

int SubtitleStyle::GetStyleCount()
{
  return sizeof(g_styleparams)/sizeof(g_styleparams[0]);
}

void SubtitleStyle::Paint(HDC dc, RECT* rc, int index, bool selected /* = false */, bool secondary /* = false */)
{
  STYLEPARAM* sp = NULL;
  if (!GetStyleParams(index, &sp))
    return;

  // there is a trick in this routine to achieve better appearance with GDI font rendering
  // we create a memory dc that is 4x as large as the target rectangle, paint the large
  // version onto it, then use stretch blt to transfer to target dc, this will achieve
  // at least bilinear quality
  WTL::CDC mdc;
  WTL::CBitmap mbmp;
  int bmp_width = (rc->right - rc->left)*2;
  int bmp_height = (rc->bottom - rc->top)*2;
  mdc.CreateCompatibleDC(dc);
  mbmp.CreateCompatibleBitmap(dc, bmp_width, bmp_height);
  HBITMAP old_bmp = mdc.SelectBitmap(mbmp);
  RECT mrc = {0, 0, bmp_width, bmp_height};
  mdc.FillRect(&mrc, COLOR_3DFACE/*COLOR_WINDOW*/);

  // selection
  if (selected)
  {
    TRIVERTEX        vert[2] ;
    GRADIENT_RECT    gRect;
    vert[0] .x      = 1;
    vert[0] .y      = 1;
    vert[0] .Red    = 0xdc00;
    vert[0] .Green  = 0xea00;
    vert[0] .Blue   = 0xfc00;
    vert[0] .Alpha  = 0x0000;

    vert[1] .x      = bmp_width-1;
    vert[1] .y      = bmp_height-1; 
    vert[1] .Red    = 0xc100;
    vert[1] .Green  = 0xdc00;
    vert[1] .Blue   = 0xfc00;
    vert[1] .Alpha  = 0x0000;

    gRect.UpperLeft  = 0;
    gRect.LowerRight = 1;
    WTL::CPen pen;
    pen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_MENUHILIGHT));
    HPEN old_pen = mdc.SelectPen(pen);
    mdc.Rectangle(0, 0, bmp_width, bmp_height);
    mdc.SelectPen(old_pen);
    ::GradientFill(mdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
  }

  // create font
  WTL::CLogFont lf;
  lf.lfHeight   = -sp->fontsize*2;
  lf.lfQuality  = ANTIALIASED_QUALITY;
  wcscpy_s(lf.lfFaceName, 32, sp->fontname);

  WTL::CFont font;
  font.CreateFontIndirect(&lf);

  // load sample text
  if (g_sampletexts.size() == 0)
  {
    WTL::CString text;
    text.LoadString(IDS_SUBTITLESTYLES);
    Strings::Split(text, L"|", g_sampletexts);
  }

  // determine actual text to paint
  std::wstring sample_text = g_sampletexts[0];
  if (secondary)
  {
    sample_text += L"\r\n";
    sample_text += g_sampletexts[1];
  }

  // assign device context font
  HFONT old_font = mdc.SelectFont(font);

  // determine vertical and horizontal center of the |rc|
  WTL::CRect rc_textraw(0, 0, bmp_width, bmp_height);
  mdc.SetBkMode(TRANSPARENT);
  mdc.DrawText(sample_text.c_str(), -1, &rc_textraw, DT_CALCRECT|DT_EDITCONTROL);
  int offset_y = (bmp_height - rc_textraw.Height())/2;
  int offset_x = offset_y;
  rc_textraw.OffsetRect(offset_x, offset_y);
  // now we have |rc_textraw| as the center position of sample text

  // paint logic 1. shadow
  if (sp->shadowoffset > 0)
  {
    rc_textraw.OffsetRect(sp->shadowoffset*2, sp->shadowoffset*2);
    // blend the shadow lighter a bit
    int color = (0x00FFFFFF + sp->shadowcolor)/2;
    mdc.SetTextColor(color);
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
      mdc.DrawText(sample_text.c_str(), -1, &rc_textraw, DT_EDITCONTROL);
    }
    rc_textraw.OffsetRect(-1, 0);
    rc_textraw.OffsetRect(-sp->shadowoffset, -sp->shadowoffset);
  }

  // paint logic 2. stroke
  if (sp->strokesize > 0)
  {
    mdc.SetTextColor(sp->strokecolor);
    // we only support two kinds of stroke, 1 pixel, and 2 pixels
    // for 1 pixel stroke, simulation strategy is:
    // * + = original text dot position
    // * S = stroke position
    // [S] [S] [S]          [1] [5] [2]
    // [S] [+] [S]    =>    [7] [+] [8]
    // [S] [S] [S]          [4] [6] [3]
    if (sp->strokesize == 1)
    {
      int seq_x[] = {-1, 2, 0, -2,  1/*5*/, 0, -1, 2};
      int seq_y[] = {-1, 0, 2,  0, -2/*5*/, 2, -1, 0};
      for (int i = 0; i < sizeof(seq_x)/sizeof(seq_x[0]); i++)
      {
        rc_textraw.OffsetRect(seq_x[i], seq_y[i]);
        mdc.DrawText(sample_text.c_str(), -1, &rc_textraw, DT_EDITCONTROL);
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
        mdc.DrawText(sample_text.c_str(), -1, &rc_textraw, DT_EDITCONTROL);
      }
      rc_textraw.OffsetRect(2, 0);
    }
  }

  // paint logic 3. body
  mdc.SetTextColor(sp->fontcolor);
  mdc.DrawText(sample_text.c_str(), -1, &rc_textraw, DT_EDITCONTROL);

  // restore context
  mdc.SelectFont(old_font);

  // blt
  ::SetStretchBltMode(dc, HALFTONE);
  POINT OldOrg = {0};
  ::SetBrushOrgEx(dc, 0, 0, &OldOrg);
  ::StretchBlt(dc, rc->left, rc->top, bmp_width/2, bmp_height/2, 
    mdc, 0, 0, bmp_width, bmp_height, SRCCOPY);
  mdc.SelectBitmap(old_bmp);
}