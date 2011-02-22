/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

// PPageAudioSwitcher.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "mplayerc.h"
#include "PPageAudioSwitcher.h"

// CPPageAudioSwitcher dialog

IMPLEMENT_DYNAMIC(CPPageAudioSwitcher, CPPageBase)
CPPageAudioSwitcher::CPPageAudioSwitcher(IFilterGraph* pFG)
	: CPPageBase(CPPageAudioSwitcher::IDD, CPPageAudioSwitcher::IDD)
	, m_fAudioNormalize(FALSE)
	, m_fAudioNormalizeRecover(FALSE)
	, m_fDownSampleTo441(FALSE)
	, m_fCustomChannelMapping(FALSE)
	, m_nChannels(0)
	, m_fEnableAudioSwitcher(FALSE)
	, m_dwChannelMask(0)
	, m_tAudioTimeShift(0)
	, m_fAudioTimeShift(FALSE)
	, m_AudioBoost(0)
{
	m_pASF = FindFilter(__uuidof(CAudioSwitcherFilter), pFG);
}

CPPageAudioSwitcher::~CPPageAudioSwitcher()
{
}

void CPPageAudioSwitcher::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK5, m_fAudioNormalize);
	DDX_Check(pDX, IDC_CHECK6, m_fAudioNormalizeRecover);	
	DDX_Slider(pDX, IDC_SLIDER1, m_AudioBoost);
	DDX_Control(pDX, IDC_SLIDER1, m_AudioBoostCtrl);
	DDX_Check(pDX, IDC_CHECK3, m_fDownSampleTo441);
	DDX_Check(pDX, IDC_CHECK1, m_fCustomChannelMapping);
	DDX_Control(pDX, IDC_EDIT1, m_nChannelsCtrl);
	DDX_Text(pDX, IDC_EDIT1, m_nChannels);
	DDX_Control(pDX, IDC_SPIN1, m_nChannelsSpinCtrl);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Check(pDX, IDC_CHECK2, m_fEnableAudioSwitcher);
	DDX_Control(pDX, IDC_CHECK3, m_fDownSampleTo441Ctrl);
	DDX_Control(pDX, IDC_CHECK1, m_fCustomChannelMappingCtrl);
	DDX_Control(pDX, IDC_EDIT2, m_tAudioTimeShiftCtrl);
	DDX_Control(pDX, IDC_SPIN2, m_tAudioTimeShiftSpin);
	DDX_Text(pDX, IDC_EDIT2, m_tAudioTimeShift);
	DDX_Check(pDX, IDC_CHECK4, m_fAudioTimeShift);
	DDX_Control(pDX, IDC_CHECK4, m_fAudioTimeShiftCtrl);
}

BEGIN_MESSAGE_MAP(CPPageAudioSwitcher, CPPageBase)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnNMClickList1)
	ON_WM_DRAWITEM()
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
	ON_UPDATE_COMMAND_UI(IDC_SLIDER1, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_CHECK5, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_CHECK6, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_CHECK3, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_CHECK4, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_EDIT2, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_SPIN2, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_CHECK1, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_EDIT1, OnUpdateChannelMapping)
	ON_UPDATE_COMMAND_UI(IDC_SPIN1, OnUpdateChannelMapping)
	ON_UPDATE_COMMAND_UI(IDC_LIST1, OnUpdateChannelMapping)
	ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdateChannelMapping)
	ON_UPDATE_COMMAND_UI(IDC_STATIC2, OnUpdateChannelMapping)
	ON_UPDATE_COMMAND_UI(IDC_STATIC3, OnUpdateChannelMapping)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CPPageAudioSwitcher message handlers

BOOL CPPageAudioSwitcher::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	m_fEnableAudioSwitcher = s.fEnableAudioSwitcher;
	m_fAudioNormalize = s.fAudioNormalize;
	m_fAudioNormalizeRecover = s.fAudioNormalizeRecover;	
	m_AudioBoost = (int)(50.0f*log10(s.AudioBoost));
	m_AudioBoostCtrl.SetRange(0, 100);
	m_fDownSampleTo441 = s.fDownSampleTo441;
	m_fAudioTimeShift = s.fAudioTimeShift;
	m_tAudioTimeShift = s.tAudioTimeShift;
	m_tAudioTimeShiftSpin.SetRange32(-1000*60*60*24, 1000*60*60*24);
	m_fCustomChannelMapping = s.fCustomChannelMapping;
	memcpy(m_pSpeakerToChannelMap2, s.pSpeakerToChannelMap2, sizeof(s.pSpeakerToChannelMap2));

	if(m_pASF)
		m_pASF->GetInputSpeakerConfig(&m_dwChannelMask);

	m_nChannels = 6;
	m_nChannelsSpinCtrl.SetRange(1, 18);

	if(m_pASF)
		m_nChannels = m_pASF->GetNumberOfInputChannels();		

	//m_nSpeakers = AfxGetMyApp()->GetNumberOfSpeakers();

	m_list.InsertColumn(0, _T(""), LVCFMT_LEFT, 100);
	m_list.InsertItem(0, _T(""));
	for(int i = 0; i < min(18, m_nSpeakers); i++ ){
		//if(m_nSpeakers > 5 && i == (m_nSpeakers-1)){
//			m_list.InsertItem(i+1, ResStr(IDS_CHANNAPMAP_TABLE_LABEL_LOW_FREQUENCY));
//			continue;
//		}
		if(m_iSS = 220){
			if(m_nSpeakers > 2 && m_nSpeakers <= 4 && i >= 2){
				m_list.InsertItem(i+1, ResStr(IDS_CHANNAPMAP_TABLE_LABEL_FRONT_LEFT+i+2));
				continue;
			}
		}
	
		m_list.InsertItem(i+1, ResStr(IDS_CHANNAPMAP_TABLE_LABEL_FRONT_LEFT+i));
	}
	
	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);

	for(int i = 1; i <= 18; i++)
	{
		m_list.InsertColumn(i, _T(""), LVCFMT_CENTER, 37);
		CString n;
		n.Format(_T("%d"), i);
		m_list.SetItemText(0, i, n);
//		m_list.SetColumnWidth(i, LVSCW_AUTOSIZE);
//		m_list.SetColumnWidth(i, m_list.GetColumnWidth(i)*8/10);
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageAudioSwitcher::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.fEnableAudioSwitcher = true;//!!m_fEnableAudioSwitcher; alaways enable
	s.fAudioNormalize = !!m_fAudioNormalize;
	s.fAudioNormalizeRecover = !!m_fAudioNormalizeRecover;
	s.AudioBoost = (float)pow(10.0, (double)m_AudioBoost/50);
	s.fDownSampleTo441 = false; //!!m_fDownSampleTo441; alaways disable
	s.fAudioTimeShift = !!m_fAudioTimeShift;
	s.tAudioTimeShift = m_tAudioTimeShift;
	//s.fCustomChannelMapping = !!m_fCustomChannelMapping;
	//memcpy(s.pSpeakerToChannelMap, m_pSpeakerToChannelMap, sizeof(m_pSpeakerToChannelMap));

	{
		int iInputChannelCount = m_nChannels; 
		int iOutputChannelCount = m_nSpeakers;
				bool bHasCustomSetting = false;
				for(int iSpeakerID = 0; iSpeakerID < iOutputChannelCount; iSpeakerID++){
					for(int iChannelID = 0; iChannelID < iInputChannelCount; iChannelID++){
						if(s.pSpeakerToChannelMap2 [iInputChannelCount-1][iOutputChannelCount-1][iSpeakerID][iChannelID] != 
							m_pSpeakerToChannelMap2 [iInputChannelCount-1][iOutputChannelCount-1][iSpeakerID][iChannelID]){
							bHasCustomSetting = true;
							break;
						}
					}
					if(bHasCustomSetting)
						break;
				}
				if(bHasCustomSetting){
					for(int iSpeakerID = 0; iSpeakerID < iOutputChannelCount; iSpeakerID++){
						for(int iChannelID = 0; iChannelID < iInputChannelCount; iChannelID++){
							s.pSpeakerToChannelMap2Custom[iInputChannelCount-1][iOutputChannelCount-1][iSpeakerID][iChannelID] =
								m_pSpeakerToChannelMap2[iInputChannelCount-1][iOutputChannelCount-1][iSpeakerID][iChannelID] ;

						}
					}
					s.InitChannelMap();
					s.ChangeChannelMapByCustomSetting();
				}
		
	}

	if(m_pASF)
	{
	//	m_pASF->SetSpeakerConfig(s.fCustomChannelMapping, s.pSpeakerToChannelMap);
		m_pASF->SetSpeakerChannelConfig(m_nSpeakers, s.pSpeakerToChannelMap2, s.pSpeakerToChannelMapOffset,0,s.iSS);
		m_pASF->EnableDownSamplingTo441(s.fDownSampleTo441);
		m_pASF->SetAudioTimeShift(s.fAudioTimeShift ? 10000i64*s.tAudioTimeShift : 0);
		m_pASF->SetNormalizeBoost(s.fAudioNormalize, s.fAudioNormalizeRecover, s.AudioBoost);
		m_pASF->SetEQControl(s.pEQBandControlPerset, s.pEQBandControlCustom);
	}

	return __super::OnApply();
}

void CPPageAudioSwitcher::OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{

	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

	if(lpnmlv->iItem > 0 && lpnmlv->iSubItem > 0 && lpnmlv->iSubItem <= m_nChannels)
	{
		UpdateData();
		//m_pSpeakerToChannelMap[m_nChannels-1][lpnmlv->iItem-1] ^= 1<<(lpnmlv->iSubItem-1);
		int iInputChannelCount = m_nChannels;
		int iChannelID = lpnmlv->iSubItem-1;
		float fTmpVal = 1.0;
		if( iInputChannelCount > 2 && iChannelID < 2){	 // 前置左右声道
			fTmpVal = 1.0;
		}else if( iInputChannelCount > 4 && iChannelID == 2){ //中置声道
			fTmpVal = 2.0;
		}else if( iInputChannelCount > 5 && iChannelID == (iInputChannelCount - 1) ){ //重低音 降低
			fTmpVal = 0.9;
		}else if(iInputChannelCount > 2 && iChannelID >= 2){ //除中置 重低音外的声道
			fTmpVal = 0.9;
		}
		if(m_pSpeakerToChannelMap2[m_nChannels-1][m_nSpeakers-1][lpnmlv->iItem-1][lpnmlv->iSubItem-1] > 0)
		{
			m_pSpeakerToChannelMap2[m_nChannels-1][m_nSpeakers-1][lpnmlv->iItem-1][lpnmlv->iSubItem-1] = 0;
		}else{
			m_pSpeakerToChannelMap2[m_nChannels-1][m_nSpeakers-1][lpnmlv->iItem-1][lpnmlv->iSubItem-1] = fTmpVal;
		}
		
		m_list.RedrawItems(lpnmlv->iItem, lpnmlv->iItem);
		SetModified();

		if(GetKeyState(VK_SHIFT) & 0x8000)
		{
			OnApply();
		}
	}

	*pResult = 0;
}

void CPPageAudioSwitcher::OnEnChangeEdit1()
{
	if(IsWindow(m_list.m_hWnd))
	{
		UpdateData();
		m_list.Invalidate();
	}
}

#include <math.h>

void CPPageAudioSwitcher::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if(nIDCtl != IDC_LIST1) return;

//	if(lpDrawItemStruct->itemID == 0)
//		UpdateData();

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	
	pDC->SetBkMode(TRANSPARENT);

	CPen p(PS_INSIDEFRAME, 1, 0xe0e0e0);
	CPen* old = pDC->SelectObject(&p);

	pDC->MoveTo(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.bottom-1);
	pDC->LineTo(lpDrawItemStruct->rcItem.right, lpDrawItemStruct->rcItem.bottom-1);

	CHeaderCtrl* pHeader = m_list.GetHeaderCtrl();
	int nColumnCount = pHeader->GetItemCount();

	for(int i = 0; i < nColumnCount; i++)
	{
		CRect r, rb;
		m_list.GetSubItemRect(lpDrawItemStruct->itemID, i, LVIR_BOUNDS, rb);
		m_list.GetSubItemRect(lpDrawItemStruct->itemID, i, LVIR_LABEL, r);

		pDC->MoveTo(r.right-1, r.top);
		pDC->LineTo(r.right-1, r.bottom-1);

		CSize s = pDC->GetTextExtent(m_list.GetItemText(lpDrawItemStruct->itemID, i));

		if(i == 0)
		{
			r.left = rb.left;

			if(lpDrawItemStruct->itemID == 0)
			{
				pDC->MoveTo(0, 0);
				pDC->LineTo(r.right, r.bottom-1);
			}
			else
			{
				pDC->SetTextColor(m_list.IsWindowEnabled() ? 0 : 0xb0b0b0);
				pDC->TextOut(r.left+1, (r.top+r.bottom-s.cy)/2, m_list.GetItemText(lpDrawItemStruct->itemID, i));
			}
		}
		else
		{
			pDC->SetTextColor(i > m_nChannels ? 0xe0e0e0 : (!m_list.IsWindowEnabled() ? 0xb0b0b0 : 0));

			if(lpDrawItemStruct->itemID == 0)
			{
				pDC->TextOut((r.left+r.right-s.cx)/2, (r.top+r.bottom-s.cy)/2, m_list.GetItemText(lpDrawItemStruct->itemID, i));
			}
			else
			{
				if(m_dwChannelMask & (1<<(lpDrawItemStruct->itemID-1)))
				{
					int nBitsSet = 0;

					for(int j = 1; j <= (1<<(lpDrawItemStruct->itemID-1)); j <<= 1)
					{
						if(m_dwChannelMask & j)
							nBitsSet++;
					}

					if(nBitsSet == i)
					{
						COLORREF tmp = pDC->GetTextColor();

						pDC->SetTextColor(0xe0e0e0);
						CFont f;
						f.CreatePointFont(MulDiv(100, 96, pDC->GetDeviceCaps(LOGPIXELSX)), _T("Marlett"));
						CFont* old = pDC->SelectObject(&f);
						s = pDC->GetTextExtent(_T("g"));
						pDC->TextOut((r.left+r.right-s.cx)/2, (r.top+r.bottom-s.cy)/2, _T("g"));

						pDC->SetTextColor(tmp);
					}
				}
                try{
				    if(m_pSpeakerToChannelMap2[m_nChannels-1][m_nSpeakers-1][lpDrawItemStruct->itemID-1][i-1] > 0)
				    {
					    if(0){
						    CFont f;
						    f.CreatePointFont(MulDiv(70, 96, pDC->GetDeviceCaps(LOGPIXELSX)), _T("MS Sans Serif"));
						    CFont* old = pDC->SelectObject(&f);
						    CString szVal;
						    szVal.Format(L"%0.2f", m_pSpeakerToChannelMap2[m_nChannels-1][m_nSpeakers-1][lpDrawItemStruct->itemID-1][i-1]);
						    s = pDC->GetTextExtent(szVal);
						    pDC->TextOut((r.left+r.right-s.cx)/2, (r.top+r.bottom-s.cy)/2, szVal);
						    pDC->SelectObject(old);
					    }else{
						    CFont f;
						    f.CreatePointFont(MulDiv(100, 96, pDC->GetDeviceCaps(LOGPIXELSX)), _T("Marlett"));
						    CFont* old = pDC->SelectObject(&f);
						    s = pDC->GetTextExtent(_T("a"));
						    pDC->TextOut((r.left+r.right-s.cx)/2, (r.top+r.bottom-s.cy)/2, _T("a"));
						    pDC->SelectObject(old);
					    }
    					
				    }
                 }catch(...) {  }
			}
            
		}
	}

	pDC->SelectObject(old);
}

void CPPageAudioSwitcher::OnUpdateAudioSwitcher(CCmdUI* pCmdUI)
{
//	UpdateData();
	pCmdUI->Enable(1/*m_fEnableAudioSwitcher*/);//IsDlgButtonChecked(IDC_CHECK2)
}

void CPPageAudioSwitcher::OnUpdateChannelMapping(CCmdUI* pCmdUI)
{
//	UpdateData();
	pCmdUI->Enable(1);
	//IsDlgButtonChecked(IDC_CHECK2)/*m_fEnableAudioSwitcher*/ 
	//&& IsDlgButtonChecked(IDC_CHECK1)/*m_fCustomChannelMapping*/
}

void CPPageAudioSwitcher::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SetModified();

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}
