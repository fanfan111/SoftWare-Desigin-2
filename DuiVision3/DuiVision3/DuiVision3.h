// DuiVisionDemo.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CDuiVision3App:
// 有关此类的实现，请参阅 DuiVision3.cpp
//

class CDuiVision3App : public CWinApp
{
public:
	CDuiVision3App();

// 重写
	public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

// SHA定制：CDlgBase的OnBaseMessage重写类
class CDlgBaseSHA : public CDlgBase
{
public:
	CDlgBaseSHA(UINT nIDTemplate, CWnd* pParent = NULL):CDlgBase(nIDTemplate, pParent){}
	virtual LRESULT OnBaseMessage(UINT uID, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

extern CDuiVision3App theApp;