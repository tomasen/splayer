#include "stdafx.h"
#include "GraphCore.h"
#include "mplayerc.h"

#include "Controller/HashController.h"

// TODO try to remove
#include "..\..\svplib\SVPToolBox.h"

//extern class MainFrame;

CGraphCore::CGraphCore(void):
  m_fCustomGraph(false),
  m_fRealMediaGraph(false),
  m_fShockwaveGraph(false),
  m_fQuicktimeGraph(false),
  m_iSubtitleSel(-1),
  m_iSubtitleSel2(-1)
{
}

CGraphCore::~CGraphCore(void)
{
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
