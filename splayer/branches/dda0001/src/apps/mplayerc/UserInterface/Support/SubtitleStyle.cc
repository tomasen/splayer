#include "stdafx.h"
#include "SubtitleStyle.h"
#include "../../Utils/Strings.h"
#include "../../mplayerc.h"
#include "../../resource.h"

// built-in definition for main subtitle styles
static SubtitleStyle::STYLEPARAM g_styleparams[] = 
{
  {SubtitleStyle::SimHei, L"SimHei", 20, 0x00FFFFFF, 1, 0x00333333, 0, 0, 90, 50},
  {SubtitleStyle::SimHei, L"SimHei", 20, 0x00FFFFFF, 2, 0x00333333, 1, 0x00333333, 90, 50},
  {SubtitleStyle::SimHei, L"SimHei", 20, 0x00FFFFFF, 1, 0x00996633, 1, 0x00333333, 90, 50},
#ifdef _WINDOWS_
  {SubtitleStyle::SimSun, L"SimSun", 16, 0x00FFFFFF, 2, 0x00996633, 0, 0x00000000, 90, 50},
  {SubtitleStyle::SimHei, L"SimHei", 20, 0x0000ecec, 2, 0x000f0f0f, 1, 0x00333333, 90, 50},
  {SubtitleStyle::KaiTi, L"KaiTi", 16, 0x0086E1FF, 2, 0x0006374A, 1, 0x00333333, 90, 50},
#endif
};

// built-in definition for secondary subtitle styles
static SubtitleStyle::STYLEPARAM g_secstyleparams[] =
{
  {SubtitleStyle::None, L"", 0, 0, 0, 0, 0, 0, 10, 50},
  {SubtitleStyle::None, L"", 0, 0, 0, 0, 0, 0, 91, 50}
};

static bool g_styleparams_inited = false;

const static wchar_t* fontlist_simhei[] = {
  L"Microsoft YaHei",
  L"WenQuanYi Micro Hei",
  L"SimHei",
  L"\x5FAE\x8F6F\x96C5\x9ED1",              // Chinese for "Microsoft YaHei"
  L"\x6587\x6CC9\x9A7F\x5FAE\x737C\x9ED1",  // Chinese for "WenQuanYi Micro Hei"
  L"\x9ED1\x4F53"                           // Chinese for "SimHei"
  L"Segoe UI",
  L"Arial"
};

const static wchar_t* fontlist_simsun[] = {
  L"SimSun",
  L"\x5B8B\x4F53",                          // Chinese for "SimSun"
  L"Trebuchet MS"
};

const static wchar_t* fontlist_kaiti[] = {
  L"KaiTi",
  L"\x6977\x4F53",                          // Chinese for "KaiTi"
  L"\x6977\x4F53_GB2312",                   // Chinese for "KaiTi_GB2312"
  L"Georgia"
};

std::vector<std::wstring> g_sampletexts;  // text pieces to paint samples in SubtitleStyle::Paint

// defined for use with EnumFontProc
typedef struct _ENUMPARAMS {
  int*    fontcount;
  wchar_t realname[32];
}ENUMPARAMS;

int CALLBACK EnumFontProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD FontType, LPARAM lParam)
{
  // EnumFontProc is called only when a font is enumerated, so we knew
  // this font is available.
  ENUMPARAMS* ep = (ENUMPARAMS*)lParam;
  (*ep->fontcount)++;
  wcscpy_s(ep->realname, 32, lpelfe->elfFullName);
  return 0; // stop enum
}

bool SubtitleStyle::GetStyleParams(int index_main, int index_sec, STYLEPARAM** param_refout)
{
  // initialize fonts first time this method is called
  if (!g_styleparams_inited)
  {
    // this process determines if our preferred fonts are available in system
    // if so, we choose to copy the preferred font name to STYLEPARAM.fontname
#ifdef _WINDOWS_
    WTL::CDC      dc;
    WTL::CLogFont lf;
    dc.CreateCompatibleDC();

    lf.lfCharSet        = DEFAULT_CHARSET;
    lf.lfPitchAndFamily = 0;

    int stylecount = GetStyleCount();
    for (int i = 0; i < stylecount; i++)
    {
      wchar_t** fontlist = NULL;
      int fontlist_count = 0;
      switch (g_styleparams[i]._fontname)
      {
      case SimHei:
        fontlist = (wchar_t**)fontlist_simhei;
        fontlist_count = sizeof(fontlist_simhei)/sizeof(fontlist_simhei[0]);
        break;
      case SimSun:
        fontlist = (wchar_t**)fontlist_simsun;
        fontlist_count = sizeof(fontlist_simsun)/sizeof(fontlist_simsun[0]);
        break;
      case KaiTi:
        fontlist = (wchar_t**)fontlist_kaiti;
        fontlist_count = sizeof(fontlist_kaiti)/sizeof(fontlist_kaiti[0]);
        break;
      }

      if (!fontlist)
        continue;

      for (int j = 0; j < fontlist_count; j++)
      {
        int font_count = 0;
        ENUMPARAMS ep = {&font_count, L""};
        wcscpy_s(lf.lfFaceName, 32, fontlist[j]);
        ::EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)EnumFontProc, (LPARAM)&ep, 0);
        if (font_count > 0)
        {
          wcscpy_s(g_styleparams[i].fontname, 128, ep.realname);
          break;
        }
      }
    }
#endif
    g_styleparams_inited = true;
  }

  if (index_main < 0 || index_main >= sizeof(g_styleparams)/sizeof(g_styleparams[0]))
    return false;

  *param_refout = (STYLEPARAM*)&g_styleparams[index_main];

  // are we retrieving a secondary entry?
  if (index_sec >= 0)
  {
    if (index_sec < 0 || index_sec >= sizeof(g_secstyleparams)/sizeof(g_secstyleparams[0]))
      return false;

    *param_refout = (STYLEPARAM*)&g_secstyleparams[index_sec];
    return true;
  }

  return true;
}

int SubtitleStyle::GetStyleCount(bool secondary /* = false */)
{
  if (secondary)
    return sizeof(g_secstyleparams)/sizeof(g_secstyleparams[0]);
  return sizeof(g_styleparams)/sizeof(g_styleparams[0]);
}

int SubtitleStyle::DetectFontType(std::wstring fontname)
{
  for (int fonttype = SimHei; fonttype < KaiTi; fonttype++)
  {
    wchar_t** fontlist = NULL;
    int fontlist_count = 0;
    switch (fonttype)
    {
    case SimHei:
      fontlist = (wchar_t**)fontlist_simhei;
      fontlist_count = sizeof(fontlist_simhei)/sizeof(fontlist_simhei[0]);
      break;
    case SimSun:
      fontlist = (wchar_t**)fontlist_simsun;
      fontlist_count = sizeof(fontlist_simsun)/sizeof(fontlist_simsun[0]);
      break;
    case KaiTi:
      fontlist = (wchar_t**)fontlist_kaiti;
      fontlist_count = sizeof(fontlist_kaiti)/sizeof(fontlist_kaiti[0]);
      break;
    }

    if (!fontlist)
      continue;

    for (int i = 0; i < fontlist_count; i++)
    {
      if (fontname == fontlist[i])
        return fonttype;
    }
  }

  return None;
}
void SubtitleStyle::Paint(HDC dc, RECT* rc, int index_main, int index_sec, bool selected /* = false */)
{
  STYLEPARAM* sp_main = NULL;
  STYLEPARAM* sp_sec = NULL;
  if (!GetStyleParams(index_main, -1, &sp_main))
    return;
  if (index_sec >= 0)
    GetStyleParams(index_main, index_sec, &sp_sec);

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
  lf.lfHeight   = sp_main->fontsize*5/2;
  lf.lfQuality  = ANTIALIASED_QUALITY;
  lf.lfCharSet  = DEFAULT_CHARSET;
  wcscpy_s(lf.lfFaceName, 32, sp_main->fontname);

  // load sample text
  if (g_sampletexts.size() == 0)
  {
    WTL::CString text;
    text.LoadString(IDS_SUBTITLESTYLES);
    Strings::Split(text, L"|", g_sampletexts);
  }

  // determine actual text to paint
  std::wstring sample_text = g_sampletexts[0];
  // paint secondary subtitle?
  if (index_sec >= 0)
  {
    if (sp_sec->pos_vert <= 50)
      sample_text = g_sampletexts[1];
    else if (sp_sec->pos_vert > sp_main->pos_vert)
    {
      sample_text += L"\r\n";
      sample_text += g_sampletexts[1];
      // in secondary subtitle painting, we choose to make the font smaller a bit
      lf.lfHeight = lf.lfHeight*2/3;
    }
    else
    {
      sample_text = g_sampletexts[1];
      sample_text += L"\r\n";
      sample_text += g_sampletexts[0];
      // in secondary subtitle painting, we choose to make the font smaller a bit
      lf.lfHeight = lf.lfHeight*2/3;
    }
  }

  WTL::CFont font;
  font.CreateFontIndirect(&lf);

  // assign device context font
  HFONT old_font = mdc.SelectFont(font);

  // determine vertical and horizontal center of the |rc|
  WTL::CRect rc_textraw(0, 0, bmp_width, bmp_height);
  mdc.SetBkMode(TRANSPARENT);
  mdc.DrawText(sample_text.c_str(), -1, &rc_textraw, DT_CALCRECT|DT_EDITCONTROL);
  int offset_y = (bmp_height - rc_textraw.Height())/2;
  int offset_x = ::GetSystemMetrics(SM_CXSMICON);
  rc_textraw.OffsetRect(offset_x, offset_y);
  // now we have |rc_textraw| as the center position of sample text

  // paint logic 1. shadow
  if (sp_main->shadowoffset > 0)
  {
    rc_textraw.OffsetRect(sp_main->shadowoffset*3, sp_main->shadowoffset*3);
    // blend the shadow lighter a bit
    int color = (0x00FFFFFF + sp_main->shadowcolor)/2;
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
    rc_textraw.OffsetRect(-sp_main->shadowoffset*3, -sp_main->shadowoffset*3);
  }

  // paint logic 2. stroke
  if (sp_main->strokesize > 0)
  {
    mdc.SetTextColor(sp_main->strokecolor);
    // we only support two kinds of stroke, 1 pixel, and 2 pixels
    // for 1 pixel stroke, simulation strategy is:
    // * + = original text dot position
    // * S = stroke position
    // [S] [S] [S]          [1] [5] [2]
    // [S] [+] [S]    =>    [7] [+] [8]
    // [S] [S] [S]          [4] [6] [3]
    if (sp_main->strokesize == 1)
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
  mdc.SetTextColor(sp_main->fontcolor);
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