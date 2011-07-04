#include <streams.h>
#include "WavPackDSDecoderAboutProp.h"
#include "resource.h"

//-----------------------------------------------------------------------------

const char* GetDllVersion(const char* filename)
{
    static TCHAR szVersion[32];	
    
    szVersion[0] = '\0';
    
    if(filename == NULL || filename[0] == '\0')
    {
        return szVersion;
    }
    
    DWORD dwHandle = 0;
    DWORD VersionInfoSize = GetFileVersionInfoSize(filename,&dwHandle);
    if(VersionInfoSize > 0)
    {
        void* pVersionInfo = new BYTE[VersionInfoSize];
        if(GetFileVersionInfo(filename, dwHandle, VersionInfoSize, pVersionInfo) == TRUE)
        {
            UINT uLen;
            VS_FIXEDFILEINFO *lpFfi;
            if(VerQueryValue(pVersionInfo, TEXT("\\"), (LPVOID *)&lpFfi, &uLen) == TRUE)
            {
                WORD w1, w2, w3, w4;
                w1 = (WORD)(lpFfi->dwFileVersionMS >> 16);
                w2 = (WORD)(lpFfi->dwFileVersionMS & 0xFFFF);
                w3 = (WORD)(lpFfi->dwFileVersionLS >> 16);
                w4 = (WORD)(lpFfi->dwFileVersionLS & 0xFFFF);
                wsprintf(szVersion,TEXT("%d.%d.%d.%d\0"),w1,w2,w3,w4);
            }
            delete[] pVersionInfo;
        }
    }
    return szVersion;
}

//-----------------------------------------------------------------------------

CUnknown *WINAPI CWavPackDSDecoderAboutProp::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
	CWavPackDSDecoderAboutProp *pNewObject = new CWavPackDSDecoderAboutProp(punk, phr);
	if (!pNewObject)
		*phr = E_OUTOFMEMORY;
	return pNewObject;
}

//-----------------------------------------------------------------------------

CWavPackDSDecoderAboutProp::CWavPackDSDecoderAboutProp(LPUNKNOWN pUnk, HRESULT *phr) :
	CBasePropertyPage(NAME("About"), pUnk, IDD_DIALOG_ABOUT, IDS_ABOUT)
{
	
}

//-----------------------------------------------------------------------------

CWavPackDSDecoderAboutProp::~CWavPackDSDecoderAboutProp()
{

}

//-----------------------------------------------------------------------------

HRESULT CWavPackDSDecoderAboutProp::OnActivate()
{
    char version[512];

    TCHAR szExeName[MAX_PATH];  

    GetModuleFileName(g_hInst, szExeName, sizeof (szExeName)); 

    wsprintf(version, "Version %s - ("__DATE__", "__TIME__")",
        GetDllVersion(szExeName));

	SetDlgItemText(m_hwnd, IDC_LABEL_VERSION, version);

	return S_OK;
}

//-----------------------------------------------------------------------------

BOOL CWavPackDSDecoderAboutProp::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);		
}

//-----------------------------------------------------------------------------