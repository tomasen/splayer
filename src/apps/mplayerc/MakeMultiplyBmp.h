#pragma once

class MakeMultiplyBmp
{
public:
  MakeMultiplyBmp(void);
  ~MakeMultiplyBmp(void);

  HBITMAP MakeMultiplyBmpFromSingleBmp(HBITMAP hbmpold);
  void MakeDifferentStatBitmap(BYTE* newbits, BYTE* oldbits, int stat, int bmWidth, int bmHeight);
  void ChangeBmpAlpha(BYTE* pPixel);
  void ChangeBmpBrightness(BYTE* pPixel);
  void SetBmpAlpha(int iAlpha);
  void SetBmpBrightness(double iBrightness);

private:
  int m_alpha;
  double m_brightness;

};
