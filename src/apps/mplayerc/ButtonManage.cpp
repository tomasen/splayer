#include "stdafx.h"
#include "ButtonManage.h"

#define BMP 11
#define FIXALIGN 12
#define FIXCRECT 13
#define NOTBUTTON 14
#define ID 15
#define HIDE 16
#define CURRENTHIDESTATE 17
#define HIDEWIDTH 18
#define RELATIVEALIGN 19
#define BUTTON 20
#define RELATIVECRECT 21
#define ADDALIGN 22

ButtonManage::ButtonManage(void)
{
#define ADDPROPERTY(x) m_property_map[L#x] = x

  ADDPROPERTY(BMP);
  ADDPROPERTY(FIXALIGN);
  ADDPROPERTY(FIXCRECT);
  ADDPROPERTY(NOTBUTTON);
  ADDPROPERTY(ID);
  ADDPROPERTY(HIDE);
  ADDPROPERTY(CURRENTHIDESTATE);
  ADDPROPERTY(HIDEWIDTH);
  ADDPROPERTY(RELATIVEALIGN);
  ADDPROPERTY(BUTTON);
  ADDPROPERTY(RELATIVECRECT);
  ADDPROPERTY(ADDALIGN);
  ADDPROPERTY(ALIGN_TOPLEFT);
  ADDPROPERTY(ALIGN_TOPRIGHT);
  ADDPROPERTY(ALIGN_BOTTOMLEFT);
  ADDPROPERTY(ALIGN_BOTTOMRIGHT);
  ADDPROPERTY(ALIGN_TOP);
  ADDPROPERTY(ALIGN_LEFT);
  ADDPROPERTY(ALIGN_RIGHT);
  ADDPROPERTY(ALIGN_BOTTOM);
}

ButtonManage::~ButtonManage(void)
{
}

void ButtonManage::SetParse(const std::vector<std::wstring>& vec, CSUIBtnList* btnlist)
{
  m_cfgfilestr_vec = vec;
  m_pbtnlist = btnlist;
}

void ButtonManage::ParseConfig(BOOL bl)
{
  m_bnotinitialize = bl;

  for (std::vector<std::wstring>::const_iterator ite = m_cfgfilestr_vec.begin();
    ite != m_cfgfilestr_vec.end(); ++ite)
  {
    std::wstring buttoninformation = *ite;
    int pos = buttoninformation.find_first_of(L":");
    m_buttonname = buttoninformation.substr(0, pos);
    buttoninformation = buttoninformation.substr(pos + 1);
    ParseStrToBtn(buttoninformation);
  }
}

void ButtonManage::ParseStrToBtn(std::wstring& buttoninformation)
{
  std::wstring s;
  std::wstring classificationname;
  int pos;
  std::wstring bmpstr;
  int align1 = 0, align2 = 0;
  CRect rect1,rect2(0,0,0,0);
  BOOL bnotbutton,bhide;
  int id;
  int width = 0;
  std::wstring pbuttonname = L"";
  CSUIButton* pbutton = 0;
  BOOL baddalign = FALSE;

  while ((pos = buttoninformation.find_first_of(L",")) != std::wstring::npos)
  {
    classificationname = buttoninformation.substr(0, pos);
    buttoninformation = buttoninformation.substr(pos + 1);
    pos = buttoninformation.find_first_of(L";");
    s   = buttoninformation.substr(0, pos);
    switch (m_property_map[classificationname])
    {
    case BMP:
      bmpstr = s;
      break;
    case FIXALIGN:
      align1 = m_property_map[s];
      break;
    case FIXCRECT:
      rect1 = GetCRect(s);
      break;
    case NOTBUTTON:
      if (buttoninformation[0] == 'T')
        bnotbutton = TRUE;
      if (buttoninformation[0] == 'F')
        bnotbutton = FALSE;
      break;
    case ID:
      id = _wtoi(s.c_str());
      break;
    case HIDE:
      if (buttoninformation[0] == 'T')
        bhide = TRUE;
      if (buttoninformation[0] == 'F')
        bhide = FALSE;
      break;
    case HIDEWIDTH:
      if (s == L"MAXINT")
        width = MAXINT;
      else
        width = _wtoi(s.c_str());
      break;
    case RELATIVEALIGN:
      align2 = m_property_map[s];
      break;
    case BUTTON:
      pbuttonname = s;
      pbutton = GetButton(pbuttonname);
      break;
    case RELATIVECRECT:
      rect2 = GetCRect(s);
      break;
    case ADDALIGN:
      baddalign = TRUE;
      break;
    }

    if (baddalign)
      break;
    
    buttoninformation = buttoninformation.substr(pos + 1);

  }
  CSUIButton* newbtn = 0;
  buttonattribute btnstructtmp = {align1, rect1, bhide, width};
  if (align2 != 0 && pbutton != 0 && rect2 != CRect(0,0,0,0))
  {
    relativebuttonattribute rtbtnstructtmp = {align2, pbutton, rect2};
    btnstructtmp.relativevec.push_back(rtbtnstructtmp);
  }
  if (!m_bnotinitialize)
    newbtn = new CSUIButton(bmpstr.c_str(), align1, rect1, bnotbutton, id, bhide,align2, pbutton, rect2,width,m_buttonname.c_str());
  else
    m_btnstruct = btnstructtmp;
  if (baddalign)
    ParseStrToBtnAddalign(buttoninformation, newbtn);
  if (!m_bnotinitialize)
    m_pbtnlist->AddTail(newbtn);
  m_attribute_map[m_buttonname] = m_btnstruct;
}

void ButtonManage::ParseStrToBtnAddalign(std::wstring& buttoninformation, CSUIButton* newbtn)
{
  std::wstring s;
  std::wstring addalignstr;
  int pos;
  int align2 = 0;
  std::wstring pbuttonname = L"";
  CSUIButton* pbutton = 0;
  CRect rect2 = CRect(0,0,0,0);

  while ((pos = buttoninformation.find_first_of(L",")) != std::wstring::npos)
  {
    s = buttoninformation.substr(0,pos);
    buttoninformation = buttoninformation.substr(pos + 1);
    pos = buttoninformation.find_first_of(L";");
    addalignstr = buttoninformation.substr(0, pos);
    switch (m_property_map[s])
    {
    case RELATIVEALIGN:
      align2 = m_property_map[addalignstr];
      break;
    case BUTTON:
      pbuttonname = addalignstr;
      pbutton = GetButton(pbuttonname);
      break;
    case RELATIVECRECT:
      rect2 = GetCRect(addalignstr);
      break;
    }
    buttoninformation = buttoninformation.substr(pos + 1);
    if ((align2 != 0) && (pbuttonname != L"") && (rect2 != CRect(0,0,0,0)))
    {
      relativebuttonattribute relativebtn = {align2, pbutton, rect2};
      if (!m_bnotinitialize)
        newbtn->addAlignRelButton(align2, pbutton, rect2);
      else
        m_btnstruct.relativevec.push_back(relativebtn);
      align2 = 0;
      pbuttonname = L"";
      rect2 = CRect(0,0,0,0);
    }
  }
}

CRect ButtonManage::GetCRect(std::wstring rectstr)
{
  int left, top, right, bottom;
  int pos = rectstr.find_first_of(L",");
  std::wstring str;
  str = rectstr.substr(0, pos);
  left = _wtoi(str.c_str());
  rectstr = rectstr.substr(pos + 1);
  pos = rectstr.find_first_of(L",");
  str = rectstr.substr(0, pos);
  top = _wtoi(str.c_str());
  rectstr = rectstr.substr(pos + 1);
  pos = rectstr.find_first_of(L",");
  str = rectstr.substr(0, pos);
  right = _wtoi(str.c_str());
  rectstr = rectstr.substr(pos + 1);
  bottom = _wtoi(rectstr.c_str());
  CRect rc(left, top, right, bottom);
  return rc;
}

void ButtonManage::ParseBtnToStr()
{
  POSITION pos = m_pbtnlist->GetHeadPosition();
  while(pos)
  {
    
    CSUIButton* cbtn = m_pbtnlist->GetNext(pos);
    m_cfgfilestr_vec.push_back(FillString(cbtn));
  }
}

std::wstring ButtonManage::FillString(CSUIButton* cbtn)
{
  std::wstring buttoninformation;
  std::wstring btname,bmp,agn1,rc1,bnbutton,id,bhd1,wdt;
  std::wstring agn2,pbtname,rc2;
  BOOL bfirstaddalign = FALSE;

  btname = cbtn->m_buttonname + L":";
  bmp = L"BMP," + cbtn->m_szBmpName + L";";
  agn1 = L"FIXALIGN," + GetAlignString(cbtn->m_iAlign, m_property_map) + L";";
  rc1 = L"FIXCRECT," + RectToString(cbtn->m_marginTownd) + L";";
  bnbutton = L"NOTBUTTON," + GetBoolString(cbtn->m_NotButton) + L";";
  id = L"ID," + GetIdString(cbtn->m_htMsgID) + L";";
  bhd1 = L"HIDE," + GetBoolString(cbtn->m_hide) + L";";
  wdt = L"HIDEWIDTH," + GetWidth(cbtn->m_hidewidth) + L";";
  buttoninformation = btname + bmp + agn1 + rc1 + bnbutton + id + bhd1 + wdt;

  POSITION pos = cbtn->btnAlignList.GetHeadPosition();
  if (pos)
  {
    CBtnAlign* cbtnalign = cbtn->btnAlignList.GetNext(pos);
    agn2 = L"RELATIVEALIGN," + GetAlignString(cbtnalign->iAlign, m_property_map) + L";";
    pbtname = L"BUTTON," + GetAddalignButtonname(cbtnalign) + L";";
    rc2 = L"RELATIVECRECT," + RectToString(cbtnalign->marginToBtn) + L";";
    buttoninformation += agn2 + pbtname + rc2;

    while(pos)
    {
      if (!bfirstaddalign)
      {
        buttoninformation += L"ADDALIGN,";
        bfirstaddalign = TRUE;
      }
      cbtnalign = cbtn->btnAlignList.GetNext(pos);
      agn2 = L"RELATIVEALIGN," + GetAlignString(cbtnalign->iAlign, m_property_map) + L";";
      pbtname = L"BUTTON," + GetAddalignButtonname(cbtnalign) + L";";
      rc2 = L"RELATIVECRECT," + RectToString(cbtnalign->marginToBtn) + L";";
      buttoninformation += agn2 + pbtname + rc2;
    }
  }

  return buttoninformation;
}

std::wstring ButtonManage::GetAlignString(int i,std::map<std::wstring, int> mp)
{
  if (i == 0)
    return L"0";
  for (std::map<std::wstring, int>::iterator ite = mp.begin();
    ite != mp.end(); ++ite)
  {
    if (ite->second == i)
      return ite->first;
  }
  return L"";
}

std::wstring ButtonManage::GetAddalignButtonname(CBtnAlign* btnalign)
{
  CSUIButton* btn = (CSUIButton*)btnalign->bBtn;
  return btn->m_buttonname;
}

std::wstring ButtonManage::GetIdString(int i)
{
  wchar_t s[10];
  _itow(i, s, 10);
  return (LPCTSTR)s;
}

std::wstring ButtonManage::GetBoolString(BOOL bl)
{
  if (bl)
    return L"TRUE";
  else
    return L"FALSE";
}

std::wstring ButtonManage::RectToString(CRect& rc)
{
  wchar_t s[10];
  std::wstring str;
  int left = rc.left;
  _itow(left, s, 10);
  str =str + s + L",";
  int top  = rc.top;
  _itow(top, s, 10);
  str =str + s + L",";
  int right = rc.right;
  _itow(right, s, 10);
  str =str + s + L",";
  int bottom = rc.bottom;
  _itow(bottom, s ,10);
  str += s;

  return str;
}

std::wstring ButtonManage::GetWidth(int wid)
{
  if (wid == MAXINT)
    return L"MAXINT";
  wchar_t s[10];
  _itow(wid, s, 10);
  return s;
}

CSUIButton* ButtonManage::GetButton(std::wstring s)
{
  return m_pbtnlist->GetButton(s.c_str());
}

std::vector<std::wstring>& ButtonManage::GetCfgString()
{
  return m_cfgfilestr_vec;
}

std::map<std::wstring, buttonattribute>& ButtonManage::GetBtnAttributeStruct()
{
  return m_attribute_map;
}