// DuiVision message handler base class
#pragma once
#include "StdAfx.h"
#include <stdio.h>
#include "stdlib.h"
#include <windows.h>

class CDuiObject;
// DUI事件处理类



class CDuiHandlerSelect : public CDuiHandler
{
public:
	CDuiHandlerSelect(void);

	BOOL Is64BitOS();
	BOOL Is64BitPorcess(DWORD dwProcessID);
	LRESULT processScan(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);
	virtual ~CDuiHandlerSelect(void);

	virtual void OnInit();
	
	virtual void OnTimer(UINT uTimerID, CString strTimerName);

    UINT m_uTimerAni;   // 动画定时器
    int m_nAniIndex;    // 动画索引
	

    // 消息处理定义
    DUI_DECLARE_MESSAGE_BEGIN(CDuiHandlerSelect)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.newInformation", MSG_BUTTON_DOWN, processScan )
    DUI_DECLARE_MESSAGE_END()
};

