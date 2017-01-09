// DuiVision message handler base class
#pragma once
#include "StdAfx.h"
#include <stdio.h>
#include "stdlib.h"
#include <windows.h>

class CDuiObject;
// DUI事件处理类



class CDuiHandlerShow : public CDuiHandler
{
public:
	CDuiHandlerShow(void);
	
	/*struct list {
		char name[50];
		struct list *next;
	}List;*/
	
	static void scanPs();
	virtual ~CDuiHandlerShow(void);

	//struct list *head;

	virtual void OnInit();
	
	virtual void OnTimer(UINT uTimerID, CString strTimerName);

	LRESULT OnDuiBtnPSStart(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDuiBtnPSEnd(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDuiBtnPSSerch(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);

    UINT m_uTimerAni;   // 动画定时器
    int m_nAniIndex;    // 动画索引
	

    // 消息处理定义
    DUI_DECLARE_MESSAGE_BEGIN(CDuiHandlerControl)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.startprotect", MSG_BUTTON_DOWN, OnDuiBtnPSStart)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.endprotect", MSG_BUTTON_DOWN, OnDuiBtnPSEnd)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.cksyjc", MSG_BUTTON_DOWN, OnDuiBtnPSSerch)
    DUI_DECLARE_MESSAGE_END()
};

