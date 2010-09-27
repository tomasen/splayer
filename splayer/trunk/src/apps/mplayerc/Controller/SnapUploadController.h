#ifndef SNAPUPLOADCONTROLLER_H
#define SNAPUPLOADCONTROLLER_H

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
  void Start(const wchar_t* hash_str, 
             const unsigned int curr_pos_sec, const unsigned int total_sec);
  void Stop(bool synced = false);

  // primary thread logics, should not be called directly
  static void _thread_dispatch(void* param);
  void _thread();

private:
  HANDLE  m_thread;
  HANDLE  m_stopevent;
};

#endif // SNAPUPLOADCONTROLLER_H