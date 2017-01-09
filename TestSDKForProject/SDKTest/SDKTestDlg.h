
// SDKTestDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "CEnhancedEdit.h"

// CSDKTestDlg 对话框
class CSDKTestDlg : public CDialogEx
{
// 构造
public:
	CSDKTestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SDKTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEnhancedEdit m_input;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
