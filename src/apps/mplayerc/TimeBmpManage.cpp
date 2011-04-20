#include "stdafx.h"
#include "TimeBmpManage.h"
#include <ResLoader.h>
#include "Controller\PlayerPreference.h"
#include "Controller\SPlayerDefs.h"

TimeBmpManage::TimeBmpManage(void)
: m_nRightTimeBmpWidth(0)
, m_bshortmovie(FALSE)
{
}

TimeBmpManage::~TimeBmpManage(void)
{
}

void TimeBmpManage::SetPlaytime(REFERENCE_TIME rTimeCur, REFERENCE_TIME rTimeStop)
{
  m_tc = RT2HMSF(rTimeCur);
  m_tcStop = RT2HMSF(rTimeStop);
  if (m_tcStop.bHours == 0)
    m_bshortmovie = TRUE;
  else
    m_bshortmovie = FALSE;
}

BITMAP TimeBmpManage::CreateRightTimeBmp(CDC& dc)
{
  using std::wstring;

  BITMAP bm = {0};
  wstring sCurDisplayType = PlayerPreference::GetInstance()->GetStringVar(STRVAR_TIMEBMP_TYPE);
  if (sCurDisplayType == L"Display_TimeLeft")
  {
    int paintwidth = 0;
    BYTE time[3] = {m_tc.bHours, m_tc.bMinutes, m_tc.bSeconds};
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

    HBITMAP oldbmp;
    CDC* dcmem = CreateBmp(dc, bm, L"playtime_minus.bmp", oldbmp);
    dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
    DeleteDcMem(dcmem, oldbmp);
    paintwidth += bm.bmWidth;

    int i = 0;
    if (m_bshortmovie)
      i = 1;
    for (; i != 3; ++i)
    {
      HBITMAP oldbmp;
      std::wstring firststr;
      std::wstring secondstr;
      std::wstring unused;
      ParseDigital(time[i], firststr, secondstr, unused);

      CDC* dcmem = 0;
      if (!firststr.empty())
      {
        dcmem = CreateBmp(dc, bm, firststr, oldbmp);
        dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
        DeleteDcMem(dcmem, oldbmp);
        paintwidth += bm.bmWidth;
      }

      if (!secondstr.empty())
      {
        dcmem = CreateBmp(dc, bm, secondstr, oldbmp);
        dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
        DeleteDcMem(dcmem, oldbmp);
        paintwidth += bm.bmWidth;
      }

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
  }
  else if (sCurDisplayType == L"Display_TimeTotal")
  {
    int paintwidth = 0;
    BYTE time[3] = {m_tcStop.bHours, m_tcStop.bMinutes, m_tcStop.bSeconds};
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

    int i = 0;
    if (m_bshortmovie)
      i = 1;
    for (; i != 3; ++i)
    {
      HBITMAP oldbmp;
      std::wstring firststr;
      std::wstring secondstr;
      std::wstring unused;
      ParseDigital(time[i], firststr, secondstr, unused);

      CDC* dcmem = 0;
      if (!firststr.empty())
      {
        dcmem = CreateBmp(dc, bm, firststr, oldbmp);
        dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
        DeleteDcMem(dcmem, oldbmp);
        paintwidth += bm.bmWidth;
      }

      if (!secondstr.empty())
      {
        dcmem = CreateBmp(dc, bm, secondstr, oldbmp);
        dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
        DeleteDcMem(dcmem, oldbmp);
        paintwidth += bm.bmWidth;
      }

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
  }
  else if (sCurDisplayType == L"Display_Power")
  {
    SYSTEM_POWER_STATUS status;
    ::GetSystemPowerStatus(&status);
    if (status.BatteryFlag != 128 && status.BatteryFlag != 255 && status.BatteryLifePercent < 101)
    {
      // is notebook
      int paintwidth = 0;
      BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

      HBITMAP oldbmp;
      CDC *dcBatteryMem = CreateBmp(dc, bm, L"playtime_battery.bmp", oldbmp);
      dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcBatteryMem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
      DeleteDcMem(dcBatteryMem, oldbmp);
      paintwidth += bm.bmWidth + 2;

      dcBatteryMem = CreateBmp(dc, bm, L"playtime_percent.bmp", oldbmp);
      dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcBatteryMem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
      DeleteDcMem(dcBatteryMem, oldbmp);
      paintwidth += bm.bmWidth + 1;

      std::wstring firststr;
      std::wstring secondstr;
      std::wstring thirdstr;
      ParseDigital(status.BatteryLifePercent, firststr, secondstr, thirdstr);

      if (!firststr.empty())
      {
        dcBatteryMem = CreateBmp(dc, bm, firststr, oldbmp);
        dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcBatteryMem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
        DeleteDcMem(dcBatteryMem, oldbmp);
        paintwidth += bm.bmWidth;
      }

      if (!secondstr.empty())
      {
        dcBatteryMem = CreateBmp(dc, bm, secondstr, oldbmp);
        dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcBatteryMem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
        DeleteDcMem(dcBatteryMem, oldbmp);
        paintwidth += bm.bmWidth;
      }

      if (!thirdstr.empty())
      {
        dcBatteryMem = CreateBmp(dc, bm, thirdstr, oldbmp);
        dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcBatteryMem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
        DeleteDcMem(dcBatteryMem, oldbmp);
        paintwidth += bm.bmWidth;
      }

      bm.bmWidth = paintwidth;
    }
    else
    {
      // is pc, restore it to the default display type
      PlayerPreference::GetInstance()->SetStringVar(STRVAR_TIMEBMP_TYPE, wstring(L"Display_TimeLeft"));
    }
  }

  m_nRightTimeBmpWidth = bm.bmWidth;

  return bm;
}

BITMAP TimeBmpManage::CreateLeftTimeBmp(CDC& dc)
{
  BITMAP bm;
  int paintwidth = 0;
  BYTE time[3] = {m_tc.bHours, m_tc.bMinutes, m_tc.bSeconds};
  BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

  int i = 0;
  if (m_bshortmovie)
    i = 1;
  for (; i != 3; ++i)
  {
    HBITMAP oldbmp;
    std::wstring firststr;
    std::wstring secondstr;
    std::wstring unused;
    ParseDigital(time[i], firststr, secondstr, unused);

    CDC* dcmem = 0;
   if (!firststr.empty())
    {
      dcmem = CreateBmp(dc, bm, firststr, oldbmp);
      dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
      DeleteDcMem(dcmem, oldbmp);
      paintwidth += bm.bmWidth;
    }

    if (!secondstr.empty())
    {
      dcmem = CreateBmp(dc, bm, secondstr, oldbmp);
      dc.AlphaBlend(paintwidth, 0, bm.bmWidth, bm.bmHeight, dcmem, 0, 0, bm.bmWidth, bm.bmHeight, bf);
      DeleteDcMem(dcmem, oldbmp);
      paintwidth += bm.bmWidth;
    }

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

void TimeBmpManage::ParseDigital(BYTE time, std::wstring& firststr, std::wstring& secondstr, std::wstring &thirdstr)
{
  if (time > 100)
    return;

  if (time == 100)
  {
    firststr = L"playtime_1.bmp";
    secondstr = L"playtime_0.bmp";
    thirdstr = L"playtime_0.bmp";
    return;
  }

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

void TimeBmpManage::ToggleDisplayType()
{
  using std::wstring;
  wstring sCurType = PlayerPreference::GetInstance()->GetStringVar(STRVAR_TIMEBMP_TYPE);
  if (sCurType == L"Display_TimeLeft")
  {
    // change
    PlayerPreference::GetInstance()->SetStringVar(STRVAR_TIMEBMP_TYPE, wstring(L"Display_TimeTotal"));
  } 
  else if (sCurType == L"Display_TimeTotal")
  {
    // change
    SYSTEM_POWER_STATUS status;
    ::GetSystemPowerStatus(&status);
    if (status.BatteryFlag != 128 && status.BatteryFlag != 255 && status.BatteryLifePercent < 101)
    {
      // if it's a notebook
      PlayerPreference::GetInstance()->SetStringVar(STRVAR_TIMEBMP_TYPE, wstring(L"Display_Power"));
    }
    else
    {
      // if it's not a notebook
      PlayerPreference::GetInstance()->SetStringVar(STRVAR_TIMEBMP_TYPE, wstring(L"Display_TimeLeft"));
    }
  }
  else if (sCurType == L"Display_Power")
  {
    // change
    PlayerPreference::GetInstance()->SetStringVar(STRVAR_TIMEBMP_TYPE, wstring(L"Display_TimeLeft"));
  }
  else
  {
    // restore to default, normally should never go here
    PlayerPreference::GetInstance()->SetStringVar(STRVAR_TIMEBMP_TYPE, wstring(L"Display_TimeLeft"));
  }
}

int TimeBmpManage::GetRightTimeBmpWidth()
{
  return m_nRightTimeBmpWidth;
}