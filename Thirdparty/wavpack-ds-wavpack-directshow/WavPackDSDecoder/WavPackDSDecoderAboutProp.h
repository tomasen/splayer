//-----------------------------------------------------------------------------
#ifndef _WAVPACKDSDECODERABOUTPROP_H_
#define _WAVPACKDSDECODERABOUTPROP_H_
//-----------------------------------------------------------------------------

class CWavPackDSDecoderAboutProp : public CBasePropertyPage
{
public:
	static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);	
	CWavPackDSDecoderAboutProp(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CWavPackDSDecoderAboutProp();
	HRESULT OnActivate();
	BOOL OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//-----------------------------------------------------------------------------
#endif // _WAVPACKDSDECODERABOUTPROP_H_
//-----------------------------------------------------------------------------