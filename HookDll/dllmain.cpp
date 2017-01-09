// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include <stdio.h>
#include <winsvc.h> 
#include <winioctl.h>
#include "KeyMap.h"

#pragma comment(lib, "user32.lib")

//DLL句柄
HMODULE g_Dll = NULL;

//Hook库的API
BOOL (__cdecl *HookFunction)(ULONG_PTR OriginalFunction, ULONG_PTR NewFunction);
VOID (__cdecl *UnhookFunction)(ULONG_PTR Function);
ULONG_PTR (__cdecl *GetOriginalFunction)(ULONG_PTR Hook);


//客户进程共享同步
#define MUTEX_NAME L"lx_test_lock"
#define SHM_NAME L"lx_test_memory"

BOOL isFirstStart = TRUE;
BOOL isFirstStop = TRUE;

HANDLE hMutex = NULL;
HANDLE hMap = NULL;

#define MAX_CLIENT 1024
#define MAX_SIZE   2049
DWORD* g_intercept = NULL;


//VT驱动
#define	DEVICE_NAME		 L"\\\\.\\KeyboardVT"

#define IOCTL_CMD_READ_KEY	 		 	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_START_INTERCEPT	 	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_STOP_INTERCEPT		 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_GLOBAL_RUN			 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_GLOBAL_STOP			 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_READ_INTERCEPT_KEY	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_CLEAN_MONITOR			 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_STATUS				 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CMD_CATCH_PAUSE_STATUS	 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)


BOOL set_intercept(DWORD Flag)
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
		for (i=0; i<g_intercept[MAX_SIZE]; i+=2)
		{
			if (g_intercept[i]==ThreadId)
				break;
		}
		if (i==g_intercept[MAX_SIZE])
		{
			g_intercept[i] = ThreadId;
			g_intercept[i+1] = 1;
			g_intercept[MAX_SIZE]+=2;

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

			if (g_intercept[i+1]==0 || g_intercept[i+1]==-1)
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

			g_intercept[i+1] = 1;
		}
	}
	else
	{
		DWORD i;
		for (i=0; i<g_intercept[MAX_SIZE]; i+=2)
		{
			if (g_intercept[i]==ThreadId)
				break;
		}
		if (i!=g_intercept[MAX_SIZE])
		{
			g_intercept[i+1] = 0;
		}

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
	}

	//关闭驱动句柄
	CloseHandle(hDevice);

	return TRUE;
}

DWORD last_status = -2;
DWORD is_pause()
{
	DWORD ThreadId = GetCurrentThreadId();

	DWORD i;
	for (i=0; i<g_intercept[MAX_SIZE]; i+=2)
	{
		if (g_intercept[i]==ThreadId)
			break;
	}
	if (i==g_intercept[MAX_SIZE])
	{
		return FALSE;
	}
	else
	{
		DWORD status = g_intercept[i+1];
		if (last_status==-2)
		{
			last_status = status;  //初态获取
			return FALSE;
		}

		if (status==-1)
		{
			if (last_status != -1)
			{
				last_status = status;
				return 2;  //切到暂停
			}
			else
			{
				last_status = status;
				return 1;  //一直暂停
			}
		}
		else
		{
			if (last_status == -1)
			{
				last_status = status;
				return 3;      //切出暂停
			}
			else
			{
				last_status = status;
				return FALSE;  //一直非暂停
			}
		}
	}
}

VOID fix_data()
{
	DWORD i;
	for (i=0; g_intercept[i]!=0; i+=2);
	g_intercept[MAX_SIZE] = i;
}

BYTE try_get_key()
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


BOOL WINAPI Fake_GetMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
	BOOL ret;
	while (true)
	{
		BOOL (WINAPI *pPeekMessageW)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
		pPeekMessageW = (BOOL (WINAPI *)(LPMSG,HWND,UINT,UINT,UINT))GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW");
		ret = pPeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE);

		if (lpMsg->message==WM_QUIT)
		{
			return FALSE;
		}
		
		if (ret)
		{
			return TRUE;
		}

		Sleep(10);
	}
}


BOOL WINAPI Fake_PeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	BOOL ret;

	//只初始化一次
	if (isFirstStart)
	{
		isFirstStart = FALSE;

		//测试驱动加载
		HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
			UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
			MessageBox(NULL, L"加载保护失败！[0x0]", L"键盘安全套件", MB_OK);
			return FALSE;
		}
		CloseHandle(hDevice);

		//打开互斥量
		hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
		if (hMutex==NULL) //打开失败
		{
			UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
			UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
			MessageBox(NULL, L"加载保护失败！[0x1]", L"键盘安全套件", MB_OK);
			return FALSE;
		}
		if (GetLastError()!=ERROR_ALREADY_EXISTS) //全新互斥量，说明是唯一拦截客户进程
		{
			//初始化共享内存
			hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_SIZE*sizeof(DWORD), SHM_NAME);
			if (hMap==NULL)
			{
				CloseHandle(hMutex);
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
				MessageBox(NULL, L"加载保护失败！[0x2]", L"键盘安全套件", MB_OK);
				return FALSE;
			}
			//映射共享内存
			g_intercept = (DWORD*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if (g_intercept==NULL)
			{
				CloseHandle(hMutex);
				CloseHandle(hMap);
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
				MessageBox(NULL, L"加载保护失败！[0x3]", L"键盘安全套件", MB_OK);
				return FALSE;
			}
			memset(g_intercept, 0, MAX_SIZE*sizeof(DWORD));

			//加载成功
			MessageBox(NULL, L"开启保护成功！[0x1]", L"键盘安全套件", MB_OK);
		}
		else //已存在互斥量，简单初始化
		{
			//打开共享内存
			hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, SHM_NAME);
			if (hMap==NULL)
			{
				CloseHandle(hMutex);
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
				MessageBox(NULL, L"加载保护失败！[0x4]", L"键盘安全套件", MB_OK);
				return FALSE;
			}
			//映射共享内存
			g_intercept = (DWORD*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if (g_intercept==NULL)
			{
				CloseHandle(hMutex);
				CloseHandle(hMap);
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
				MessageBox(NULL, L"加载保护失败！[0x5]", L"键盘安全套件", MB_OK);
				return FALSE;
			}

			//加载成功
			MessageBox(NULL, L"开启保护成功！[0x0]", L"键盘安全套件", MB_OK);
		}
	}

	BOOL (WINAPI *pPeekMessageW)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
	pPeekMessageW = (BOOL (WINAPI *)(LPMSG,HWND,UINT,UINT,UINT))GetOriginalFunction((ULONG_PTR)Fake_PeekMessageW);
	ret = pPeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	if (ret && lpMsg->message!=WM_KEYDOWN && lpMsg->message!=WM_KEYUP)
	{
		if (lpMsg->message==WM_QUIT && isFirstStop)
		{
			isFirstStop = FALSE;
			DWORD status = WaitForSingleObject(hMutex, INFINITE);
			if (status==WAIT_ABANDONED)
			{
				fix_data();
			}
			set_intercept(0);
			ReleaseMutex(hMutex);
			CloseHandle(hMutex);
			UnmapViewOfFile(g_intercept);
			CloseHandle(hMap);
		}
	}
	else
	{
		DWORD status = WaitForSingleObject(hMutex, 0);
		if (status==WAIT_OBJECT_0 || status==WAIT_ABANDONED)
		{
			if (status==WAIT_ABANDONED)
			{
				fix_data();
			}

			status = is_pause();
			if (status)
			{
				ReleaseMutex(hMutex);
				if (status==2)
				{
					MessageBox(NULL, L"关闭保护成功！[0x0]", L"键盘安全套件", MB_OK);
				}
				if (status==3)
				{
					MessageBox(NULL, L"开启保护成功！[0x0]", L"键盘安全套件", MB_OK);
				}
				return ret;
			}

			HWND focusWnd = GetFocus();
			if (focusWnd!=NULL)
			{
				if(!set_intercept(1))
				{
					set_intercept(0);
					ReleaseMutex(hMutex);
					CloseHandle(hMutex);
					UnmapViewOfFile(g_intercept);
					CloseHandle(hMap);
					UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
					UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
					MessageBox(NULL, L"驱动未启动！[0x0]", L"键盘安全套件", MB_OK);
					return FALSE;
				}
				BYTE keycode = try_get_key();
				if (keycode==-1)
				{
					set_intercept(0);
					ReleaseMutex(hMutex);
					CloseHandle(hMutex);
					UnmapViewOfFile(g_intercept);
					CloseHandle(hMap);
					UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
					UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
					MessageBox(NULL, L"驱动未启动！[0x1]", L"键盘安全套件", MB_OK);
					return FALSE;
				}
				if (keycode)
				{
					char ascii = transSacnCode(keycode);
					if (ascii != 0)
					{
						DWORD scancode = keycode & 0x7F;
						lpMsg->message = WM_CHAR;
						lpMsg->hwnd = focusWnd;
						lpMsg->wParam = ascii;
						lpMsg->lParam = 0x00000001 | (scancode << 16);
						ret = TRUE;
					}
				}
			}
			else
			{
				if(!set_intercept(0))
				{
					set_intercept(0);
					ReleaseMutex(hMutex);
					CloseHandle(hMutex);
					UnmapViewOfFile(g_intercept);
					CloseHandle(hMap);
					UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
					UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
					MessageBox(NULL, L"驱动未启动！[0x2]", L"键盘安全套件", MB_OK);
					return FALSE;
				}
				clearTransFlags();
			}

			ReleaseMutex(hMutex);
		}
	}

	return ret;
}


BOOL InitHook()
{
#ifdef WIN64
	HMODULE hHookEngineDll = LoadLibraryW(L"C:\\KeyboardSecurityTool\\NtHookEngine64.dll");
#else
	HMODULE hHookEngineDll = LoadLibraryW(L"C:\\KeyboardSecurityTool\\NtHookEngine32.dll");
#endif
	//获取Hook库的API
	HookFunction = (BOOL (__cdecl *)(ULONG_PTR, ULONG_PTR))GetProcAddress(hHookEngineDll, "HookFunction");
	UnhookFunction = (VOID (__cdecl *)(ULONG_PTR))GetProcAddress(hHookEngineDll, "UnhookFunction");
	GetOriginalFunction = (ULONG_PTR (__cdecl *)(ULONG_PTR))GetProcAddress(hHookEngineDll, "GetOriginalFunction");

	//获取失败
	if (HookFunction == NULL || UnhookFunction == NULL || GetOriginalFunction == NULL)
	{
		MessageBox(NULL, L"加载保护失败！[0x6]", L"键盘安全套件", MB_OK);
		return FALSE;
	}

	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if (InitHook())
		{
			HookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"), (ULONG_PTR)&Fake_PeekMessageW);
			HookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"), (ULONG_PTR)&Fake_GetMessageW);
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
		UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
		break;
	}
	return TRUE;
}