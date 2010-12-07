#ifndef SUBTRANSCONTROLLER_H
#define SUBTRANSCONTROLLER_H
#include "NetworkControlerImpl.h"
#include <threadhelper.h>

#define ID_COMPLETE_QUERY_SUBTITLE 32931

class SubTransController:
  public ThreadHelperImpl<SubTransController>,
  public NetworkControlerImpl
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

  // set frame window HWND
  void SetFrame(HWND hwnd);

  void SetSubfile(std::wstring subfile);
  void SetDelayMs(int ms);
  void SetSubperf(std::wstring str);
  void SetLanuage(std::wstring str);

  void SetMsgs(std::list<std::wstring>* msgs);
  // starting and ending main thread for upload / download
  void Start(const wchar_t* video_filename, SubTransOperation operation,
             StringList files_upload = StringList(), int subnum = 1);

  void _Thread();

private:
  void _thread_download();
  void _thread_upload();

  SubTransOperation m_operation;
  int m_subnum;

  HWND    m_frame;

  std::wstring m_subfile;
  std::wstring m_videofile;
  int m_delayms;

  std::wstring m_subperf;
  std::wstring m_language;

  std::list<std::wstring>* m_handlemsgs;

  void UploadSubFileByVideoAndHash(refptr<pool> pool,refptr<task> task,
                                refptr<request> req,
                                std::wstring fnVideoFilePath,
                                std::wstring szFileHash,
                                std::wstring szSubHash,
                                std::vector<std::wstring>* fnSubPaths,
                                int iDelayMS, int sid, std::wstring oem);
  void WetherNeedUploadSub(refptr<pool> pool, refptr<task> task, refptr<request> req,
                        std::wstring fnVideoFilePath, std::wstring szFileHash, 
                        std::wstring fnSubHash, int iDelayMS, int sid, std::wstring oem);
};
#endif // SUBTRANSCONTROLLER_H