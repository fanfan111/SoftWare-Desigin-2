#include "StdAfx.h"
#include "DuiHandlerMain.h"
#include "registry.h"
#include "DuiHandlerHome.h"
#include "DuiHandlerControl.h"
#include "DuiHandlerSelect.h"
#include "DuiHanderShow.h"
#include <time.h>
//////////////////////////////////////////////////////////////
// CDuiHandlerMain

CDuiHandlerMain::CDuiHandlerMain(void) : CDuiHandler()
{
    m_pDlg = NULL;
    m_uTimerAni = 0;
    m_nAniIndex = 0;
}

CDuiHandlerMain::~CDuiHandlerMain(void)
{
}

// 初始化
void CDuiHandlerMain::OnInit()
{
    // 初始化托盘图标
    DuiSystem::Instance()->InitTray();

    // 将tab页'首页'注册事件处理对象
    CDuiHandlerHome* pDuiHandlerHome = new CDuiHandlerHome();
    DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerHome, _T("tab.Home"));
    pDuiHandlerHome->OnInit();
    // 将tab页'控件'注册事件处理对象

	CDuiHandlerControl* pDuiHandlerControl = new CDuiHandlerControl();
	//DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerControl, _T("tab.Controlr0"));
	//DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerControl, _T("tab.Controlr3"));
   
    DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerControl, _T("tab.Control"));
    pDuiHandlerControl->OnInit(m_pDlg);



	 // 将tab页'控件'注册事件处理对象
   // CDuiHandlerShow* pDuiHandlerShow = new CDuiHandlerShow();
    //DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerShow, _T("tab.Show"));
   // pDuiHandlerShow->OnInit();
	
	// 将tab页'控件'注册事件处理对象
    CDuiHandlerSelect* pDuiHandlerSelect = new CDuiHandlerSelect();
    DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerSelect, _T("tab.Select"));
    pDuiHandlerSelect->OnInit();


    // 启动动画定时器
    m_uTimerAni = DuiSystem::AddDuiTimer(500);
}

// 皮肤消息处理(实现皮肤的保存和获取)
LRESULT CDuiHandlerMain::OnDuiMsgSkin(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if(Msg == MSG_GET_SKIN_TYPE)    // 获取Skin类型
    {
        CRegistryUtil reg(HKEY_CURRENT_USER);
        int nBkType = reg.GetDWordValue(NULL, REG_CONFIG_SUBKEY, REG_CONFIG_BKTYPE);
        *(int*)wParam = nBkType;
        return TRUE;
    }else
    if(Msg == MSG_GET_SKIN_VALUE)   // 获取Skin值
    {
        CRegistryUtil reg(HKEY_CURRENT_USER);
        if(wParam == BKTYPE_IMAGE_RESOURCE)
        {
            *(int*)lParam = reg.GetDWordValue(NULL, REG_CONFIG_SUBKEY, REG_CONFIG_BKPIC_RES);
            return TRUE;
        }else
        if(wParam == BKTYPE_COLOR)
        {
            *(COLORREF*)lParam = reg.GetDWordValue(NULL, REG_CONFIG_SUBKEY, REG_CONFIG_BKCOLOR);
            return TRUE;
        }else
        if(wParam == BKTYPE_IMAGE_FILE)
        {
            *(CString*)lParam = reg.GetStringValue(NULL, REG_CONFIG_SUBKEY, REG_CONFIG_BKPIC_FILE);
            return TRUE;
        }
    }else
    if(Msg == MSG_SET_SKIN_VALUE)   // 设置Skin值
    {
        CRegistryUtil reg(HKEY_CURRENT_USER);
        reg.SetDWordValue(HKEY_CURRENT_USER, REG_CONFIG_SUBKEY, REG_CONFIG_BKTYPE, wParam);
        if(wParam == BKTYPE_IMAGE_RESOURCE)
        {
            reg.SetDWordValue(HKEY_CURRENT_USER, REG_CONFIG_SUBKEY, REG_CONFIG_BKPIC_RES, lParam);
        }else
        if(wParam == BKTYPE_COLOR)
        {
            reg.SetDWordValue(HKEY_CURRENT_USER, REG_CONFIG_SUBKEY, REG_CONFIG_BKCOLOR, lParam);
        }else
        if(wParam == BKTYPE_IMAGE_FILE)
        {
            CString* pstrImgFile = (CString*)lParam;
            reg.SetStringValue(HKEY_CURRENT_USER, REG_CONFIG_SUBKEY, REG_CONFIG_BKPIC_FILE, *pstrImgFile);
        }
        return TRUE;
    }
    return FALSE;
}

// 托盘双击消息处理
LRESULT CDuiHandlerMain::OnDuiMsgTrayIconDClick(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    DuiSystem::ShowDuiDialog(_T("dlg_login"), NULL);
    return TRUE;
}

// DUI定时器事件处理
void CDuiHandlerMain::OnTimer(UINT uTimerID, CString strTimerName)
{
    if(uTimerID == m_uTimerAni)
    {
    }
}

// 进程间消息处理
LRESULT CDuiHandlerMain::OnDuiMsgInterprocess(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    // 命令行参数,可以对命令行参数进行处理,也可以直接显示主窗口
    DUI_INTERPROCESS_MSG* pInterMsg = (DUI_INTERPROCESS_MSG*)lParam;
    CString strCmd = pInterMsg->wInfo;
    if(!strCmd.IsEmpty())
    {
        DuiSystem::DuiMessageBox(NULL, L"执行了命令行参数:" + strCmd);
    }else
    {
        CDlgBase* pDlg = DuiSystem::Instance()->GetDuiDialog(L"dlg_main");
        if(pDlg)
        {
            pDlg->SetForegroundWindow();
            pDlg->ShowWindow(SW_NORMAL);
            pDlg->ShowWindow(SW_SHOW);
            pDlg->BringWindowToTop();
        }
    }
    return TRUE;
}

LRESULT CDuiHandlerMain::OnDuiBtnSystemStart(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam){
	CDlgBase* pDlg_m =CDuiHandlerMain::m_pDlg;
	CDuiTabCtrl* pTabCtrl_F = (CDuiTabCtrl*)(pDlg_m->GetControl(_T("button.systemStart")));
	CString m=pTabCtrl_F->GetTitle();
	if(m.Compare(L"未开启VT防护")==0){
		pTabCtrl_F->SetControlTitle(L"已开启VT防护");
		//pTabCtrl_F->SetImage(L"IDB_BT_ICON");
		((CDuiTabCtrl*)(pDlg_m->GetControl(_T("button.newInformation"))))->SetControlDisable(FALSE);
		
		//开启VT保护
		startProtect();
		DuiSystem::DuiMessageBox(NULL, _T("VT保护已开启"), _T("VT键盘保护"));

	}else{
		pTabCtrl_F->SetControlTitle(L"未开启VT防护");
		//pTabCtrl_F->RemoveControl(pTabCtrl_F);
		//关闭逻辑
		//取消所有信息显示
		((CDuiGridCtrl*)GetControl(_T("gridctrl_2")))->ClearItems();
		((CDuiTabCtrl*)(pDlg_m->GetControl(_T("button.newInformation"))))->SetControlDisable(TRUE);

		//关闭VT保护
		stopProtect();
		DuiSystem::DuiMessageBox(NULL, _T("VT保护已关闭"), _T("VT键盘保护"));
	}
	
	return TRUE;
}