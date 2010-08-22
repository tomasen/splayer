// UpdaterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Updater.h"
#include "UpdaterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUpdaterDlg dialog

UINT __cdecl ThreadCheckUpdate( LPVOID lpParam ) 
{ 
	cupdatenetlib* cup = (cupdatenetlib*)lpParam;
	
	cup->procUpdate();

	
	return 0; 
}
CString getSizeMeasure(__int64 iSize){
	CString measure = _T("B");
	if(iSize > 1024) iSize /= 1024, measure = L"KB";
	if(iSize > 1024) iSize /= 1024, measure = L"MB";
	if(iSize > 1024) iSize /= 1024, measure = L"GB";
	return measure;
}
CString getShortSize(__int64 iSize){
	
	double shortsize = iSize;
	CString measure = _T("B");
	if(shortsize > 1024) shortsize /= 1024, measure = L"KB";
	if(shortsize > 1024) shortsize /= 1024, measure = L"MB";
	if(shortsize > 1024) shortsize /= 1024, measure = L"GB";

	CString szRet ;
	szRet.Format(_T("%0.2f%s"), shortsize , measure);
	return  szRet;
}
CUpdaterDlg::CUpdaterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdaterDlg::IDD, pParent)
,bHide(1)
,m_bGoodToGo(0)
,verbose(0)
, notYetShow(1)
, m_firstDown(0)
, m_nLanguage(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, STATIC_CURRENT, csCurTask);
	DDX_Control(pDX, STATIC_TOTAL, csTotalProgress);
	DDX_Control(pDX, IDC_PROGRESS1, prg_total);
	DDX_Control(pDX, IDC_STATIC_DONE, cs_stat);
	DDX_Control(pDX, IDC_SYSLINK1, cslink);
	DDX_Control(pDX, IDC_BUTTON1, cb_stop);
	DDX_Control(pDX, IDC_STATIC_SPEED, szSpeed);
	DDX_Control(pDX, STATIC_CURRENT2, cszSizeTotal);
}

BEGIN_MESSAGE_MAP(CUpdaterDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_NOTIFYICON, On_WM_NOTIFYICON)
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CUpdaterDlg::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CUpdaterDlg::OnBnClickedButton1)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, &CUpdaterDlg::OnNMClickSyslink1)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


// CUpdaterDlg message handlers

BOOL CUpdaterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CSVPToolBox svpTool;
	CString szLangDefault = svpTool.GetPlayerPath(  _T("lang\\default") );
	CString szLangSeting;
	m_nLanguage = 0;
		BOOL langSeted = false;
		//get default lang setting

		if(svpTool.ifFileExist(szLangDefault)){
			szLangSeting = svpTool.fileGetContent(szLangDefault);
			if(!szLangSeting.IsEmpty()){
				m_nLanguage = _wtoi( szLangSeting );
				langSeted = true;
			}
		}
		if(!langSeted){
			
			switch(GetSystemDefaultLangID()){ //http://www.science.co.il/Language/Locale-Codes.asp?s=codepage
					case 0x0804:
					case 0x1004:
					case 0x1404:
					case 0x0c04:
					case 0x0404: //Chinese 
						m_nLanguage = 0;
						break;
					default:
						m_nLanguage = 1;
						break;
			}
			
		}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	prg_total.SetRange(0, 1000);
	
	HDC hdc = ::GetDC(NULL);
	double dDefaultSize = 22;
	double dIntroSize = 14;
	m_scale = (double)GetDeviceCaps(hdc, LOGPIXELSY) / 96.0;
	double scale = 1.0;
	::ReleaseDC(0, hdc);

	m_hBigFont.m_hObject = NULL;
	

	if(!(::GetVersion()&0x80000000)){
		m_hBigFont.CreateFont(int(dDefaultSize * scale), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, 
		_T("Microsoft Sans Serif"));

		m_hIntroFont.CreateFont(int(dIntroSize * scale), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, 
			_T("Microsoft Sans Serif"));
	}
	if(!m_hBigFont.m_hObject){
		m_hBigFont.CreateFont(int(dDefaultSize * scale), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, 
		_T("MS Sans Serif"));


		m_hIntroFont.CreateFont(int(dIntroSize * scale), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, 
			_T("MS Sans Serif"));

	}
	tnid.cbSize = sizeof(NOTIFYICONDATA); 
	tnid.hWnd = this->m_hWnd; 
	tnid.uID = IDR_MAINFRAME; 
	tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
	tnid.uCallbackMessage = WM_NOTIFYICON; 
	tnid.hIcon = this->m_hIcon; 

	if(m_nLanguage ){
		SetWindowText(L"SPlayer Updater");
		csCurTask.SetWindowText(L"Current Task:");
		csTotalProgress.SetWindowText(L"Total:");
		szSpeed.SetWindowText(L"Speed:");
		cb_stop.SetWindowText(L"Cancel");
		cslink.SetWindowText(L"<a href=\"http://blog.splayer.org\">ChangeLog...</a>");
		cszSizeTotal.SetWindowText(L"Size:");
		
		cs_stat.SetWindowText(_T("This program will update lastest SPlayer, will exit automaticly after finished."));
		csCurTask.SetWindowText(_T("Current Task: Calc..."));
		wcscpy_s(tnid.szTip, _T("SPlayer Auto Updater"));

		szaIntro.Add(L"SPlayer support DXVA、DXVA2、EVR");

		szaIntro.Add(_T("SPlayer optimization of a variety of creative techniques, smooth playback. Maximize the efficiency are always our mission "));
		szaIntro.Add(_T("SPlayer mainly focused on user experience"));
		szaIntro.Add(_T("SPlayer is an open source software "));
		szaIntro.Add(_T("Designer Raven from Milan created a set of interfaces. Beautiful skin can let you feel more comfortable while playing"));
		szaIntro.Add(_T("SPlayer is world's smallest and most sophisticated players, complete installation package only 5M, still as powerful as ever ..."));
		szaIntro.Add(_T("SPlayer origin from MPCHC ffmpeg and thanks everyone with them"));
		szaIntro.Add(_T("SPlayer can download subtitle aotomaticely, you can disable it in setting panel if you want"));

	}else{
		cs_stat.SetWindowText(_T("本程序下载最新版的射手播放器，将在更新完成后5分钟内退出"));
		csCurTask.SetWindowText(_T("当前任务：正在计算下载量"));
		wcscpy_s(tnid.szTip, _T("射手影音播放器自动更新程序"));

		szaIntro.Add(_T("只要在设置中启用“智能拖拽”，鼠标拖住画面就可以方便的改变画面比例"));
		szaIntro.Add(_T("在界面设置中可以设置自定义的背景界面"));
		szaIntro.Add(_T("射手播放器无需安装额外解码包即可全能解码"));

		szaIntro.Add(_T("免配置智能启用硬件高清加速：DXVA、DXVA2、EVR和CUDA"));
		szaIntro.Add(_T("十余种画面效果功能组合，全新的视频观赏体验。"));
		szaIntro.Add(_T("全自动网络字幕匹配。抛弃浏览器，也不必再收集。只要一人播放过且同意共享，全球华人都可在播放时得到恰好匹配影片的字幕。"));
		szaIntro.Add(_T("双字幕显示，同时显示中英双语，共同学习提高。"));
		szaIntro.Add(_T("绿色版免安装，同时支持海量视频格式，永不流氓！"));
		szaIntro.Add(_T("与众不同的字幕默认字体与颜色，对抗视觉疲劳。"));
		szaIntro.Add(_T("智能识别简体或繁体字幕。告别乱码，也不用再手动转码。"));
		szaIntro.Add(_T("自动减小英文字码，双语字幕更美观优雅。"));
		szaIntro.Add(_T("根据使用习惯优化右键菜单，方便快速切换字幕和显示模式。"));
		szaIntro.Add(_T("更加易用、精简的控制面板，更方便且易于理解的设置选项。"));
		szaIntro.Add(_T("自动升级随时更新最新版本、体验最新功能。"));
		szaIntro.Add(_T("射手播放器拥护GPL授权协议，是开源软件。"));

	}

	SetTimer(IDT_START_CHECK, 1000, NULL);



	
	Shell_NotifyIcon(NIM_ADD, &tnid); 



	
	if(bHide){
		ShowWindow(SW_MINIMIZE);
		ShowWindow(SW_HIDE);

	}else{
		notYetShow = false;
		ShowWindow(SW_SHOW);
		ShowWindow(SW_RESTORE);
	}
	
	return FALSE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CUpdaterDlg::OnPaint()
{
	if (IsIconic())
	{
		
			CPaintDC dc(this); // device context for painting

			SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

			// Center icon in client rectangle
			int cxIcon = GetSystemMetrics(SM_CXICON);
			int cyIcon = GetSystemMetrics(SM_CYICON);
			CRect rect;
			GetClientRect(&rect);
			int x = (rect.Width() - cxIcon + 1) / 2;
			int y = (rect.Height() - cyIcon + 1) / 2;

			// Draw the icon
			dc.DrawIcon(x, y, m_hIcon);
		
	}else{

		//CDialog::OnPaint();
		
		CPaintDC dc(this);
		dc.DrawIcon(m_scale*34, m_scale*25, m_hIcon);
		dc.SetBkMode(TRANSPARENT);
		HFONT oldFont = (HFONT)dc.SelectObject((HFONT) m_hBigFont);
		dc.SetTextColor( 0x353535);
		if(m_nLanguage){
			dc.DrawText(_T("SPlayer Updater"), CRect(m_scale*80, m_scale*25, m_scale * 230, m_scale * 85),DT_LEFT );
		}else
			dc.DrawText(_T("射手播放器\r\n自动升级程序"), CRect(m_scale*80, m_scale*25, m_scale * 230, m_scale * 85),DT_LEFT );
		dc.SelectObject((HFONT) m_hIntroFont);
		if(m_nLanguage){
			dc.DrawText(_T("Intro:"), CRect(m_scale*39, m_scale*110, m_scale*200,m_scale*130),DT_LEFT );
		}else
			dc.DrawText(_T("软件介绍："), CRect(m_scale*39, m_scale*110, m_scale*200,m_scale*130),DT_LEFT );

		dc.SelectObject((HFONT) oldFont);
	}
	
	
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CUpdaterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CUpdaterDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if ( nType == SIZE_MINIMIZED )
	{
		this->ShowWindow(SW_HIDE);
	}
}


LRESULT CUpdaterDlg::On_WM_NOTIFYICON(WPARAM wParam, LPARAM lParam) 
{ 
	UINT uID, uMouseMsg; 

	uID = (UINT) wParam; 
	uMouseMsg = (UINT) lParam; 

	if ( uID == IDR_MAINFRAME && ( uMouseMsg == WM_LBUTTONUP || uMouseMsg == WM_RBUTTONUP)){

		bHide = FALSE;
 		this->ShowWindow(SW_SHOW);
 		this->ShowWindow(SW_RESTORE);
 		this->BringWindowToTop();
	}
	return 1; 
}
void CUpdaterDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	switch(nIDEvent){
		case IDT_START_CHECK:
			{
				KillTimer(IDT_START_CHECK);
				notYetShow = false;
				if(cup.downloadList()){
					
					if(m_nLanguage){
						cs_stat.SetWindowText(_T("Updating to latest SPlayer..."));

						csCurTask.SetWindowText(_T("Current Task: Updating..."));

						cszSizeTotal.SetWindowText(  CString(L"Fize: ") + getShortSize(cup.iSVPCU_TOTAL_FILEBYTE) );

						csTotalProgress.SetWindowText( CString(L"Decompressed： ") + getShortSize(cup.iSVPCU_CURRENT_FILEBYTE_DONE) );
					}else{
						cs_stat.SetWindowText(_T("正在更新到射手播放器的最新版本..."));

						csCurTask.SetWindowText(_T("当前任务： 正在更新..."));
						
						cszSizeTotal.SetWindowText(  CString(L"文件大小： ") + getShortSize(cup.iSVPCU_TOTAL_FILEBYTE) );

						csTotalProgress.SetWindowText( CString(L"已解压： ") + getShortSize(cup.iSVPCU_CURRENT_FILEBYTE_DONE) );
					}

					SetTimer(IDT_REAL_START_CHECK, 800, NULL);


					SetTimer(IDT_SHOW_INTRO,3500, NULL);

					
				}else{
					if(m_nLanguage){
						cs_stat.SetWindowText(_T("You already have latest version, there is no need to update.\r\nUpdater will exit in 30 sec."));
						csCurTask.SetWindowText(_T("Current Task: Closing..."));
					}else{
						cs_stat.SetWindowText(_T("您已经拥有最新版的射手播放器，目前无需升级。\r\n升级程序将在30秒后退出"));
						csCurTask.SetWindowText(_T("当前任务：正在准备关闭..."));
					}
					SetTimer(IDT_CLOSE_DLG, 30000, NULL);
				}

			}
			break;
		case IDT_SHOW_INTRO:
			if(cup.bWaiting && rand() % 2 == 1){
					if(m_nLanguage){
						cs_stat.SetWindowText( _T("Exit SPlayer or reboot to finish update."));
					}else
						cs_stat.SetWindowText( _T("关闭播放器或重新启动完成本次更新"));

			}else
				cs_stat.SetWindowText(szaIntro.GetAt( rand() % szaIntro.GetCount() ));

			break;
		case IDT_REAL_START_CHECK:
			{
				
				KillTimer(IDT_REAL_START_CHECK);
				//cb_backgd.EnableWindow(true);
				AfxBeginThread(ThreadCheckUpdate , (LPVOID)&cup);
				if(m_nLanguage){
					csCurTask.SetWindowText(_T("Current Task: Downloading..."));
				}else
					csCurTask.SetWindowText(_T("当前任务： 正在下载..."));
				iTimeStart = time(NULL);
				SetTimer(IDT_REFRESH_STAT, 700, NULL);
			}
			break;
		case IDT_REFRESH_STAT:
			{
				CString szTmp;
				

				if(cup.iSVPCU_TOTAL_FILEBYTE < cup.iSVPCU_TOTAL_FILEBYTE_DONE + cup.iSVPCU_CURRENT_FILEBYTE_DONE){
					cup.iSVPCU_TOTAL_FILEBYTE = cup.iSVPCU_TOTAL_FILEBYTE_DONE + cup.iSVPCU_CURRENT_FILEBYTE_DONE;
				}
				if (cup.iSVPCU_TOTAL_FILEBYTE  <= 0){
					cup.iSVPCU_TOTAL_FILEBYTE = 1;
				}
				double progress = 0;
				if(cup.iSVPCU_TOTAL_FILEBYTE){
					progress = (double)( cup.iSVPCU_TOTAL_FILEBYTE_DONE + cup.iSVPCU_CURRENT_FILEBYTE_DONE ) * 100/ (cup.iSVPCU_TOTAL_FILEBYTE);
				}
				//szTmp.Format( _T("射手影音自动更新程序\n文件：%d/%d 下载：%0.2f%%") , cup.iSVPCU_CURRETN_FILE , cup.iSVPCU_TOTAL_FILE ,progress );
				//SVP_LogMsg(szTmp);

				if(cup.bWaiting){
					if(m_nLanguage){
						szTmp = _T("To be polite, we don't KILL any running program. Pleas exit SPlayer or reboot to finish update.");
						csCurTask.SetWindowText(_T("Current: Trying to overwrite..."));
					}else{
						szTmp = _T("出于礼貌，升级程序不会强行关闭正在运行中的程序。请您关闭播放器或重新启动后将自动更新到最新版本");
						csCurTask.SetWindowText(_T("当前任务：正在覆盖..."));
					}
					cs_stat.SetWindowText(szTmp);


					if(m_firstDown == 80){
						ShowWindow(SW_MINIMIZE);
						ShowWindow(SW_HIDE);
					}
					m_firstDown+=10;

				}
				wcscpy_s(tnid.szTip, szTmp);

//szTmp.Format( _T("正在下载文件： %s （%d / %d）") , cup.szCurFilePath , cup.iSVPCU_CURRETN_FILE , cup.iSVPCU_TOTAL_FILE);
				if(!cup.bWaiting){
					
					double downloadedTotal = cup.iSVPCU_TOTAL_FILEBYTE_DONE + cup.iSVPCU_CURRENT_FILEBYTE_DONE;
					UINT timeNowNow = time(NULL);
					if( timeNowNow  - iTimeStart){
						double speed = downloadedTotal / ( timeNowNow  - iTimeStart);
						CString szBufSpeed;
						if(m_nLanguage){
							szBufSpeed.Format( _T("Speed: %s/sec") , getShortSize(speed) );
						}else
							szBufSpeed.Format( _T("速度： %s/秒") , getShortSize(speed) );
						szSpeed.SetWindowText(szBufSpeed);
					}

					if(m_nLanguage){
						csTotalProgress.SetWindowText( CString(L"Decompressed: ") + getShortSize(downloadedTotal) );
					}else
						csTotalProgress.SetWindowText( CString(L"已解压： ") + getShortSize(downloadedTotal) );

					prg_total.SetPos(int(progress * 10));
					
				}
				//SetWindowText(szTmp);

				//szTmp.Format( _T("总进度：%0.2f%%") , progress);
				

				
				//Shell_NotifyIcon(NIM_MODIFY,&tnid);

				if(cup.bSVPCU_DONE){
					KillTimer(IDT_REFRESH_STAT);
					if(m_nLanguage){
						szTmp = (L"You have finished updating to latest version of SPlayer.\r\nUpdater will exit in 30 sec.");
						csCurTask.SetWindowText(_T("Current Task: Finished"));
					}else{
						szTmp = _T("射手影音播放器已经更新到最新版本！\r\n本程序将在半分钟内关闭");
						csCurTask.SetWindowText(_T("当前任务： 更新已结束"));
					}
					cs_stat.SetWindowText(szTmp);

					if(m_firstDown%10 == 8){
						ShowWindow(SW_MINIMIZE);
						ShowWindow(SW_HIDE);
					}
					m_firstDown++;
					KillTimer(IDT_SHOW_INTRO);

					//cb_backgd.SetWindowText(_T("关闭"));
					SetTimer(IDT_CLOSE_DLG, 15000, NULL);
				}
			}
			break;
		case IDT_CLOSE_DLG:
			KillTimer(IDT_CLOSE_DLG);
			Shell_NotifyIcon(NIM_DELETE, &tnid); 
			OnOK();
			break;
	}
	CDialog::OnTimer(nIDEvent);
}

void CUpdaterDlg::OnBnClickedOk()
{
	
	//OnOK();
	if(m_bGoodToGo){
		KillTimer(IDT_REAL_START_CHECK);
		SetTimer(IDT_REAL_START_CHECK, 1700, NULL);
	}else{
		ShowWindow(SW_MINIMIZE);
	}
}

void CUpdaterDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	Shell_NotifyIcon(NIM_DELETE, &tnid); 
	OnOK();
	//CDialog::OnClose();
}

void CUpdaterDlg::OnBnClickedButton1()
{
	Shell_NotifyIcon(NIM_DELETE, &tnid); 
	OnOK();
	/*
		if(!cup.bWaiting && cup.bSVPCU_DONE){
				Shell_NotifyIcon(NIM_DELETE, &tnid); 
				OnOK();
			}else
				ShowWindow(SW_MINIMIZE);*/
		
	
}

void CUpdaterDlg::OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here

	ShellExecute( NULL, _T("open"), _T("http://splayer.org/history.html#introcontainer"), _T("") , NULL , SW_SHOW);

	*pResult = 0;
}

int CUpdaterDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//lpCreateStruct->style &= ~WS_VISIBLE;;
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

void CUpdaterDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	//if(notYetShow)
		CDialog::OnShowWindow(bShow, nStatus);
	//else	
	//	CDialog::OnShowWindow(bShow, nStatus);

	
	// TODO: Add your message handler code here
}

INT_PTR CUpdaterDlg::DoModal()
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::DoModal();
}

void CUpdaterDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if ( notYetShow )
		lpwndpos->flags &= ~SWP_SHOWWINDOW ;

	CDialog::OnWindowPosChanging(lpwndpos);

	// TODO: Add your message handler code here
}
