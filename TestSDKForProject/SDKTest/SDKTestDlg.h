
// SDKTestDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "CEnhancedEdit.h"

// CSDKTestDlg �Ի���
class CSDKTestDlg : public CDialogEx
{
// ����
public:
	CSDKTestDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SDKTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEnhancedEdit m_input;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
