#include "stdafx.h"
#include "GraphCore.h"


CGraphCore::CGraphCore(void)
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

