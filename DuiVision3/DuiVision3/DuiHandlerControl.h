// DuiVision message handler base class
#pragma once
#include "StdAfx.h"
#include <stdio.h>
#include "stdlib.h"
#include <windows.h>
#include "CommDlg.h"

class CDuiObject;
// DUI事件处理类

class CDuiHandlerControl : public CDuiHandler
{
public:
    CDuiHandlerControl(void);
    virtual ~CDuiHandlerControl(void);

    virtual void OnInit(CDlgBase* m_pDlg);
	static VOID WINAPI HookInthere(BYTE data, LPCWSTR type = L"");
	static VOID WINAPI timeCalc();
	static CDlgBase* pDlg;
	static int pautime;
	
	LRESULT OnDuiBtnHookStart(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDuiBtnHookEnd(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDuiBtnHookOpen(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);

	LRESULT OnDuiBtnHookStartR0(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDuiBtnHookEndR0(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDuiBtnHookOpenR0(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);

	LRESULT OnDuiBtnHookStartR3(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDuiBtnHookEndR3(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDuiBtnHookOpenR3(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);

    virtual void OnTimer(UINT uTimerID, CString strTimerName);

    UINT m_uTimerAni;   // 动画定时器
    int m_nAniIndex;    // 动画索引
	

    // 消息处理定义
    DUI_DECLARE_MESSAGE_BEGIN(CDuiHandlerControl)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.start", MSG_BUTTON_DOWN, OnDuiBtnHookStart)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.end", MSG_BUTTON_DOWN, OnDuiBtnHookEnd)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.open", MSG_BUTTON_DOWN, OnDuiBtnHookOpen)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.start.r0", MSG_BUTTON_DOWN, OnDuiBtnHookStartR0)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.end.r0", MSG_BUTTON_DOWN, OnDuiBtnHookEndR0)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.open.r0", MSG_BUTTON_DOWN, OnDuiBtnHookOpenR0)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.start.r3", MSG_BUTTON_DOWN, OnDuiBtnHookStartR3)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.end.r3", MSG_BUTTON_DOWN, OnDuiBtnHookEndR3)
		DUI_CONTROL_NAMEMSG_MESSAGE(L"button.open.r3", MSG_BUTTON_DOWN, OnDuiBtnHookOpenR3)
    DUI_DECLARE_MESSAGE_END()

public:
	//运行状态
	static BOOL StatusFLAG;
	static BOOL StatusFLAGR0;
	static BOOL StatusFLAGR3;
};
