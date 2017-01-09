#include "StdAfx.h"
#include "DuiHanderShow.h"
#include <Tlhelp32.h>  
//////////////////////////////////////////////////////////////
// CDuiHandlerHome

CDuiHandlerShow::CDuiHandlerShow(void) : CDuiHandler()
{
	m_uTimerAni = 0;
	m_nAniIndex = 0;
}

CDuiHandlerShow::~CDuiHandlerShow(void)
{
}
 
// 初始化
void CDuiHandlerShow::OnInit()
{
	
}

// DUI定时器事件处理
void CDuiHandlerShow::OnTimer(UINT uTimerID, CString strTimerName)
{
    if(uTimerID == m_uTimerAni)
	{
	}
}
LRESULT CDuiHandlerShow::OnDuiBtnPSEnd(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CDuiGridCtrl* pGridCtrl = (CDuiGridCtrl*)GetControl(_T("gridctrl_1"));
	// 演示通过API添加gridctrl行
	//pGridCtrl->GetRowCount;
	//pGridCtrl->GetRowCheck;
	//pGridCtrl->GetRowInfo;
	if(pGridCtrl ){
		int num = 0;
		for(int i=0;i<pGridCtrl->GetRowCount();i++){
			if(pGridCtrl->GetRowCheck(i) == 1)
				num++;
		}
		CString strId;
		strId.Format(_T("%d"), num);
		DuiSystem::DuiMessageBox(NULL, strId); 
	}
	
    return TRUE;
}


LRESULT CDuiHandlerShow::OnDuiBtnPSStart(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CDuiGridCtrl* pGridCtrl = (CDuiGridCtrl*)GetControl(_T("gridctrl_1"));
	// 演示通过API添加gridctrl行
	//pGridCtrl->GetRowCount;
	//pGridCtrl->GetRowCheck;
	//pGridCtrl->GetRowInfo;
	if(pGridCtrl ){
		
		DuiSystem::DuiMessageBox(NULL, _T("进程保护成功！")); 
	}
    return TRUE;
}

LRESULT CDuiHandlerShow::OnDuiBtnPSSerch(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CDuiGridCtrl* pButtons = (CDuiGridCtrl*)GetControl(_T("button.startprotect"));
	CDuiGridCtrl* pButtone = (CDuiGridCtrl*)GetControl(_T("button.endprotect"));
	pButtons->SetDisable(FALSE);
	pButtone->SetDisable(FALSE);
	CDuiGridCtrl* pGridCtrl = (CDuiGridCtrl*)GetControl(_T("gridctrl_1"));
	// 演示通过API添加gridctrl行
	//pGridCtrl->GetRowCount;
	//pGridCtrl->GetRowCheck;
	//pGridCtrl->GetRowInfo;
	if(pGridCtrl ){
		
		if(pGridCtrl->GetRowCount()!=0)
		{
			pGridCtrl->ClearItems();
		}
		if(pGridCtrl->GetRowCount()!=0){
			return FALSE;
		}
			HANDLE hProcessSnp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  
			if(NULL == hProcessSnp)  
			{  
				return TRUE;
			}  
			  
			PROCESSENTRY32 pe32 = {0};  
			pe32.dwSize = sizeof(pe32);  
			BOOL bResult = Process32First(hProcessSnp, &pe32);  
			int i=0;
			while(bResult)  
			{  
				CString strId;
				
				strId.Format(_T("id_%d"), i);
				int nRow = pGridCtrl->InsertRow(-1,	// 插入的行序号,-1表示添加到最后
					strId,							// 行id字符串
					-1,								// 行左侧图片(索引图片方式,无索引图片填-1)
					Color(0, 0, 0, 0),				// 行文字颜色,全0表示默认(不使用行文字颜色,使用表格全局颜色)
					_T(" "),	// 行左侧的图片文件
					-1,								// 行右侧图片(索引图片方式,无索引图片填-1)
					_T(""),							// 行右侧的图片文件
					0);						// 行左侧的检查框状态(-1表示不显示检查框)
				CString strText;
				strText.Format(_T("%s"), pe32.szExeFile);
				CString strContent;
				strContent.Format(_T(""));
				pGridCtrl->SetSubItem(nRow, 0, strText ,strContent,FALSE);
				bResult = Process32Next(hProcessSnp, &pe32);  
				i++;
			}  
			CloseHandle(hProcessSnp); 
			
	}
	 return TRUE;
}