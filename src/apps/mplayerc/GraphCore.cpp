#include "stdafx.h"
#include "GraphCore.h"

// TODO try to remove
#include "..\..\svplib\SVPToolBox.h"

CGraphCore::CGraphCore(void):
  m_fCustomGraph(false),
  m_fRealMediaGraph(false),
  m_fShockwaveGraph(false),
  m_fQuicktimeGraph(false)
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
