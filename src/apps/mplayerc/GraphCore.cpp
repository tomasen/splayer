#include "stdafx.h"
#include "GraphCore.h"
#include <Strings.h>

#include "mplayerc.h"
#include "MainFrm.h"
#include "Controller/HashController.h"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include "jpeg.h"
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
  m_fCapturing(false),
  m_iAudioChannelMaping(0),
  m_fOpeningAborted(false),
  m_iMediaLoadState(MLS_CLOSED),
  m_is_resume_from_last_exit_point(false),
  m_fLiveWM(false),
  m_fEndOfStream(false),
  m_iSpeedLevel(0),
  m_rtDurationOverride(-1),
  m_fOpenedThruThread(FALSE),
  m_iPlaybackMode(PM_NONE),
  m_iSubtitleSel2(-1),
  _skip_ui(0)
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
    //�û�Ԥ��Ļ�٣�ͼ� �Ļ.delay �delay�
    sub_delay_ms = _wtoi ( svTool.fileGetContent( fn+_T(".delay")) );
  }else{
    //���Ļ�٣ ��playlist subtitles 浽.delay�
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
  //SendStatusMessage(_T("��Ļ�ر�) , 4000 );
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
    //SendStatusMessage(_T("��Ļ�ر�) , 4000 );
  }
}

void CGraphCore::SetSubtitleDelay(int delay_ms)
{
  if(m_pCAP) {
    m_pCAP->SetSubtitleDelay(delay_ms);
    getCurPlayingSubfile();
    //CString str;
    //str.Format(_T("��Ļ�ʱ��Ϊ %d ms"), delay_ms);
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
  //SendStatusMessage(_T("ڶ�Ļ�ر�) , 4000 );
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
    //SendStatusMessage(_T("ڶ�Ļ�ر�) , 4000 );
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
    // 		str.Format(_T("ڶ�Ļ�ʱ��Ϊ %d ms"), delay_ms);
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
              //SVP_LogMsg5(L"SVP ��� );
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

      if (m_pMC && !_skip_ui)
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

      if (_skip_ui)
       return true;

      GetMainFrame()->PostMessage(WM_COMMAND, ID_PLAY_PAUSE);

      if(!(AfxGetAppSettings().nCLSwitches&CLSW_OPEN))
        GetMainFrame()->PostMessage(WM_COMMAND, ID_PLAY_PLAY);

      AfxGetAppSettings().nCLSwitches &= ~CLSW_OPEN;

      if(OpenFileData* p = dynamic_cast<OpenFileData*>(pOMD.m_p))
      {
        if(p->rtStart > 0)
          GetMainFrame()->PostMessage(WM_RESUMEFROMSTATE, (WPARAM)PM_FILE, (LPARAM)(p->rtStart/10000)); // REFERENCE_TIME doesn't fit in LPARAM under a 32bit env.
        else
        {
          if (CComQIPtr<IAudioSwitcherFilter> pASF = FindFilter(__uuidof(CAudioSwitcherFilter), pGB))
            pHashController::GetInstance()->Check(p->rtStart, pMS, pASF, m_fnCurPlayingFile);
        }
      }
      else if(OpenDVDData* p = dynamic_cast<OpenDVDData*>(pOMD.m_p))
      {
        if(p->pDvdState)
          GetMainFrame()->PostMessage(WM_RESUMEFROMSTATE, (WPARAM)PM_DVD, (LPARAM)(CComPtr<IDvdState>(p->pDvdState).Detach())); // must be released by the called message handler
        else
        {
          if (CComQIPtr<IAudioSwitcherFilter> pASF = FindFilter(__uuidof(CAudioSwitcherFilter), pGB))
          {
            REFERENCE_TIME st = 0;
            pHashController::GetInstance()->Check(st, pMS, pASF, m_fnCurPlayingFile);
          }
        }
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

  pHashController::GetInstance()->PHashCommCfg.stop = TRUE;

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

  AppSettings& s = AfxGetAppSettings();

  s.i3DStereo = 0;
  s.i3DStereoKeepAspectRatio = 0;

  try{
    if (s.szCurrentExtension != L".csf")
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


  s.bIsIVM = false;
  s.szCurrentExtension.Empty();
  s.nCLSwitches &= CLSW_OPEN|CLSW_PLAY|CLSW_AFTERPLAYBACK_MASK|CLSW_NOFOCUS|CLSW_HTPCMODE;

  m_iMediaLoadState = MLS_CLOSED;

  SetThreadExecutionState(0); //this is the right way, only this work under vista . no ES_CONTINUOUS  so it can goes to sleep when not playing

}

// Function: make snapshot for media
// command line: splayer /snapshot "\\file_01\reflections\-=Test File=-\01.rmvb" 128_128 5
void CGraphCore::GetSnapShotSliently(const std::vector<std::wstring> &args)
{
  using namespace boost;
  using namespace boost::filesystem;

  // deal arguments
  std::wstring sFilePath = args.front();
  std::pair<int, int> prSnapshotSize;  // e.g: 128 * 128
  int nSnapshotTime = 5;  // e.g: default is 5 minutes, unit is minute now

  wsmatch what;
  wregex pattern(L"(\\d+)_(\\d+)");
  if (regex_search(args[1], what, pattern))
  {
    prSnapshotSize.first = ::_wtoi(what.str(1).c_str());
    prSnapshotSize.second = ::_wtoi(what.str(2).c_str());
  }

  if (prSnapshotSize.first == 0)
    prSnapshotSize.first = 128;   // set to a default size

  if (prSnapshotSize.second == 0)
    prSnapshotSize.second = 128;   // set to a default size

  if (!args[2].empty())
    nSnapshotTime = ::_wtoi(args[2].c_str());

  if (!exists(sFilePath))
    return;

  // do snapshot
  OpenFileData* p = new OpenFileData();
  if (!p)
    return;
  _skip_ui = true;

  p->fns.AddTail(sFilePath.c_str());
  p->rtStart = 0;
  CloseMedia();
  // disable graph thread
  AppSettings& s = AfxGetAppSettings();
  s.fEnableWorkerThreadForOpening = 0;
  s.fMute = true;
  s.useGPUAcel = false;
  // skip read/write setting/playlist/download sub
  CAutoPtr<OpenMediaData> pOMD((OpenMediaData*)p);
  OpenMedia(pOMD);
  s.fEnableWorkerThreadForOpening = 1;
  if(!pMS)
    return;

  // set duration
  __int64 rtDur = 0;
  rtDur = (__int64)nSnapshotTime * 60 * 10000000;

  // return if snapshot time greater than media's stop time
  __int64 rtStop = 0;
  pMS->GetPositions(0, &rtStop);
  if (rtDur > rtStop)
    return;

  // otherwise set the position
  pMS->SetPositions(&rtDur, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);

  HRESULT hr = pFS ? pFS->Step(3, NULL) : E_FAIL;

  HANDLE hGraphEvent = NULL;
  pME->GetEventHandle((OAEVENT*)&hGraphEvent);

  while(hGraphEvent && WaitForSingleObject(hGraphEvent, 5000) == WAIT_OBJECT_0)
  {
    LONG evCode = 0, evParam1, evParam2;
    while(SUCCEEDED(pME->GetEvent(&evCode, (LONG_PTR*)&evParam1, (LONG_PTR*)&evParam2, 0)))
    {
      pME->FreeEventParams(evCode, evParam1, evParam2);
      if(EC_STEP_COMPLETE == evCode) hGraphEvent = NULL;
    }
  }

  BYTE* pData = NULL;
  long size = 0;
  bool dib_stat = GetDIB(&pData, size, true);

  CloseMediaPrivate();

  if (!dib_stat)
    return;

  std::string szFileHash = Strings::WStringToUtf8String(HashController::GetInstance()->GetSPHash(sFilePath.c_str()));
  std::wstring szJpgName = HashController::GetInstance()->GetMD5Hash(szFileHash.c_str(), szFileHash.length());

  CSVPToolBox toolbox;
  std::wstring snapshot_fn;
  toolbox.GetAppDataPath(snapshot_fn);
  snapshot_fn += L"\\mc\\cover\\";

  std::wstringstream ssFinalName;
  ssFinalName << szJpgName << L"_" << prSnapshotSize.first << L"_"
              << prSnapshotSize.second << L"_" << nSnapshotTime << L".jpg";

  snapshot_fn += ssFinalName.str();

  BITMAPINFO* bi = (BITMAPINFO*)pData;

  // just start the gdi+
  CImage igTemp;
  igTemp.Load(L"");

  // Zoom the bitmap
  Gdiplus::Bitmap *pbmOrigin = Gdiplus::Bitmap::FromBITMAPINFO(bi, (char *)bi + sizeof(BITMAPINFO));
  if (pbmOrigin)
  {
    Gdiplus::Bitmap bmResult(prSnapshotSize.first, prSnapshotSize.second);
    Gdiplus::Graphics gpResult(&bmResult);
    gpResult.DrawImage(pbmOrigin, 0, 0, prSnapshotSize.first, prSnapshotSize.second);

    HBITMAP hbmResult = 0;
    bmResult.GetHBITMAP(Gdiplus::Color(255, 255, 255), &hbmResult);
    if (hbmResult)
    {
      CImage image;
      image.Attach(hbmResult);
      image.Save(snapshot_fn.c_str());
    }
  }
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
      //���� ]�����Ļ
      CSVPToolBox svpTool;
      //��Ŀ¼�ͬ��Ļ
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
            //SendStatusMessage(  _T("��ŵ�ļ��4�Ҫ�Ļ�ֹ���ƥ�"), 1000);
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

  m_fOpenedThruThread = FALSE;
  if(m_pGraphThread && fUseThread
    && AfxGetAppSettings().fEnableWorkerThreadForOpening)
  {
    m_pGraphThread->PostThreadMessage(CGraphThread::TM_OPEN, 0, (LPARAM)pOMD.Detach());
    m_fOpenedThruThread = TRUE;
  }
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

  if(m_pGraphThread && m_fOpenedThruThread)
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

      m_fOpenedThruThread = FALSE;
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

CSize CGraphCore::GetVideoSize()
{
  bool fKeepAspectRatio = AfxGetAppSettings().fKeepAspectRatio;

  CSize ret(0,0);
  if(m_iMediaLoadState != MLS_LOADED || m_fAudioOnly)
    return ret;

  CSize wh(0, 0), arxy(0, 0);

  if (m_pMFVDC)
  {
    m_pMFVDC->GetNativeVideoSize(&wh, &arxy);	// TODO : check AR !!
  }
  else if(m_pCAPR)
  {
    wh = m_pCAPR->GetVideoSize(false);
    arxy = m_pCAPR->GetVideoSize(fKeepAspectRatio);


  }
  else
  {
    pBV->GetVideoSize(&wh.cx, &wh.cy);

    long arx = 0, ary = 0;
    CComQIPtr<IBasicVideo2> pBV2 = pBV;
    if(pBV2 && SUCCEEDED(pBV2->GetPreferredAspectRatio(&arx, &ary)) && arx > 0 && ary > 0)
      arxy.SetSize(arx, ary);
  }

  //CString szLog;
  //szLog.Format(_T("vSize %d %d %d %d") , wh.cx, wh.cy , arxy.cx , arxy.cy);
  //SVP_LogMsg(szLog);

  if(wh.cx <= 0 || wh.cy <= 0)
    return ret;

  // with the overlay mixer IBasicVideo2 won't tell the new AR when changed dynamically
  DVD_VideoAttributes VATR;
  if(m_iPlaybackMode == PM_DVD && SUCCEEDED(pDVDI->GetCurrentVideoAttributes(&VATR)))
    arxy.SetSize(VATR.ulAspectX, VATR.ulAspectY);

  CSize& ar = AfxGetAppSettings().AspectRatio;
  if(ar.cx && ar.cy) arxy = ar;

  ret = (!fKeepAspectRatio || arxy.cx <= 0 || arxy.cy <= 0)
    ? wh
    : CSize(MulDiv(wh.cy, arxy.cx, arxy.cy), wh.cy);

    return ret;
}

void CGraphCore::UpdateShaders(CString label)
{
  if(!m_pCAP) return;

  if(m_shaderlabels.GetCount() <= 1)
    m_shaderlabels.RemoveAll();

  if(m_shaderlabels.IsEmpty() && !label.IsEmpty())
    m_shaderlabels.AddTail(label);

  bool fUpdate = m_shaderlabels.IsEmpty();

  POSITION pos = m_shaderlabels.GetHeadPosition();
  while(pos)
  {
    if(label == m_shaderlabels.GetNext(pos))
    {
      fUpdate = true;
      break;
    }
  }

  if(fUpdate)
    SetShaders();

}

HRESULT CGraphCore::BuildCapture(IPin* pPin, IBaseFilter* pBF[3],
                                 const GUID& majortype, AM_MEDIA_TYPE* pmt)
{
  IBaseFilter* pBuff = pBF[0];
  IBaseFilter* pEnc = pBF[1];
  IBaseFilter* pMux = pBF[2];

  if(!pPin || !pMux) return E_FAIL;

  CString err;

  HRESULT hr = S_OK;

  CFilterInfo fi;
  if(FAILED(pMux->QueryFilterInfo(&fi)) || !fi.pGraph)
    pGB->AddFilter(pMux, L"Multiplexer");

  CStringW prefix, prefixl;
  if(majortype == MEDIATYPE_Video) prefix = L"Video ";
  else if(majortype == MEDIATYPE_Audio) prefix = L"Audio ";
  prefixl = prefix;
  prefixl.MakeLower();

  if(pBuff)
  {
    hr = pGB->AddFilter(pBuff, prefix + L"Buffer");
    if(FAILED(hr))
    {
      err = _T("Can't add ") + CString(prefixl) + _T("buffer filter");
      AfxMessageBox(err);
      return hr;
    }

    hr = pGB->ConnectFilter(pPin, pBuff);
    if(FAILED(hr))
    {
      err = _T("Error connecting the ") + CString(prefixl) + _T("buffer filter");
      AfxMessageBox(err);
      return(hr);
    }

    pPin = GetFirstPin(pBuff, PINDIR_OUTPUT);
  }

  if(pEnc)
  {
    hr = pGB->AddFilter(pEnc, prefix + L"Encoder");
    if(FAILED(hr))
    {
      err = _T("Can't add ") + CString(prefixl) + _T("encoder filter");
      AfxMessageBox(err);
      return hr;
    }

    hr = pGB->ConnectFilter(pPin, pEnc);
    if(FAILED(hr))
    {
      err = _T("Error connecting the ") + CString(prefixl) + _T("encoder filter");
      AfxMessageBox(err);
      return(hr);
    }

    pPin = GetFirstPin(pEnc, PINDIR_OUTPUT);

    if(CComQIPtr<IAMStreamConfig> pAMSC = pPin)
    {
      if(pmt->majortype == majortype)
      {
        hr = pAMSC->SetFormat(pmt);
        if(FAILED(hr))
        {
          err = _T("Can't set compression format on the ") + CString(prefixl) + _T("encoder filter");
          AfxMessageBox(err);
          return(hr);
        }
      }
    }

  }

  //	if(pMux)
  {
    hr = pGB->ConnectFilter(pPin, pMux);
    if(FAILED(hr))
    {
      err = _T("Error connecting ") + CString(prefixl) + _T(" to the muliplexer filter");
      AfxMessageBox(err);
      return(hr);
    }
  }

  CleanGraph();

  return S_OK;
}

bool CGraphCore::BuildToCapturePreviewPin(
  IBaseFilter* pVidCap, IPin** ppVidCapPin, IPin** ppVidPrevPin, 
  IBaseFilter* pAudCap, IPin** ppAudCapPin, IPin** ppAudPrevPin)
{
  HRESULT hr;

  *ppVidCapPin = *ppVidPrevPin = NULL; 
  *ppAudCapPin = *ppAudPrevPin = NULL;

  CComPtr<IPin> pDVAudPin;

  if(pVidCap)
  {
    CComPtr<IPin> pPin;
    if(!pAudCap // only look for interleaved stream when we don't use any other audio capture source
      && SUCCEEDED(pCGB->FindPin(pVidCap, PINDIR_OUTPUT, &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, TRUE, 0, &pPin)))
    {
      CComPtr<IBaseFilter> pDVSplitter;
      hr = pDVSplitter.CoCreateInstance(CLSID_DVSplitter);
      hr = pGB->AddFilter(pDVSplitter, L"DV Splitter");

      hr = pCGB->RenderStream(NULL, &MEDIATYPE_Interleaved, pPin, NULL, pDVSplitter);

      pPin = NULL;
      hr = pCGB->FindPin(pDVSplitter, PINDIR_OUTPUT, NULL, &MEDIATYPE_Video, TRUE, 0, &pPin);
      hr = pCGB->FindPin(pDVSplitter, PINDIR_OUTPUT, NULL, &MEDIATYPE_Audio, TRUE, 0, &pDVAudPin);

      CComPtr<IBaseFilter> pDVDec;
      hr = pDVDec.CoCreateInstance(CLSID_DVVideoCodec);
      hr = pGB->AddFilter(pDVDec, L"DV Video Decoder");

      hr = pGB->ConnectFilter(pPin, pDVDec);

      pPin = NULL;
      hr = pCGB->FindPin(pDVDec, PINDIR_OUTPUT, NULL, &MEDIATYPE_Video, TRUE, 0, &pPin);
    }
    else if(SUCCEEDED(pCGB->FindPin(pVidCap, PINDIR_OUTPUT, &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, TRUE, 0, &pPin)))
    {
    }
    else
    {
      AfxMessageBox(_T("No video capture pin was found"));
      return(false);
    }

    CComPtr<IBaseFilter> pSmartTee;
    hr = pSmartTee.CoCreateInstance(CLSID_SmartTee);
    hr = pGB->AddFilter(pSmartTee, L"Smart Tee (video)");

    hr = pGB->ConnectFilter(pPin, pSmartTee);

    hr = pSmartTee->FindPin(L"Preview", ppVidPrevPin);
    hr = pSmartTee->FindPin(L"Capture", ppVidCapPin);
  }

  if(pAudCap || pDVAudPin)
  {
    CComPtr<IPin> pPin;
    if(pDVAudPin)
    {
      pPin = pDVAudPin;
    }
    else if(SUCCEEDED(pCGB->FindPin(pAudCap, PINDIR_OUTPUT, &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, TRUE, 0, &pPin)))
    {
    }
    else
    {
      AfxMessageBox(_T("No audio capture pin was found"));
      return(false);
    }

    CComPtr<IBaseFilter> pSmartTee;
    hr = pSmartTee.CoCreateInstance(CLSID_SmartTee);
    hr = pGB->AddFilter(pSmartTee, L"Smart Tee (audio)");

    hr = pGB->ConnectFilter(pPin, pSmartTee);

    hr = pSmartTee->FindPin(L"Preview", ppAudPrevPin);
    hr = pSmartTee->FindPin(L"Capture", ppAudCapPin);
  }

  return true;
}

#define SaveMediaState \
  OAFilterState __fs = GetMediaState(); \
  \
  REFERENCE_TIME __rt = 0; \
  if(m_iMediaLoadState == MLS_LOADED) __rt = GetMainFrame()->GetPos(); \
  \
  if(__fs != State_Stopped) \
  GetMainFrame()->SendMessage(WM_COMMAND, ID_PLAY_STOP); \


#define RestoreMediaState \
  if(m_iMediaLoadState == MLS_LOADED) \
{ \
  GetMainFrame()->SeekTo(__rt); \
  \
  if(__fs == State_Stopped) \
  GetMainFrame()->SendMessage(WM_COMMAND, ID_PLAY_STOP); \
    else if(__fs == State_Paused) \
    GetMainFrame()->SendMessage(WM_COMMAND, ID_PLAY_PAUSE); \
    else if(__fs == State_Running) \
    GetMainFrame()->SendMessage(WM_COMMAND, ID_PLAY_PLAY); \
} 
#define AUDIOBUFFERLEN 500

static void SetLatency(IBaseFilter* pBF, int cbBuffer)
{
  BeginEnumPins(pBF, pEP, pPin)
  {
    if(CComQIPtr<IAMBufferNegotiation> pAMBN = pPin)
    {
      ALLOCATOR_PROPERTIES ap;
      ap.cbAlign = -1;  // -1 means no preference.
      ap.cbBuffer = cbBuffer;
      ap.cbPrefix = -1;
      ap.cBuffers = -1;
      pAMBN->SuggestAllocatorProperties(&ap);
    }
  }
  EndEnumPins
}

bool CGraphCore::BuildGraphVideoAudio(int fVPreview, bool fVCapture, int fAPreview, bool fACapture)
{
  if(!pCGB) return(false);

  SaveMediaState;

  HRESULT hr;

  pGB->NukeDownstream(pVidCap);
  pGB->NukeDownstream(pAudCap);

  CleanGraph();
  
  if(pAMVSCCap) hr = pAMVSCCap->SetFormat(&(GetCaptureBar()->m_capdlg.m_mtv));
  if(pAMVSCPrev) hr = pAMVSCPrev->SetFormat(&(GetCaptureBar()->m_capdlg.m_mtv));
  if(pAMASC) hr = pAMASC->SetFormat(&(GetCaptureBar()->m_capdlg.m_mta));

  CComPtr<IBaseFilter> pVidBuffer = GetCaptureBar()->m_capdlg.m_pVidBuffer;
  CComPtr<IBaseFilter> pAudBuffer = GetCaptureBar()->m_capdlg.m_pAudBuffer;
  CComPtr<IBaseFilter> pVidEnc = GetCaptureBar()->m_capdlg.m_pVidEnc;
  CComPtr<IBaseFilter> pAudEnc = GetCaptureBar()->m_capdlg.m_pAudEnc;
  CComPtr<IBaseFilter> pMux = GetCaptureBar()->m_capdlg.m_pMux;
  CComPtr<IBaseFilter> pDst = GetCaptureBar()->m_capdlg.m_pDst;
  CComPtr<IBaseFilter> pAudMux = GetCaptureBar()->m_capdlg.m_pAudMux;
  CComPtr<IBaseFilter> pAudDst = GetCaptureBar()->m_capdlg.m_pAudDst;

  bool fFileOutput = (pMux && pDst) || (pAudMux && pAudDst);
  bool fCapture = (fVCapture || fACapture);

  if(pAudCap)
  {
    AM_MEDIA_TYPE* pmt = &(GetCaptureBar()->m_capdlg.m_mta);
    int ms = (fACapture && fFileOutput && GetCaptureBar()->m_capdlg.m_fAudOutput) ? AUDIOBUFFERLEN : 60;
    if(pMux != pAudMux && fACapture) SetLatency(pAudCap, -1);
    else if(pmt->pbFormat) SetLatency(pAudCap, ((WAVEFORMATEX*)pmt->pbFormat)->nAvgBytesPerSec * ms / 1000);
  }

  CComPtr<IPin> pVidCapPin, pVidPrevPin, pAudCapPin, pAudPrevPin;
  BuildToCapturePreviewPin(pVidCap, &pVidCapPin, &pVidPrevPin, pAudCap, &pAudCapPin, &pAudPrevPin);

  //	if(pVidCap)
  {
    bool fVidPrev = pVidPrevPin && fVPreview;
    bool fVidCap = pVidCapPin && fVCapture && fFileOutput && GetCaptureBar()->m_capdlg.m_fVidOutput;

    if(fVPreview == 2 && !fVidCap && pVidCapPin)
    {
      pVidPrevPin = pVidCapPin;
      pVidCapPin = NULL;
    }

    if(fVidPrev)
    {
      m_pCAP = NULL;
      m_pCAP2 = NULL;
      m_pCAPR = NULL;
      pGB->Render(pVidPrevPin);
      pGB->FindInterface(__uuidof(ISubPicAllocatorPresenter), (void**)&m_pCAP, FALSE);
      pGB->FindInterface(__uuidof(ISubPicAllocatorPresenterRender), (void**)&m_pCAPR, TRUE);
      pGB->FindInterface(__uuidof(ISubPicAllocatorPresenter2), (void**)&m_pCAP2, TRUE);
    }

    if(fVidCap)
    {
      IBaseFilter* pBF[3] = {pVidBuffer, pVidEnc, pMux};
      HRESULT hr = BuildCapture(pVidCapPin, pBF, MEDIATYPE_Video, &(GetCaptureBar()->m_capdlg.m_mtcv));
    }

    pAMDF = NULL;
    pCGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pVidCap, IID_IAMDroppedFrames, (void**)&pAMDF);
  }

  //	if(pAudCap)
  {
    bool fAudPrev = pAudPrevPin && fAPreview;
    bool fAudCap = pAudCapPin && fACapture && fFileOutput && GetCaptureBar()->m_capdlg.m_fAudOutput;

    if(fAPreview == 2 && !fAudCap && pAudCapPin)
    {
      pAudPrevPin = pAudCapPin;
      pAudCapPin = NULL;
    }

    if(fAudPrev)
    {
      pGB->Render(pAudPrevPin);
    }

    if(fAudCap)
    {
      IBaseFilter* pBF[3] = {pAudBuffer, pAudEnc, pAudMux ? pAudMux : pMux};
      HRESULT hr = BuildCapture(pAudCapPin, pBF, MEDIATYPE_Audio, &(GetCaptureBar()->m_capdlg.m_mtca));
    }
  }

  if((pVidCap || pAudCap) && fCapture && fFileOutput)
  {
    if(pMux != pDst)
    {
      hr = pGB->AddFilter(pDst, L"File Writer V/A");
      hr = pGB->ConnectFilter(GetFirstPin(pMux, PINDIR_OUTPUT), pDst);
    }

    if(CComQIPtr<IConfigAviMux> pCAM = pMux)
    {
      int nIn, nOut, nInC, nOutC;
      CountPins(pMux, nIn, nOut, nInC, nOutC);
      pCAM->SetMasterStream(nInC-1);
      //			pCAM->SetMasterStream(-1);
      pCAM->SetOutputCompatibilityIndex(FALSE);
    }

    if(CComQIPtr<IConfigInterleaving> pCI = pMux)
    {
      //			if(FAILED(pCI->put_Mode(INTERLEAVE_CAPTURE)))
      if(FAILED(pCI->put_Mode(INTERLEAVE_NONE_BUFFERED)))
        pCI->put_Mode(INTERLEAVE_NONE);

      REFERENCE_TIME rtInterleave = 10000i64*AUDIOBUFFERLEN, rtPreroll = 0;//10000i64*500
      pCI->put_Interleaving(&rtInterleave, &rtPreroll);
    }

    if(pMux != pAudMux && pAudMux != pAudDst)
    {
      hr = pGB->AddFilter(pAudDst, L"File Writer A");
      hr = pGB->ConnectFilter(GetFirstPin(pAudMux, PINDIR_OUTPUT), pAudDst);
    }
  }

  REFERENCE_TIME stop = MAX_TIME;
  hr = pCGB->ControlStream(&PIN_CATEGORY_CAPTURE, NULL, NULL, NULL, &stop, 0, 0); // stop in the infinite

  CleanGraph();

  OpenSetupVideo();
  OpenSetupAudio();

  RestoreMediaState;

  return true;
}

void CGraphCore::SetBalance(int balance)
{
  AfxGetAppSettings().nBalance = balance;

  int sign = balance>0?-1:1;
  balance = max(100-abs(balance), 1);
  balance = (int)((log10(1.0*balance)-2)*5000*sign);
  balance = max(min(balance, 10000), -10000);

  if(m_iMediaLoadState == MLS_LOADED) 
    pBA->put_Balance(balance);
}

OAFilterState CGraphCore::GetMediaState()
{
  OAFilterState ret = -1;
  if(m_iMediaLoadState == MLS_LOADED) pMC->GetState(0, &ret);
  return(ret);
}

bool CGraphCore::GetDIB(BYTE** ppData, long& size, bool fSilent, bool with_sub)
{
  if(!ppData) return false;

  *ppData = NULL;
  size = 0;

  bool fNeedsToPause = !m_pCAP;
  if(fNeedsToPause) fNeedsToPause = !IsVMR7InGraph(pGB);
  if(fNeedsToPause) fNeedsToPause = !IsVMR9InGraph(pGB);

  OAFilterState fs = GetMediaState();

  if(!(m_iMediaLoadState == MLS_LOADED && !m_fAudioOnly && (fs == State_Paused || fs == State_Running)))
    return false;

  if(fs == State_Running && fNeedsToPause)
  {
    pMC->Pause();
    GetMediaState(); // wait for completion of the pause command
  }

  HRESULT hr = S_OK;
  CString errmsg;

  do
  {
    if(m_pCAP)
    {
      hr = m_pCAP->GetDIB(NULL, (DWORD*)&size, with_sub);
      if(FAILED(hr))
      {
        GetMainFrame()->OnPlayPause();
        GetMediaState(); // Pause and retry to support ffdshow queueing.
        int retry = 0;
        while(FAILED(hr) && retry < 20)
        {
          hr = m_pCAP->GetDIB(*ppData, (DWORD*)&size, with_sub);
          if(SUCCEEDED(hr)) break;
          Sleep(1);
          retry++;
        }
        if(FAILED(hr))
        {errmsg.Format(_T("GetDIB failed, hr = %08x"), hr); break;}
      }

      if(!(*ppData = new BYTE[size])) return false;

      hr = m_pCAP->GetDIB(*ppData, (DWORD*)&size, with_sub);
      if(FAILED(hr)) {errmsg.Format(_T("GetDIB failed, hr = %08x"), hr); break;}
    }
    else
    {
      hr = pBV->GetCurrentImage(&size, NULL);
      if(FAILED(hr) || size == 0) {errmsg.Format(_T("GetCurrentImage failed, hr = %08x"), hr); break;}

      if(!(*ppData = new BYTE[size])) return false;

      hr = pBV->GetCurrentImage(&size, (long*)*ppData);
      if(FAILED(hr)) {errmsg.Format(_T("GetCurrentImage failed, hr = %08x"), hr); break;}
    }
  }
  while(0);

  if(!fSilent)
  {
    if(!errmsg.IsEmpty())
    {
      AfxMessageBox(errmsg, MB_OK);
    }
  }

  if(fs == State_Running && GetMediaState() != State_Running)
  {
    pMC->Run();
  }

  if(FAILED(hr))
  {
    if(*ppData) {ASSERT(0); delete [] *ppData; *ppData = NULL;} // huh?
    return false;
  }

  return true;
}

void CGraphCore::SaveDIB(LPCTSTR fn, BYTE* pData, long size)
{
  CString ext = CString(CPath(fn).GetExtension()).MakeLower();

  if(ext == _T(".bmp"))
  {
    if(FILE* f = _tfopen(fn, _T("wb")))
    {
      BITMAPINFO* bi = (BITMAPINFO*)pData;

      BITMAPFILEHEADER bfh;
      bfh.bfType = 'MB';
      bfh.bfOffBits = sizeof(bfh) + sizeof(bi->bmiHeader);
      bfh.bfSize = sizeof(bfh) + size;
      bfh.bfReserved1 = bfh.bfReserved2 = 0;

      if(bi->bmiHeader.biBitCount <= 8)
      {
        if(bi->bmiHeader.biClrUsed) bfh.bfOffBits += bi->bmiHeader.biClrUsed * sizeof(bi->bmiColors[0]);
        else bfh.bfOffBits += (1 << bi->bmiHeader.biBitCount) * sizeof(bi->bmiColors[0]);
      }

      fwrite(&bfh, 1, sizeof(bfh), f);
      fwrite(pData, 1, size, f);

      fclose(f);
    }
    else
    {
      AfxMessageBox(_T("Cannot create file"), MB_OK);
    }
  }
  else if(ext == _T(".jpg"))
  {
    CJpegEncoderFile(fn).Encode(pData);
  }

  CPath p(fn);
  /*

  if(CDC* pDC = m_wndStatusBar.m_status.GetDC())
  {
  CRect r;
  //m_wndStatusBar.m_status.GetClientRect(r);

  m_wndStatusBar.m_status.ReleaseDC(pDC);
  }*/

  /*
  CRect rcView;
  GetClientRect(rcView);
  if(HDC hDC = ::GetDC(0))
  {
  p.CompactPath(hDC, rcView.Width()/3);
  ::ReleaseDC(0, hDC);
  }*/


  CString szMsg;

  szMsg.Format(ResStr(IDS_OSD_MSG_IMAGE_CAPTURE_TO),(LPCTSTR)p);
  SendStatusMessage(szMsg, 3000);
}

void CGraphCore::SaveThumbnails(LPCTSTR fn)
{
  if(!pMC || !pMS || m_iPlaybackMode != PM_FILE /*&& m_iPlaybackMode != PM_DVD*/) 
    return;

  REFERENCE_TIME rtPos = GetMainFrame()->GetPos();
  REFERENCE_TIME rtDur = GetMainFrame()->GetDur();

  if(rtDur <= 0)
  {
    AfxMessageBox(_T("Cannot create thumbnails for files with no duration"));
    return;
  }

  pMC->Pause();
  GetMediaState(); // wait for completion of the pause command

  //

  CSize video, wh(0, 0), arxy(0, 0);

  if (m_pMFVDC)
  {
    m_pMFVDC->GetNativeVideoSize(&wh, &arxy);
  }
  else if(m_pCAPR)
  {
    wh = m_pCAPR->GetVideoSize(false);
    arxy = m_pCAPR->GetVideoSize(true);
  }
  else
  {
    pBV->GetVideoSize(&wh.cx, &wh.cy);

    long arx = 0, ary = 0;
    CComQIPtr<IBasicVideo2> pBV2 = pBV;
    if(pBV2 && SUCCEEDED(pBV2->GetPreferredAspectRatio(&arx, &ary)) && arx > 0 && ary > 0)
      arxy.SetSize(arx, ary);
  }

  if(wh.cx <= 0 || wh.cy <= 0)
  {
    AfxMessageBox(_T("Failed to get video frame size"));
    return;
  }

  // with the overlay mixer IBasicVideo2 won't tell the new AR when changed dynamically
  DVD_VideoAttributes VATR;
  if(m_iPlaybackMode == PM_DVD && SUCCEEDED(pDVDI->GetCurrentVideoAttributes(&VATR)))
    arxy.SetSize(VATR.ulAspectX, VATR.ulAspectY);

  video = (arxy.cx <= 0 || arxy.cy <= 0) ? wh : CSize(MulDiv(wh.cy, arxy.cx, arxy.cy), wh.cy);

  //

  AppSettings& s = AfxGetAppSettings();

  int cols = max(1, min(8, s.ThumbCols));
  int rows = max(1, min(8, s.ThumbRows));

  int margin = 5;
  int infoheight = 70;
  int width = max(256, min(2048, s.ThumbWidth));
  int height = width * video.cy / video.cx * rows / cols + infoheight;

  int dibsize = sizeof(BITMAPINFOHEADER) + width*height*4;

  CAutoVectorPtr<BYTE> dib;
  if(!dib.Allocate(dibsize))
  {
    AfxMessageBox(_T("Out of memory, go buy some more!"));
    return;
  }

  BITMAPINFOHEADER* bih = (BITMAPINFOHEADER*)(BYTE*)dib;
  memset(bih, 0, sizeof(BITMAPINFOHEADER));
  bih->biSize = sizeof(BITMAPINFOHEADER);
  bih->biWidth = width;
  bih->biHeight = height;
  bih->biPlanes = 1;
  bih->biBitCount = 32;
  bih->biCompression = BI_RGB;
  bih->biSizeImage = width*height*4;
  memsetd(bih + 1, 0xffffff, bih->biSizeImage);

  SubPicDesc spd;
  spd.w = width;
  spd.h = height;
  spd.bpp = 32;
  spd.pitch = -width*4;
  spd.bits = (BYTE*)(bih + 1) + (width*4)*(height-1);

  {
    BYTE* p = (BYTE*)spd.bits;
    for(int y = 0; y < spd.h; y++, p += spd.pitch)
      for(int x = 0; x < spd.w; x++)
        ((DWORD*)p)[x] = 0x010101 * (0xe0 + 0x08*y/spd.h + 0x18*(spd.w-x)/spd.w);
  }

  CCritSec csSubLock;
  RECT bbox;

  for(int i = 1, pics = cols*rows; i <= pics; i++)
  {
    REFERENCE_TIME rt = rtDur * i / (pics+1);
    DVD_HMSF_TIMECODE hmsf = RT2HMSF(rt, 25);

    GetMainFrame()->SeekTo(rt, 0);

    m_VolumeBeforeFrameStepping = GetToolBar()->Volume;
    pBA->put_Volume(-10000);

    HRESULT hr = pFS ? pFS->Step(1, NULL) : E_FAIL;

    if(FAILED(hr))
    {
      pBA->put_Volume(m_VolumeBeforeFrameStepping);
      AfxMessageBox(_T("Cannot frame step, try a different video renderer."));
      return;
    }

    HANDLE hGraphEvent = NULL;
    pME->GetEventHandle((OAEVENT*)&hGraphEvent);

    while(hGraphEvent && WaitForSingleObject(hGraphEvent, INFINITE) == WAIT_OBJECT_0)
    {
      LONG evCode = 0, evParam1, evParam2;
      while(SUCCEEDED(pME->GetEvent(&evCode, (LONG_PTR*)&evParam1, (LONG_PTR*)&evParam2, 0)))
      {
        pME->FreeEventParams(evCode, evParam1, evParam2);
        if(EC_STEP_COMPLETE == evCode) hGraphEvent = NULL;
      }
    }

    pBA->put_Volume(m_VolumeBeforeFrameStepping);

    int col = (i-1)%cols;
    int row = (i-1)/cols;

    CSize s((width-margin*2)/cols, (height-margin*2-infoheight)/rows);
    CPoint p(margin+col*s.cx, margin+row*s.cy+infoheight);
    CRect r(p, s);
    r.DeflateRect(margin, margin);

    CRenderedTextSubtitle rts(&csSubLock);
    rts.CreateDefaultStyle(0);
    rts.m_dstScreenSize.SetSize(width, height);
    STSStyle* style = new STSStyle();
    style->marginRect.SetRectEmpty();
    rts.AddStyle(_T("thumbs"), style);

    CStringW str;
    str.Format(L"{\\an7\\1c&Hffffff&\\4a&Hb0&\\bord1\\shad4\\be1}{\\p1}m %d %d l %d %d %d %d %d %d{\\p}", 
      r.left, r.top, r.right, r.top, r.right, r.bottom, r.left, r.bottom);
    rts.Add(str, true, 0, 1, _T("thumbs"));
    str.Format(L"{\\an3\\1c&Hffffff&\\3c&H000000&\\alpha&H80&\\fs16\\b1\\bord2\\shad0\\pos(%d,%d)}%02d:%02d:%02d", 
      r.right-5, r.bottom-3, hmsf.bHours, hmsf.bMinutes, hmsf.bSeconds);
    rts.Add(str, true, 1, 2, _T("thumbs"));

    rts.Render(spd, 0, 25, bbox);

    BYTE* pData = NULL;
    long size = 0;
    if(!GetDIB(&pData, size)) return;

    BITMAPINFO* bi = (BITMAPINFO*)pData;

    if(bi->bmiHeader.biBitCount != 32)
    {
      delete [] pData;
      CString str;
      str.Format(ResStr(IDS_MSG_WARN_NOT_CAPABLE_IMAGE_CAPTURE), bi->bmiHeader.biBitCount);
      AfxMessageBox(str);
      return;
    }

    int sw = bi->bmiHeader.biWidth;
    int sh = abs(bi->bmiHeader.biHeight);
    int sp = sw*4;
    const BYTE* src = pData + sizeof(bi->bmiHeader);
    if(bi->bmiHeader.biHeight >= 0) {src += sp*(sh-1); sp = -sp;}

    int dw = spd.w;
    int dh = spd.h;
    int dp = spd.pitch;
    BYTE* dst = (BYTE*)spd.bits + spd.pitch*r.top + r.left*4;

    for(DWORD h = r.bottom - r.top, y = 0, yd = (sh<<8)/h; h > 0; y += yd, h--)
    {
      DWORD yf = y&0xff;
      DWORD yi = y>>8;

      DWORD* s0 = (DWORD*)(src + yi*sp);
      DWORD* s1 = (DWORD*)(src + yi*sp + sp);
      DWORD* d = (DWORD*)dst;

      for(DWORD w = r.right - r.left, x = 0, xd = (sw<<8)/w; w > 0; x += xd, w--)
      {
        DWORD xf = x&0xff;
        DWORD xi = x>>8;

        DWORD c0 = s0[xi];
        DWORD c1 = s0[xi+1];
        DWORD c2 = s1[xi];
        DWORD c3 = s1[xi+1];

        c0 = ((c0&0xff00ff) + ((((c1&0xff00ff) - (c0&0xff00ff)) * xf) >> 8)) & 0xff00ff
          | ((c0&0x00ff00) + ((((c1&0x00ff00) - (c0&0x00ff00)) * xf) >> 8)) & 0x00ff00;

        c2 = ((c2&0xff00ff) + ((((c3&0xff00ff) - (c2&0xff00ff)) * xf) >> 8)) & 0xff00ff
          | ((c2&0x00ff00) + ((((c3&0x00ff00) - (c2&0x00ff00)) * xf) >> 8)) & 0x00ff00;

        c0 = ((c0&0xff00ff) + ((((c2&0xff00ff) - (c0&0xff00ff)) * yf) >> 8)) & 0xff00ff
          | ((c0&0x00ff00) + ((((c2&0x00ff00) - (c0&0x00ff00)) * yf) >> 8)) & 0x00ff00;

        *d++ = c0;
      }

      dst += dp;
    }

    rts.Render(spd, 10000, 25, bbox);

    delete [] pData;
  }

  {
    CRenderedTextSubtitle rts(&csSubLock);
    rts.CreateDefaultStyle(0);
    rts.m_dstScreenSize.SetSize(width, height);
    STSStyle* style = new STSStyle();
    style->marginRect.SetRect(margin*2, margin*2, margin*2, height-infoheight-margin);
    style->fontName = s.subdefstyle.fontName;
    rts.AddStyle(_T("thumbs"), style);

    CStringW str;
    str.Format(L"{\\an9\\fs%d\\b0\\bord0\\shad0\\1c&H555555x&}%s", infoheight-10,  
      width >= 550 ? 	ResStr(IDR_MAINFRAME): ResStr(IDR_MAINFRAME_SHORTNAME));

    rts.Add(str, true, 0, 1, _T("thumbs"), _T(""), _T(""), CRect(0,0,0,0), -1);

    DVD_HMSF_TIMECODE hmsf = RT2HMSF(rtDur, 25);

    CPath path(GetPlaylistBar()->GetCur());
    path.StripPath();
    CStringW fn = (LPCTSTR)path;

    CStringW fs;
    WIN32_FIND_DATA wfd;
    HANDLE hFind = FindFirstFile(GetPlaylistBar()->GetCur(), &wfd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
      FindClose(hFind);

      __int64 size = (__int64(wfd.nFileSizeHigh)<<32)|wfd.nFileSizeLow;
      __int64 shortsize = size;
      CStringW measure = _T("B");
      if(shortsize > 10240) shortsize /= 1024, measure = L"KB";
      if(shortsize > 10240) shortsize /= 1024, measure = L"MB";
      if(shortsize > 10240) shortsize /= 1024, measure = L"GB";
      fs.Format(ResStr(IDS_FORMAT_FILE_SIZE), shortsize, measure, size);
    }

    CStringW ar;
    if(arxy.cx > 0 && arxy.cy > 0 && arxy.cx != wh.cx && arxy.cy != wh.cy)
      ar.Format(L"(%d:%d)", arxy.cx, arxy.cy);

    CString szRt = L"\\N";;
    if(fn.GetLength() > 15){
      szRt = L"   ";
    }

    // use bigger font size for english language
    int font_size = (s.iLanguage == 0 || s.iLanguage == 2) ? 18 : 30;
    str.Format(L"{\\an7\\1c&H000000&\\fs%d\\b0\\bord0\\shad0}%s %s\\N%s%s%dx%d %s%s%s %02d:%02d:%02d", 
      font_size, ResStr(IDS_THUMBNAIL_FILENAME),fn, fs,ResStr(IDS_THUMBNAIL_RESOLUTION), 
      wh.cx, wh.cy, ar,szRt, ResStr(IDS_THUMBNAIL_LENGTH), hmsf.bHours,
      hmsf.bMinutes, hmsf.bSeconds);
    rts.Add(str, true, 0, 1, _T("thumbs"));

    rts.Render(spd, 0, 25, bbox);
  }

  SaveDIB(fn, (BYTE*)dib, dibsize);

  GetMainFrame()->SeekTo(rtPos);
}