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
    HBITMAP oldbmp;
    std::wstring firststr;
    std::wstring secondstr;
    ParseDigital(time[i], firststr, secondstr);
    CDC* dcmem = CreateBmp(dc, bm, firststr, oldbmp);
    dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
    DeleteDcMem(dcmem, oldbmp);
    paintwidth += bm.bmWidth;
    dcmem = CreateBmp(dc, bm, secondstr, oldbmp);
    dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
    DeleteDcMem(dcmem, oldbmp);
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

void TimeBmpManage::ParseDigital(BYTE time, std::wstring& firststr, std::wstring& secondstr)
{
  int decimal = time / 10;
  wchar_t s[10];
  _itow(decimal, s, 10);
  std::wstring bmpname(L"playtime_");
  bmpname += s;
  bmpname += L".bmp";
  firststr = bmpname;

  bmpname = bmpname.substr(0, bmpname.find(s));
  int single = time % 10;
  _itow(single, s, 10);
  bmpname += s;
  bmpname += L".bmp";
  secondstr = bmpname;
}

