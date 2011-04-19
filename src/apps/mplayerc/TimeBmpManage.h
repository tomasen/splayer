#pragma once

class TimeBmpManage
{
public:
  TimeBmpManage(void);
  ~TimeBmpManage(void);

  BITMAP CreateLeftTimeBmp(CDC& dc);
  BITMAP CreateRightTimeBmp(CDC& dc);

  CDC* CreateBmp(CDC& dc, BITMAP& bm, std::wstring bmpname, HBITMAP oldbmp);

  void DeleteDcMem(CDC* dcmem, HBITMAP oldbmp);

  void SetPlaytime(REFERENCE_TIME rTimeCur, REFERENCE_TIME rTimeStop);

  void ParseDigital(BYTE time, std::wstring& firststr, std::wstring& secondstr);
  void ToggleDisplayType();
  int GetRightTimeBmpWidth();

private:
  DVD_HMSF_TIMECODE m_tc;
  DVD_HMSF_TIMECODE m_tcStop;
  BOOL              m_bshortmovie;
  int               m_nRightTimeBmpWidth;
};
