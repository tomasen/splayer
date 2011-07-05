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

// OpenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "OpenURLDlg.h"

// COpenDlg dialog

//IMPLEMENT_DYNAMIC(COpenDlg, CResizableDialog)
COpenURLDlg::COpenURLDlg(CWnd* pParent /*=NULL*/)
: CResizableDialog(COpenURLDlg::IDD, pParent)
, m_path(_T(""))
, m_path2(_T(""))
, m_fMultipleFiles(false)
, m_fAppendPlaylist(FALSE)
{
}

COpenURLDlg::~COpenURLDlg()
{
}

void COpenURLDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_mrucombo);
	DDX_CBString(pDX, IDC_COMBO1, m_path);

	DDX_Control(pDX, IDC_STATIC1, m_label2);

}


BEGIN_MESSAGE_MAP(COpenURLDlg, CResizableDialog)

	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdateDub)


END_MESSAGE_MAP()


// COpenDlg message handlers

BOOL COpenURLDlg::OnInitDialog()
{
	__super::OnInitDialog();

	CRecentFileList& MRUUrl = AfxGetAppSettings().MRUUrl;
	MRUUrl.ReadList();
	m_mrucombo.ResetContent();
	for(int i = 0; i < MRUUrl.GetSize(); i++)
		if(!MRUUrl[i].IsEmpty())
			m_mrucombo.AddString(MRUUrl[i]);
	CorrectComboListWidth(m_mrucombo, GetFont());

	if(m_mrucombo.GetCount() > 0) m_mrucombo.SetCurSel(0);

	AddAnchor(m_mrucombo, TOP_LEFT, TOP_RIGHT);

	AddAnchor(IDOK, TOP_RIGHT);
	AddAnchor(IDCANCEL, TOP_RIGHT);
	AddAnchor(IDC_STATIC1, TOP_LEFT, TOP_RIGHT);

	CRect r;
	GetWindowRect(r);
	CSize s = r.Size();
	SetMinTrackSize(s);
	s.cx = 1000;
	SetMaxTrackSize(s);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



void COpenURLDlg::OnBnClickedOk()
{
	UpdateData();

	m_fns.RemoveAll();
	m_fns.AddTail(m_path);
	

	m_fMultipleFiles = false;

	OnOK();
}

void COpenURLDlg::OnUpdateDub(CCmdUI* pCmdUI)
{
	m_mrucombo.GetWindowText(m_path);
	pCmdUI->Enable(AfxGetAppSettings().Formats.GetEngine(m_path) == DirectShow);
}
