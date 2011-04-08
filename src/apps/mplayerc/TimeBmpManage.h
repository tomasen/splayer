#pragma once

class TimeBmpManage
{
public:
  TimeBmpManage(void);
  ~TimeBmpManage(void);

  BITMAP CreateTimeBmp(CDC& dc, BOOL bRemain);

  CDC* CreateBmp(CDC& dc, BITMAP& bm, std::wstring bmpname, HBITMAP oldbmp);

  CDC* CreateDigitalBmp(BYTE time, CDC& dc, BITMAP& bm);

  void DeleteDcMem(CDC* dcmem, HBITMAP oldbmp);

  void SetPlaytime(REFERENCE_TIME rTimeCur, REFERENCE_TIME rTimeStop);

private:
  DVD_HMSF_TIMECODE m_tc;
  HBITMAP           m_oldbmp;
  BOOL              m_bshortmovie;
};
