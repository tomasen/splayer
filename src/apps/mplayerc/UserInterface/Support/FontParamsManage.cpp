#include "stdafx.h"
#include "Strings.h"
#include "json\json.h"
#include "FontParamsManage.h"
#include "../../Controller/SPlayerDefs.h"
#include "../../Controller/PlayerPreference.h"
#include "logging.h"

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

FontParamsManage::FontParamsManage(void)
{
  m_fontlist_simhei.push_back(L"Microsoft YaHei");
  m_fontlist_simhei.push_back(L"WenQuanYi Micro Hei");
  m_fontlist_simhei.push_back(L"SimHei");
  m_fontlist_simhei.push_back(L"\x5FAE\x8F6F\x96C5\x9ED1");  // Chinese for "Microsoft YaHei"
  m_fontlist_simhei.push_back(L"\x6587\x6CC9\x9A7F\x5FAE\x737C\x9ED1"); // Chinese for "WenQuanYi Micro Hei"
  m_fontlist_simhei.push_back(L"\x9ED1\x4F53"); // Chinese for "SimHei"
  m_fontlist_simhei.push_back(L"Segoe UI");
  m_fontlist_simhei.push_back(L"Arial");

  m_fontlist_simsun.push_back(L"SimSun");
  m_fontlist_simsun.push_back(L"\x5B8B\x4F53"); // Chinese for "SimSun"
  m_fontlist_simsun.push_back(L"Trebuchet MS");

  m_fontlist_kaiti.push_back(L"KaiTi");
  m_fontlist_kaiti.push_back(L"\x6977\x4F53"); // Chinese for "KaiTi"
  m_fontlist_kaiti.push_back(L"\x6977\x4F53_GB2312"); // Chinese for "KaiTi_GB2312"
  m_fontlist_kaiti.push_back(L"Georgia");

  Initialize();
}

FontParamsManage::~FontParamsManage(void)
{
  std::vector<StyleParam*>::iterator it = m_mainsp.begin();
  while (it != m_mainsp.end())
  {
    delete *it;
    ++it;
  }

  it = m_secondarysp.begin();
  while (it != m_secondarysp.end())
  {
    delete *it;
    ++it;
  }
}

void FontParamsManage::Initialize()
{
  std::wstring mainstr = PlayerPreference::GetInstance()->GetStringVar(STRVAR_MAINSUBTITLEFONT);
  if (mainstr.empty())
    DefaultInitialize();
  else
    ReadProfile();
}

void FontParamsManage::DefaultInitialize()
{
  DefaultInitializeMainspVec();
  DefaultInitializeSecondVec();

  if (!m_mainsp.empty())
    EnumFontToVec(m_mainsp);
  if (!m_secondarysp.empty())
    EnumFontToVec(m_secondarysp);
}

void FontParamsManage::DefaultInitializeMainspVec()
{
  StyleParam* sp1 = new StyleParam(SimHei, L"SimHei", 20, 0x00FFFFFF, 1, 0x00996633, 1, 0x00333333);
  m_mainsp.push_back(sp1);
  StyleParam* sp2 = new StyleParam(SimSun, L"SimSun", 16, 0x00FFFFFF, 2, 0x00996633, 0, 0x00000000);
  m_mainsp.push_back(sp2);
  StyleParam* sp3 = new StyleParam(SimHei, L"SimHei", 20, 0x00FFFFFF, 1, 0x00333333, 0, 0);
  m_mainsp.push_back(sp3);
  StyleParam* sp4 = new StyleParam(SimHei, L"SimHei", 20, 0x00FFFFFF, 2, 0x00333333, 1, 0x00333333);
  m_mainsp.push_back(sp4);
  StyleParam* sp5 = new StyleParam(SimHei, L"SimHei", 20, 0x0000ecec, 2, 0x000f0f0f, 1, 0x00333333);
  m_mainsp.push_back(sp5);
  StyleParam* sp6 = new StyleParam(KaiTi, L"KaiTi", 16, 0x0086E1FF, 2, 0x0006374A, 1, 0x00333333);
  m_mainsp.push_back(sp6);
  StyleParam* sp7 = new StyleParam(KaiTi, L"KaiTi", 16, 0x00FFFFFF, 2, 0x00996633, 0, 0x00000000);
  m_mainsp.push_back(sp7);
}

void FontParamsManage::DefaultInitializeSecondVec()
{
  StyleParam* sp1 = new StyleParam(SimHei, L"SimHei", 14, 0x00FFFFFF, 1, 0x00996633, 1, 0x00333333);
  m_secondarysp.push_back(sp1);
  StyleParam* sp2 = new StyleParam(SimSun, L"SimSun", 11, 0x00FFFFFF, 2, 0x00996633, 0, 0x00000000);
  m_secondarysp.push_back(sp2);
  StyleParam* sp3 = new StyleParam(SimHei, L"SimHei", 14, 0x00FFFFFF, 1, 0x00333333, 0, 0);
  m_secondarysp.push_back(sp3);
  StyleParam* sp4 = new StyleParam(SimHei, L"SimHei", 14, 0x00FFFFFF, 2, 0x00333333, 1, 0x00333333);
  m_secondarysp.push_back(sp4);
  StyleParam* sp5 = new StyleParam(SimHei, L"SimHei", 14, 0x0000ecec, 2, 0x000f0f0f, 1, 0x00333333);
  m_secondarysp.push_back(sp5);
  StyleParam* sp6 = new StyleParam(KaiTi, L"KaiTi", 11, 0x0086E1FF, 2, 0x0006374A, 1, 0x00333333);
  m_secondarysp.push_back(sp6);
  StyleParam* sp7 = new StyleParam(KaiTi, L"KaiTi", 11, 0x00FFFFFF, 2, 0x00996633, 0, 0x00000000);
  m_secondarysp.push_back(sp7);
}

void FontParamsManage::EnumFontToVec(std::vector<StyleParam*>& vec)
{
  WTL::CDC      dc;
  WTL::CLogFont lf;
  dc.CreateCompatibleDC();

  lf.lfCharSet        = DEFAULT_CHARSET;
  lf.lfPitchAndFamily = 0;

  int stylecount = vec.size();
  for (int i = 0; i < stylecount; i++)
  {
    std::vector<std::wstring>* fontlist = NULL;
    int fontlist_count = 0;
    switch (vec[i]->_fontname)
    {
    case SimHei:
      fontlist = &m_fontlist_simhei;
      fontlist_count = m_fontlist_simhei.size();
      break;
    case SimSun:
      fontlist = &m_fontlist_simsun;
      fontlist_count = m_fontlist_simsun.size();
      break;
    case KaiTi:
      fontlist = &m_fontlist_kaiti;
      fontlist_count = m_fontlist_kaiti.size();
      break;
    }

    if (!fontlist)
      continue;

    for (int j = 0; j < fontlist_count; j++)
    {
      int font_count = 0;
      ENUMPARAMS ep = {&font_count, L""};
      wcscpy_s(lf.lfFaceName, 32, (*fontlist)[j].c_str());
      ::EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)EnumFontProc, (LPARAM)&ep, 0);
      if (font_count > 0)
      {
        vec[i]->fontname = ep.realname;
        break;
      }
    }
  }
}

void FontParamsManage::ReadProfile()
{
  std::wstring mainstr = PlayerPreference::GetInstance()->GetStringVar(STRVAR_MAINSUBTITLEFONT);
  std::wstring secondstr = PlayerPreference::GetInstance()->GetStringVar(STRVAR_SECONDARYSUBTITLEFONT);
  
  if (!mainstr.empty())
    ParseStringToParams(m_mainsp, mainstr);
  if (!secondstr.empty())
    ParseStringToParams(m_secondarysp, secondstr);
}

void FontParamsManage::ParseStringToParams(std::vector<StyleParam*>& vec, const std::wstring& wstr)
{
  std::string str = Strings::WStringToString(wstr);
  Json::Value root;
  Json::Reader reader;
  BOOL bparsesuccess = reader.parse(str, root);
  if (!bparsesuccess)
    return;
  
  Json::Value::Members member = root.getMemberNames();

  int count = 0;
  while(count != member.size())
  {
    StyleParam* sp = new StyleParam;
    sp->fontname = Strings::StringToWString(root[member[count]]["FontName"].asString());
    sp->fontsize = root[member[count]]["FontSize"].asInt();
    sp->fontcolor = root[member[count]]["FontColor"].asInt();
    sp->strokesize = root[member[count]]["StrokeSize"].asInt();
    sp->strokecolor = root[member[count]]["StrokeColor"].asInt();
    sp->shadowsize = root[member[count]]["ShadowSize"].asInt();
    sp->shadowcolor = root[member[count]]["ShadowColor"].asInt();
      
    vec.push_back(sp);

    ++count;

  }
}

void FontParamsManage::WriteProfile()
{
  if (!m_mainsp.empty())
  {
    std::wstring manstr = TranslateParamsToString(m_mainsp);
    PlayerPreference::GetInstance()->SetStringVar(STRVAR_MAINSUBTITLEFONT, manstr);
  }

  if (!m_secondarysp.empty())
  {
    std::wstring secondstr = TranslateParamsToString(m_secondarysp);
    PlayerPreference::GetInstance()->SetStringVar(STRVAR_SECONDARYSUBTITLEFONT, secondstr);
  }
}

std::wstring FontParamsManage::TranslateParamsToString(const std::vector<StyleParam*>& vec)
{
  int i = 0;
  Json::Value root;
  Json::StyledWriter writer;
 
  while(i != vec.size())
  {
    std::string idstr = "FontParam";
    idstr += TranslateIntToString(i);
 
    root[idstr]["FontName"] = Strings::WStringToString(vec[i]->fontname);
    root[idstr]["FontSize"] = vec[i]->fontsize;
    root[idstr]["FontColor"] = vec[i]->fontcolor;
    root[idstr]["StrokeSize"] = vec[i]->strokesize;
    root[idstr]["StrokeColor"] = vec[i]->strokecolor;
    root[idstr]["ShadowSize"] = vec[i]->shadowsize;
    root[idstr]["ShadowColor"] = vec[i]->shadowcolor;

     ++ i ;
  }
 
  std::string outputConfig = writer.write(root);
    return Strings::StringToWString(outputConfig);
}

std::string FontParamsManage::TranslateIntToString(int i)
{
  char str[100];
  _itoa(i, str, 10);
  
  return str;
}

int  FontParamsManage::GetStyleCount()
{
  return m_mainsp.size();
}

StyleParam*  FontParamsManage::DetectFontType(std::wstring fontname, BOOL bmainsub)
{
  std::vector<StyleParam*>::iterator it;
  std::vector<StyleParam*>::iterator end;

  if (bmainsub)
  {
    it = m_mainsp.begin();
    end = m_mainsp.end();
  }
  else
  {
    it = m_secondarysp.begin();
    end = m_secondarysp.end();
  }

  while (it != end)
  {
    if (fontname == (*it)->fontname)
      return *it;

    ++it;
  }

  return 0;
}

void FontParamsManage::ModifyFontParam(int index, StyleParam* sp, BOOL bmainsub)
{
  int count;
  if (bmainsub)
    count = m_mainsp.size();
  else
    count = m_secondarysp.size();

  if (index < 0 || index >= count)
    return;

  if (bmainsub)
    m_mainsp[index] = sp;
  else
    m_secondarysp[index] = sp;
}

StyleParam* FontParamsManage::GetFontParam(int index, BOOL bmainsub)
{
  int count;
  if (bmainsub)
    count = m_mainsp.size();
  else
    count = m_secondarysp.size();

  if (index < 0 || index >= count)
    return 0;
 
  if (bmainsub)
    return m_mainsp[index];
  else
    return m_secondarysp[index];
}

// void FontParamsManage::AddFontParam(StyleParam* sp, BOOL bmainsub)
// {
//   if (bmainsub)
//     m_mainsp.push_back(sp);
//   else
//     m_secondarysp.push_back(sp);
// }
// 
// void FontParamsManage::DeleteFontParam(int index, BOOL bmainsub)
// {
//   std::vector<StyleParam>::iterator it;
//   std::vector<StyleParam>::iterator end;
// 
//   if (bmainsub)
//   {
//     it = m_mainsp.begin();
//     end = m_mainsp.end();
//   }
//   else
//   {
//     it = m_secondarysp.begin();
//     end = m_secondarysp.end();
//   }
// 
//   int count = 0;
//   while (it != end)
//   {
//     if (count == index)
//       break;
// 
//     ++count;
//     ++index;
//   }
// 
//   if (it != end)
//   {
//     if (bmainsub)
//       m_mainsp.erase(it);
//     else
//       m_secondarysp.erase(it);
//   }
// }