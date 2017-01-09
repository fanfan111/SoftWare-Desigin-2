
// CEnhancedEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "CEnhancedEdit.h"
#include "KeyMap.h"

#include <winsvc.h> 
#include <winioctl.h>


BEGIN_MESSAGE_MAP(CEnhancedEdit, CEdit)
    ON_WM_CHAR()  
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_MESSAGE(WM_EDIT_GET_KEY, OnEditGetKey) 
END_MESSAGE_MAP()  


CEnhancedEdit::CEnhancedEdit(CWnd* pWnd)
	:m_pWnd(pWnd), 
	 m_isFailedInit(FALSE),
	 m_isProtect(FALSE),
	 m_running(FALSE),
	 m_isGloalPause(FALSE),
	 m_hMutex(NULL),
	 m_hMap(NULL),
	 m_intercept(NULL),
	 m_startCallback(NULL), 
	 m_stopCallback(NULL), 
	 m_errorCallback(NULL),
	 m_normalCallback(NULL)
{
	//测试驱动加载
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		m_isFailedInit = TRUE;
		MessageBoxW(L"初始化保护失败！[0x0]", L"键盘安全SDK", MB_OK);
		return;
	}
	CloseHandle(hDevice);

	//打开互斥量
	m_hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
	if (m_hMutex==NULL) //打开失败
	{
		m_isFailedInit = TRUE;
		MessageBoxW(L"初始化保护失败！[0x1]", L"键盘安全SDK", MB_OK);
		return;
	}
	if (GetLastError()!=ERROR_ALREADY_EXISTS) //全新互斥量，说明是唯一拦截客户进程
	{
		//初始化共享内存
		m_hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_SIZE*sizeof(DWORD), SHM_NAME);
		if (m_hMap==NULL)
		{
			CloseHandle(m_hMutex);
			m_isFailedInit = TRUE;
			MessageBoxW(L"初始化保护失败！[0x2]", L"键盘安全SDK", MB_OK);
			return;
		}
		//映射共享内存
		m_intercept = (DWORD*)MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (m_intercept==NULL)
		{
			CloseHandle(m_hMutex);
			CloseHandle(m_hMap);
			m_isFailedInit = TRUE;
			MessageBoxW(L"初始化保护失败！[0x3]", L"键盘安全SDK", MB_OK);
			return;
		}
		memset(m_intercept, 0, MAX_SIZE*sizeof(DWORD));
	}
	else //已存在互斥量，简单初始化
	{
		//打开共享内存
		m_hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, SHM_NAME);
		if (m_hMap==NULL)
		{
			CloseHandle(m_hMutex);
			m_isFailedInit = TRUE;
			MessageBox(L"初始化保护失败！[0x4]", L"键盘安全SDK", MB_OK);
			return;
		}
		//映射共享内存
		m_intercept = (DWORD*)MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (m_intercept==NULL)
		{
			CloseHandle(m_hMutex);
			CloseHandle(m_hMap);
			m_isFailedInit = TRUE;
			MessageBox(L"初始化保护失败！[0x5]", L"键盘安全SDK", MB_OK);
			return;
		}
	}
}
  

CEnhancedEdit::~CEnhancedEdit()
{
	DWORD status = WaitForSingleObject(m_hMutex, INFINITE);
	if (status==WAIT_ABANDONED)
	{
		fix_data();
	}
	set_intercept(0);
	ReleaseMutex(m_hMutex);
	CloseHandle(m_hMutex);
	UnmapViewOfFile(m_intercept);
	CloseHandle(m_hMap);
}


void CEnhancedEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_isFailedInit || !m_isProtect || !m_running || m_isGloalPause)
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
	return;
}


void CEnhancedEdit::OnSetFocus(CWnd* pOldWnd)
{
	CEdit::OnSetFocus(pOldWnd);

	// TODO: 在此处添加消息处理程序代码

	//初始化失败，或，非保护，直接返回
	if (m_isFailedInit || !m_isProtect)
		return;

	//线程运行标志置真
	m_running = TRUE;

	//获取VT驱动句柄
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		//线程运行标志置假
		m_running = FALSE;

		MessageBox(L"驱动未启动！[0x5]", L"键盘安全SDK", MB_OK);

		//错误回调
		m_errorCallback(m_pWnd);

		return;
	}
	//关闭VT驱动句柄
	CloseHandle(hDevice);


	//清理翻译标志
	clearTransFlags();


	//开启处理线程
	DWORD dwReadThreadID;
	HANDLE hReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CEnhancedEdit::ThreadFunc, this, 0, &dwReadThreadID);
	CloseHandle(hReadThread);

}


void CEnhancedEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	// TODO: 在此处添加消息处理程序代码

	//线程运行标志置假
	m_running = FALSE;
}


LRESULT CEnhancedEdit::OnEditGetKey(WPARAM wParam, LPARAM lParam)
{
	BYTE keyData = (BYTE)wParam;
	char ascii = transSacnCode(keyData);

	if (ascii != 0)
	{
		DefWindowProc(WM_CHAR, ascii, 0x00000001|((keyData&0x7F)< 16));
	}

	return 0;
}


VOID CEnhancedEdit::is_global_pause()
{
	//打开驱动句柄
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return;
	}

	//读取全局状态
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_STATUS, NULL, 0, &keyData, size, &size, NULL);

	if (keyData==3 || keyData==4)
	{
		//全局暂停
		m_isGloalPause = TRUE;
		m_normalCallback(m_pWnd);
	}
	else
	{
		m_isGloalPause = FALSE;
		m_startCallback(m_pWnd);
	}

	//关闭驱动句柄
	CloseHandle(hDevice);

	return;
}


VOID CEnhancedEdit::fix_data()
{
	DWORD i;
	for (i=0; m_intercept[i]!=0; i+=2);
	m_intercept[MAX_SIZE] = i;
}


BOOL CEnhancedEdit::set_intercept(DWORD Flag)
{
	//打开驱动句柄
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	DWORD ThreadId = GetCurrentThreadId();
	if (Flag)
	{
		DWORD i;
		for (i=0; i<m_intercept[MAX_SIZE]; i+=2)
		{
			if (m_intercept[i]==ThreadId)
				break;
		}
		if (i==m_intercept[MAX_SIZE])
		{
			m_intercept[i] = ThreadId;
			m_intercept[i+1] = 1;
			m_intercept[MAX_SIZE]+=2;

			//开启拦截
			BYTE keyData = 0;
			DWORD size = sizeof(BYTE);
			DeviceIoControl(hDevice, IOCTL_CMD_START_INTERCEPT, NULL, 0, &keyData, size, &size, NULL);
		}
		else
		{
			BYTE keyData = 0;
			DWORD size = sizeof(BYTE);
			DeviceIoControl(hDevice, IOCTL_CMD_STATUS, NULL, 0, &keyData, size, &size, NULL);

			if (m_intercept[i+1]==0 || m_intercept[i+1]==-1)
			{
				//开启拦截
				keyData = 0;
				size = sizeof(BYTE);
				DeviceIoControl(hDevice, IOCTL_CMD_START_INTERCEPT, NULL, 0, &keyData, size, &size, NULL);
			}
			else if (keyData==1 || keyData==3)
			{
				//开启拦截
				keyData = 0;
				size = sizeof(BYTE);
				DeviceIoControl(hDevice, IOCTL_CMD_START_INTERCEPT, NULL, 0, &keyData, size, &size, NULL);
			}

			m_intercept[i+1] = 1;
		}
	}
	else
	{
		DWORD i;
		for (i=0; i<m_intercept[MAX_SIZE]; i+=2)
		{
			if (m_intercept[i]==ThreadId)
				break;
		}
		if (i!=m_intercept[MAX_SIZE])
		{
			m_intercept[i+1] = 0;
		}

		//清理死亡线程
		for (i=0; i<m_intercept[MAX_SIZE]; i+=2)
		{
			HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, m_intercept[i]);
			if (hThread==NULL)
			{
				for (DWORD j=i+2; j<m_intercept[MAX_SIZE]; j+=2)
				{
					m_intercept[j-2] = m_intercept[j];
					m_intercept[j-1] = m_intercept[j+1];
				}
				m_intercept[MAX_SIZE] -= 2;
			}
			CloseHandle(hThread);
		}

		//判断关闭拦截
		for (i=0; i<m_intercept[MAX_SIZE]; i+=2)
		{
			if (m_intercept[i+1]==1)
				break;
		}
		if (i==m_intercept[MAX_SIZE])
		{
			BYTE keyData = 0;
			DWORD size = sizeof(BYTE);
			DeviceIoControl(hDevice, IOCTL_CMD_STOP_INTERCEPT, NULL, 0, &keyData, size, &size, NULL);
		}
	}

	//关闭驱动句柄
	CloseHandle(hDevice);

	return TRUE;
}


BYTE CEnhancedEdit::try_get_key()
{
	//打开驱动句柄
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	//读取拦截扫描码
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_READ_INTERCEPT_KEY, NULL, 0, &keyData, size, &size, NULL);

	//关闭驱动句柄
	CloseHandle(hDevice);

	return keyData;
}


VOID WINAPI CEnhancedEdit::ThreadFunc(LPVOID lpParam)
{
	CEnhancedEdit* lpEnEdit;
	lpEnEdit = (CEnhancedEdit*)lpParam;

	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);

	//测试VT驱动
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		//线程运行标志置假
		lpEnEdit->m_running = FALSE;
		lpEnEdit->MessageBox(L"驱动未启动！[0x5]", L"键盘安全SDK", MB_OK);
		//错误回调
		lpEnEdit->m_errorCallback(lpEnEdit->m_pWnd);
		return;
	}
	CloseHandle(hDevice);

	//开始回调
	lpEnEdit->m_startCallback(lpEnEdit->m_pWnd);

	//获取按键
	while (lpEnEdit->m_isProtect && lpEnEdit->m_running)
	{
		DWORD status = WaitForSingleObject(lpEnEdit->m_hMutex, 0);
		if (status==WAIT_OBJECT_0 || status==WAIT_ABANDONED)
		{
			if (status==WAIT_ABANDONED)
			{
				lpEnEdit->fix_data();
			}

			//判断全局暂停
			lpEnEdit->is_global_pause();

			if(!lpEnEdit->set_intercept(1))
			{
				lpEnEdit->set_intercept(0);
				ReleaseMutex(lpEnEdit->m_hMutex);
				//线程运行标志置假
				lpEnEdit->m_running = FALSE;
				lpEnEdit->MessageBox(L"驱动未启动！[0x5]", L"键盘安全SDK", MB_OK);
				//错误回调
				lpEnEdit->m_errorCallback(lpEnEdit->m_pWnd);
				return;
			}

			BYTE keycode = lpEnEdit->try_get_key();
			if (keycode==-1)
			{
				lpEnEdit->set_intercept(0);
				ReleaseMutex(lpEnEdit->m_hMutex);
				//线程运行标志置假
				lpEnEdit->m_running = FALSE;
				lpEnEdit->MessageBox(L"驱动未启动！[0x5]", L"键盘安全SDK", MB_OK);
				//错误回调
				lpEnEdit->m_errorCallback(lpEnEdit->m_pWnd);
				return;
			}
			if (keycode)
			{
				lpEnEdit->PostMessageW(WM_EDIT_GET_KEY, keycode, 0);
			}

			ReleaseMutex(lpEnEdit->m_hMutex);
		}

		Sleep(10);
	}

	//停止拦截
	DWORD status = WaitForSingleObject(lpEnEdit->m_hMutex, INFINITE);
	if (status==WAIT_ABANDONED)
	{
		lpEnEdit->fix_data();
	}
	if(!lpEnEdit->set_intercept(0))
	{
		lpEnEdit->set_intercept(0);
		ReleaseMutex(lpEnEdit->m_hMutex);
		//线程运行标志置假
		lpEnEdit->m_running = FALSE;
		lpEnEdit->MessageBox(L"驱动未启动！[0x5]", L"键盘安全SDK", MB_OK);
		//错误回调
		lpEnEdit->m_errorCallback(lpEnEdit->m_pWnd);
		return;
	}
	clearTransFlags();
	ReleaseMutex(lpEnEdit->m_hMutex);

	//线程运行标志置假
	lpEnEdit->m_running = FALSE;

	//结束回调
	lpEnEdit->m_stopCallback(lpEnEdit->m_pWnd);
}

