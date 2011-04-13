#include "stdafx.h"
#include "GraphCore.h"

#include "mplayerc.h"
#include "MainFrm.h"
#include "Controller/HashController.h"

#include "FGManager.h"
#include "KeyProvider.h"
#include "MediaTypesDlg.h"
#include "Utils/ContentType.h"
#include "..\..\filters\filters.h"
#include "..\..\..\include\moreuuids.h"
#include "../../filters/misc/SyncClock/SyncClock.h"

// TODO try to remove
#include "..\..\svplib\SVPToolBox.h"
#include "../../svplib/SVPRarLib.h"

CGraphCore::CGraphCore(void):
  m_fCustomGraph(false),
  m_fRealMediaGraph(false),
  m_fShockwaveGraph(false),
  m_fQuicktimeGraph(false),
  m_iSubtitleSel(-1),
  m_fAudioOnly(0),
  m_iAudioChannelMaping(0),
  m_fOpeningAborted(false),
  m_iMediaLoadState(MLS_CLOSED),
  m_is_resume_from_last_exit_point(false),
  m_fLiveWM(false),
  m_fEndOfStream(false),
  m_iSpeedLevel(0),
  m_rtDurationOverride(-1),
  m_iPlaybackMode(PM_NONE),
  m_iSubtitleSel2(-1)
{
}

CGraphCore::~CGraphCore(void)
{
}

void CGraphCore::MediaTypeDlg()
{
  CComQIPtr<IGraphBuilderDeadEnd> pGBDE = pGB;
  if(pGBDE && pGBDE->GetCount())
    CMediaTypesDlg(pGBDE, GetMainFrame()).DoModal();
}

void CGraphCore::ApplyOptionToCaptureBar(OpenDeviceData* p)
{
  GetCaptureBar()->m_capdlg.SetVideoInput(p->vinput);
  GetCaptureBar()->m_capdlg.SetVideoChannel(p->vchannel);
  GetCaptureBar()->m_capdlg.SetAudioInput(p->ainput);
}

bool CGraphCore::LoadSubtitle(CString fn, int sub_delay_ms, BOOL bIsForPlayList)
{
  CString szBuf;
  szBuf.Format(_T("Loading subtile %s delay %d %s"), fn , sub_delay_ms , ( bIsForPlayList ? _T("for playlist") : _T("") ) );
  SVP_LogMsg(szBuf);

  CComPtr<ISubStream> pSubStream;
  CComPtr<ISubStream> pSubStream2;

  // TMP: maybe this will catch something for those who get a runtime error dialog when opening subtitles from cds
  try
  {
    if(!pSubStream)
    {
      CAutoPtr<CVobSubFile> p(new CVobSubFile(&m_csSubLock));
      if(CString(CPath(fn).GetExtension()).MakeLower() == _T(".idx") && p && p->Open(fn) && p->GetStreamCount() > 0)
        pSubStream = p.Detach();

      CAutoPtr<CVobSubFile> p2(new CVobSubFile(&m_csSubLock2));
      if(CString(CPath(fn).GetExtension()).MakeLower() == _T(".idx") && p2 && p2->Open(fn) && p2->GetStreamCount() > 0)
        pSubStream2 = p2.Detach();
    }

    if(!pSubStream)
    {
      CAutoPtr<CRenderedTextSubtitle> p(new CRenderedTextSubtitle(&m_csSubLock));
      //detect fn charst
      CSVPToolBox svt;
      int cset = svt.DetectFileCharset(fn);

      if(p && p->Open(fn, cset) && p->GetStreamCount() > 0)
        pSubStream = p.Detach();

      CAutoPtr<CRenderedTextSubtitle> p2(new CRenderedTextSubtitle(&m_csSubLock2));
      if(p2 && p2->Open(fn, cset) && p2->GetStreamCount() > 0)
        pSubStream2 = p2.Detach();
    }

    if(!pSubStream)
    {
      CAutoPtr<ssf::CRenderer> p(new ssf::CRenderer(&m_csSubLock));
      if(p && p->Open(fn) && p->GetStreamCount() > 0)
        pSubStream = p.Detach();

      CAutoPtr<ssf::CRenderer> p2(new ssf::CRenderer(&m_csSubLock2));
      if(p2 && p2->Open(fn) && p2->GetStreamCount() > 0)
        pSubStream2 = p2.Detach();
    }
  }
  catch(CException* e)
  {
    e->Delete();
  }


  CSVPToolBox svTool;
  if(!sub_delay_ms){
    //如果没有预设字幕延迟，视图读取 字幕.delay 获得delay参数
    sub_delay_ms = _wtoi ( svTool.fileGetContent( fn+_T(".delay")) );
  }else{
    //如果有字幕延迟， 而且不是playlist subtitles， 保存到.delay文件
    if(!bIsForPlayList){
      szBuf.Format(_T("%d"), sub_delay_ms);
      svTool.filePutContent(  fn+_T(".delay"), szBuf );
    }
  }

  if(pSubStream)
  {
    pSubStream->notSaveDelay = bIsForPlayList;
    pSubStream->sub_delay_ms = sub_delay_ms;
    m_pSubStreams.AddTail(pSubStream);
  }

  if(pSubStream2)
  {
    pSubStream2->notSaveDelay = bIsForPlayList;
    pSubStream2->sub_delay_ms = sub_delay_ms;
    m_pSubStreams2.AddTail(pSubStream2);
  }

  return(!!pSubStream);
}

void CGraphCore::CleanGraph()
{
  if(!pGB) return;

  BeginEnumFilters(pGB, pEF, pBF)
  {
    CComQIPtr<IAMFilterMiscFlags> pAMMF(pBF);
    if(pAMMF && (pAMMF->GetMiscFlags()&AM_FILTER_MISC_FLAGS_IS_SOURCE))
      continue;

    // some capture filters forget to set AM_FILTER_MISC_FLAGS_IS_SOURCE 
    // or to implement the IAMFilterMiscFlags interface
    if(pBF == pVidCap || pBF == pAudCap) 
      continue;

    if(CComQIPtr<IFileSourceFilter>(pBF))
      continue;

    int nIn, nOut, nInC, nOutC;
    if(CountPins(pBF, nIn, nOut, nInC, nOutC) > 0 && (nInC+nOutC) == 0)
    {
      TRACE(CStringW(L"Removing: ") + GetFilterName(pBF) + '\n');

      pGB->RemoveFilter(pBF);
      pEF->Reset();
    }
  }
  EndEnumFilters
}

BOOL CGraphCore::SetVMR9ColorControl(float dBrightness, float dContrast, float dHue, float dSaturation, BOOL silent)
{
  BOOL ret = TRUE;

  if (m_pCAPR)
  {
    if (!silent)
    {
      m_pCAPR->SetPixelShader(NULL, NULL);
      if (m_pCAP2)
        m_pCAP2->SetPixelShader2(NULL, NULL, true);
    }

    if (dContrast != 1.0 || dBrightness != 100.0)
    {
      CStringA szSrcData;
      szSrcData.Format(("sampler s0:register(s0);float4 p0 : register(c0);float4 main(float2 tex : TEXCOORD0) : COLOR { return (tex2D(s0,tex) - 0.3) * %0.3f + 0.3 + %0.3f; }")
        , dContrast , dBrightness / 100 - 1.0  );

      HRESULT hr = m_pCAPR->SetPixelShader(szSrcData, ("ps_2_0"));
      if (FAILED(hr))
        ret = FALSE;
    }
  }
  return ret;
}

void CGraphCore::UpdateSubtitle(bool fApplyDefStyle)
{
  if(!m_pCAP) return;

  int i = m_iSubtitleSel;

  POSITION pos = m_pSubStreams.GetHeadPosition();
  while(pos && i >= 0)
  {
    CComPtr<ISubStream> pSubStream = m_pSubStreams.GetNext(pos);

    if(i < pSubStream->GetStreamCount()) 
    {
      CAutoLock cAutoLock(&m_csSubLock);
      pSubStream->SetStream(i);
      SetSubtitle(pSubStream, fApplyDefStyle);
      return;
    }

    i -= pSubStream->GetStreamCount();
  }
  //SendStatusMessage(_T("主字幕已关闭") , 4000 );
  m_pCAP->SetSubPicProvider(NULL);
}

void CGraphCore::SetSubtitle(ISubStream* pSubStream, bool fApplyDefStyle, bool bShowOSD )
{
  AppSettings& s = AfxGetAppSettings();

  if(pSubStream)
  {
    CLSID clsid;
    pSubStream->GetClassID(&clsid);

    int xalign = 1, yalign = 1;
    if(s.subdefstyle.scrAlignment > 0){
      xalign = (s.subdefstyle.scrAlignment - 1) % 3;
      yalign = 2 - (int)( (s.subdefstyle.scrAlignment - 1) / 3 );
    }

    if(clsid == __uuidof(CVobSubFile))
    {
      CVobSubFile* pVSF = (CVobSubFile*)(ISubStream*)pSubStream;

      if(fApplyDefStyle)
      {
        pVSF->SetAlignment(s.fOverridePlacement, s.nHorPos, s.nVerPos,  xalign, yalign);//1, 1);
      }
    }
    else if(clsid == __uuidof(CVobSubStream))
    {
      CVobSubStream* pVSS = (CVobSubStream*)(ISubStream*)pSubStream;

      if(fApplyDefStyle)
      {
        pVSS->SetAlignment(s.fOverridePlacement, s.nHorPos, s.nVerPos, xalign, yalign);// 1, 1);
      }
    }
    else if(clsid == __uuidof(CRenderedTextSubtitle))
    {
      CRenderedTextSubtitle* pRTS = (CRenderedTextSubtitle*)(ISubStream*)pSubStream;

      STSStyle style;
      CRect nPosRect(0,0,0,0);


      if(fApplyDefStyle || pRTS->m_fUsingAutoGeneratedDefaultStyle)
      {
        style = s.subdefstyle;

        if(s.fOverridePlacement)
        {
          //style.scrAlignment = 2;
          int w = pRTS->m_dstScreenSize.cx;
          int h = pRTS->m_dstScreenSize.cy;
          int mw = w - style.marginRect.left - style.marginRect.right;
          style.marginRect.bottom = h - MulDiv(h, s.nVerPos, 100);
          style.marginRect.left = MulDiv(w, s.nHorPos, 100) - mw/2;
          style.marginRect.right = w - (style.marginRect.left + mw);

          nPosRect = style.marginRect;
        }

        pRTS->SetDefaultStyle(style);

        pRTS->SetAliPos(style.scrAlignment, nPosRect );
      }

      if(pRTS->GetDefaultStyle(style) /*&& style.relativeTo == 2*/)
      {
        style.relativeTo = s.subdefstyle.relativeTo;
        pRTS->SetDefaultStyle(style);
      }

      pRTS->Deinit();
    }
  }

  ///if(!fApplyDefStyle) // no idea why doing this?? -- Tomas
  //{
  m_iSubtitleSel = -1;

  if(pSubStream)
  {

    int i = 0;

    POSITION pos = m_pSubStreams.GetHeadPosition();
    while(pos)
    {
      CComPtr<ISubStream> pSubStream2 = m_pSubStreams.GetNext(pos);

      if(pSubStream == pSubStream2)
      {
        m_iSubtitleSel = i + pSubStream2->GetStream();
        break;
      }

      i += pSubStream2->GetStreamCount();
    }

  }
  //}

  m_nSubtitleId = (DWORD_PTR)pSubStream;

  if(m_pCAP && pSubStream)
  {
    //m_wndSubresyncBar.SetSubtitle(pSubStream, m_pCAP->GetFPS());
    CString szBuf;
    CString subName;
    WCHAR* pName = NULL;
    if(SUCCEEDED(pSubStream->GetStreamInfo(pSubStream->GetStream(), &pName, NULL)))
    {
      subName = CString(pName);
      s.sSubStreamName1 = subName;
      subName.Replace(_T("&"), _T("&&"));
      CoTaskMemFree(pName);
    }

    if(bShowOSD && subName != L"No subtitles"){
      szBuf.Format(ResStr(IDS_OSD_MSG_CURRENT_MAINSUB_INFO), GetAnEasyToUnderstoodSubtitleName( subName),  pSubStream->sub_delay_ms,s.nVerPos);
      SVP_LogMsg(szBuf);
      SendStatusMessage(szBuf , 4000 );
    }

    std::wstring szFileHash = HashController::GetInstance()->GetSPHash(m_fnCurPlayingFile);
    CString szSQLUpdate, szSQLInsert;
    time_t tNow = time(NULL);
    szSQLInsert.Format(L"INSERT OR IGNORE INTO histories  ( fpath, subid, modtime ) VALUES ( \"%s\", '%d', '%d') ", szFileHash.c_str(), m_iSubtitleSel, tNow);
    szSQLUpdate.Format(L"UPDATE histories SET subid = '%d' , modtime = '%d' WHERE fpath = \"%s\" ", m_iSubtitleSel, tNow, szFileHash.c_str());
    if(AfxGetMyApp()->sqlite_local_record )
      AfxGetMyApp()->sqlite_local_record->exec_insert_update_sql_u(szSQLInsert.GetBuffer(), szSQLUpdate.GetBuffer());

    m_pCAP->SetSubPicProvider(CComQIPtr<ISubPicProvider>(pSubStream));
    SetSubtitleDelay(pSubStream->sub_delay_ms); 


  }else{
    //SendStatusMessage(_T("主字幕已关闭") , 4000 );
  }
}

void CGraphCore::SetSubtitleDelay(int delay_ms)
{
  if(m_pCAP) {
    m_pCAP->SetSubtitleDelay(delay_ms);
    getCurPlayingSubfile();
    //CString str;
    //str.Format(_T("主字幕延时已经设为： %d ms"), delay_ms);
    //SendStatusMessage(str, 5000);
  }
  time(&m_tPlayStartTime);
}

void CGraphCore::UpdateSubtitle2(bool fApplyDefStyle)
{
  if(!m_pCAP) return;

  int i = m_iSubtitleSel2;

  POSITION pos = m_pSubStreams2.GetHeadPosition();
  while(pos && i >= 0)
  {
    CComPtr<ISubStream> pSubStream = m_pSubStreams2.GetNext(pos);

    if(i < pSubStream->GetStreamCount()) 
    {
      CAutoLock cAutoLock(&m_csSubLock2);
      pSubStream->SetStream(i);
      SetSubtitle2(pSubStream, fApplyDefStyle);
      return;
    }

    i -= pSubStream->GetStreamCount();
  }
  //SendStatusMessage(_T("第二字幕已关闭") , 4000 );
  m_pCAP->SetSubPicProvider2(NULL);
}

void CGraphCore::SetSubtitle2(ISubStream* pSubStream, bool fApplyDefStyle, bool bShowOSD)
{
  AppSettings& s = AfxGetAppSettings();

  if(pSubStream)
  {
    CLSID clsid;
    pSubStream->GetClassID(&clsid);

    int xalign = 1, yalign = 0;
    if(s.subdefstyle2.scrAlignment > 0){
      xalign = (s.subdefstyle2.scrAlignment - 1) % 3;
      yalign = 2 - (int)( (s.subdefstyle2.scrAlignment - 1) / 3 );
    }
    if(clsid == __uuidof(CVobSubFile))
    {
      CVobSubFile* pVSF = (CVobSubFile*)(ISubStream*)pSubStream;

      if(fApplyDefStyle)
      {
        // 0: left/top, 1: center, 2: right/bottom
        pVSF->SetAlignment(TRUE, s.nHorPos2, s.nVerPos2,xalign, yalign);// 1, 0);
      }
    }
    else if(clsid == __uuidof(CVobSubStream))
    {
      CVobSubStream* pVSS = (CVobSubStream*)(ISubStream*)pSubStream;

      if(fApplyDefStyle)
      {
        pVSS->SetAlignment(TRUE, s.nHorPos2, s.nVerPos2, xalign, yalign);// 1, 0);
      }
    }
    else if(clsid == __uuidof(CRenderedTextSubtitle))
    {
      CRenderedTextSubtitle* pRTS = (CRenderedTextSubtitle*)(ISubStream*)pSubStream;

      STSStyle style;
      CRect nPosRect(0,0,0,0);
      if(fApplyDefStyle || pRTS->m_fUsingAutoGeneratedDefaultStyle)
      {
        style = s.subdefstyle2;


        if(s.fOverridePlacement2)
        {
          //style.scrAlignment = 8; // 1 - 9: as on the numpad, 0: default
          int w = pRTS->m_dstScreenSize.cx;
          int h = pRTS->m_dstScreenSize.cy;
          int mw = w - style.marginRect.left - style.marginRect.right;
          style.marginRect.bottom = h - MulDiv(h, s.nVerPos2, 100);
          style.marginRect.left = MulDiv(w, s.nHorPos2, 100) - mw/2;
          style.marginRect.right = w - (style.marginRect.left + mw);

          nPosRect = style.marginRect;
        }

        pRTS->SetDefaultStyle(style);
        pRTS->SetAliPos(  style.scrAlignment, nPosRect);
      }

      if(pRTS->GetDefaultStyle(style)/* && style.relativeTo == 8*/)
      {
        style.relativeTo = s.subdefstyle2.relativeTo;
        pRTS->SetDefaultStyle(style);
      }


      pRTS->Deinit();
    }
  }

  // 	if(!fApplyDefStyle)
  // 	{
  m_iSubtitleSel2 = -1;

  if(pSubStream)
  {

    int i = 0;

    POSITION pos = m_pSubStreams2.GetHeadPosition();
    while(pos)
    {
      CComPtr<ISubStream> pSubStream2 = m_pSubStreams2.GetNext(pos);

      if(pSubStream == pSubStream2)
      {
        m_iSubtitleSel2 = i + pSubStream2->GetStream();
        break;
      }

      i += pSubStream2->GetStreamCount();
    }

  }
  /*}*/

  m_nSubtitleId2 = (DWORD_PTR)pSubStream;

  if(m_pCAP)
  {
    CString szBuf;
    CString subName;
    WCHAR* pName = NULL;
    if(SUCCEEDED(pSubStream->GetStreamInfo(pSubStream->GetStream(), &pName, NULL)))
    {
      subName = CString(pName);
      s.sSubStreamName2 = subName;
      subName.Replace(_T("&"), _T("&&"));
      CoTaskMemFree(pName);
    }

    if(bShowOSD  && subName != L"No subtitles"){
      szBuf.Format(ResStr(IDS_OSD_MSG_CURRENT_2NDSUB_INFO), GetAnEasyToUnderstoodSubtitleName(subName), pSubStream->sub_delay_ms, s.nVerPos2);
      SVP_LogMsg(szBuf);
      SendStatusMessage(szBuf , 4000 );
    }

    std::wstring szFileHash = HashController::GetInstance()->GetSPHash(m_fnCurPlayingFile);
    CString szSQLUpdate, szSQLInsert;
    time_t tNow = time(NULL);
    szSQLInsert.Format(L"INSERT OR IGNORE INTO histories  ( fpath, subid2, modtime ) VALUES ( \"%s\", '%d', '%d') ", szFileHash.c_str(), m_iSubtitleSel2, tNow);
    szSQLUpdate.Format(L"UPDATE histories SET subid2 = '%d' , modtime = '%d' WHERE fpath = \"%s\" ", m_iSubtitleSel2, tNow, szFileHash.c_str());
    if(AfxGetMyApp()->sqlite_local_record )
      AfxGetMyApp()->sqlite_local_record->exec_insert_update_sql_u(szSQLInsert.GetBuffer(), szSQLUpdate.GetBuffer());

    m_pCAP->SetSubPicProvider2(CComQIPtr<ISubPicProvider>(pSubStream));
    SetSubtitleDelay2(pSubStream->sub_delay_ms); 

  }else{
    //SendStatusMessage(_T("第二字幕已关闭") , 4000 );
  }
}

void CGraphCore::ReplaceSubtitle(ISubStream* pSubStreamOld, ISubStream* pSubStreamNew, int secondSub)
{
  POSITION pos = m_pSubStreams.GetHeadPosition();
  if(secondSub){ pos = m_pSubStreams2.GetHeadPosition();}
  while(pos) 
  {
    POSITION cur = pos;
    if(secondSub){
      if(pSubStreamOld == m_pSubStreams2.GetNext(pos))
      {
        m_pSubStreams2.SetAt(cur, pSubStreamNew);
        UpdateSubtitle2();
        break;
      }
    }else{
      if(pSubStreamOld == m_pSubStreams.GetNext(pos))
      {
        m_pSubStreams.SetAt(cur, pSubStreamNew);
        UpdateSubtitle();
        break;
      }
    }
  }
}

void CGraphCore::InvalidateSubtitle(DWORD_PTR nSubtitleId, REFERENCE_TIME rtInvalidate)
{
  if(m_pCAP)
  {
    if(nSubtitleId == -1 || nSubtitleId == m_nSubtitleId)
      m_pCAP->Invalidate(rtInvalidate);
  }
}

void CGraphCore::ReloadSubtitle()
{
  POSITION pos = m_pSubStreams.GetHeadPosition();
  while(pos) m_pSubStreams.GetNext(pos)->Reload();
  pos = m_pSubStreams2.GetHeadPosition();
  while(pos) m_pSubStreams2.GetNext(pos)->Reload();
  UpdateSubtitle();
  UpdateSubtitle2();
}

void CGraphCore::SetSubtitleDelay2(int delay_ms)
{
  if(m_pCAP) {
    m_pCAP->SetSubtitleDelay2(delay_ms);
    getCurPlayingSubfile(NULL, 2);
    // 		CString str;
    // 		str.Format(_T("第二字幕延时已经设为： %d ms"), delay_ms);
    // 		SendStatusMessage(str, 5000);
  }
  time(&m_tPlayStartTime);
}

long find_string_in_buf ( char *buf, size_t len,
                         const char *s)
{
  long i, j;
  int slen = strlen(s);
  long imax = len - slen - 1;
  long ret = -1;
  int match;

  for (i=0; i<imax; i++) {
    match = 1;
    for (j=0; j<slen; j++) {
      if (buf[i+j] != s[j]) {
        match = 0;
        break;
      }
    }
    if (match) {
      ret = i;
      break;
    }
  }

  return ret;
}

bool CGraphCore::OpenMediaPrivate(CAutoPtr<OpenMediaData> pOMD)
{
  CString err, aborted(_T("Aborted"));
  AppSettings& s = AfxGetAppSettings();

  {
    CAutoLock mOpenCloseLock(&m_csOpenClose);

    s.bIsIVM = false;
    s.szCurrentExtension.Empty();

    if(m_iMediaLoadState != MLS_CLOSED && m_iMediaLoadState != MLS_LOADING)
    {
      ASSERT(0);
      return(false);
    }

    s.bExternalSubtitleTime = false;

    GetSeekBar()->SetRange(0, 100);

    s.szFGMLog.Empty();

    s.SetNumberOfSpeakers(s.iDecSpeakers , -1);

    m_iMediaLoadState = MLS_LOADING;

    GetMainFrame()->SetTimer(CMainFrame::TIMER_LOADING, 2500, NULL);

    // FIXME: Don't show "Closed" initially
    GetMainFrame()->PostMessage(WM_KICKIDLE);

    m_fUpdateInfoBar = false;

    try
    {
      if(m_fOpeningAborted) throw aborted;

      if(OpenFileData* pOFD = dynamic_cast<OpenFileData*>(pOMD.m_p))
      {
        if(pOFD->fns.IsEmpty()) throw ResStr(IDS_MSG_THROW_FILE_NOT_FOUND);

        CString fn = pOFD->fns.GetHead();

        s.szCurrentExtension = fn.Right(4).MakeLower();
        if(s.szCurrentExtension == _T(".ivm"))
          s.bIsIVM = true;

        s.bDisableSoftCAVCForce = false;
        if(!(s.useGPUAcel && s.bHasCUDAforCoreAVC) && !s.bDisableSoftCAVC 
          && s.szCurrentExtension == _T(".mkv"))
        {
          FILE* fp;
          if ( _wfopen_s( &fp, fn, _T("rb")) == 0)
          {
            char matchbuf[0x4000];
            size_t iRead = fread(matchbuf, sizeof( char ), 0x4000 ,fp);
            if( iRead > 200 && find_string_in_buf(matchbuf, iRead-100, "wpredp=2") > 0 )
              s.bDisableSoftCAVCForce = true;

            fclose(fp);
          }
        }

        int i = fn.Find(_T(":\\"));
        if(i > 0)
        {
          CString drive = fn.Left(i+2);
          UINT type = GetDriveType(drive);
          CAtlList<CString> sl;
          if(type == DRIVE_REMOVABLE || type == DRIVE_CDROM && GetCDROMType(drive[0], sl) != CDROM_Audio)
          {
            int ret = IDRETRY;
            while(ret == IDRETRY)
            {
              WIN32_FIND_DATA findFileData;
              HANDLE h = FindFirstFile(fn, &findFileData);
              if(h != INVALID_HANDLE_VALUE)
              {
                FindClose(h);
                ret = IDOK;
              }
              else
              {
                CString msg;
                msg.Format(ResStr(IDS_MSG_WARN_NOT_FOUND_AND_PLS_INSERT_DISK), fn);
                ret = AfxMessageBox(msg, MB_RETRYCANCEL);
              }
            }

            if(ret != IDOK) throw aborted;
          }else{
            CSVPToolBox svpTool;
            if(!svpTool.ifFileExist(fn, true)){
              //SVP_LogMsg5(L"SVP 文件不存在" );
              throw ResStr(IDS_MSG_THROW_FILE_NOT_EXIST);
            }
          }
        }
      }

      if(m_fOpeningAborted) throw aborted;

      OpenCreateGraphObject(pOMD);

      if(m_fOpeningAborted) throw aborted;

      m_pCAP2 = NULL;
      m_pCAP = NULL;
      m_pSVPSub = NULL;

      if(OpenFileData* p = dynamic_cast<OpenFileData*>(pOMD.m_p)) OpenFile(p);
      else if(OpenDVDData* p = dynamic_cast<OpenDVDData*>(pOMD.m_p)) OpenDVD(p);
      else if(OpenDeviceData* p = dynamic_cast<OpenDeviceData*>(pOMD.m_p)) OpenCapture(p);
      else throw _T("Can't open, invalid input parameters");


      pGB->FindInterface(__uuidof(ISubPicAllocatorPresenter), (void**)&m_pCAP, FALSE);
      pGB->FindInterface(__uuidof(ISubPicAllocatorPresenterRender), (void**)&m_pCAPR, TRUE);
      pGB->FindInterface(__uuidof(ISubPicAllocatorPresenter2), (void**)&m_pCAP2, TRUE);
      pGB->FindInterface(__uuidof(IVMRMixerControl9),			(void**)&m_pMC,  TRUE);


      if(!m_pCAP){
        CComQIPtr<ISubPicAllocatorPresenter> pCAP =  FindFilter(__uuidof(CSVPSubFilter), pGB);
        if(pCAP){
          m_pCAP = pCAP;
          CComQIPtr<ISVPSubFilter> pSVPSub= pCAP;
          if(pSVPSub)
            m_pSVPSub = pSVPSub;
        }
      }

      if (m_pMC)
      {
        if (SetVMR9ColorControl(s.dBrightness, s.dContrast, s.dHue, s.dSaturation) == FALSE)
          OsdMsg_SetShader();
        else
        {
          CString  szMsg;
          szMsg.Format(ResStr(IDS_OSD_MSG_BRIGHT_CONTRAST_CHANGED), s.dBrightness, s.dContrast);
          SendStatusMessage(szMsg, 3000);
        }
        SetShaders(true);
      }
      // === EVR !
      pGB->FindInterface(__uuidof(IMFVideoDisplayControl), (void**)&m_pMFVDC,  TRUE);
      if (m_pMFVDC)
      {
        RECT		Rect;
        ::GetClientRect (GetVideoView()->m_hWnd, &Rect);
        m_pMFVDC->SetVideoWindow (GetVideoView()->m_hWnd);
        m_pMFVDC->SetVideoPosition(NULL, &Rect);
      }


      if(m_fOpeningAborted) throw aborted;

      OpenCustomizeGraph();

      if(m_fOpeningAborted) throw aborted;

      OpenSetupVideo();

      if(m_fOpeningAborted) throw aborted;

      OpenSetupAudio();

      if(m_fOpeningAborted) throw aborted;

      if(m_fOpeningAborted) throw aborted;

      if(m_iAudioChannelMaping)
        GetMainFrame()->OnAudioChannalMapMenu(IDS_AUDIOCHANNALMAPNORMAL+m_iAudioChannelMaping);

      m_iMediaLoadState = MLS_LOADED;

      time(&m_tPlayStartTime);

      GetMainFrame()->PostMessage(WM_COMMAND, ID_PLAY_PAUSE);

      if(!(AfxGetAppSettings().nCLSwitches&CLSW_OPEN))
        GetMainFrame()->PostMessage(WM_COMMAND, ID_PLAY_PLAY);

      AfxGetAppSettings().nCLSwitches &= ~CLSW_OPEN;

      if(OpenFileData* p = dynamic_cast<OpenFileData*>(pOMD.m_p))
      {
        if(p->rtStart > 0)
          GetMainFrame()->PostMessage(WM_RESUMEFROMSTATE, (WPARAM)PM_FILE, (LPARAM)(p->rtStart/10000)); // REFERENCE_TIME doesn't fit in LPARAM under a 32bit env.
      }
      else if(OpenDVDData* p = dynamic_cast<OpenDVDData*>(pOMD.m_p))
      {
        if(p->pDvdState)
          GetMainFrame()->PostMessage(WM_RESUMEFROMSTATE, (WPARAM)PM_DVD, (LPARAM)(CComPtr<IDvdState>(p->pDvdState).Detach())); // must be released by the called message handler
      }
      else if(OpenDeviceData* p = dynamic_cast<OpenDeviceData*>(pOMD.m_p))
        ApplyOptionToCaptureBar(p);

      if(m_pCAP && (!m_fAudioOnly || m_fRealMediaGraph))
      {
        POSITION pos = pOMD->subs.GetHeadPosition();
        while(pos){ LoadSubtitle(pOMD->subs.GetNext(pos));}
      }

      if(::GetCurrentThreadId() == AfxGetApp()->m_nThreadID)
        GetMainFrame()->OnFilePostOpenmedia();
      else
        GetMainFrame()->PostMessage(WM_COMMAND, ID_FILE_POST_OPENMEDIA);

      OpenSetupWindowTitle(pOMD->title);

      time_t tOpening = time(NULL);
      while(m_iMediaLoadState != MLS_LOADED 
        && m_iMediaLoadState != MLS_CLOSING // FIXME
        )
      {
        if( (time(NULL) - tOpening) > 10)
          throw aborted;
        Sleep(50);
      }
    }
    catch(LPCTSTR msg)
    {
      err = msg;
    }
    catch(CString msg)
    {
      err = msg;
    }
  }
  if(!err.IsEmpty())
  {
    SendStatusMessage(err, 3000);
    CloseMediaPrivate();
    m_closingmsg = err;

    OpenFileData* p = dynamic_cast<OpenFileData*>(pOMD.m_p);
    if(p && err != aborted)
    {
      GetPlaylistBar()->SetCurValid(false);
      if(GetPlaylistBar()->GetCount() > 1)
      {
        CPlaylistItem pli[2];
        GetPlaylistBar()->GetCur(pli[0]);
        GetPlaylistBar()->SetNext();
        GetPlaylistBar()->GetCur(pli[1]);
        if(pli[0].m_id != pli[1].m_id)
        {
          //CAutoPtr<OpenMediaData> p(m_wndPlaylistBar.GetCurOMD());
          //if(p) OpenMediaPrivate(p);
        }
      }
    }
  }
  else
  {
    GetPlaylistBar()->SetCurValid(true);
  }

  GetMainFrame()->PostMessage(WM_KICKIDLE); // calls main thread to update things

  return(err.IsEmpty());
}

void CGraphCore::CloseMediaPrivate()
{
  CAutoLock mOpenCloseLock(&m_csOpenClose);
  SVP_LogMsg5(L"CloseMediaPrivate");
  m_iMediaLoadState = MLS_CLOSING;

  OnPlayStop(); // SendMessage(WM_COMMAND, ID_PLAY_STOP);

  m_iPlaybackMode = PM_NONE;
  m_iSpeedLevel = 0;

  m_fLiveWM = false;

  m_fEndOfStream = false;

  m_rtDurationOverride = -1;

  m_kfs.RemoveAll();

  m_pCB = NULL;

  m_is_resume_from_last_exit_point = false;

  //	if(pVW) pVW->put_Visible(OAFALSE);
  //	if(pVW) pVW->put_MessageDrain((OAHWND)NULL), pVW->put_Owner((OAHWND)NULL);

  m_pCAP = NULL; // IMPORTANT: IVMRSurfaceAllocatorNotify/IVMRSurfaceAllocatorNotify9 has to be released before the VMR/VMR9, otherwise it will crash in Release()
  m_pCAP2  = NULL;
  m_pCAPR = NULL;
  m_pMC	 = NULL;
  m_pMFVDC = NULL;
  m_pSVPSub = NULL;

  pAMXBar.Release(); pAMTuner.Release(); pAMDF.Release();
  pAMVCCap.Release(); pAMVCPrev.Release(); pAMVSCCap.Release(); pAMVSCPrev.Release(); pAMASC.Release();
  pVidCap.Release(); pAudCap.Release();
  pCGB.Release();
  pDVDC.Release(); pDVDI.Release();
  pQP.Release(); pBI.Release(); pAMOP.Release(); pFS.Release();
  pMC.Release(); pME.Release(); pMS.Release();
  pVW.Release(); pBV.Release();
  pBA.Release();

  m_pRefClock = NULL;
  m_pSyncClock = NULL;

  try{
    if (AfxGetAppSettings().szCurrentExtension != L".csf")
    {
      if(pGB) pGB->RemoveFromROT();
      //UnloadExternalObjects();
    }

    pGB.Release();
    m_fRealMediaGraph = m_fShockwaveGraph = m_fQuicktimeGraph = false;
  }catch(...){}

  m_pSubClock = NULL;

  m_pProv.Release();

  {
    CAutoLock cAutoLock(&m_csSubLock);
    m_pSubStreams.RemoveAll();

    CAutoLock cAutoLock2(&m_csSubLock2);
    m_pSubStreams2.RemoveAll();
  }

  m_VidDispName.Empty();
  m_AudDispName.Empty();

  m_closingmsg = ResStr(IDS_CONTROLS_CLOSED);


  AfxGetAppSettings().bIsIVM = false;
  AfxGetAppSettings().szCurrentExtension.Empty();
  AfxGetAppSettings().nCLSwitches &= CLSW_OPEN|CLSW_PLAY|CLSW_AFTERPLAYBACK_MASK|CLSW_NOFOCUS|CLSW_HTPCMODE;

  m_iMediaLoadState = MLS_CLOSED;

  SetThreadExecutionState(0); //this is the right way, only this work under vista . no ES_CONTINUOUS  so it can goes to sleep when not playing

}

void CGraphCore::OpenCreateGraphObject(OpenMediaData* pOMD)
{
  ASSERT(pGB == NULL);

  m_fCustomGraph = false;
  m_fRealMediaGraph = m_fShockwaveGraph = m_fQuicktimeGraph = false;

  AppSettings& s = AfxGetAppSettings();

  if(OpenFileData* p = dynamic_cast<OpenFileData*>(pOMD))
  {
    engine_t engine = s.Formats.GetEngine(p->fns.GetHead());

    std::wstring ct = ContentType::Get(p->fns.GetHead());

    if(ct == L"video/x-ms-asf")
    {
      // TODO: put something here to make the windows media source filter load later
    }
    else if (ct == L"audio/x-pn-realaudio"
      || ct == L"audio/x-pn-realaudio-plugin"
      || ct == L"audio/x-realaudio-secure"
      || ct == L"video/vnd.rn-realvideo-secure"
      || ct == L"application/vnd.rn-realmedia"
      || ct.find(L"vnd.rn-") != ct.npos
      || ct.find(L"realaudio") != ct.npos
      || ct.find(L"realvideo") != ct.npos)
    {
      // TODO: go fuck this!!!
      engine = RealMedia;
    }
    else if (ct == L"application/x-shockwave-flash")
    {
      engine = ShockWave;
    }
    else if (ct == L"video/quicktime"
      || ct == L"application/x-quicktimeplayer")
    {
      engine = QuickTime;
    }
    if (engine == RealMedia)
      engine = DirectShow;

    SVP_LogMsg5(L"got content type %s %d", ct.c_str(), engine );

    HRESULT hr;
    CComPtr<IUnknown> pUnk;

    if(engine == RealMedia)
    {			
      if(!(pUnk = (IUnknown*)(INonDelegatingUnknown*)new CRealMediaGraph(GetVideoView()->m_hWnd, hr)))
        throw _T("Out of memory");

      if(SUCCEEDED(hr) && !!(pGB = CComQIPtr<IGraphBuilder>(pUnk)))
        m_fRealMediaGraph = true;
    }
    else if(engine == ShockWave)
    {
      if(!(pUnk = (IUnknown*)(INonDelegatingUnknown*)new CShockwaveGraph(GetVideoView()->m_hWnd, hr)))
        throw _T("Out of memory");

      if(FAILED(hr) || !(pGB = CComQIPtr<IGraphBuilder>(pUnk)))
        throw _T("Can't create shockwave control");

      m_fShockwaveGraph = true;
    }
    else if(engine == QuickTime)
    {
      if(!(pUnk = (IUnknown*)(INonDelegatingUnknown*)new CQuicktimeGraph(GetVideoView()->m_hWnd, hr)))
        throw _T("Out of memory");

      if(SUCCEEDED(hr) && !!(pGB = CComQIPtr<IGraphBuilder>(pUnk)))
        m_fQuicktimeGraph = true;
    }

    m_fCustomGraph = m_fRealMediaGraph || m_fShockwaveGraph || m_fQuicktimeGraph;

    if(!m_fCustomGraph)
    {
      pGB = new CFGManagerPlayer(_T("CFGManagerPlayer"), NULL, s.SrcFilters, s.TraFilters, GetVideoView()->m_hWnd);
    }
  }
  else if(OpenDVDData* p = dynamic_cast<OpenDVDData*>(pOMD))
  {
    pGB = new CFGManagerDVD(_T("CFGManagerDVD"), NULL, s.SrcFilters, s.TraFilters, GetVideoView()->m_hWnd);
  }
  else if(OpenDeviceData* p = dynamic_cast<OpenDeviceData*>(pOMD))
  {
    pGB = new CFGManagerCapture(_T("CFGManagerCapture"), NULL, s.SrcFilters, s.TraFilters, GetVideoView()->m_hWnd);
  }

  if(!pGB)
  {
    throw _T("Failed to create the filter graph object");
  }

  pGB->AddToROT();

  pMC = pGB; pME = pGB; pMS = pGB; // general
  pVW = pGB; pBV = pGB; // video
  pBA = pGB; // audio
  pFS = pGB;

  if(!(pMC && pME && pMS)
    || !(pVW && pBV)
    || !(pBA))
  {
    throw ResStr(IDS_MSG_THROW_BROKEN_DIRECTX_SUPPORT);
  }

  if(FAILED(pME->SetNotifyWindow((OAHWND)GetMainFrame()->m_hWnd, WM_GRAPHNOTIFY, 0)))
  {
    throw _T("Could not set target window for graph notification");
  }

  m_pProv = (IUnknown*)new CKeyProvider();

  if(CComQIPtr<IObjectWithSite> pObjectWithSite = pGB)
    pObjectWithSite->SetSite(m_pProv);

  m_pCB = new CDSMChapterBag(NULL, NULL);
}

HRESULT CGraphCore::OpenMMSUrlStream(CString szFn){
  HRESULT ret = E_FAIL;
  IBaseFilter* pSrcFilter;
  IFileSourceFilter* pReader;
  CoCreateInstance(CLSID_WMAsfReader, NULL, CLSCTX_INPROC, IID_IBaseFilter, 
    (LPVOID *)&pSrcFilter);
  pGB->AddFilter(pSrcFilter,_T( "WMASFReader" ));

  ret = pSrcFilter->QueryInterface( IID_IFileSourceFilter, (LPVOID *)&pReader );
  if (FAILED(ret))
  {
    pSrcFilter->Release();
    return ret;
  }

  pReader->Load(szFn, NULL);
  {
    IEnumPins  *pEnum = NULL;
    IPin       *pPin = NULL;
    HRESULT    hr;

    hr = pSrcFilter->EnumPins(&pEnum);
    if (FAILED(hr))
    {
      return hr;
    }
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
      PIN_DIRECTION PinDirThis;
      hr = pPin->QueryDirection(&PinDirThis);
      if (FAILED(hr))
      {
        pPin->Release();
        ret = hr;
        break;
      }
      if (PINDIR_OUTPUT == PinDirThis)
      {
        // Found a match. Return the IPin pointer to the caller.
        HRESULT hr2 = pGB->Render(pPin);
        if (SUCCEEDED(hr2))
        {
          ret = S_OK;
        }else{

        }
      }
      // Release the pin for the next time through the loop.
      pPin->Release();

    }
    // No more pins. We did not find a match.
    pEnum->Release();

  }
  pSrcFilter->Release();
  pReader->Release();
  return ret;
}

void CGraphCore::OpenFile(OpenFileData* pOFD)
{
  if(pOFD->fns.IsEmpty())
    throw _T("Invalid argument");

  AppSettings& s = AfxGetAppSettings();

  bool fFirst = true;

  POSITION pos = pOFD->fns.GetHeadPosition();
  while(pos)
  {
    CString fn = pOFD->fns.GetNext(pos);

    fn.Trim();
    if(fn.IsEmpty() && !fFirst)
      break;

    CString fnLower = fn;
    fnLower.MakeLower();

    HRESULT hr = -1;
    if(FAILED(hr) && ( fnLower.Find(_T("mms://")) == 0 || fnLower.Find(_T("mmsh://")) == 0 )){ // 
      //render mms our own way
      hr = OpenMMSUrlStream(fn);
    }
    if(FAILED(hr))
      hr = pGB->RenderFile(CStringW(fn), NULL);

    if(FAILED(hr) && ( fnLower.Find(_T("http") == 0 || fnLower.Find(_T("https://")) == 0
      || fnLower.Find(_T("udp://")) == 0 || fnLower.Find(_T("tcp://")) == 0)) ){ // 
        //render mms our own way
        hr = OpenMMSUrlStream(fn);
    }
    /* not sure why this is not work for http youku etc
    HRESULT hr = -1;
    if( ( fn.MakeLower().Find(_T("mms://")) == 0 || fn.MakeLower().Find(_T("mmsh://")) == 0 || (fn.MakeLower().Find(_T("http:")) == 0 && fn.MakeLower().Find(_T(":8902")) > 0 ))){ 
    //render mms our own way	
    hr = OpenMMSUrlStream(fn);
    }

    if(FAILED(hr))
    hr = pGB->RenderFile(CStringW(fn), NULL);
    */

    if(FAILED(hr))
    {
      if(fFirst)
      {
        //if(s.fReportFailedPins)
        MediaTypeDlg();
        CString err;

        switch(hr)
        {
        case E_ABORT: err = ResStr(IDS_MSG_THROW_OPRATION_CANCELED); break;
        case E_FAIL: case E_POINTER: default: 
          err.Format(ResStr(IDS_MSG_THROW_UNABLE_OPEN_FILE) , hr);
          break;
        case E_INVALIDARG: err = ResStr(IDS_MSG_THROW_ILLEGE_FILENAME); break;
        case E_OUTOFMEMORY: err = ResStr(IDS_MSG_THROW_OUTOF_MEMORY); break;
        case VFW_E_CANNOT_CONNECT: err = ResStr(IDS_MSG_THROW_UNABLE_DECODE); break;
        case VFW_E_CANNOT_LOAD_SOURCE_FILTER: err = ResStr(IDS_MSG_THROW_UNSUPPORT_SOURCE); break;
        case VFW_E_CANNOT_RENDER: err = ResStr(IDS_MSG_THROW_FAIL_CREATE_RENDER); break;
        case VFW_E_INVALID_FILE_FORMAT: err = _T("Invalid file format"); break;
        case VFW_E_NOT_FOUND: err = ResStr(IDS_MSG_THROW_FILE_NOT_FOUND); break;
        case VFW_E_UNKNOWN_FILE_TYPE: err = ResStr(IDS_MSG_THROW_UNKNOWN_FILE_TYPE); break;
        case VFW_E_UNSUPPORTED_STREAM: err = ResStr(IDS_MSG_THROW_UNSUPPORT_STREAM_TYPE); break;
        }

        throw err;
      }
    }

    if(s.fKeepHistory)
    {
      if(this->m_lastUrl == fn){
        CRecentFileList* pMRU = &s.MRUUrl;
        pMRU->ReadList();
        pMRU->Add(fn);
        pMRU->WriteList();
        this->m_lastUrl.Empty();
      }else{
        CRecentFileList* pMRU = fFirst ? &s.MRU : &s.MRUDub;
        pMRU->ReadList();
        pMRU->Add(fn);
        pMRU->WriteList();
      }
    }

    if(fFirst)
    {
      AppSettings& s = AfxGetAppSettings();
      pOFD->title = fn;
      m_fnCurPlayingFile = fn;
      //是否有字幕？ 沒有则下载字幕
      CSVPToolBox svpTool;
      //搜索目录下同名字幕
      CAtlArray<CString> subSearchPaths;
      subSearchPaths.Add(_T("."));
      subSearchPaths.Add(s.GetSVPSubStorePath());
      subSearchPaths.Add(svpTool.GetPlayerPath(L"SVPSub"));
      subSearchPaths.Add(_T(".\\subtitles"));
      subSearchPaths.Add(_T(".\\Subs"));
      subSearchPaths.Add(_T("c:\\subtitles"));

      CAtlArray<SubFile> ret;

      POSITION pos = pOFD->subs.GetHeadPosition();
      while(pos){
        POSITION cur = pos;
        CString szSubFn = pOFD->subs.GetNext(pos);
        if(!svpTool.ifFileExist(szSubFn))
          pOFD->subs.RemoveAt(cur);
      }
      CSVPRarLib svpRar;
      if( svpRar.SplitPath(fn) ){
        GetSubFileNames(svpRar.m_fnRAR, subSearchPaths, ret);
        CAtlArray<SubFile> ret2;
        GetSubFileNames(svpRar.m_fnInsideRar, subSearchPaths, ret2);
        ret.Append(ret2);
      }else{
        GetSubFileNames(fn, subSearchPaths, ret);
        //AfxMessageBox(fn);
      }
      for(size_t i = 0; i < ret.GetCount(); i++){
        SubFile szBuf = ret.GetAt(i);
        //AfxMessageBox(szBuf.fn);
        if ( pOFD->subs.Find( szBuf.fn ) == NULL && svpTool.ifFileExist(szBuf.fn)){
          pOFD->subs.AddTail(szBuf.fn);
          //AfxMessageBox(szBuf.fn);
        }
      }
      //AfxMessageBox(_T("1"));
      if ( pOFD->subs.GetCount() <= 0){
        //	AfxMessageBox(_T("2"));


        if(s.autoDownloadSVPSub){
          CPath fPath(fn);
          CString szExt;
          szExt.Format(_T(" %s;"),fPath.GetExtension());
          if(s.CheckSVPSubExts.Find(szExt) >= 0 ){
            SVPSubDownloadByVPath(fn);
          }else{
            //SendStatusMessage(  _T("正在播放的文件类型看来不需要字幕，终止自动智能匹配"), 1000);
          }


        }
      }

    }

    fFirst = false;

    if(m_fCustomGraph) break;
  }

  //if(s.fReportFailedPins)
  MediaTypeDlg();

  if(!(pAMOP = pGB))
  {
    BeginEnumFilters(pGB, pEF, pBF)
      if(pAMOP = pBF) break;
    EndEnumFilters
  }

  if(FindFilter(__uuidof(CShoutcastSource), pGB))
    m_fUpdateInfoBar = true;

  SetupChapters();

  CComQIPtr<IKeyFrameInfo> pKFI;
  BeginEnumFilters(pGB, pEF, pBF)
    if(pKFI = pBF) break;
  EndEnumFilters
    UINT nKFs = 0, nKFsTmp = 0;
  if(pKFI && S_OK == pKFI->GetKeyFrameCount(nKFs) && nKFs > 0)
  {
    m_kfs.SetCount(nKFsTmp = nKFs);
    if(S_OK != pKFI->GetKeyFrames(&TIME_FORMAT_MEDIA_TIME, m_kfs.GetData(), nKFsTmp) || nKFsTmp != nKFs)
      m_kfs.RemoveAll();
  }

  m_iPlaybackMode = PM_FILE;
}

void CGraphCore::SetupChapters()
{
  ASSERT(m_pCB);

  m_pCB->ChapRemoveAll();

  CInterfaceList<IBaseFilter> pBFs;
  BeginEnumFilters(pGB, pEF, pBF) 
    pBFs.AddTail(pBF);
  EndEnumFilters

    POSITION pos;

  pos = pBFs.GetHeadPosition();
  while(pos && !m_pCB->ChapGetCount())
  {
    IBaseFilter* pBF = pBFs.GetNext(pos);

    CComQIPtr<IDSMChapterBag> pCB = pBF;
    if(!pCB) continue;

    for(DWORD i = 0, cnt = pCB->ChapGetCount(); i < cnt; i++)
    {
      REFERENCE_TIME rt;
      CComBSTR name;
      if(SUCCEEDED(pCB->ChapGet(i, &rt, &name)))
        m_pCB->ChapAppend(rt, name);
    }
  }

  pos = pBFs.GetHeadPosition();
  while(pos && !m_pCB->ChapGetCount())
  {
    IBaseFilter* pBF = pBFs.GetNext(pos);

    CComQIPtr<IChapterInfo> pCI = pBF;
    if(!pCI) continue;

    CHAR iso6391[3];
    ::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, iso6391, 3);
    CStringA iso6392 = ISO6391To6392(iso6391);
    if(iso6392.GetLength() < 3) iso6392 = "eng";

    UINT cnt = pCI->GetChapterCount(CHAPTER_ROOT_ID);
    for(UINT i = 1; i <= cnt; i++)
    {
      UINT cid = pCI->GetChapterId(CHAPTER_ROOT_ID, i);

      ChapterElement ce;
      if(pCI->GetChapterInfo(cid, &ce))
      {
        char pl[3] = {iso6392[0], iso6392[1], iso6392[2]};
        char cc[] = "  ";
        CComBSTR name;
        name.Attach(pCI->GetChapterStringInfo(cid, pl, cc));
        m_pCB->ChapAppend(ce.rtStart, name);
      }
    }
  }

  pos = pBFs.GetHeadPosition();
  while(pos && !m_pCB->ChapGetCount())
  {
    IBaseFilter* pBF = pBFs.GetNext(pos);

    CComQIPtr<IAMExtendedSeeking, &IID_IAMExtendedSeeking> pES = pBF;
    if(!pES) continue;

    long MarkerCount = 0;
    if(SUCCEEDED(pES->get_MarkerCount(&MarkerCount)))
    {
      for(long i = 1; i <= MarkerCount; i++)
      {
        double MarkerTime = 0;
        if(SUCCEEDED(pES->GetMarkerTime(i, &MarkerTime)))
        {
          CStringW name;
          name.Format(L"Chapter %d", i);

          CComBSTR bstr;
          if(S_OK == pES->GetMarkerName(i, &bstr))
            name = bstr;

          m_pCB->ChapAppend(REFERENCE_TIME(MarkerTime*10000000), name);
        }
      }
    }
  }

  pos = pBFs.GetHeadPosition();
  while(pos && !m_pCB->ChapGetCount())
  {
    IBaseFilter* pBF = pBFs.GetNext(pos);

    if(GetCLSID(pBF) != CLSID_OggSplitter)
      continue;

    BeginEnumPins(pBF, pEP, pPin)
    {
      if(m_pCB->ChapGetCount()) break;

      if(CComQIPtr<IPropertyBag> pPB = pPin)
      {
        for(int i = 1; ; i++)
        {
          CStringW str;
          CComVariant var;

          var.Clear();
          str.Format(L"CHAPTER%02d", i);
          if(S_OK != pPB->Read(str, &var, NULL)) 
            break;

          int h, m, s, ms;
          WCHAR wc;
          if(7 != swscanf(CStringW(var), L"%d%c%d%c%d%c%d", &h, &wc, &m, &wc, &s, &wc, &ms)) 
            break;

          CStringW name;
          name.Format(L"Chapter %d", i);
          var.Clear();
          str += L"NAME";
          if(S_OK == pPB->Read(str, &var, NULL))
            name = var;

          m_pCB->ChapAppend(10000i64*(((h*60 + m)*60 + s)*1000 + ms), name);
        }
      }
    }
    EndEnumPins
  }

  m_pCB->ChapSort();
}

void CGraphCore::OpenDVD(OpenDVDData* pODD)
{
  HRESULT hr = pGB->RenderFile(CStringW(pODD->path), NULL);

  AppSettings& s = AfxGetAppSettings();

  //if(s.fReportFailedPins)
  MediaTypeDlg();

  BeginEnumFilters(pGB, pEF, pBF)
  {
    if((pDVDC = pBF) && (pDVDI = pBF))
      break;
  }
  EndEnumFilters

    if(hr == E_INVALIDARG)
      throw _T("Cannot find DVD directory");
    else if(hr == VFW_E_CANNOT_RENDER)
      throw _T("Failed to render all pins of the DVD Navigator filter");
    else if(hr == VFW_S_PARTIAL_RENDER){
      //throw _T("Failed to render some of the pins of the DVD Navigator filter");
    }else if(hr == E_NOINTERFACE || !pDVDC || !pDVDI)
      throw _T("Failed to query the needed interfaces for DVD playback");
    else if(hr == VFW_E_CANNOT_LOAD_SOURCE_FILTER)
      throw _T("Can't create the DVD Navigator filter");
    else if(FAILED(hr))
      throw _T("Failed");

    WCHAR buff[MAX_PATH];
    ULONG len = 0;
    if(SUCCEEDED(hr = pDVDI->GetDVDDirectory(buff, countof(buff), &len)))
      pODD->title = CString(CStringW(buff));

    if(s.fKeepHistory)
    {

      //AfxMessageBox(pODD->path + pODD->title);
      //CRecentFileList* pMRU = &s.MRU ; not ready
      //pMRU->ReadList();
      //pMRU->Add(pODD->path);
      //pMRU->WriteList();

    }
    // TODO: resetdvd

    pDVDC->SetOption(DVD_ResetOnStop, FALSE);
    pDVDC->SetOption(DVD_HMSF_TimeCodeEvents, TRUE);

    if(s.idMenuLang) pDVDC->SelectDefaultMenuLanguage(s.idMenuLang);
    if(s.idAudioLang) pDVDC->SelectDefaultAudioLanguage(s.idAudioLang, DVD_AUD_EXT_NotSpecified);
    if(s.idSubtitlesLang) pDVDC->SelectDefaultSubpictureLanguage(s.idSubtitlesLang, DVD_SP_EXT_NotSpecified);

    m_iDVDDomain = DVD_DOMAIN_Stop;

    m_iPlaybackMode = PM_DVD;
}

void CGraphCore::OpenCustomizeGraph()
{
  if(m_iPlaybackMode == PM_CAPTURE)
    return;

  CleanGraph();

  if(m_iPlaybackMode == PM_FILE)
  {
    if(m_pCAP) {
      if(AfxGetAppSettings().fAutoloadSubtitles) {
        AddTextPassThruFilter();
      }
    }
  }
  AppSettings& s = AfxGetAppSettings();
  if (s.m_RenderSettings.bSynchronizeVideo)
  {
    HRESULT hr;
    m_pRefClock = DNew CSyncClockFilter(NULL, &hr);
    CStringW name;
    name.Format(L"SyncClock Filter");
    pGB->AddFilter(m_pRefClock, name);

    CComPtr<IReferenceClock> refClock;
    m_pRefClock->QueryInterface(IID_IReferenceClock, reinterpret_cast<void**>(&refClock));
    CComPtr<IMediaFilter> mediaFilter;
    pGB->QueryInterface(IID_IMediaFilter, reinterpret_cast<void**>(&mediaFilter));
    mediaFilter->SetSyncSource(refClock);
    mediaFilter = NULL;
    refClock = NULL;

    m_pRefClock->QueryInterface(IID_ISyncClock, reinterpret_cast<void**>(&m_pSyncClock));
  }

  BeginEnumFilters(pGB, pEF, pBF)
  {
    if(GetCLSID(pBF) == CLSID_OggSplitter)
    {
      if(CComQIPtr<IAMStreamSelect> pSS = pBF)
      {
        LCID idAudio = AfxGetAppSettings().idAudioLang;
        if(!idAudio) idAudio = GetUserDefaultLCID();
        LCID idSub = AfxGetAppSettings().idSubtitlesLang;
        if(!idSub) idSub = GetUserDefaultLCID();

        DWORD cnt = 0;
        pSS->Count(&cnt);
        for(DWORD i = 0; i < cnt; i++)
        {
          AM_MEDIA_TYPE* pmt = NULL;
          DWORD dwFlags = 0;
          LCID lcid = 0;
          DWORD dwGroup = 0;
          WCHAR* pszName = NULL;
          if(SUCCEEDED(pSS->Info((long)i, &pmt, &dwFlags, &lcid, &dwGroup, &pszName, NULL, NULL)))
          {
            CStringW name(pszName), sound(L"Sound"), subtitle(L"Subtitle");

            if(idAudio != -1 && (idAudio&0x3ff) == (lcid&0x3ff) // sublang seems to be zeroed out in ogm...
              && name.GetLength() > sound.GetLength()
              && !name.Left(sound.GetLength()).CompareNoCase(sound))
            {
              if(SUCCEEDED(pSS->Enable(i, AMSTREAMSELECTENABLE_ENABLE)))
                idAudio = -1;
            }

            if(idSub != -1 && (idSub&0x3ff) == (lcid&0x3ff) // sublang seems to be zeroed out in ogm...
              && name.GetLength() > subtitle.GetLength()
              && !name.Left(subtitle.GetLength()).CompareNoCase(subtitle)
              && name.Mid(subtitle.GetLength()).Trim().CompareNoCase(L"off"))
            {
              if(SUCCEEDED(pSS->Enable(i, AMSTREAMSELECTENABLE_ENABLE)))
                idSub = -1;
            }

            if(pmt) DeleteMediaType(pmt);
            if(pszName) CoTaskMemFree(pszName);
          }
        }
      }
    }
  }
  EndEnumFilters

    CleanGraph();
}

void CGraphCore::OpenCapture(OpenDeviceData* pODD)
{
  CStringW vidfrname, audfrname;
  CComPtr<IBaseFilter> pVidCapTmp, pAudCapTmp;

  m_VidDispName = pODD->DisplayName[0];

  if(!m_VidDispName.IsEmpty())
  {
    if(!CreateFilter(m_VidDispName, &pVidCapTmp, vidfrname))
      throw _T("Can't create video capture filter");
  }

  m_AudDispName = pODD->DisplayName[1];

  if(!m_AudDispName.IsEmpty())
  {
    if(!CreateFilter(m_AudDispName, &pAudCapTmp, audfrname))
      throw _T("Can't create video capture filter");
  }

  if(!pVidCapTmp && !pAudCapTmp)
  {
    throw _T("No capture filters");
  }

  pCGB = NULL;
  pVidCap = NULL;
  pAudCap = NULL;

  if(FAILED(pCGB.CoCreateInstance(CLSID_CaptureGraphBuilder2)))
  {
    throw _T("Can't create capture graph builder object");
  }

  HRESULT hr;

  pCGB->SetFiltergraph(pGB);

  if(pVidCapTmp)
  {
    if(FAILED(hr = pGB->AddFilter(pVidCapTmp, vidfrname)))
    {
      throw _T("Can't add video capture filter to the graph");
    }

    pVidCap = pVidCapTmp;

    if(!pAudCapTmp)
    {
      if(FAILED(pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, pVidCap, IID_IAMStreamConfig, (void **)&pAMVSCCap))
        && FAILED(pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pVidCap, IID_IAMStreamConfig, (void **)&pAMVSCCap)))
        TRACE(_T("Warning: No IAMStreamConfig interface for vidcap capture"));

      if(FAILED(pCGB->FindInterface(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Interleaved, pVidCap, IID_IAMStreamConfig, (void **)&pAMVSCPrev))
        && FAILED(pCGB->FindInterface(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pVidCap, IID_IAMStreamConfig, (void **)&pAMVSCPrev)))
        TRACE(_T("Warning: No IAMStreamConfig interface for vidcap capture"));

      if(FAILED(pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, pVidCap, IID_IAMStreamConfig, (void **)&pAMASC))
        && FAILED(pCGB->FindInterface(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Audio, pVidCap, IID_IAMStreamConfig, (void **)&pAMASC)))
      {
        TRACE(_T("Warning: No IAMStreamConfig interface for vidcap"));
      }
      else
      {
        pAudCap = pVidCap;
      }
    }
    else
    {
      if(FAILED(pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pVidCap, IID_IAMStreamConfig, (void **)&pAMVSCCap)))
        TRACE(_T("Warning: No IAMStreamConfig interface for vidcap capture"));

      if(FAILED(pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pVidCap, IID_IAMStreamConfig, (void **)&pAMVSCPrev)))
        TRACE(_T("Warning: No IAMStreamConfig interface for vidcap capture"));
    }

    if(FAILED(pCGB->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pVidCap, IID_IAMCrossbar, (void**)&pAMXBar)))
      TRACE(_T("Warning: No IAMCrossbar interface was found\n"));

    if(FAILED(pCGB->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pVidCap, IID_IAMTVTuner, (void**)&pAMTuner)))
      TRACE(_T("Warning: No IAMTVTuner interface was found\n"));
    /*
    if(pAMVSCCap) 
    {
    //DumpStreamConfig(_T("c:\\mpclog.txt"), pAMVSCCap);
    CComQIPtr<IAMVfwCaptureDialogs> pVfwCD = pVidCap;
    if(!pAMXBar && pVfwCD)
    {
    m_wndCaptureBar.m_capdlg.SetupVideoControls(viddispname, pAMVSCCap, pVfwCD);
    }
    else
    {
    m_wndCaptureBar.m_capdlg.SetupVideoControls(viddispname, pAMVSCCap, pAMXBar, pAMTuner);
    }
    }
    */
    // TODO: init pAMXBar

    if(pAMTuner) // load saved channel
    {
      pAMTuner->put_CountryCode(AfxGetMyApp()->GetProfileInt(_T("Capture"), _T("Country"), 1));

      int vchannel = pODD->vchannel;
      if(vchannel < 0) vchannel = AfxGetMyApp()->GetProfileInt(_T("Capture\\") + CString(m_VidDispName), _T("Channel"), -1);
      if(vchannel >= 0)
      {
        OAFilterState fs = State_Stopped;
        pMC->GetState(0, &fs);
        if(fs == State_Running) pMC->Pause();
        pAMTuner->put_Channel(vchannel, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
        if(fs == State_Running) pMC->Run();
      }
    }
  }

  if(pAudCapTmp)
  {
    if(FAILED(hr = pGB->AddFilter(pAudCapTmp, CStringW(audfrname))))
    {
      throw _T("Can't add audio capture filter to the graph");
    }

    pAudCap = pAudCapTmp;

    if(FAILED(pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, pAudCap, IID_IAMStreamConfig, (void **)&pAMASC))
      && FAILED(pCGB->FindInterface(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Audio, pAudCap, IID_IAMStreamConfig, (void **)&pAMASC)))
    {
      TRACE(_T("Warning: No IAMStreamConfig interface for vidcap"));
    }
    /*
    CInterfaceArray<IAMAudioInputMixer> pAMAIM;

    BeginEnumPins(pAudCap, pEP, pPin)
    {
    PIN_DIRECTION dir;
    if(FAILED(pPin->QueryDirection(&dir)) || dir != PINDIR_INPUT)
    continue;

    if(CComQIPtr<IAMAudioInputMixer> pAIM = pPin) 
    pAMAIM.Add(pAIM);
    }
    EndEnumPins

    if(pAMASC)
    {
    m_wndCaptureBar.m_capdlg.SetupAudioControls(auddispname, pAMASC, pAMAIM);
    }
    */
  }

  if(!(pVidCap || pAudCap))
  {
    throw _T("Couldn't open any device");
  }

  pODD->title = _T("Live");

  m_iPlaybackMode = PM_CAPTURE;
}



void CGraphCore::OpenSetupVideo()
{
  m_fAudioOnly = true;

  if (m_pMFVDC)		// EVR 
  {
    m_fAudioOnly = false;
  }
  else if(m_pCAPR)
  {
    CSize vs = m_pCAPR->GetVideoSize();
    m_fAudioOnly = (vs.cx <= 0 || vs.cy <= 0);
  }
  else
  {
    {
      long w = 0, h = 0;

      if(CComQIPtr<IBasicVideo> pBV = pGB)
      {
        pBV->GetVideoSize(&w, &h);
      }

      if(w > 0 && h > 0)
      {
        m_fAudioOnly = false;
      }
    }

    if(m_fAudioOnly)
    {
      BeginEnumFilters(pGB, pEF, pBF)
      {
        long w = 0, h = 0;

        if(CComQIPtr<IVideoWindow> pVW = pBF)
        {
          long lVisible;
          if(FAILED(pVW->get_Visible(&lVisible)))
            continue;

          pVW->get_Width(&w);
          pVW->get_Height(&h);
        }

        if(w > 0 && h > 0)
        {
          m_fAudioOnly = false;
          break;
        }
      }
      EndEnumFilters
    }
  }

  if(m_fShockwaveGraph)
  {
    m_fAudioOnly = false;
  }

  if(m_pCAP)
  {
    SetShaders();
  }
  // else
  {
    // TESTME

    pVW->put_Owner((OAHWND)GetVideoView()->m_hWnd);
    pVW->put_WindowStyle(WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
    pVW->put_MessageDrain((OAHWND)GetMainFrame()->m_hWnd);

    for(CWnd* pWnd = GetVideoView()->GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow())
      pWnd->EnableWindow(FALSE); // little trick to let WM_SETCURSOR thru
  }
}

void CGraphCore::OpenSetupAudio()
{
  pBA->put_Volume(GetToolBar()->Volume);

  // FIXME
  int balance = AfxGetAppSettings().nBalance;
  int sign = balance>0?-1:1;
  balance = max(100-abs(balance), 1);
  balance = (int)((log10(1.0*balance)-2)*5000*sign);
  balance = max(min(balance, 10000), -10000);
  pBA->put_Balance(balance);
}

void CGraphCore::SetShaders( BOOL silent )
{
  if(!m_pCAPR) return;

  AppSettings& s = AfxGetAppSettings();

  CAtlStringMap<const AppSettings::Shader*> s2s;

  POSITION pos = s.m_shaders.GetHeadPosition();
  while(pos)
  {
    const AppSettings::Shader* pShader = &s.m_shaders.GetNext(pos);
    s2s[pShader->label] = pShader;
  }
  if(!silent){
    m_pCAPR->SetPixelShader(NULL, NULL);
    if (m_pCAP2)
      m_pCAP2->SetPixelShader2(NULL, NULL, true);

  }

  CAtlList<CString> labels;

  pos = m_shaderlabels.GetHeadPosition();
  while(pos)
  {
    const AppSettings::Shader* pShader = NULL;
    if(s2s.Lookup(m_shaderlabels.GetNext(pos), pShader))
    {
      CStringA target = pShader->target;
      CStringA srcdata = pShader->srcdata;

      HRESULT hr = m_pCAPR->SetPixelShader(srcdata, target );

      if(FAILED(hr))
      {
        //m_pCAP->SetPixelShader(NULL, NULL);
        if (m_pCAP2)
          hr = m_pCAP2->SetPixelShader2(srcdata, target, true);
        if(FAILED(hr)){
          //	if (m_pCAP2)
          //		m_pCAP2->SetPixelShader2(NULL, NULL, true,  !silent);
          if(!silent)
          {
            CString label = pShader->label;
            OsdMsg_SetShader(&label);
          }
        }
        return;
      }

      labels.AddTail(pShader->label);
    }
  }

  if(m_iMediaLoadState == MLS_LOADED)
  {
    CString str = Implode(labels, '|');
    str.Replace(_T("|"), _T(", "));
    if(!silent) SendStatusMessage(_T("Shader: ") + str, 3000);
  }
  if(!silent){
    if (SetVMR9ColorControl(s.dBrightness , s.dContrast, 0, 0, true) == FALSE)
      OsdMsg_SetShader();
  }
}

void CGraphCore::AddTextPassThruFilter()
{
  BeginEnumFilters(pGB, pEF, pBF)
  {
    if(!IsSplitter(pBF)) continue;

    BeginEnumPins(pBF, pEP, pPin)
    {
      CComPtr<IPin> pPinTo;
      AM_MEDIA_TYPE mt;
      if(FAILED(pPin->ConnectedTo(&pPinTo)) || !pPinTo 
        || FAILED(pPin->ConnectionMediaType(&mt)) 
        || mt.majortype != MEDIATYPE_Text && mt.majortype != MEDIATYPE_Subtitle)
        continue;

      CComQIPtr<IBaseFilter> pTPTF = new CTextPassThruFilter(this);
      CStringW name;
      name.Format(L"TextPassThru%08x", pTPTF);
      if(FAILED(pGB->AddFilter(pTPTF, name)))
        continue;

      HRESULT hr;

      hr = pPinTo->Disconnect();
      hr = pPin->Disconnect();

      if(FAILED(hr = pGB->ConnectDirect(pPin, GetFirstPin(pTPTF, PINDIR_INPUT), NULL))
        || FAILED(hr = pGB->ConnectDirect(GetFirstPin(pTPTF, PINDIR_OUTPUT), pPinTo, NULL)))
        hr = pGB->ConnectDirect(pPin, pPinTo, NULL);
      else{
        m_pSubStreams.AddTail(CComQIPtr<ISubStream>(pTPTF));
        m_pSubStreams2.AddTail(CComQIPtr<ISubStream>(pTPTF));
      }
    }
    EndEnumPins
  }
  EndEnumFilters
}

void CGraphCore::OpenMedia(CAutoPtr<OpenMediaData> pOMD)
{
  // shortcut
  if(OpenDeviceData* p = dynamic_cast<OpenDeviceData*>(pOMD.m_p))
  {
    if(m_iMediaLoadState == MLS_LOADED && pAMTuner
      && m_VidDispName == p->DisplayName[0] && m_AudDispName == p->DisplayName[1])
    {
      ApplyOptionToCaptureBar(p);
      return;
    }
  }

  if(m_iMediaLoadState != MLS_CLOSED)
    CloseMedia();

  m_iMediaLoadState = MLS_LOADING; // HACK: hides the logo
  GetVideoView()->Invalidate();

  AppSettings& s = AfxGetAppSettings();

  bool fUseThread = true;

  if(OpenFileData* p = dynamic_cast<OpenFileData*>(pOMD.m_p))
  {
    if(p->fns.GetCount() > 0)
    {
      engine_t e = s.Formats.GetEngine(p->fns.GetHead());
      fUseThread = e == DirectShow /*|| e == RealMedia || e == QuickTime*/;

    }
  }
  else if(OpenDeviceData* p = dynamic_cast<OpenDeviceData*>(pOMD.m_p))
  {
    fUseThread = false;

  }

  if(m_pGraphThread && fUseThread
    && AfxGetAppSettings().fEnableWorkerThreadForOpening)
    m_pGraphThread->PostThreadMessage(CGraphThread::TM_OPEN, 0, (LPARAM)pOMD.Detach());
  else
    OpenMediaPrivate(pOMD);
}

void CGraphCore::CloseMedia()
{

  GetMainFrame()->SendMessage(WM_COMMAND, ID_PLAY_PAUSE);

  CString fnx = GetPlaylistBar()->GetCur();

  if(m_iMediaLoadState == MLS_CLOSING || MLS_CLOSED == m_iMediaLoadState)
  {
    TRACE(_T("WARNING: CMainFrame::CloseMedia() called twice or more\n"));
    return;
  }


  if( m_iMediaLoadState == MLS_LOADED){
    //save time for next play
    OnFavoritesAddReal(TRUE);
  }
  int nTimeWaited = 0;

  while(m_iMediaLoadState == MLS_LOADING)
  {


    m_fOpeningAborted = true;


    if(nTimeWaited > 3*1000 && m_pGraphThread)
    {
      if(pGB) pGB->Abort(); // TODO: lock on graph objects somehow, this is not thread safe
      SVP_LogMsg5(L"OpenAbort");
      break;
    }

    Sleep(50);

    nTimeWaited += 50;
  }


  m_fOpeningAborted = false;

  m_closingmsg.clear();

  m_iMediaLoadState = MLS_CLOSING;

  GetMainFrame()->OnFilePostClosemedia();

  if(m_pGraphThread )
  {

    CAMEvent e;
    m_pGraphThread->PostThreadMessage(CGraphThread::TM_CLOSE, 0, (LPARAM)&e);
    // either opening or closing has to be blocked to prevent reentering them, closing is the better choice
    if(!e.Wait(5000))
    {
      TRACE(_T("ERROR: Must call TerminateThread() on CMainFrame::m_pGraphThread->m_hThread\n")); 
      TerminateThread(m_pGraphThread->m_hThread, -1);
      m_pGraphThread = (CGraphThread*)AfxBeginThread(RUNTIME_CLASS(CGraphThread));
      if(m_pGraphThread)
        m_pGraphThread->SetMainFrame(GetMainFrame());

    }
  }
  else
  {
    CloseMediaPrivate();
  }

  if(m_pMC){
    m_pMC = NULL;
  }

  m_iRedrawAfterCloseCounter = 0;
  GetMainFrame()->SetTimer(CMainFrame::TIMER_REDRAW_WINDOW,120,NULL);
}