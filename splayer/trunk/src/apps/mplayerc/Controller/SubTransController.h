#ifndef SUBTRANSCONTROLLER_H
#define SUBTRANSCONTROLLER_H

class SubTransController
{
public:
  SubTransController(void);
  ~SubTransController(void);

  typedef enum {
    Unknown,
    DownloadSubtitle,
    UploadSubtitle
  } SubTransOperation;

  typedef std::vector<std::wstring> StringList;

  void SetFrame(HWND hwnd);

  // starting and ending main thread for upload / download
  void Start(const wchar_t* video_filename, SubTransOperation operation,
             StringList files_upload = StringList());
  void Stop();

  // primary thread logics, should not be called directly
  static void _thread_dispatch(void* param);
  void _thread();

private:
  void _thread_download();
  void _thread_upload();

  SubTransOperation m_operation;

  HANDLE  m_thread;
  HANDLE  m_stopevent;
};

#endif // SUBTRANSCONTROLLER_H