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
END_MESSAGE_MAP()


// CUpdaterDlg message handlers

BOOL CUpdaterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	prg_total.SetRange(0, 1000);
	
	HDC hdc = ::GetDC(NULL);
	double dDefaultSize = 22;
	double dIntroSize = 14;
	double scale = 1.0;//*GetDeviceCaps(hdc, LOGPIXELSY) / 96.0;
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


	cs_stat.SetWindowText(_T("本程序下载最新版的射手播放器，将在更新完成后5分钟内退出"));
	csCurTask.SetWindowText(_T("当前任务：正在计算下载量"));
	SetTimer(IDT_START_CHECK, 1000, NULL);

	
	tnid.cbSize = sizeof(NOTIFYICONDATA); 
	tnid.hWnd = this->m_hWnd; 
	tnid.uID = IDR_MAINFRAME; 
	tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
	tnid.uCallbackMessage = WM_NOTIFYICON; 
	tnid.hIcon = this->m_hIcon; 
	wcscpy_s(tnid.szTip, _T("射手影音播放器自动更新程序"));
	Shell_NotifyIcon(NIM_ADD, &tnid); 

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

	if(bHide){
		ShowWindow(SW_HIDE);
		ShowWindow(SW_MINIMIZE);
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
		dc.DrawIcon(34, 25, m_hIcon);
		dc.SetBkMode(TRANSPARENT);
		HFONT oldFont = (HFONT)dc.SelectObject((HFONT) m_hBigFont);
		dc.SetTextColor( 0x353535);
		dc.DrawText(_T("射手播放器\r\n自动升级程序"), CRect(80, 25, 230,85),DT_LEFT );
		dc.SelectObject((HFONT) m_hIntroFont);
		dc.DrawText(_T("软件介绍："), CRect(39, 110, 200,130),DT_LEFT );

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

				
				if(cup.downloadList()){
					
					cs_stat.SetWindowText(_T("正在更新到射手播放器的最新版本..."));

					csCurTask.SetWindowText(_T("当前任务： 正在更新..."));
					
					cszSizeTotal.SetWindowText(  CString(L"文件大小： ") + getShortSize(cup.iSVPCU_TOTAL_FILEBYTE) );

					csTotalProgress.SetWindowText( CString(L"已下载： ") + getShortSize(cup.iSVPCU_CURRENT_FILEBYTE_DONE) );

					SetTimer(IDT_REAL_START_CHECK, 800, NULL);


					SetTimer(IDT_SHOW_INTRO,3500, NULL);

					
				}else{
					cs_stat.SetWindowText(_T("您已经拥有最新版的射手播放器，目前无需升级。\r\n升级程序将在30秒后退出"));
					csCurTask.SetWindowText(_T("当前任务：正在准备关闭..."));
					SetTimer(IDT_CLOSE_DLG, 30000, NULL);
				}

			}
			break;
		case IDT_SHOW_INTRO:
			cs_stat.SetWindowText(szaIntro.GetAt( rand() % szaIntro.GetCount() ));
			break;
		case IDT_REAL_START_CHECK:
			{
				
				KillTimer(IDT_REAL_START_CHECK);
				cb_backgd.EnableWindow(true);
				AfxBeginThread(ThreadCheckUpdate , (LPVOID)&cup);
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
					szTmp = _T("关闭播放器或重新启动后将自动更新到最新版本");
					csCurTask.SetWindowText(_T("当前任务：正在覆盖..."));
					
				}
				wcscpy_s(tnid.szTip, szTmp);

//szTmp.Format( _T("正在下载文件： %s （%d / %d）") , cup.szCurFilePath , cup.iSVPCU_CURRETN_FILE , cup.iSVPCU_TOTAL_FILE);
				if(!cup.bWaiting){
					
					double downloadedTotal = cup.iSVPCU_TOTAL_FILEBYTE_DONE + cup.iSVPCU_CURRENT_FILEBYTE_DONE;
					UINT timeNowNow = time(NULL);
					if( timeNowNow  - iTimeStart){
						double speed = downloadedTotal / ( timeNowNow  - iTimeStart);
						CString szBufSpeed;
						szBufSpeed.Format( _T("速度： %s/秒") , getShortSize(speed) );
						szSpeed.SetWindowText(szBufSpeed);
					}


					csTotalProgress.SetWindowText( CString(L"已下载： ") + getShortSize(downloadedTotal) );

					prg_total.SetPos(int(progress * 10));
					
				}
				//SetWindowText(szTmp);

				//szTmp.Format( _T("总进度：%0.2f%%") , progress);
				

				
				//Shell_NotifyIcon(NIM_MODIFY,&tnid);

				if(cup.bSVPCU_DONE){
					KillTimer(IDT_REFRESH_STAT);
					szTmp = _T("射手影音播放器已经更新到最新版本！\r\n本程序将在2分钟内关闭");
					cs_stat.SetWindowText(szTmp);
					csCurTask.SetWindowText(_T("当前任务： 更新已结束"));
					KillTimer(IDT_SHOW_INTRO);
					//cb_backgd.SetWindowText(_T("关闭"));
					SetTimer(IDT_CLOSE_DLG, 120000, NULL);
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

	ShellExecute( NULL, _T("open"), _T("http://blog.splayer.org"), _T("") , NULL , SW_SHOW);

	*pResult = 0;
}

int CUpdaterDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	lpCreateStruct->style &= ~WS_VISIBLE;;
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}
