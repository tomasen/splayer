#include "stdafx.h"
#include "SubtitleStyle.h"
#include "../../Utils/Strings.h"
#include "../../mplayerc.h"
#include "../../resource.h"

// built-in definition for main subtitle styles
const static SubtitleStyle::STYLEPARAM g_styleparams[] = 
{
  {L"", 20, 0x00FFFFFF, 1, 0x00333333, 0, 0, 90, 50},
  {L"", 20, 0x00FFFFFF, 2, 0x00333333, 3, 0x00333333, 90, 50},
  {L"", 20, 0x00FFFFFF, 2, 0x00336699, 3, 0x00333333, 90, 50},
#ifdef _WINDOWS_
  {L"SimSun", 20, 0x00FFFFFF, 2, 0x00336699, 3, 0x00333333, 90, 50},
  {L"Kaiti", 20, 0x00FFE186, 2, 0x004A3706, 3, 0x00333333, 90, 50}
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

    std::wstring  preferred_font = L"SimHei";

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

    wcscpy_s((wchar_t*)g_styleparams[0].fontname, 128, preferred_font.c_str());
    wcscpy_s((wchar_t*)g_styleparams[1].fontname, 128, preferred_font.c_str());
    wcscpy_s((wchar_t*)g_styleparams[2].fontname, 128, preferred_font.c_str());
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

void SubtitleStyle::Paint(HDC dc, RECT* rc, int index, bool secondary /* = false */)
{
  STYLEPARAM* sp = NULL;
  if (!GetStyleParams(index, &sp))
    return;

  // create font
  WTL::CLogFont lf;
  lf.lfHeight = sp->fontsize;
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
  HFONT old_font = (HFONT)::SelectObject(dc, font);

  // determine vertical and horizontal center of the |rc|
  WTL::CRect rc_textraw = *rc;
  ::SetBkMode(dc, TRANSPARENT);
  ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CALCRECT|DT_EDITCONTROL);
  int offset_x = (rc->right - rc->left - rc_textraw.Width())/2;
  int offset_y = (rc->bottom - rc->top - rc_textraw.Height())/2;
  rc_textraw.OffsetRect(offset_x, offset_y);
  // now we have |rc_textraw| as the center position of sample text

  // paint logic 1. shadow
  if (sp->shadowoffset > 0)
  {
    rc_textraw.OffsetRect(sp->shadowoffset, sp->shadowoffset);
    // use a fake algorithm to blend shadow half place to light gray
    int color = (0x00EFEFEF + sp->shadowcolor)/2;
    ::SetTextColor(dc, color);
    ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
    rc_textraw.OffsetRect(-sp->shadowoffset, -sp->shadowoffset);
  }

  // paint logic 2. stroke
  if (sp->strokesize > 0)
  {
    ::SetTextColor(dc, sp->strokecolor);
    // we only support two kinds of stroke, 1 pixel, and 2 pixels
    // for 1 pixel stroke, simulation strategy is:
    // * + = original text dot position
    // * S = stroke position
    // [S] [ ] [S]          [1] [ ] [2]
    // [ ] [+] [ ]    =>    [ ] [+] [ ]
    // [S] [ ] [S]          [4] [ ] [3]
    if (sp->strokesize == 1)
    {
      rc_textraw.OffsetRect(-1, -1);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(2, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(0, 2);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-2, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(1, -1);
    }
    // for 2 pixels stroke, simulation strategy is:
    // [ ] [S] [ ] [S] [ ]          [ ] [1] [ ] [2] [ ]
    // [S] [S] [S] [S] [S]          [7] [6] [5] [4] [3]
    // [ ] [S] [+] [S] [ ]    =>    [ ] [9] [+] [8] [ ]
    // [S] [S] [S] [S] [S]          [4] [3] [2] [1] [0]
    // [ ] [S] [ ] [S] [ ]          [ ] [6] [ ] [5] [ ]
    else
    {
      rc_textraw.OffsetRect(-2, -2);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(2, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(1, 1);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-1, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-1, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-1, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-1, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(3, 1);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-2, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(3, 1);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-1, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-1, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-1, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-1, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(3, 1);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(-2, 0);
      ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
      rc_textraw.OffsetRect(1, -2);
    }
  }

  // paint logic 3. body
  ::SetTextColor(dc, sp->fontcolor);
  // we choose to paint the body 3 times to even out dark stroke edges
  ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
  ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);
  ::DrawText(dc, sample_text.c_str(), -1, &rc_textraw, DT_CENTER|DT_EDITCONTROL);

  // restore font context
  ::SelectObject(dc, old_font);
}