// DuiVisionDemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CDuiVision3App:
// �йش����ʵ�֣������ DuiVision3.cpp
//

class CDuiVision3App : public CWinApp
{
public:
	CDuiVision3App();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

// SHA���ƣ�CDlgBase��OnBaseMessage��д��
class CDlgBaseSHA : public CDlgBase
{
public:
	CDlgBaseSHA(UINT nIDTemplate, CWnd* pParent = NULL):CDlgBase(nIDTemplate, pParent){}
	virtual LRESULT OnBaseMessage(UINT uID, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

extern CDuiVision3App theApp;