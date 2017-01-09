
// SDKTestDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SDKTest.h"
#include "SDKTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSDKTestDlg �Ի���


void startCallback(CWnd* pWnd)
{
	CRect prect;
	pWnd->GetDlgItem(IDC_PIC_STATE)->GetClientRect(&prect);
	FillRect(pWnd->GetDlgItem(IDC_PIC_STATE)->GetDC()->GetSafeHdc(),&prect,CBrush(RGB(0,255,0)));

	pWnd->GetDlgItem(IDC_STATIC_STATE)->SetWindowText(L"������");
}

void stopCallback(CWnd* pWnd)
{
	CRect prect;
	pWnd->GetDlgItem(IDC_PIC_STATE)->GetClientRect(&prect);
	FillRect(pWnd->GetDlgItem(IDC_PIC_STATE)->GetDC()->GetSafeHdc(),&prect,CBrush(RGB(255,255,255)));

	pWnd->GetDlgItem(IDC_STATIC_STATE)->SetWindowText(L"������");
}

void errorCallback(CWnd* pWnd)
{
	CRect prect;
	pWnd->GetDlgItem(IDC_PIC_STATE)->GetClientRect(&prect);
	FillRect(pWnd->GetDlgItem(IDC_PIC_STATE)->GetDC()->GetSafeHdc(),&prect,CBrush(RGB(255,0,0)));

	pWnd->GetDlgItem(IDC_STATIC_STATE)->SetWindowText(L"δ����");
}

void normalCallback(CWnd* pWnd)
{
	CRect prect;
	pWnd->GetDlgItem(IDC_PIC_STATE)->GetClientRect(&prect);
	FillRect(pWnd->GetDlgItem(IDC_PIC_STATE)->GetDC()->GetSafeHdc(),&prect,CBrush(RGB(255,255,255)));

	pWnd->GetDlgItem(IDC_STATIC_STATE)->SetWindowText(L"��������");
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


// CSDKTestDlg ��Ϣ�������

BOOL CSDKTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSDKTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSDKTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSDKTestDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	//::ShellExecuteW(GetSafeHwnd(), NULL, _T("explorer.exe"), _T("C:\\Windows"), NULL, SW_NORMAL);

	m_input.SetWindowText(L"");
}


void CSDKTestDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CString text;
	GetDlgItem(IDC_BUTTON2)->GetWindowText(text);

	if (text=="��������")
	{
		GetDlgItem(IDC_BUTTON2)->SetWindowText(L"�رձ���");
		m_input.m_isProtect = TRUE;
	}
	else if (text=="�رձ���")
	{
		GetDlgItem(IDC_BUTTON2)->SetWindowText(L"��������");
		m_input.m_isProtect = FALSE;
		normalCallback(this);
	}
}
