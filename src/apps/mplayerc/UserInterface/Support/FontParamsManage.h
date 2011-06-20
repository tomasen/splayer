#pragma once

#include "SubtitleStyle.h"

class FontParamsManage
{
public:
  FontParamsManage(void);
  ~FontParamsManage(void);

  void Initialize();
  void DefaultInitialize();
  void DefaultInitializeMainspVec();
  void DefaultInitializeSecondVec();
  void EnumFontToVec(std::vector<StyleParam*>& vec);
  void ReadProfile();
  void ParseStringToParams(std::vector<StyleParam*>& vec, const std::wstring& wstr);
  void CheckFontIsExist(std::vector<StyleParam*>& vec);
  void WriteProfile();
  std::wstring TranslateParamsToString(const std::vector<StyleParam*>& vec);
  int  GetStyleCount();
  StyleParam*  DetectFontType(std::wstring fontname, BOOL bmainsub = TRUE);
  void ModifyFontParam(int index, StyleParam* sp, BOOL bmainsub = TRUE);
  //void AddFontParam(StyleParam* sp, BOOL bmainsub);
  //void DeleteFontParam(int index, BOOL bmainsub);
  StyleParam* GetFontParam(int index, BOOL bmainsub = TRUE);
  
private:
  std::string TranslateIntToString(int i);

  std::vector<StyleParam*> m_mainsp;
  std::vector<StyleParam*> m_secondarysp;
  std::vector<std::wstring> m_fontlist_simhei;
  std::vector<std::wstring> m_fontlist_simsun;
  std::vector<std::wstring> m_fontlist_kaiti;
};
