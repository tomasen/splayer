#include "stdafx.h"
#include "TimeBmpManage.h"
#include <ResLoader.h>

TimeBmpManage::TimeBmpManage(void)
{
}

TimeBmpManage::~TimeBmpManage(void)
{
}

void TimeBmpManage::SetPlaytime(REFERENCE_TIME rTimeCur, REFERENCE_TIME rTimeStop)
{
  m_tc = RT2HMSF(rTimeCur);
  DVD_HMSF_TIMECODE tc = RT2HMSF(rTimeStop);
  if (tc.bHours == 0)
    m_bshortmovie = TRUE;
  else
    m_bshortmovie = FALSE;
}

BITMAP TimeBmpManage::CreateTimeBmp(CDC& dc, BOOL bRemain)
{
  BITMAP bm;
  int paintwidth = 0;
  BYTE time[3] = {m_tc.bHours, m_tc.bMinutes, m_tc.bSeconds};
  BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

  if (bRemain)
  {
    HBITMAP oldbmp;
    CDC* dcmem = CreateBmp(dc, bm, L"playtime_minus.bmp", oldbmp);
    dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
    DeleteDcMem(dcmem, oldbmp);
    paintwidth += bm.bmWidth;
  }

  int i = 0;
  if (m_bshortmovie)
    i = 1;
  for (; i != 3; ++i)
  {
    CDC* dcmem = CreateDigitalBmp(time[i], dc, bm);
    dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
    dcmem->DeleteDC();
    delete dcmem;
    paintwidth += bm.bmWidth;

    if (i != 2)
    {
      HBITMAP oldbmp;
      CDC* dcColon = CreateBmp(dc, bm, L"playtime_colon.bmp", oldbmp);
      dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcColon, 0, 0, bm.bmWidth, bm.bmHeight, bf);
      DeleteDcMem(dcColon, oldbmp);
      paintwidth += bm.bmWidth;
    }
  }

  bm.bmWidth = paintwidth;
  return bm;
}

CDC* TimeBmpManage::CreateBmp(CDC& dc, BITMAP& bm, std::wstring bmpname, HBITMAP oldbmp)
{
  ResLoader resLoad;
  CDC* dcmem = new CDC;
  dcmem->CreateCompatibleDC(&dc);
  CBitmap bmp;
  bmp.Attach(resLoad.LoadBitmap(bmpname));
  bmp.GetBitmap(&bm);
  if (bm.bmBitsPixel != 32)
    return 0;
  for (int y=0; y<bm.bmHeight; y++)
  {
    BYTE * pPixel = (BYTE *) bm.bmBits + bm.bmWidth * 4 * y;
    for (int x=0; x<bm.bmWidth; x++)
    {
      pPixel[0] = pPixel[0] * pPixel[3] / 255; 
      pPixel[1] = pPixel[1] * pPixel[3] / 255; 
      pPixel[2] = pPixel[2] * pPixel[3] / 255; 
      pPixel += 4;
    }
  }
  oldbmp = (HBITMAP)dcmem->SelectObject(bmp);
  return dcmem;
}

void TimeBmpManage::DeleteDcMem(CDC* dcmem, HBITMAP oldbmp)
{
  dcmem->SelectObject(oldbmp);
  dcmem->DeleteDC();
  delete dcmem;
}

CDC* TimeBmpManage::CreateDigitalBmp(BYTE time, CDC& dc, BITMAP& bm)
{
  BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  CDC* dcmem = new CDC;
  dcmem->CreateCompatibleDC(&dc);

  int decimal = time / 10;
  wchar_t s[10];
  _itow(decimal, s, 10);
  std::wstring bmpname(L"playtime_");
  bmpname += s;
  bmpname += L".bmp";

  BITMAP bm1;
  HBITMAP oldbmp1;
  CDC* dctomem1 = CreateBmp(dc, bm1, bmpname, oldbmp1);
  
  bmpname = bmpname.substr(0, bmpname.find(s));
  int single = time % 10;
  _itow(single, s, 10);
  bmpname += s;
  bmpname += L".bmp";

  BITMAP bm2;
  HBITMAP oldbmp2;
  CDC* dctomem2 = CreateBmp(*dcmem, bm2, bmpname, oldbmp2);
  
  CBitmap cbmp;
  cbmp.CreateCompatibleBitmap(&dc, bm1.bmWidth + bm2.bmWidth, max(bm1.bmHeight, bm2.bmHeight));
  dcmem->SelectObject(cbmp);
  dcmem->AlphaBlend(0, 0, bm1.bmWidth, bm1.bmHeight, dctomem1, 0, 0, bm1.bmWidth, bm1.bmHeight, bf);
  dcmem->AlphaBlend(bm1.bmWidth, 0, bm2.bmWidth, bm2.bmHeight, dctomem2, 0, 0, bm2.bmWidth, bm2.bmHeight, bf);
  DeleteDcMem(dctomem1, oldbmp1);
  DeleteDcMem(dctomem2, oldbmp2);

  bm.bmWidth = bm1.bmWidth + bm2.bmWidth;
  bm.bmHeight = max(bm1.bmHeight, bm2.bmHeight);

  return dcmem;
}