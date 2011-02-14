#include "stdafx.h"
#include "ToolBarButtonInitialize.h"
#include <fstream>

#define BMP 11
#define ALIGN1 12
#define CRECT1 13
#define NOTBUTTON 14
#define ID 15
#define HIDE 16
#define HIDEWIDTH 17
#define ALIGN2 18
#define BUTTON 19
#define CRECT2 110
#define ADDALIGN 111
#define ID_VOLUME_THUMB 126356

ToolBarButton::ToolBarButton()
{
  m_mybutton = NULL;
  m_buttonname = L"";
  m_bmpstr = L"";
  m_pbuttonname = L"";  
  m_align1 = 0;
  m_rect1 = CRect(0,0,0,0);
  m_bnotbutton = FALSE;
  m_id = 0;
  m_bhide = FALSE;
  m_width = 0;
  m_align2 = 0;
  m_rect2 = CRect(0,0,0,0);
  m_baddalign = FALSE;
  m_pbutton = NULL;
}

ToolBarButton::ToolBarButton(std::wstring btname, int agn1, CRect rc1)
{
  m_mybutton = 0;
  m_buttonname = btname;
  m_align1 = agn1;
  m_rect1 = rc1;
  m_bnotbutton = 0;
  m_id = 0;
  m_bhide = 0;
  m_width = 0;
  m_align2 = 0;
  m_rect2 = CRect(0,0,0,0);
  m_baddalign = FALSE;
  m_pbutton = 0;
  m_pbuttonname = L"";
}

ToolBarButton::ToolBarButton(std::wstring btname, std::wstring bmp, int agn1, CRect rc1, BOOL bnbutton,
                             int idi, BOOL bhd, int wdt)
{
  m_mybutton = 0;
  m_buttonname = btname;
  m_bmpstr = bmp;
  m_align1 = agn1;
  m_rect1 = rc1;
  m_bnotbutton = bnbutton;
  m_id = idi;
  m_bhide = bhd;
  m_width = wdt;
  m_align2 = 0;
  m_rect2 = CRect(0,0,0,0);
  m_baddalign = FALSE;
  m_pbutton = 0;
  m_pbuttonname = L"";
}

ToolBarButton::ToolBarButton(std::wstring btname, std::wstring bmp,int agn1, CRect rc1, 
                             BOOL bnbutton, int idi, BOOL bhd, int wdt, int agn2, 
                             std::wstring pbtname,CRect rc2, BOOL badd)
{
  m_buttonname = btname;
  m_bmpstr = bmp;
  m_pbuttonname = pbtname;
  m_align1 = agn1;
  m_rect1 = rc1;
  m_bnotbutton = bnbutton;
  m_id =idi;
  m_bhide = bhd;
  m_width = wdt;
  m_align2 = agn2;
  m_rect2 = rc2;
  m_baddalign = badd;
  m_pbutton = 0;
  m_mybutton = 0;
}

ToolBarButton::~ToolBarButton()
{
  for (std::vector<AddButton*>::iterator ite = m_addbuttonvec.begin();
       ite != m_addbuttonvec.end(); ++ite)
    delete *ite;
}

AddButton::AddButton()
{
  m_align = 0;
  m_pbuttonname = L"";
  m_pbutton = 0;
  m_rect = CRect(0,0,0,0);
}

AddButton::AddButton(int agn, std::wstring pn, CRect rc)
{
  m_align = agn;
  m_pbuttonname = pn;
  m_rect = rc;
  m_pbutton = 0;
}

AddButton::~AddButton(){}

CToolBarButtonInitialize::CToolBarButtonInitialize()
{
#define ADDCLASSIFICATIONNAME(x) m_classificationname_map[L#x] = x

  ADDCLASSIFICATIONNAME(BMP);
  ADDCLASSIFICATIONNAME(ALIGN1);
  ADDCLASSIFICATIONNAME(CRECT1);
  ADDCLASSIFICATIONNAME(NOTBUTTON);
  ADDCLASSIFICATIONNAME(ID);
  ADDCLASSIFICATIONNAME(HIDE);
  ADDCLASSIFICATIONNAME(HIDEWIDTH);
  ADDCLASSIFICATIONNAME(ALIGN2);
  ADDCLASSIFICATIONNAME(BUTTON);
  ADDCLASSIFICATIONNAME(CRECT2);
  ADDCLASSIFICATIONNAME(ADDALIGN);

#define ADDALIGN1(x) m_align1_map[L#x] = x

  ADDALIGN1(ALIGN_TOPLEFT);
  ADDALIGN1(ALIGN_TOPRIGHT);
  ADDALIGN1(ALIGN_BOTTOMLEFT);
  ADDALIGN1(ALIGN_BOTTOMRIGHT);

#define ADDALIGN2(x) m_align2_map[L#x] = x

  ADDALIGN2(ALIGN_TOP);
  ADDALIGN2(ALIGN_LEFT);
  ADDALIGN2(ALIGN_RIGHT);
  ADDALIGN2(ALIGN_BOTTOM);
}

CToolBarButtonInitialize::~CToolBarButtonInitialize(void)
{
  for (std::vector<ToolBarButton*>::iterator ite = m_button_vec.begin();
       ite != m_button_vec.end(); ++ite)
    delete *ite;
}

void CToolBarButtonInitialize::ButtonInitialize()
{
  m_breadfromfile = ReadFromFile();
  if (m_breadfromfile)
  {
    LineStringToVector();
    SetButton();
  }
  else
    DefaultInitializeButton();
}

BOOL CToolBarButtonInitialize::ReadFromFile()
{
  std::wifstream in_file;
  std::wstring  buttoninformation;
  in_file.open(m_filename.c_str());
  if (!in_file)
    return FALSE;

  while (getline(in_file, buttoninformation))
  {
    if (buttoninformation[0] == ' ' || buttoninformation == L"")
      continue;

    m_string_vec.push_back(buttoninformation);
  }

  in_file.close();
  return TRUE;
}

void CToolBarButtonInitialize::LineStringToVector()
{
  for (std::vector<std::wstring>::iterator ite = m_string_vec.begin();
    ite != m_string_vec.end(); ++ite)
  {
    std::wstring buttoninformation = *ite;
    std::wstring buttonname;
    ToolBarButton* buttonstruct = new ToolBarButton;
    int pos = buttoninformation.find_first_of(L":");
    buttonname = buttoninformation.substr(0, pos);
    buttonstruct->m_buttonname = buttonname;
    buttoninformation = buttoninformation.substr(pos + 1);
    StringToButtonAttribute(buttoninformation, buttonstruct);
    m_button_vec.push_back(buttonstruct);
  }
}

void CToolBarButtonInitialize::StringToButtonAttribute(std::wstring& buttoninformation, ToolBarButton* button)
{
  FillButtonAttribute(buttoninformation, button);
  SolveAddalign(buttoninformation, button);
}

void CToolBarButtonInitialize::FillButtonAttribute(std::wstring& buttoninformation, ToolBarButton* button)
{
  std::wstring s;
  std::wstring classificationname;
  int pos;

  while ((pos = buttoninformation.find_first_of(L",")) != std::wstring::npos)
  {
    classificationname = buttoninformation.substr(0, pos);
    buttoninformation = buttoninformation.substr(pos + 1);
    pos = buttoninformation.find_first_of(L";");
    s   = buttoninformation.substr(0, pos);
    switch (m_classificationname_map[classificationname])
    {
    case BMP:
      button->m_bmpstr = s;
      break;
    case ALIGN1:
      button->m_align1 = m_align1_map[s];
      break;
    case CRECT1:
      button->m_rect1 = GetCRect(s);
      break;
    case NOTBUTTON:
      if (buttoninformation[0] == 'T')
        button->m_bnotbutton = TRUE;
      if (buttoninformation[0] == 'F')
        button->m_bnotbutton = FALSE;
      break;
    case ID:
      button->m_id = m_id_map[s];
      break;
    case HIDE:
      if (buttoninformation[0] == 'T')
        button->m_bhide = TRUE;
      if (buttoninformation[0] == 'F')
        button->m_bhide = FALSE;
      break;
    case HIDEWIDTH:
      if (s == L"MAXINT")
        button->m_width = MAXINT;
      else
        button->m_width = _wtoi(s.c_str());
      break;
    case ALIGN2:
      button->m_align2 = m_align2_map[s];
      break;
    case BUTTON:
      button->m_pbuttonname = s;
      break;
    case CRECT2:
      button->m_rect2 = GetCRect(s);
      break;
    case ADDALIGN:
      button->m_baddalign = TRUE;
      return;
    }
    buttoninformation = buttoninformation.substr(pos + 1);
  }
}

void CToolBarButtonInitialize::SolveAddalign(std::wstring& buttoninformation, ToolBarButton* button)
{
  if (!button->m_baddalign)
    return;

  std::wstring s;
  std::wstring addalignstr;
  int pos;
  int align2 = 0;
  std::wstring pbuttonname = L"";
  CRect rect2 = CRect(0,0,0,0);

  while ((pos = buttoninformation.find_first_of(L",")) != std::wstring::npos)
  {
    s = buttoninformation.substr(0,pos);
    buttoninformation = buttoninformation.substr(pos + 1);
    pos = buttoninformation.find_first_of(L";");
    addalignstr = buttoninformation.substr(0, pos);
    switch (m_classificationname_map[s])
    {
    case ALIGN2:
      align2 = m_align2_map[addalignstr];
      break;
    case BUTTON:
      pbuttonname = addalignstr;
      break;
    case CRECT2:
      rect2 = GetCRect(addalignstr);
      break;
    }
    buttoninformation = buttoninformation.substr(pos + 1);
    if ((align2 != 0) && (pbuttonname != L"") && (rect2 != CRect(0,0,0,0)))
    {
      AddButton* addbutton = new AddButton(align2, pbuttonname,rect2);
      button->m_addbuttonvec.push_back(addbutton);

      align2 = 0;
      pbuttonname = L"";
      rect2 = CRect(0,0,0,0);
    }
  }
}

void CToolBarButtonInitialize::SetButton()
{

  for (std::vector<ToolBarButton*>::iterator ite = m_button_vec.begin();
    ite != m_button_vec.end(); ++ite)
  {
    if ((*ite)->m_buttonname != L"PLAYTIME")
    {
      if (!(*ite)->m_pbuttonname.empty())
        (*ite)->m_pbutton = m_pbutton_map[(*ite)->m_pbuttonname];
        (*ite)->m_mybutton = new CSUIButton((*ite)->m_bmpstr.c_str(), (*ite)->m_align1, (*ite)->m_rect1, 
        (*ite)->m_bnotbutton, (*ite)->m_id, (*ite)->m_bhide, (*ite)->m_align2, 
        (*ite)->m_pbutton, (*ite)->m_rect2);
        m_pbutton_map[(*ite)->m_buttonname] = (*ite)->m_mybutton;

      if ((*ite)->m_baddalign)
      {
        for (std::vector<AddButton*>::iterator iter = (*ite)->m_addbuttonvec.begin();
          iter != (*ite)->m_addbuttonvec.end(); ++iter)
        {
          (*iter)->m_pbutton = m_pbutton_map[(*iter)->m_pbuttonname];
          (*ite)->m_mybutton->addAlignRelButton((*iter)->m_align, (*iter)->m_pbutton, (*iter)->m_rect);
        }
      }

      m_pbtnList->AddTail((*ite)->m_mybutton);

      if ((*ite)->m_buttonname == L"VOLUMEBG")
        m_btnvolbg = (*ite)->m_mybutton;
      if ((*ite)->m_buttonname == L"VOLUMETM")
        m_btnvoltm = (*ite)->m_mybutton;
      if ((*ite)->m_buttonname == L"SUBSWITCH")
        m_btnsubswitch = (*ite)->m_mybutton;
      if ((*ite)->m_buttonname == L"CLOSE")
        m_btnclose = (*ite)->m_mybutton;

   }
  }
}

CRect CToolBarButtonInitialize::GetCRect(std::wstring rectstr)
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

void CToolBarButtonInitialize::ButtonAttributeToString()
{
  for (std::vector<ToolBarButton*>::iterator ite = m_button_vec.begin();
    ite != m_button_vec.end(); ++ite)
    m_string_vec.push_back(FillString(*ite));
}

std::wstring CToolBarButtonInitialize::FillString(ToolBarButton* ttb)
{
  std::wstring buttoninformation;
  std::wstring btname,bmp,agn1,rc1,bnbutton,id,bhd,wdt;
  std::wstring agn2,pbtname,rc2;

  btname = ttb->m_buttonname + L":";
  bmp = L"BMP," + ttb->m_bmpstr + L";";
  agn1 = L"ALIGN1," + GetAlignorIdString(ttb->m_align1, m_align1_map) + L";";
  rc1 = L"CRECT1," + RectToString(ttb->m_rect1) + L";";
  bnbutton = L"NOTBUTTON," + BoolString(ttb->m_bnotbutton) + L";";
  id = L"ID," + GetAlignorIdString(ttb->m_id, m_id_map) + L";";
  bhd = L"HIDE," + BoolString(ttb->m_bhide) + L";";
  wdt = L"HIDEWIDTH," + GetWidth(ttb->m_width) + L";";
  if (ttb->m_buttonname == L"PLAYTIME")
    buttoninformation = btname + agn1 + rc1;
  else
    buttoninformation = btname + bmp + agn1 + rc1 + bnbutton + id + bhd + wdt;

  if (ttb->m_align2 != 0)
  {
    agn2 = L"ALIGN2," + GetAlignorIdString(ttb->m_align2, m_align2_map) + L";";
    pbtname = L"BUTTON," + ttb->m_pbuttonname + L";";
    rc2 = L"CRECT2," + RectToString(ttb->m_rect2) + L";";
    buttoninformation += agn2 + pbtname + rc2;
  }

  if (ttb->m_baddalign)
  {
    buttoninformation += L"ADDALIGN,";
    for (std::vector<AddButton*>::iterator ite = ttb->m_addbuttonvec.begin();
      ite != ttb->m_addbuttonvec.end(); ++ite)
    {
      agn2 = L"ALIGN2," + GetAlignorIdString((*ite)->m_align, m_align2_map) + L";";
      pbtname = L"BUTTON," + (*ite)->m_pbuttonname + L";";
      rc2 = L"CRECT2," + RectToString((*ite)->m_rect) + L";";
      buttoninformation += agn2 + pbtname + rc2;
    }
  }

  return buttoninformation;

}

std::wstring CToolBarButtonInitialize::GetAlignorIdString(int i,std::map<std::wstring, int> mp)
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

std::wstring CToolBarButtonInitialize::RectToString(CRect& rc)
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

std::wstring CToolBarButtonInitialize::BoolString(BOOL bl)
{
  if (bl)
    return L"TRUE";
  else
    return L"FALSE";
}

std::wstring CToolBarButtonInitialize::GetWidth(int wid)
{
  if (wid == MAXINT)
    return L"MAXINT";
  wchar_t s[10];
  _itow(wid, s, 10);
  return s;
}

void CToolBarButtonInitialize::WriteToFile()
{
  std::wofstream outfile;
  outfile.open(m_filename.c_str(), std::wofstream::out | std::wofstream::trunc);
  if (outfile)
    for (std::vector<std::wstring>::iterator ite = m_string_vec.begin();
      ite != m_string_vec.end(); ++ite)
      outfile<<(*ite)<<L"\n";
}

void CToolBarButtonInitialize::DefaultInitializeButton()
{
  FillButtonAttribute();
  SetButton();
  //ButtonAttributeToString();
  //WriteToFile();
}

BOOL CToolBarButtonInitialize::ReturnBReadFromFile()
{
  return m_breadfromfile;
}

void CToolBarButtonInitialize::FillButtonAttribute()
{
  
}

void CToolBarButtonInitialize::SetCfgPath(std::wstring file)
{
  m_filename = file;
}

std::wstring CToolBarButtonInitialize::GetCfgPath()
{
  return m_filename;
}

void CToolBarButtonInitialize::SetBtnList(CSUIBtnList* plist)
{
  m_pbtnList = plist;
}

CSUIBtnList* CToolBarButtonInitialize::GetBtnList()
{
  return m_pbtnList;
}

void CToolBarButtonInitialize::HackBottomToolBar(CSUIButton** pbtnbg, CSUIButton** pbtntm, CSUIButton** pbtnsub)
{
  *pbtnbg = m_btnvolbg;
  *pbtntm = m_btnvoltm;
  *pbtnsub = m_btnsubswitch;
}
void CToolBarButtonInitialize::HackTopToolBar(CSUIButton** pbtnclose)
{
  *pbtnclose = m_btnclose;
}