#include "stdafx.h"
#include "MakeMultiplyBmp.h"


////////////////////////////////////////////////////////////////////
////This class can make a multiply bitmap from a single bitmap,
////this bitmap is a 32 bits picture with alpha blending.
////A multiply bitmap has 4 pictures for button's 4 stat one by one.
////////////////////////////////////////////////////////////////////

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

  if (bmold.bmBitsPixel != 32)
    return 0;

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

  MakeDifferentStatBitmap(newbits, oldbits, 0, bmold.bmWidth, bmold.bmHeight);
  MakeDifferentStatBitmap(newbits + bmold.bmWidth * bmold.bmHeight * 4, 
    oldbits, 1, bmold.bmWidth, bmold.bmHeight);
  MakeDifferentStatBitmap(newbits + bmold.bmWidth * bmold.bmHeight * 4 * 2,
    oldbits, 2, bmold.bmWidth, bmold.bmHeight);
  MakeDifferentStatBitmap(newbits + bmold.bmWidth * bmold.bmHeight * 4 * 3,
    oldbits, 3, bmold.bmWidth, bmold.bmHeight);

  DeleteObject(hbmpold);
  if (hbmpnew)
    return hbmpnew;
}

void MakeMultiplyBmp::MakeDifferentStatBitmap(BYTE* newbits, BYTE* oldbits, int stat, 
                                              int bmWidth, int bmHeight)
{
  switch (stat)
  {
  case 0:
  case 2:
  case 3:
    for (int ynew = 0, yold = 0; ynew != bmHeight; ++ynew, ++yold)
    {
      BYTE* newPixel = newbits + bmWidth * 4 * ynew;
      BYTE* oldPixel = oldbits + bmWidth * 4 * yold;
      for (int x = 0; x != bmWidth; ++x)
      {
        newPixel[0] = oldPixel[0];
        newPixel[1] = oldPixel[1];
        newPixel[2] = oldPixel[2];
        newPixel[3] = oldPixel[3];
        if (stat == 2)
         ChangeBmpBrightness(newPixel);
        newPixel += 4;
        oldPixel += 4;
      }
    }
    break;
  case 1:
    for (int ynew = 0, yold = 1; ynew != bmHeight; ++ynew, ++yold)
    {
      BYTE* newPixel = newbits + bmWidth * 4 * ynew;
      BYTE* oldPixel = oldbits + bmWidth * 4 * yold;
      
      for (int x = 0; x != bmWidth; ++x)
      {
        if (ynew == bmHeight - 1)
        {
          newPixel[0] = 0;
          newPixel[1] = 0;
          newPixel[2] = 0;
          newPixel[3] = 0;
        }
        else
        {
          newPixel[0] = oldPixel[0];
          newPixel[1] = oldPixel[1];
          newPixel[2] = oldPixel[2];
          newPixel[3] = oldPixel[3];
          ChangeBmpAlpha(newPixel);
          newPixel += 4;
          oldPixel += 4;
        }
      }
    }
    break;
  default:
    break;
  }
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