#include "stdafx.h"
#include "SubtitleStyle.h"
#include <Strings.h>
#include "../../mplayerc.h"
#include "../../resource.h"

// defined for use with EnumFontProc
static bool g_styleparams_inited = false;

static std::vector<SubtitleStyle::STYLEPARAM > g_vtStyleParams;  // main subtitle
static std::vector<SubtitleStyle::STYLEPARAM > g_vtSecStyleParams; // second subtitle

int CALLBACK EnumFontProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD FontType, LPARAM lParam)
{
  // enum only Chinese font(font name is also Chinese)
  //if ((unsigned char)(*(lpelfe->elfFullName)) > (unsigned char)('z'))
  if (*(lpelfe->elfFullName) > L'~')
  {
    // dynamic create main and second subtitle styles
    SubtitleStyle::STYLEPARAM styleparam;
    ::wcscpy(styleparam.fontname, lpelfe->elfFullName);
    styleparam.fontsize = 20;  // default is 20, will be changed
    styleparam.fontcolor = 0x00FFFFFF; // will be changed
    styleparam.strokesize = 1;
    styleparam.strokecolor = 0x00333333;
    styleparam.shadowoffset = 1;
    styleparam.shadowcolor = 0x00333333;
    styleparam.pos_vert = 90;
    styleparam.pos_horz = 50;

    g_vtStyleParams.push_back(styleparam);
    g_vtSecStyleParams.push_back(styleparam);
  }

  return 1;
}

void InitStyleParams()
{
  if (!g_styleparams_inited)
  {
    // init main and second subtitle style
    WTL::CDC      dc;
    WTL::CLogFont lf;
    dc.CreateCompatibleDC();

    lf.lfCharSet        = GB2312_CHARSET;  // only enum Chinese font
    lf.lfPitchAndFamily = 0;
    lf.lfFaceName[0] = '\0';
    ::EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)EnumFontProc, 0, 0);

    g_styleparams_inited = true;
  }
}

std::vector<std::wstring> g_sampletexts;  // text pieces to paint samples in SubtitleStyle::Paint

bool SubtitleStyle::GetStyleParams(int index_main, int index_sec, STYLEPARAM** param_refout)
{
  // initialize fonts first time this method is called
  InitStyleParams();

  // get the main style
  if (index_main >= 0 && index_main <= g_vtStyleParams.size() - 1)
  {
    // get the default style, but will change its size and color according to the appsettings
    *param_refout = &(g_vtStyleParams[index_main]);

    AppSettings &s = AfxGetAppSettings();
    if (s.subdefstyle.fontSize > 0)
      (*param_refout)->fontsize = s.subdefstyle.fontSize;
    (*param_refout)->fontcolor = s.subdefstyle.colors[0];

    return true;
  }

  // are we retrieving a secondary entry?
  if (index_sec >= 0 && index_sec <= g_vtSecStyleParams.size() - 1)
  {
    *param_refout = &g_vtSecStyleParams[index_sec];

    AppSettings &s = AfxGetAppSettings();
    if (s.subdefstyle2.fontSize > 0)
      (*param_refout)->fontsize = s.subdefstyle2.fontSize;
    (*param_refout)->fontcolor = s.subdefstyle2.colors[0];

    return true;
  }

  return false;
}

int SubtitleStyle::GetStyleCount(bool secondary /* = false */)
{
  // initialize fonts first time this method is called
  InitStyleParams();

  if (secondary)
    return g_vtSecStyleParams.size();
  return g_vtStyleParams.size();
}

void SubtitleStyle::Paint(HDC dc, RECT* rc, int index_main, int index_sec, bool selected /* = false */)
{
  STYLEPARAM* sp_main = 0;
  STYLEPARAM* sp_sec = 0;
  GetStyleParams(index_main, -1, &sp_main);
  GetStyleParams(-1, index_sec, &sp_sec);
  if ((!sp_main && !sp_sec) || (sp_main && sp_sec))
    return;  // only allow paint main subtitle or second subtitle

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

  // load sample text
  if (g_sampletexts.size() == 0)
  {
    WTL::CString text;
    text.LoadString(IDS_SUBTITLESTYLES);
    Strings::Split(text, L"|", g_sampletexts);
  }
  std::wstring sSampleText = sp_main ? g_sampletexts[0] : g_sampletexts[1];

  // some pre-paint jobs
  STYLEPARAM *pPaintStyleParam = 0;
  pPaintStyleParam = sp_main ? sp_main : sp_sec;

  // create font
  WTL::CLogFont lf;
  lf.lfHeight   = pPaintStyleParam->fontsize * 5 / 2;
  lf.lfQuality  = ANTIALIASED_QUALITY;
  lf.lfCharSet  = DEFAULT_CHARSET;
  wcscpy_s(lf.lfFaceName, 32, pPaintStyleParam->fontname);

  WTL::CFont font;
  font.CreateFontIndirect(&lf);

  // assign device context font
  HFONT old_font = mdc.SelectFont(font);

  // determine vertical and horizontal center of the |rc|
  WTL::CRect rc_textraw(0, 0, bmp_width, bmp_height);
  mdc.SetBkMode(TRANSPARENT);
  mdc.DrawText(sSampleText.c_str(), -1, &rc_textraw, DT_CALCRECT|DT_EDITCONTROL);
  int offset_y = (bmp_height - rc_textraw.Height())/2;
  int offset_x = ::GetSystemMetrics(SM_CXSMICON);
  rc_textraw.OffsetRect(offset_x, offset_y);
  // now we have |rc_textraw| as the center position of sample text

  // paint logic 1. shadow
  if (pPaintStyleParam->shadowoffset > 0)
  {
    rc_textraw.OffsetRect(pPaintStyleParam->shadowoffset*3, pPaintStyleParam->shadowoffset*3);
    // blend the shadow lighter a bit
    int color = (0x00FFFFFF + pPaintStyleParam->shadowcolor)/2;
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
      mdc.DrawText(sSampleText.c_str(), -1, &rc_textraw, DT_EDITCONTROL);
    }
    rc_textraw.OffsetRect(-1, 0);
    rc_textraw.OffsetRect(-pPaintStyleParam->shadowoffset*3, -pPaintStyleParam->shadowoffset*3);
  }

  // paint logic 2. stroke
  if (pPaintStyleParam->strokesize > 0)
  {
    mdc.SetTextColor(pPaintStyleParam->strokecolor);
    // we only support two kinds of stroke, 1 pixel, and 2 pixels
    // for 1 pixel stroke, simulation strategy is:
    // * + = original text dot position
    // * S = stroke position
    // [S] [S] [S]          [1] [5] [2]
    // [S] [+] [S]    =>    [7] [+] [8]
    // [S] [S] [S]          [4] [6] [3]
    if (pPaintStyleParam->strokesize == 1)
    {
      int seq_x[] = {-1, 2, 0, -2,  1/*5*/, 0, -1, 2};
      int seq_y[] = {-1, 0, 2,  0, -2/*5*/, 2, -1, 0};
      for (int i = 0; i < sizeof(seq_x)/sizeof(seq_x[0]); i++)
      {
        rc_textraw.OffsetRect(seq_x[i], seq_y[i]);
        mdc.DrawText(sSampleText.c_str(), -1, &rc_textraw, DT_EDITCONTROL);
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
        mdc.DrawText(sSampleText.c_str(), -1, &rc_textraw, DT_EDITCONTROL);
      }
      rc_textraw.OffsetRect(2, 0);
    }
  }

  // paint logic 3. body
  mdc.SetTextColor(pPaintStyleParam->fontcolor);
  mdc.DrawText(sSampleText.c_str(), -1, &rc_textraw, DT_EDITCONTROL);

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