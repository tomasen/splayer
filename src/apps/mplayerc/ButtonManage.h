#ifndef BUTTONMANAGE_H
#define BUTTONMANAGE_H

#include "SUIButton.h"
#include "VolumeCtrl.h"

class ButtonManage
{
public:
  ButtonManage(void);
  ~ButtonManage(void);

  void SetParse(const std::vector<std::wstring>& vec, CSUIBtnList* btnlist);

  void ParseConfig();
  void ParseStrToBtn(std::wstring& buttoninformation);
  void ParseStrToBtnAddalign(std::wstring& buttoninformation, CSUIButton* newbtn);
  CRect GetCRect(std::wstring rectstr);

  void ParseBtnToStr();
  std::wstring FillString(CSUIButton* cbtn);
  std::wstring GetAlignString(int i,std::map<std::wstring, int> mp);
  std::wstring GetAddalignButtonname(CBtnAlign* btnalign);
  std::wstring GetIdString(int i);
  std::wstring GetBoolString(BOOL bl);
  std::wstring RectToString(CRect& rc);
  std::wstring GetWidth(int wid);

  CSUIButton* GetButton(std::wstring s);

  std::vector<std::wstring>& GetCfgString();
private:
  std::vector<std::wstring> m_cfgfilestr_vec;
  CSUIBtnList* m_pbtnlist;
  std::wstring m_buttonname;
  std::map<std::wstring, int> m_property_map;

};

#endif
