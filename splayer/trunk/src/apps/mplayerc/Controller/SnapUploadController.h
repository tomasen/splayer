#ifndef SNAPUPLOADCONTROLLER_H
#define SNAPUPLOADCONTROLLER_H

#define ID_CONTROLLER_SAVE_IMAGE 32930

class SnapUploadController
{
public:
  SnapUploadController(void);
  ~SnapUploadController(void);

  // retrieve location to store screenshot files
  std::wstring GetTempDir();
  // set frame window HWND
  void SetFrame(HWND hwnd);

  // starting and ending capture & uploading
  void Start(const wchar_t* hash_str);
  void Stop();

  // set and notify current play time
  void SetCurTime(int curtime, long cursystime);

  // get hash str
  std::wstring GetHashStr();

  // get movie total time
  unsigned int GetTotalTime();

  // primary thread logics, should not be called directly
  static void _thread_dispatch(void* param);
  void _thread();

  // remove item in m_shottime
  void RemoveShotTime(int usedtime);

  // set the last saved snap file
  void SetLastSnapFile(std::wstring fn);

  // upload the snap file
  void UploadImage();

private:
  HWND            m_frame;
  std::wstring    m_hash_str;
  std::wstring    m_lastsavedsanp;
  unsigned int    m_curtime;
  unsigned int    m_totaltime;
  long            m_cursystime;
  HANDLE          m_thread;
  HANDLE          m_stopevent;
  std::vector<unsigned int> m_shottime;
};

#endif // SNAPUPLOADCONTROLLER_H