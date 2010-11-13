#ifndef MAINFRAMESCMD_H
#define MAINFRAMESCMD_H

#include "Controller/SPlayerDefs.h"

template<class T>
class MainFrameSPlayerCmd
{
public:
  void Proc(int cmd, void* ptr)
  {
    switch (cmd)
    {
    case CMD_OPEN_CURRENTPLAYLISTITEM:
      OpenCurrentPlaylistItem();
      break;
    case CMD_OPEN_PLAYLIST:
      OpenPlaylist();
      break;
    }
  }

private:
  void OpenCurrentPlaylistItem()
  {
    T* pthis = static_cast<T*>(this);

    PlaylistController* plcon = PlaylistController::GetInstance();
    std::vector<std::wstring> list = plcon->GetListDisplay();
    if(list.size() == 0)
      return;

    int index = PlayerPreference::GetInstance()->GetIntVar(INTVAR_PLAYLIST_CURRENT);
    if (index < 0 || index >= (int)list.size())
      return;

    //   CPlaylistItem pli;
    //   if(!m_wndPlaylistBar.GetCur(pli)) m_wndPlaylistBar.SetFirst();
    //   if(!m_wndPlaylistBar.GetCur(pli)) return;
    //   AppSettings& s = AfxGetAppSettings();
    //SVP_LogMsg5(L"OpenCurPlaylistItem");
    //   if(rtStart == 0){
    //     CString fn;
    //     fn = pli.m_fns.GetHead();
    //	SVP_LogMsg5(L"GetFav Start1 %s", fn);
    //     favtype ft ;
    //     ft = FAV_FILE;
    //     if (!fn.IsEmpty() && s.autoResumePlay){
    //       CMD5Checksum cmd5;
    //       CStringA szMD5data(fn);
    //       CString szMatchmd5 = cmd5.GetMD5((BYTE*)szMD5data.GetBuffer() , szMD5data.GetLength()).c_str();
    //       szMD5data.ReleaseBuffer();
    //       SVP_LogMsg5(L"GetFav Start %s", szMatchmd5);
    //       CAtlList<CString> sl;
    //       s.GetFav(ft, sl, TRUE);
    //       CString PosStr ;
    //       POSITION pos = sl.GetHeadPosition();
    //       while(pos){
    //         PosStr = sl.GetNext(pos) ;
    //         if( PosStr.Find(szMatchmd5 + _T(";")) == 0 ){
    //           break;
    //         }else{
    //           PosStr.Empty();
    //         }
    //       }
    //       if(!PosStr.IsEmpty()){
    // 
    //         int iPos = PosStr.ReverseFind( _T(';') );
    //         if(iPos >= 0){
    //           CString s2 = PosStr.Right( PosStr.GetLength() - iPos - 1 );
    //           _stscanf(s2, _T("%I64d"), &rtStart); // pos
    //         }
    //         SVP_LogMsg5(L"Got %f", double(rtStart) );
    //       }
    //       m_is_resume_from_last_exit_point = TRUE;
    //       //SVP_LogMsg5(L"GetFav Done");
    //     }
    //   }else if(rtStart == -1){
    //     rtStart = 0;
    //   }
    CAutoPtr<OpenMediaData> omd;

    std::wstring file = list[index];
    transform(file.begin(), file.end(), file.begin(), tolower);

    if(file.find(L"video_ts.ifo") != std::wstring::npos ||
      file.find(L".ratdvd") != std::wstring::npos)
    {
      if(OpenDVDData* p = new OpenDVDData())
      {
        p->path = file.c_str(); 
        omd.Attach(p);
      }
    }

    //   if(pli.m_type == CPlaylistItem::device)
    //   {
    //     if(OpenDeviceData* p = new OpenDeviceData())
    //     {
    //       POSITION pos = pli.m_fns.GetHeadPosition();
    //       for(int i = 0; i < countof(p->DisplayName) && pos; i++)
    //         p->DisplayName[i] = pli.m_fns.GetNext(pos);
    //       p->vinput = pli.m_vinput;
    //       p->vchannel = pli.m_vchannel;
    //       p->ainput = pli.m_ainput;
    //       return p;
    //     }
    //   }
    //   else
    {
      if(OpenFileData* p = new OpenFileData())
      {
        p->fns.AddTail(file.c_str());
        //p->subs.AddTailList(&pli.m_subs);
        //p->rtStart = rtStart;
        omd.Attach(p);
      }
    }

    if (omd)
      pthis->OpenMedia(omd);
  }

  void OpenPlaylist()
  {
    T* pthis = static_cast<T*>(this);
    pthis->OnFileOpenQuick();
  }
};

#endif // MAINFRAMESCMD_H