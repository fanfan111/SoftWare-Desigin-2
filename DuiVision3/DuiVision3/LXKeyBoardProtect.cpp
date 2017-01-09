#include "stdafx.h"

#include "LXKeyBoardProtect.h"
#include "DriverLoader.h"

//VT保护开启标志
BOOL g_isVTProtected = FALSE;  //这个值要在一些初始化获取

//VT保护互斥量
HANDLE hMutex = NULL;

//VT保护共享内存
DWORD* g_intercept = NULL;


BOOL Is64BitOS()
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

BOOL Is64BitPorcess(DWORD dwProcessID)
{
    if (!Is64BitOS())
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

void fix_data()
{
	DWORD i;
	for (i=0; g_intercept[i]!=0; i+=2);
	g_intercept[MAX_SIZE] = i;
}

map<DWORD, DWORD> all_thread()
{
	map<DWORD, DWORD> ret;

	WaitForSingleObject(hMutex, INFINITE);
	fix_data();
	for (DWORD i=0; i<g_intercept[MAX_SIZE]; i+=2)
		ret.insert(pair<DWORD, DWORD>(g_intercept[i], g_intercept[i+1]));
	ReleaseMutex(hMutex);

	return ret;
}

void guard_proc()
{
	while (true)
	{
		DWORD status = WaitForSingleObject(hMutex, INFINITE);
		if (status==WAIT_ABANDONED)
		{
			fix_data();
			ReleaseMutex(hMutex);
			continue;
		}
		else if (status==WAIT_OBJECT_0)
		{
			//打开驱动句柄
			HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (hDevice == INVALID_HANDLE_VALUE)
			{
				ReleaseMutex(hMutex);
				DuiSystem::DuiMessageBox(NULL, _T("驱动意外关闭！"), _T("运行时错误"));
				exit(0);
			}

			DWORD i;

			//清理死亡线程
			for (i=0; i<g_intercept[MAX_SIZE]; i+=2)
			{
				HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, g_intercept[i]);
				if (hThread==NULL)
				{
					for (DWORD j=i+2; j<g_intercept[MAX_SIZE]; j+=2)
					{
						g_intercept[j-2] = g_intercept[j];
						g_intercept[j-1] = g_intercept[j+1];
					}
					g_intercept[MAX_SIZE] -= 2;
				}
				CloseHandle(hThread);
			}

			//判断关闭拦截
			for (i=0; i<g_intercept[MAX_SIZE]; i+=2)
			{
				if (g_intercept[i+1]==1)
					break;
			}
			if (i==g_intercept[MAX_SIZE])
			{
				BYTE keyData = 0;
				DWORD size = sizeof(BYTE);
				DeviceIoControl(hDevice, IOCTL_CMD_STOP_INTERCEPT, NULL, 0, &keyData, size, &size, NULL);
			}

			//计算客户进程总数、暂停数
			int pause_count = 0;
			for (i=0; i<g_intercept[MAX_SIZE]; i+=2)
			{
				if (g_intercept[i+1]==-1)
					pause_count++;
			}
			char tmp1[100];
			sprintf(tmp1, "%d|%d", g_intercept[MAX_SIZE]/2, pause_count);

			//获取VT运行状态
			BYTE keyData = 0;
			DWORD size = sizeof(BYTE);
			DeviceIoControl(hDevice, IOCTL_CMD_STATUS, NULL, 0, &keyData, size, &size, NULL);
			char tmp2[100];
			sprintf(tmp2, "u: %d", keyData);

			//关闭驱动句柄
			CloseHandle(hDevice);

			ReleaseMutex(hMutex);

			//必须先释放锁,再更新UI

			//tmp1: 客户进程总数|暂停数
			//tmp2: VT运行状态
		}
		Sleep(10);
	}
}

void pause_proc()
{
	while (g_isVTProtected)
	{
		DWORD status = WaitForSingleObject(hMutex, INFINITE);
		if (status==WAIT_ABANDONED)
		{
			//修复数据
			fix_data();
			ReleaseMutex(hMutex);
			continue;
		}
		else if (status==WAIT_OBJECT_0)
		{
			//判断VT保护标志关闭,结束线程
			if (!g_isVTProtected)
			{
				ReleaseMutex(hMutex);
				return;
			}

			//打开驱动句柄
			HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (hDevice == INVALID_HANDLE_VALUE)
			{
				ReleaseMutex(hMutex);
				DuiSystem::DuiMessageBox(NULL, _T("驱动意外关闭！"), _T("运行时错误"));
				exit(0);
			}

			//暂停状态查询
			BYTE pauseStatus = 0;
			DWORD size = sizeof(BYTE);
			DeviceIoControl(hDevice, IOCTL_CMD_CATCH_PAUSE_STATUS, NULL, 0, &pauseStatus, size, &size, NULL);

			if (pauseStatus)
			{
				HWND hWnd = GetForegroundWindow();
				if (hWnd)
				{
					DWORD processId = 0;
				    GetWindowThreadProcessId(hWnd, &processId);
					BOOL is64Process = Is64BitPorcess(processId);

					//暂停/恢复进程的全部Hook线程

					//枚举进程所有线程
					HANDLE hThreads;
					THREADENTRY32 te32;
					hThreads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, processId);
					if( hThreads == INVALID_HANDLE_VALUE )
					{
						ReleaseMutex(hMutex);
						DuiSystem::DuiMessageBox(NULL, _T("进程枚举线程失败！"), _T("运行时警告"));
						exit(0);
						continue;
					}
					te32.dwSize = sizeof(THREADENTRY32);

					//DLL是否加载过
					BOOL isProcessLoadedDll = FALSE;
					BOOL isProcessPause = FALSE;

					BOOL anyMore = Thread32First(hThreads, &te32);
					while(anyMore)
					{
						if(te32.th32OwnerProcessID == processId)
						{
							DWORD threadId = te32.th32ThreadID;
							DWORD i;
							for (i=0; i<g_intercept[MAX_SIZE]; i+=2)
							{
								if (g_intercept[i]==threadId)
								{
									isProcessLoadedDll = TRUE;
									if (g_intercept[i+1]==-1)
										isProcessPause = TRUE;
									break;
								}
							}
							if (isProcessLoadedDll == TRUE)
								break;
						}
						anyMore = Thread32Next(hThreads, &te32);
					}

					//DLL已经加载过，翻转状态
					if (isProcessLoadedDll)
					{
						anyMore = Thread32First(hThreads, &te32);
						while(anyMore)
						{
							if(te32.th32OwnerProcessID == processId)
							{
								DWORD threadId = te32.th32ThreadID;
								DWORD i;
								for (i=0; i<g_intercept[MAX_SIZE]; i+=2)
								{
									if (g_intercept[i]==threadId)
									{
										break;
									}
								}
								if (i != g_intercept[MAX_SIZE])
								{
									//翻转状态
									g_intercept[i+1] = isProcessPause ? 0 : -1;
								}
							}
							anyMore = Thread32Next(hThreads, &te32);
						}
					}

					CloseHandle(hThreads);

					//DLL全新注入
					if (!isProcessLoadedDll)
					{
						if (is64Process)
						{
							WCHAR params[100];
							swprintf(params, L"%d %ws", processId, INJECT_X64_DLL_PATH);
							ShellExecuteW(NULL, NULL, INJECT_X64_EXE_PATH, params, NULL, SW_NORMAL);
						}
						else
						{
							WCHAR params[100];
							swprintf(params, L"%d %ws", processId, INJECT_X86_DLL_PATH);
							ShellExecuteW(NULL, NULL, INJECT_X86_EXE_PATH, params, NULL, SW_NORMAL);
						}
					}

				}
			}

			//关闭驱动句柄
			CloseHandle(hDevice);

			ReleaseMutex(hMutex);
		}
		Sleep(10);
	}
}

bool judgeProtect()
{
	//VT驱动获取运行状态
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		DuiSystem::DuiMessageBox(NULL, _T("驱动未开启！"), _T("初始化错误"));
		exit(0);
	}
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_STATUS, NULL, 0, &keyData, size, &size, NULL);
	CloseHandle(hDevice);

	//返回VT保护状态
	if (keyData==0 || keyData==1)
		return true;
	else
		return false;
}

void stopProtect()
{
	WaitForSingleObject(hMutex, INFINITE);

	//修复数据
	fix_data();

	//取消VT保护状态
	g_isVTProtected = FALSE;

	//间接中止pause状态获取线程

	//VT驱动设置停止保护
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		ReleaseMutex(hMutex);
		DuiSystem::DuiMessageBox(NULL, _T("驱动意外关闭！"), _T("运行时错误"));
		exit(0);
	}
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_GLOBAL_STOP, NULL, 0, &keyData, size, &size, NULL);
	CloseHandle(hDevice);

	//暂停全部客户线程
	for (DWORD i=0; i<g_intercept[MAX_SIZE]; i+=2)
	{
		g_intercept[i+1] = -1;
	}

	ReleaseMutex(hMutex);
}

void initProtect()
{
	//创建互斥对象
	hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
	if (hMutex==NULL)
	{
		DuiSystem::DuiMessageBox(NULL, _T("初始化互斥量失败！"), _T("初始化错误"));
		exit(0);
	}

	//全新互斥量
	if (GetLastError()!=ERROR_ALREADY_EXISTS) 
	{
		//初始化共享内存
		HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_SIZE*sizeof(DWORD), SHM_NAME);
		if (hMap==NULL)
		{
			CloseHandle(hMutex);
			DuiSystem::DuiMessageBox(NULL, _T("初始化共享内存失败！"), _T("初始化错误"));
			exit(0);
		}
		//映射共享内存
		g_intercept = (DWORD*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (g_intercept==NULL)
		{
			CloseHandle(hMutex);
			CloseHandle(hMap);
			DuiSystem::DuiMessageBox(NULL, _T("映射共享内存失败！"), _T("初始化错误"));
			exit(0);
		}
		//共享内存清零
		memset(g_intercept, 0, MAX_SIZE*sizeof(DWORD));
	}
	//已存在的互斥量
	else
	{
		//打开共享内存
		HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, SHM_NAME);
		if (hMap==NULL)
		{
			CloseHandle(hMutex);
			DuiSystem::DuiMessageBox(NULL, _T("打开共享内存失败！"), _T("初始化错误"));
			exit(0);
		}
		//映射共享内存
		g_intercept = (DWORD*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (g_intercept==NULL)
		{
			CloseHandle(hMutex);
			CloseHandle(hMap);
			DuiSystem::DuiMessageBox(NULL, _T("映射共享内存失败！"), _T("初始化错误"));
			exit(0);
		}
	}

	//开启守护线程
	DWORD dwReadThreadID;
	HANDLE hGuardThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)guard_proc, NULL, 0, &dwReadThreadID);
	CloseHandle(hGuardThread);
}

void startProtect()
{
	WaitForSingleObject(hMutex, INFINITE);

	//修复数据
	fix_data();

	//VT驱动设置运行
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		ReleaseMutex(hMutex);
		DuiSystem::DuiMessageBox(NULL, _T("驱动意外关闭！"), _T("运行时错误"));
		exit(0);
	}
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_GLOBAL_RUN, NULL, 0, &keyData, size, &size, NULL);
	CloseHandle(hDevice);

	//恢复全部客户线程
	for (DWORD i=0; i<g_intercept[MAX_SIZE]; i+=2)
	{
		g_intercept[i+1] = 0;
	}

	//VT驱动设置开启保护
	g_isVTProtected = TRUE;

	//开启pause状态获取线程
	DWORD dwReadThreadID;
	HANDLE hPauseThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pause_proc, 0, 0, &dwReadThreadID);
	CloseHandle(hPauseThread);

	ReleaseMutex(hMutex);
}