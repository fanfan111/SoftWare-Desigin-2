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

// ��ʼ��
void CDuiHandlerMain::OnInit()
{
    // ��ʼ������ͼ��
    DuiSystem::Instance()->InitTray();

    // ��tabҳ'��ҳ'ע���¼��������
    CDuiHandlerHome* pDuiHandlerHome = new CDuiHandlerHome();
    DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerHome, _T("tab.Home"));
    pDuiHandlerHome->OnInit();
    // ��tabҳ'�ؼ�'ע���¼��������

	CDuiHandlerControl* pDuiHandlerControl = new CDuiHandlerControl();
	//DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerControl, _T("tab.Controlr0"));
	//DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerControl, _T("tab.Controlr3"));
   
    DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerControl, _T("tab.Control"));
    pDuiHandlerControl->OnInit(m_pDlg);



	 // ��tabҳ'�ؼ�'ע���¼��������
   // CDuiHandlerShow* pDuiHandlerShow = new CDuiHandlerShow();
    //DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerShow, _T("tab.Show"));
   // pDuiHandlerShow->OnInit();
	
	// ��tabҳ'�ؼ�'ע���¼��������
    CDuiHandlerSelect* pDuiHandlerSelect = new CDuiHandlerSelect();
    DuiSystem::RegisterHandler(m_pDlg, pDuiHandlerSelect, _T("tab.Select"));
    pDuiHandlerSelect->OnInit();


    // ����������ʱ��
    m_uTimerAni = DuiSystem::AddDuiTimer(500);
}

// Ƥ����Ϣ����(ʵ��Ƥ���ı���ͻ�ȡ)
LRESULT CDuiHandlerMain::OnDuiMsgSkin(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if(Msg == MSG_GET_SKIN_TYPE)    // ��ȡSkin����
    {
        CRegistryUtil reg(HKEY_CURRENT_USER);
        int nBkType = reg.GetDWordValue(NULL, REG_CONFIG_SUBKEY, REG_CONFIG_BKTYPE);
        *(int*)wParam = nBkType;
        return TRUE;
    }else
    if(Msg == MSG_GET_SKIN_VALUE)   // ��ȡSkinֵ
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
    if(Msg == MSG_SET_SKIN_VALUE)   // ����Skinֵ
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

// ����˫����Ϣ����
LRESULT CDuiHandlerMain::OnDuiMsgTrayIconDClick(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    DuiSystem::ShowDuiDialog(_T("dlg_login"), NULL);
    return TRUE;
}

// DUI��ʱ���¼�����
void CDuiHandlerMain::OnTimer(UINT uTimerID, CString strTimerName)
{
    if(uTimerID == m_uTimerAni)
    {
    }
}

// ���̼���Ϣ����
LRESULT CDuiHandlerMain::OnDuiMsgInterprocess(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    // �����в���,���Զ������в������д���,Ҳ����ֱ����ʾ������
    DUI_INTERPROCESS_MSG* pInterMsg = (DUI_INTERPROCESS_MSG*)lParam;
    CString strCmd = pInterMsg->wInfo;
    if(!strCmd.IsEmpty())
    {
        DuiSystem::DuiMessageBox(NULL, L"ִ���������в���:" + strCmd);
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
	if(m.Compare(L"δ����VT����")==0){
		pTabCtrl_F->SetControlTitle(L"�ѿ���VT����");
		//pTabCtrl_F->SetImage(L"IDB_BT_ICON");
		((CDuiTabCtrl*)(pDlg_m->GetControl(_T("button.newInformation"))))->SetControlDisable(FALSE);
		
		//����VT����
		startProtect();
		DuiSystem::DuiMessageBox(NULL, _T("VT�����ѿ���"), _T("VT���̱���"));

	}else{
		pTabCtrl_F->SetControlTitle(L"δ����VT����");
		//pTabCtrl_F->RemoveControl(pTabCtrl_F);
		//�ر��߼�
		//ȡ��������Ϣ��ʾ
		((CDuiGridCtrl*)GetControl(_T("gridctrl_2")))->ClearItems();
		((CDuiTabCtrl*)(pDlg_m->GetControl(_T("button.newInformation"))))->SetControlDisable(TRUE);

		//�ر�VT����
		stopProtect();
		DuiSystem::DuiMessageBox(NULL, _T("VT�����ѹر�"), _T("VT���̱���"));
	}
	
	return TRUE;
}