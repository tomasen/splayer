#include "stdafx.h"
#include "MakeMultiplyBmp.h"

MakeMultiplyBmp::MakeMultiplyBmp(void):
m_alpha(255),
m_brightness(1.0)
{
}

MakeMultiplyBmp::~MakeMultiplyBmp(void)
{
}

HBITMAP MakeMultiplyBmp::MakeMultiplyBmpFromSingleBmp(HBITMAP hbmpold)
{
  BITMAP bmold;
  ::GetObject(hbmpold, sizeof(bmold), &bmold);
  BYTE* newbits = (BYTE*)malloc(bmold.bmWidthBytes * bmold.bmHeight);
  BYTE* oldbits = (BYTE*)bmold.bmBits;

  BITMAPINFOHEADER bmih;
  HBITMAP hbmpnew;
  memset(&bmih, 0, sizeof(bmih));
  bmih.biSize = sizeof(BITMAPINFOHEADER);
  bmih.biWidth = bmold.bmWidth;
  bmih.biHeight = bmold.bmHeight * 4;
  bmih.biPlanes = bmold.bmPlanes;
  bmih.biBitCount = bmold.bmBitsPixel;
  bmih.biCompression = BI_RGB;
  hbmpnew = CreateDIBSection(NULL, (BITMAPINFO*)&bmih, DIB_RGB_COLORS, (void**)&newbits, 0, 0);

  int part = 0;
  for (int ynew = 0, yold = 0; ynew != bmold.bmHeight * 4; ++ynew, ++yold)
  {
    BYTE* newPixel = newbits + bmold.bmWidthBytes * ynew;
    if (yold == bmold.bmHeight)
    {
      ++part;
      yold = 0;
    }
    BYTE* oldPixel = oldbits + bmold.bmWidthBytes * yold;

    for (int x = 0; x != bmold.bmWidth; ++x)
    {
      newPixel[0] = oldPixel[0];
      newPixel[1] = oldPixel[1];
      newPixel[2] = oldPixel[2];
      newPixel[3] = oldPixel[3];

      switch (part)
      {
      case 1:
        ChangeBmpAlpha(newPixel);
        break;
      case 2:
        ChangeBmpBrightness(newPixel);
        break;
      default:
        break;
      }

      newPixel += 4;
      oldPixel += 4;
    }
  }
  
  DeleteObject(hbmpold);
  if (hbmpnew)
    return hbmpnew;
}

void MakeMultiplyBmp::ChangeBmpAlpha(BYTE* pPixel)
{
  if (pPixel[3] == 255)
    pPixel[3] = m_alpha;
}

void MakeMultiplyBmp::ChangeBmpBrightness(BYTE* pPixel)
{ 
  if (pPixel[0] != 0 || pPixel[1] != 0 || pPixel[2] != 0)
  {
    pPixel[0] = min(pPixel[0] * m_brightness, 255);
    pPixel[1] = min(pPixel[1] * m_brightness, 255);
    pPixel[2] = min(pPixel[2] * m_brightness, 255);
  }
}

void MakeMultiplyBmp::SetBmpAlpha(int iAlpha)
{
  m_alpha = iAlpha;
}

void MakeMultiplyBmp::SetBmpBrightness(double iBrightness)
{
  m_brightness = iBrightness;
}