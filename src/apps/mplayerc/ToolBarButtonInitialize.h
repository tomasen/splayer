#ifndef TOOLBARBUTTONINITIALIZE_H
#define TOOLBARBUTTONINITIALIZE_H


#include "SUIButton.h"
#include "VolumeCtrl.h"

class AddButton
{
public: 
  AddButton();
  AddButton(int agn, std::wstring pn, CRect rc);
  ~AddButton();
  int m_align;
  std::wstring m_pbuttonname;
  CSUIButton* m_pbutton;
  CRect m_rect;
};


class ToolBarButton  
{
public:
  ToolBarButton();
  ToolBarButton(std::wstring btname, int agn1, CRect rc1);
  ToolBarButton(std::wstring btname, std::wstring bmp, int agn1, CRect rc1, BOOL bnbutton,
                int idi, BOOL bhd, int wdt);
  ToolBarButton(std::wstring btname, std::wstring bmp,int agn1, CRect rc1, 
                BOOL bnbutton, int idi, BOOL bhd, int wdt, int agn2,
                std::wstring pbtname,CRect rc2, BOOL badd);
  ~ToolBarButton();
  CSUIButton*  m_mybutton;
  std::wstring m_buttonname;
  std::wstring m_bmpstr;
  std::wstring m_pbuttonname;  
  int          m_align1;
  CRect        m_rect1;
  BOOL         m_bnotbutton;
  int          m_id;
  BOOL         m_bhide;
  int          m_width;
  int          m_align2;
  CRect        m_rect2;
  BOOL         m_baddalign;
  CSUIButton*  m_pbutton;
  std::vector<AddButton* > m_addbuttonvec;
};

class CToolBarButtonInitialize
{
public:
  CToolBarButtonInitialize();
  ~CToolBarButtonInitialize();

public:

  void virtual ButtonInitialize();

  void virtual FillButtonAttribute();
  void virtual SetButton();
  BOOL virtual ReturnBReadFromFile();

  void SetCfgPath(std::wstring file);
  std::wstring GetCfgPath();

  void SetBtnList(CSUIBtnList* plist);
  CSUIBtnList* GetBtnList();

  // TODO: design too bad
  void HackBottomToolBar(CSUIButton** pbtnbg, CSUIButton** pbtntm, CSUIButton** pbtnsub);
  void HackTopToolBar(CSUIButton** pbtnclose);

  std::map<std::wstring, int> m_classificationname_map;
  std::map<std::wstring, int> m_align1_map;
  std::map<std::wstring, int> m_align2_map;
  std::map<std::wstring, int> m_id_map;
  std::map<std::wstring, CSUIButton*> m_pbutton_map;
  std::vector<std::wstring> m_string_vec;
  std::vector<ToolBarButton* > m_button_vec;
  CSUIBtnList* m_pbtnList;

private:
  void virtual DefaultInitializeButton();
  BOOL virtual ReadFromFile();
  void virtual LineStringToVector();

  void virtual ButtonAttributeToString();
  void virtual WriteToFile();

  void virtual StringToButtonAttribute(std::wstring& buttoninformation, ToolBarButton* button);
  void virtual FillButtonAttribute(std::wstring& buttoninformation, ToolBarButton* button);
  void virtual SolveAddalign(std::wstring& buttoninformation, ToolBarButton* button);
  CRect GetCRect(std::wstring rectstr);

  std::wstring virtual FillString(ToolBarButton* ttb);
  std::wstring GetAlignorIdString(int i,std::map<std::wstring, int> mp);
  std::wstring RectToString(CRect& rc);
  std::wstring BoolString(BOOL bl);
  std::wstring GetWidth(int wid);

private:
  std::wstring m_filename;
  BOOL m_breadfromfile;

  CSUIButton* m_btnvolbg;
  CSUIButton* m_btnvoltm;
  CSUIButton* m_btnsubswitch;
  CSUIButton* m_btnclose;
};

#endif