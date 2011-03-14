#include "stdafx.h"
#include "GraphCore.h"


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
