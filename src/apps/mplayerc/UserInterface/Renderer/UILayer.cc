#include "stdafx.h"
#include "UILayer.h"
#include <ResLoader.h>

UILayer::UILayer(std::wstring respath, BOOL display /* = TRUE */)
{
  ResLoader rs;
  SetTexture(rs.LoadBitmap(respath));
  SetDisplay(display);
}

UILayer::~UILayer()
{

}

BOOL UILayer::SetTexture(HBITMAP texture)
{
  if (texture == NULL)
    return FALSE;

  m_texture = texture;

  m_texture.GetBitmap(&m_bm);

  if(m_bm.bmBitsPixel == 32)
  {
    for (int y=0; y<m_bm.bmHeight; y++)
    {
      BYTE * pPixel = (BYTE *) m_bm.bmBits + m_bm.bmWidth * 4 * y;
      for (int x=0; x<m_bm.bmWidth; x++)
      {
        pPixel[0] = pPixel[0] * pPixel[3] / 255; 
        pPixel[1] = pPixel[1] * pPixel[3] / 255; 
        pPixel[2] = pPixel[2] * pPixel[3] / 255; 
        pPixel += 4;
      }
    }
  }

  return TRUE;
}

BOOL UILayer::GetTextureRect(RECT& rc)
{
  rc.top = 0;
  rc.left = 0;
  rc.right = m_bm.bmWidth;
  rc.bottom = m_bm.bmHeight;

  return TRUE;
}

BOOL UILayer::GetTexturePos(POINT& pt)
{
  pt = m_texturepos;
  return TRUE;
}

BOOL UILayer::SetTexturePos(const POINT& pt)
{
  m_texturepos = pt;
  return TRUE;
}

BOOL UILayer::SetDisplay(BOOL display)
{
  m_display = display;
  return TRUE;
}

BOOL UILayer::DoPaint(WTL::CDC& dc)
{
  if (m_display == FALSE)
    return FALSE;

  WTL::CDC texturedc;
  HBITMAP hold_texture;

  texturedc.CreateCompatibleDC(dc);
  hold_texture = texturedc.SelectBitmap(m_texture);

  BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  dc.AlphaBlend(m_texturepos.x, m_texturepos.y, m_bm.bmWidth, m_bm.bmHeight,
                texturedc, 0, 0, m_bm.bmWidth, m_bm.bmHeight, bf);

  texturedc.SelectBitmap(hold_texture);
  texturedc.DeleteDC();
  
  return TRUE;
}