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

// ��ʼ��
void CDuiHandlerHome::OnInit()
{
}

// DUI��ʱ���¼�����
void CDuiHandlerHome::OnTimer(UINT uTimerID, CString strTimerName)
{
    if(uTimerID == m_uTimerAni)
	{
	}
}
