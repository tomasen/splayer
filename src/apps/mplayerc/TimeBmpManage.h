#pragma once

class TimeBmpManage
{
public:
  TimeBmpManage(void);
  ~TimeBmpManage(void);

  BITMAP CreateTimeBmp(CDC& dc, BOOL bRemain);

  CDC* CreateBmp(CDC& dc, BITMAP& bm, std::wstring bmpname, HBITMAP oldbmp);

  void DeleteDcMem(CDC* dcmem, HBITMAP oldbmp);

  void SetPlaytime(REFERENCE_TIME rTimeCur, REFERENCE_TIME rTimeStop);

  void ParseDigital(BYTE time, std::wstring& firststr, std::wstring& secondstr);

private:
  DVD_HMSF_TIMECODE m_tc;
  HBITMAP           m_oldbmp;
  BOOL              m_bshortmovie;
};
