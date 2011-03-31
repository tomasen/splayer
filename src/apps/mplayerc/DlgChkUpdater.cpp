// DlgChkUpdater.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "DlgChkUpdater.h"
#include "../../svplib/svplib.h"
#include "MainFrm.h"
#include "..\..\..\Updater\cupdatenetlib.h"
#include "Controller\UpdateController.h"

// CDlgChkUpdater dialog

IMPLEMENT_DYNAMIC(CDlgChkUpdater, CDialog)

CDlgChkUpdater::CDlgChkUpdater(CWnd* pParent /*=NULL*/)
: CDialog(CDlgChkUpdater::IDD, pParent)
{

}

CDlgChkUpdater::~CDlgChkUpdater()
{
}

void CDlgChkUpdater::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_PROGRESS1, cprog_chker);
  DDX_Control(pDX, IDC_STATIC1, cs_stat);
  DDX_Control(pDX, IDCANCEL, cb_close);
}


BEGIN_MESSAGE_MAP(CDlgChkUpdater, CDialog)
  ON_WM_TIMER()
  ON_BN_CLICKED(IDCANCEL, &CDlgChkUpdater::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgChkUpdater message handlers

BOOL CDlgChkUpdater::OnInitDialog()
{
  CDialog::OnInitDialog();

  // TODO:  Add extra initialization here
  SetTimer(IDT_CHECK_TICK, 600, NULL);
  cprog_chker.SetRange(0, 1000);
  cprog_chker.SetPos(0);
  m_lostPos = 0;
  moreTick = 0;
  CMPlayerCApp* pApp = (CMPlayerCApp*) AfxGetMyApp();
  if(!pApp->m_cnetupdater)
    UpdateController::GetInstance()->Start();

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgChkUpdater::OnTimer(UINT_PTR nIDEvent)
{
  // TODO: Add your message handler code here and/or call default
  switch( nIDEvent){
    case IDT_CHECK_TICK:
      {
        CMPlayerCApp* pApp = (CMPlayerCApp*) AfxGetMyApp();
        if(pApp->m_cnetupdater){
          if(!pApp->m_cnetupdater->bSVPCU_DONE){
            m_lostPos = (int)(pApp->m_cnetupdater->getProgressBytes() * 9);
            cprog_chker.SetPos(m_lostPos);
            break;
          }
        }
        if(m_lostPos < 500){
          m_lostPos += 80;
        }else if(m_lostPos < 800){
          m_lostPos += 20;
        }else if(m_lostPos < 900){
          m_lostPos += 10;
        }else if(m_lostPos < 960){
          m_lostPos += 4;
        }else{
          m_lostPos += 1;
        }


        cprog_chker.SetPos(m_lostPos);

        HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS,	FALSE, _T("SPLAYER_REAL_UPDATER"));
        if (hMutex ) {
          KillTimer(IDT_CHECK_TICK);
          cprog_chker.ShowWindow(SW_HIDE);
          cs_stat.ShowWindow(SW_SHOW);
          cb_close.SetWindowText(ResStr(IDS_DIABLOG_CLOSE_BUTTON));

          SetTimer(IDT_CHECK_WND, 700, NULL);
        }
      }
      break;
    case IDT_CHECK_WND: 
      {
        HWND hWndPrevious = ::FindWindow( NULL , ResStr(IDS_UPDATER_DIALOG_TITLE_KEYWORD));// ::GetDesktopWindow(),GW_CHILD);
        if(::IsWindow(hWndPrevious)){
          ::ShowWindow(hWndPrevious,SW_SHOWNORMAL);
          ::SetWindowPos(hWndPrevious, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
          KillTimer(IDT_CHECK_WND);
        }
        SetTimer(IDT_CLOSE_WND, 9000, NULL);
      }
      break;
    case IDT_CLOSE_WND:
      KillTimer(IDT_CLOSE_WND);
      OnCancel();
      break;
  }

  CDialog::OnTimer(nIDEvent);
}

void CDlgChkUpdater::OnBnClickedCancel()
{
  // TODO: Add your control notification handler code here
  OnCancel();
}
