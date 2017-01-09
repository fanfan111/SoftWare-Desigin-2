#include "StdAfx.h"
#include "DuiHandlerControl.h"
#include "DuiHandlerMain.h"
#include "ceshi.h"
#include "sort1.h"
#define SLEEP_TIME 2000
//////////////////////////////////////////////////////////////
// CDuiHandlerControl
BOOL CDuiHandlerControl::StatusFLAG=FALSE;
BOOL CDuiHandlerControl::StatusFLAGR0=FALSE;
BOOL CDuiHandlerControl::StatusFLAGR3=FALSE;

int CDuiHandlerControl::pautime=50;
CDuiHandlerControl::CDuiHandlerControl(void) : CDuiHandler()
{
    m_uTimerAni = 0;
    m_nAniIndex = 0;
}

CDuiHandlerControl::~CDuiHandlerControl(void)
{
}
CDlgBase*  CDuiHandlerControl::pDlg;
// 初始化
void CDuiHandlerControl::OnInit(CDlgBase* m_pDlg)
{
	CDuiHandlerControl::pDlg=m_pDlg;
	CDuiHandlerControl::StatusFLAG=FALSE;
	CDuiHandlerControl::StatusFLAGR0=FALSE;
	CDuiHandlerControl::StatusFLAGR3=FALSE;

	/*将所有高亮按钮取消展示，因为点击进入页面所以不用刷新控件*/
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.esc_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f1_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f2_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f3_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f4_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f5_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f6_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f7_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f8_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f9_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f10_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f11_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f12_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.`_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.1_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.2_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.3_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.4_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.5_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.6_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.7_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.8_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.9_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.0_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.-_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.=_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.backspace_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.tab_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.q_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.w_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.e_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.r_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.t_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.y_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.u_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.i_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.o_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.p_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.[_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.]_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.fxg_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lock_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.a_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.s_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.d_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.g_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.h_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.j_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.k_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.l_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.;_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.'_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.enter_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lshift_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.z_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.x_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.c_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.v_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.b_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.n_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.m_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.,_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.._T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.xg_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.rshift_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lctrl_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lwin_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lalt_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.space_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.ralt_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.rwin_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.app_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.rctrl_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.prtsc_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.scrlock_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.pause_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.insert_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.home_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.pgup_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.delete_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.end_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.pgdn_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.shang_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.zuo_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.xia_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.you_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.numberlock_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp +_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp -_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp cheng_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp xg_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp ._T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp enter_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 0_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 1_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 2_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 3_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 4_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 5_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 6_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 7_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 8_T"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 9_T"))))->SetControlVisible(FALSE);
	
	//r3
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.esc_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f1_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f2_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f3_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f4_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f5_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f6_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f7_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f8_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f9_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f10_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f11_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f12_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.`_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.1_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.2_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.3_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.4_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.5_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.6_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.7_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.8_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.9_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.0_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.-_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.=_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.backspace_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.tab_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.q_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.w_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.e_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.r_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.t_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.y_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.u_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.i_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.o_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.p_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.[_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.]_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.fxg_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lock_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.a_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.s_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.d_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.g_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.h_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.j_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.k_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.l_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.;_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.'_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.enter_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lshift_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.z_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.x_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.c_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.v_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.b_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.n_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.m_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.,_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.._T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.xg_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.rshift_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lctrl_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lwin_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lalt_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.space_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.ralt_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.rwin_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.app_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.rctrl_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.prtsc_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.scrlock_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.pause_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.insert_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.home_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.pgup_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.delete_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.end_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.pgdn_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.shang_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.zuo_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.xia_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.you_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.numberlock_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp +_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp -_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp cheng_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp xg_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp ._T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp enter_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 0_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 1_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 2_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 3_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 4_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 5_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 6_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 7_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 8_T.r3"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 9_T.r3"))))->SetControlVisible(FALSE);
	//r0
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.esc_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f1_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f2_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f3_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f4_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f5_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f6_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f7_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f8_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f9_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f10_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f11_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f12_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.`_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.1_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.2_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.3_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.4_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.5_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.6_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.7_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.8_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.9_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.0_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.-_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.=_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.backspace_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.tab_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.q_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.w_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.e_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.r_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.t_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.y_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.u_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.i_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.o_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.p_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.[_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.]_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.fxg_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lock_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.a_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.s_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.d_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.f_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.g_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.h_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.j_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.k_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.l_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.;_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.'_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.enter_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lshift_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.z_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.x_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.c_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.v_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.b_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.n_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.m_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.,_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.._T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.xg_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.rshift_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lctrl_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lwin_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.lalt_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.space_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.ralt_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.rwin_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.app_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.rctrl_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.prtsc_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.scrlock_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.pause_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.insert_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.home_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.pgup_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.delete_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.end_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.pgdn_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.shang_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.zuo_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.xia_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.you_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.numberlock_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp +_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp -_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp cheng_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp xg_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp ._T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp enter_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 0_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 1_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 2_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 3_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 4_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 5_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 6_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 7_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 8_T.r0"))))->SetControlVisible(FALSE);
	((CDuiTabCtrl*)(m_pDlg->GetControl(_T("keyboard.kp 9_T.r0"))))->SetControlVisible(FALSE);
}


// DUI定时器事件处理
void CDuiHandlerControl::OnTimer(UINT uTimerID, CString strTimerName)
{
    if(uTimerID == m_uTimerAni)
    {
    }
}

typedef BOOL(*lpAdd)(bool falg); //宏定义函数指针类型
typedef LPWORD(*lpGet)(); //宏定义函数指针类型
typedef BOOL(*lpIsEmpty)(); //宏定义函数指针类型


VOID WINAPI CDuiHandlerControl::HookInthere(BYTE data, LPCWSTR type)
{
	struct returnchar result = Sort::find(data);
	char* p= result.name;
	if(strcmp(p,"wait")!=0 && strcmp(p,"error")!=0)
	{
		if(result.flag)
		{
			CString m;
			m.Format(_T("%s%s"), CStringW(p), CStringW(type));
			CDlgBase* pDlg_m =CDuiHandlerControl::pDlg;
			CDuiTabCtrl* pTabCtrl_F = (CDuiTabCtrl*)(pDlg_m->GetControl(m));
			pTabCtrl_F->SetControlVisible(TRUE);
			pTabCtrl_F->UpdateControl(TRUE);
		}
		else
		{
			CString m;
			m.Format(_T("%s%s"), CStringW(p), CStringW(type));
			CDlgBase* pDlg_m =CDuiHandlerControl::pDlg;
			CDuiTabCtrl* pTabCtrl_F = (CDuiTabCtrl*)(pDlg_m->GetControl(m));
			pTabCtrl_F->SetControlVisible(FALSE);
			pTabCtrl_F->UpdateControl(TRUE,TRUE);
		}
		
	}
}








LRESULT CDuiHandlerControl::OnDuiBtnHookStart(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(!CDuiHandlerControl::StatusFLAG)
	{
		CDuiHandlerControl::StatusFLAG = TRUE;

		//关闭其它
		stopR3Record();
		CDuiHandlerControl::StatusFLAGR3 = FALSE;
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start.r3"))))->SetControlDisable(FALSE);
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end.r3"))))->SetControlDisable(TRUE);
		stopR0Record();
		CDuiHandlerControl::StatusFLAGR0 = FALSE;
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start.r0"))))->SetControlDisable(FALSE);
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end.r0"))))->SetControlDisable(TRUE);


		startVTRecord();
		
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start"))))->SetControlDisable(TRUE);
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end"))))->SetControlDisable(FALSE);
	}
	
    return TRUE;
}

LRESULT CDuiHandlerControl::OnDuiBtnHookEnd(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CDuiHandlerControl::StatusFLAG = FALSE;	

	stopVTRecord();

	((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start"))))->SetControlDisable(FALSE);
	((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end"))))->SetControlDisable(TRUE);

    return TRUE;
}

LRESULT CDuiHandlerControl::OnDuiBtnHookOpen(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam){
	ShellExecute(NULL, NULL, _T("explorer.exe"), VT_RECORD_DIR, NULL, SW_NORMAL);
	return TRUE;
}






LRESULT CDuiHandlerControl::OnDuiBtnHookStartR0(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(!CDuiHandlerControl::StatusFLAGR0)
	{
		CDuiHandlerControl::StatusFLAGR0 = TRUE;

		//关闭其它
		stopR3Record();
		CDuiHandlerControl::StatusFLAGR3 = FALSE;
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start.r3"))))->SetControlDisable(FALSE);
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end.r3"))))->SetControlDisable(TRUE);
		stopVTRecord();
		CDuiHandlerControl::StatusFLAG = FALSE;
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start"))))->SetControlDisable(FALSE);
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end"))))->SetControlDisable(TRUE);


		startR0Record();

		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start.r0"))))->SetControlDisable(TRUE);
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end.r0"))))->SetControlDisable(FALSE);
	}
	
    return TRUE;
}

LRESULT CDuiHandlerControl::OnDuiBtnHookEndR0(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CDuiHandlerControl::StatusFLAGR0 = FALSE;

	stopR0Record();

	((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start.r0"))))->SetControlDisable(FALSE);
	((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end.r0"))))->SetControlDisable(TRUE);

	return TRUE;
}

LRESULT CDuiHandlerControl::OnDuiBtnHookOpenR0(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	
	ShellExecute(NULL, NULL, _T("explorer.exe"), R0_RECORD_DIR, NULL, SW_NORMAL);
	return TRUE;
}





























LRESULT CDuiHandlerControl::OnDuiBtnHookStartR3(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(!CDuiHandlerControl::StatusFLAGR3)
	{
		CDuiHandlerControl::StatusFLAGR3 = TRUE;
		
		//关闭其它
		stopR0Record();
		CDuiHandlerControl::StatusFLAGR0 = FALSE;
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start.r0"))))->SetControlDisable(FALSE);
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end.r0"))))->SetControlDisable(TRUE);
		stopVTRecord();
		CDuiHandlerControl::StatusFLAG = FALSE;
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start"))))->SetControlDisable(FALSE);
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end"))))->SetControlDisable(TRUE);


		startR3Record();

		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start.r3"))))->SetControlDisable(TRUE);
		((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end.r3"))))->SetControlDisable(FALSE);
	}
	
    return TRUE;
}

LRESULT CDuiHandlerControl::OnDuiBtnHookEndR3(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CDuiHandlerControl::StatusFLAGR3 = FALSE;

	stopR3Record();

	((CDuiTabCtrl*)(pDlg->GetControl(_T("button.start.r3"))))->SetControlDisable(FALSE);
	((CDuiTabCtrl*)(pDlg->GetControl(_T("button.end.r3"))))->SetControlDisable(TRUE);

	return TRUE;
}

LRESULT CDuiHandlerControl::OnDuiBtnHookOpenR3(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	
	ShellExecute(NULL, NULL, _T("explorer.exe"), R3_RECORD_DIR, NULL, SW_NORMAL);
	return TRUE;
}