#ifndef CUSTOMDRAWBTN_H
#define CUSTOMDRAWBTN_H

class CustomDrawBtn : public CButton
{
  DECLARE_DYNAMIC(CustomDrawBtn)

public:
  CustomDrawBtn(void);
protected:
  DECLARE_MESSAGE_MAP()

  void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
private:
  int m_textcolor;
  int m_textcolor_hilite;
  int m_basecolor;
  int m_basecolor2;
  int m_basecolor3;
  int m_basecolor4;

  int m_caption_height;
  int m_bottom_height;
  int m_button_height;
  int m_entry_height;
  int m_entry_padding;
  int m_padding;

  WTL::CFont    m_font_bold;
  WTL::CFont    m_font_normal;
  WTL::CFont    m_font_symbol;
  WTL::CBrush   m_br_list;

};

#endif // CUSTOMDRAWBTN_H