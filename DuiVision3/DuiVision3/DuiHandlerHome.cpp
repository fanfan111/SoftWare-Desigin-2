#include "StdAfx.h"
#include "DuiHandlerHome.h"

//////////////////////////////////////////////////////////////
// CDuiHandlerHome

CDuiHandlerHome::CDuiHandlerHome(void) : CDuiHandler()
{
	m_uTimerAni = 0;
	m_nAniIndex = 0;
}

CDuiHandlerHome::~CDuiHandlerHome(void)
{
}

// 初始化
void CDuiHandlerHome::OnInit()
{
}

// DUI定时器事件处理
void CDuiHandlerHome::OnTimer(UINT uTimerID, CString strTimerName)
{
    if(uTimerID == m_uTimerAni)
	{
	}
}
