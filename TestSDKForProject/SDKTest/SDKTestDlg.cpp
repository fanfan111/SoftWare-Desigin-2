
// SDKTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SDKTest.h"
#include "SDKTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSDKTestDlg 对话框


void startCallback(CWnd* pWnd)
{
	CRect prect;
	pWnd->GetDlgItem(IDC_PIC_STATE)->GetClientRect(&prect);
	FillRect(pWnd->GetDlgItem(IDC_PIC_STATE)->GetDC()->GetSafeHdc(),&prect,CBrush(RGB(0,255,0)));

	pWnd->GetDlgItem(IDC_STATIC_STATE)->SetWindowText(L"保护中");
}

void stopCallback(CWnd* pWnd)
{
	CRect prect;
	pWnd->GetDlgItem(IDC_PIC_STATE)->GetClientRect(&prect);
	FillRect(pWnd->GetDlgItem(IDC_PIC_STATE)->GetDC()->GetSafeHdc(),&prect,CBrush(RGB(255,255,255)));

	pWnd->GetDlgItem(IDC_STATIC_STATE)->SetWindowText(L"焦点外");
}

void errorCallback(CWnd* pWnd)
{
	CRect prect;
	pWnd->GetDlgItem(IDC_PIC_STATE)->GetClientRect(&prect);
	FillRect(pWnd->GetDlgItem(IDC_PIC_STATE)->GetDC()->GetSafeHdc(),&prect,CBrush(RGB(255,0,0)));

	pWnd->GetDlgItem(IDC_STATIC_STATE)->SetWindowText(L"未保护");
}

void normalCallback(CWnd* pWnd)
{
	CRect prect;
	pWnd->GetDlgItem(IDC_PIC_STATE)->GetClientRect(&prect);
	FillRect(pWnd->GetDlgItem(IDC_PIC_STATE)->GetDC()->GetSafeHdc(),&prect,CBrush(RGB(255,255,255)));

	pWnd->GetDlgItem(IDC_STATIC_STATE)->SetWindowText(L"常规输入");
}

CSDKTestDlg::CSDKTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSDKTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_input.m_pWnd = this;
	m_input.m_startCallback = startCallback;
	m_input.m_stopCallback = stopCallback;
	m_input.m_errorCallback = errorCallback;
	m_input.m_normalCallback = normalCallback;
}

void CSDKTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_input);
}

BEGIN_MESSAGE_MAP(CSDKTestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CSDKTestDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSDKTestDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CSDKTestDlg 消息处理程序

BOOL CSDKTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSDKTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSDKTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSDKTestDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	//::ShellExecuteW(GetSafeHwnd(), NULL, _T("explorer.exe"), _T("C:\\Windows"), NULL, SW_NORMAL);

	m_input.SetWindowText(L"");
}


void CSDKTestDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码

	CString text;
	GetDlgItem(IDC_BUTTON2)->GetWindowText(text);

	if (text=="开启保护")
	{
		GetDlgItem(IDC_BUTTON2)->SetWindowText(L"关闭保护");
		m_input.m_isProtect = TRUE;
	}
	else if (text=="关闭保护")
	{
		GetDlgItem(IDC_BUTTON2)->SetWindowText(L"开启保护");
		m_input.m_isProtect = FALSE;
		normalCallback(this);
	}
}
