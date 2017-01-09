#include "StdAfx.h"
#include "DuiHandlerSelect.h"
#include <Tlhelp32.h>  
//////////////////////////////////////////////////////////////
// CDuiHandlerHome

CDuiHandlerSelect::CDuiHandlerSelect(void) : CDuiHandler()
{
	m_uTimerAni = 0;
	m_nAniIndex = 0;
}

CDuiHandlerSelect::~CDuiHandlerSelect(void)
{
}
 
// 初始化
void CDuiHandlerSelect::OnInit()
{
	
}

// DUI定时器事件处理
void CDuiHandlerSelect::OnTimer(UINT uTimerID, CString strTimerName)
{
    if(uTimerID == m_uTimerAni)
	{
	}
}

LRESULT CDuiHandlerSelect::processScan(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam){
	
	CDuiGridCtrl* pGridCtrl = (CDuiGridCtrl*)GetControl(_T("gridctrl_2"));
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

			//获取保护客户端线程列表
			map<DWORD, DWORD> threadInfoMap = all_thread();

			set<DWORD> threadIds;
			map<DWORD, DWORD>::iterator it;
			for (it=threadInfoMap.begin(); it!=threadInfoMap.end(); ++it)
			{
				threadIds.insert(it->first);
			}

			//set<DWORD> threadIds = all_thread();

			//建立进程ID线程ID的Map
			map<DWORD, set<DWORD> > pidTotidsMap;

			HANDLE hThreads;
			THREADENTRY32 te32;
			hThreads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
			if( hThreads == INVALID_HANDLE_VALUE )
			{
				return TRUE;
			}

			//遍历系统线程(用于构造Map)
			te32.dwSize = sizeof(THREADENTRY32);
			BOOL anyMore = Thread32First(hThreads, &te32);
			while(anyMore)
			{
				if (threadIds.find(te32.th32ThreadID)!=threadIds.end())
				{
					DWORD nowPID = te32.th32OwnerProcessID;
					DWORD nowTID = te32.th32ThreadID;
					map<DWORD, set<DWORD> >::iterator it = pidTotidsMap.find(nowPID);
					if (it==pidTotidsMap.end())
					{
						pidTotidsMap.insert(pair<DWORD, set<DWORD> >(nowPID, set<DWORD>(&nowTID, &nowTID+1)));
					}
					else
					{
						(it->second).insert(nowTID);
					}
				}
				anyMore = Thread32Next(hThreads, &te32);
			}

			//遍历系统进程(用于更新UI)
			SHFILEINFO shSmall;
			PROCESSENTRY32 pe32 = {0};  
			pe32.dwSize = sizeof(pe32);  
			BOOL bResult = Process32First(hProcessSnp, &pe32);
			int i=0;
			while(bResult)  
			{  
				map<DWORD, set<DWORD> >::iterator it = pidTotidsMap.find(pe32.th32ProcessID);
				if (it!=pidTotidsMap.end())
				{
					set<DWORD> tids = it->second;
					if (tids.empty())
					{
						bResult = Process32Next(hProcessSnp, &pe32);
						continue;
					}

					DWORD tid = *tids.begin();
					tids.erase(tid);
					it->second = tids;

					//获取进程文件的信息  
					SHGetFileInfo(pe32.szExeFile,0,&shSmall,sizeof(shSmall),SHGFI_DISPLAYNAME); 
					CString strId;
					strId.Format(_T("id_%d"), i);

					int nRow = pGridCtrl->InsertRow(-1,	strId,0,Color(0, 0, 0, 0),L"skins\\simage\\update_suc.png",-1,	_T(""),-1);					
					CString strText;
					strText.Format(_T("%s"), pe32.szExeFile);
			

					CString strContent;
					strContent.Format(_T(""));
					pGridCtrl->SetSubItem(nRow, 0, strText ,strContent,FALSE);
					CString strText1;
					strText1.Format(_T("%d"), pe32.th32ProcessID);
					pGridCtrl->SetSubItem(nRow, 1, strText1 ,strContent,FALSE);
					
					CString strText2;
					strText2.Format(_T("%d"), tid);
					pGridCtrl->SetSubItem(nRow, 2, strText2 ,strContent,FALSE);
				
					if(CDuiHandlerSelect::Is64BitPorcess(pe32.th32ProcessID)){
					
						pGridCtrl->SetSubItem(nRow, 3, L"x64" ,strContent,FALSE);
					}else{
					
						pGridCtrl->SetSubItem(nRow, 3, L"x86" ,strContent,FALSE);
					}

					if (threadInfoMap.find(tid)==threadInfoMap.end())
					{
						pGridCtrl->SetSubItem(nRow, 4, L"未知" ,strContent,FALSE);
					}
					else
					{
						DWORD status = threadInfoMap.find(tid)->second;
						if (status==-1)
						{
							pGridCtrl->SetSubItem(nRow, 4, L"已暂停" ,strContent,FALSE);
						}
						else
						{
							pGridCtrl->SetSubItem(nRow, 4, L"保护中" ,strContent,FALSE);
						}
					}

					i++;
				}
				else
				{
					bResult = Process32Next(hProcessSnp, &pe32);
				}
			}  
			CloseHandle(hProcessSnp); 
			
	}
	return TRUE;
}

BOOL CDuiHandlerSelect::Is64BitOS()
{
    typedef VOID (WINAPI *LPFN_GetNativeSystemInfo)( __out LPSYSTEM_INFO lpSystemInfo );
    LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress( GetModuleHandleW(L"kernel32"),"GetNativeSystemInfo");
    if(fnGetNativeSystemInfo)
    {
        SYSTEM_INFO stInfo = {0};
        fnGetNativeSystemInfo( &stInfo);
        if( stInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64
            || stInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CDuiHandlerSelect::Is64BitPorcess(DWORD dwProcessID)
{
    if (! CDuiHandlerSelect::Is64BitOS())
    {
        return FALSE;
    }else
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,dwProcessID);
        if(hProcess)
        {
            typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
            LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandleW(L"kernel32"),"IsWow64Process");
            if (NULL != fnIsWow64Process)
            {
                BOOL bIsWow64 = FALSE;
                fnIsWow64Process(hProcess,&bIsWow64);
                CloseHandle(hProcess);
                if (bIsWow64)
                {
                    return FALSE;
                }
                else
                {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}