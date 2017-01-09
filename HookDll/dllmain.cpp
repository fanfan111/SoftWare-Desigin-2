// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"

#include <stdio.h>
#include <winsvc.h> 
#include <winioctl.h>
#include "KeyMap.h"

#pragma comment(lib, "user32.lib")

//DLL���
HMODULE g_Dll = NULL;

//Hook���API
BOOL (__cdecl *HookFunction)(ULONG_PTR OriginalFunction, ULONG_PTR NewFunction);
VOID (__cdecl *UnhookFunction)(ULONG_PTR Function);
ULONG_PTR (__cdecl *GetOriginalFunction)(ULONG_PTR Hook);


//�ͻ����̹���ͬ��
#define MUTEX_NAME L"lx_test_lock"
#define SHM_NAME L"lx_test_memory"

BOOL isFirstStart = TRUE;
BOOL isFirstStop = TRUE;

HANDLE hMutex = NULL;
HANDLE hMap = NULL;

#define MAX_CLIENT 1024
#define MAX_SIZE   2049
DWORD* g_intercept = NULL;


//VT����
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
	//���������
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

			//��������
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
				//��������
				keyData = 0;
				size = sizeof(BYTE);
				DeviceIoControl(hDevice, IOCTL_CMD_START_INTERCEPT, NULL, 0, &keyData, size, &size, NULL);
			}
			else if (keyData==1 || keyData==3)
			{
				//��������
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

		//���������߳�
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

		//�жϹر�����
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

	//�ر��������
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
			last_status = status;  //��̬��ȡ
			return FALSE;
		}

		if (status==-1)
		{
			if (last_status != -1)
			{
				last_status = status;
				return 2;  //�е���ͣ
			}
			else
			{
				last_status = status;
				return 1;  //һֱ��ͣ
			}
		}
		else
		{
			if (last_status == -1)
			{
				last_status = status;
				return 3;      //�г���ͣ
			}
			else
			{
				last_status = status;
				return FALSE;  //һֱ����ͣ
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
	//���������
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	//��ȡ����ɨ����
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_READ_INTERCEPT_KEY, NULL, 0, &keyData, size, &size, NULL);

	//�ر��������
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

	//ֻ��ʼ��һ��
	if (isFirstStart)
	{
		isFirstStart = FALSE;

		//������������
		HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
			UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
			MessageBox(NULL, L"���ر���ʧ�ܣ�[0x0]", L"���̰�ȫ�׼�", MB_OK);
			return FALSE;
		}
		CloseHandle(hDevice);

		//�򿪻�����
		hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
		if (hMutex==NULL) //��ʧ��
		{
			UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
			UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
			MessageBox(NULL, L"���ر���ʧ�ܣ�[0x1]", L"���̰�ȫ�׼�", MB_OK);
			return FALSE;
		}
		if (GetLastError()!=ERROR_ALREADY_EXISTS) //ȫ�»�������˵����Ψһ���ؿͻ�����
		{
			//��ʼ�������ڴ�
			hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_SIZE*sizeof(DWORD), SHM_NAME);
			if (hMap==NULL)
			{
				CloseHandle(hMutex);
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
				MessageBox(NULL, L"���ر���ʧ�ܣ�[0x2]", L"���̰�ȫ�׼�", MB_OK);
				return FALSE;
			}
			//ӳ�乲���ڴ�
			g_intercept = (DWORD*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if (g_intercept==NULL)
			{
				CloseHandle(hMutex);
				CloseHandle(hMap);
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
				MessageBox(NULL, L"���ر���ʧ�ܣ�[0x3]", L"���̰�ȫ�׼�", MB_OK);
				return FALSE;
			}
			memset(g_intercept, 0, MAX_SIZE*sizeof(DWORD));

			//���سɹ�
			MessageBox(NULL, L"���������ɹ���[0x1]", L"���̰�ȫ�׼�", MB_OK);
		}
		else //�Ѵ��ڻ��������򵥳�ʼ��
		{
			//�򿪹����ڴ�
			hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, SHM_NAME);
			if (hMap==NULL)
			{
				CloseHandle(hMutex);
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
				MessageBox(NULL, L"���ر���ʧ�ܣ�[0x4]", L"���̰�ȫ�׼�", MB_OK);
				return FALSE;
			}
			//ӳ�乲���ڴ�
			g_intercept = (DWORD*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if (g_intercept==NULL)
			{
				CloseHandle(hMutex);
				CloseHandle(hMap);
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"GetMessageW"));
				UnhookFunction((ULONG_PTR)GetProcAddress(GetModuleHandle(L"user32.dll"),"PeekMessageW"));
				MessageBox(NULL, L"���ر���ʧ�ܣ�[0x5]", L"���̰�ȫ�׼�", MB_OK);
				return FALSE;
			}

			//���سɹ�
			MessageBox(NULL, L"���������ɹ���[0x0]", L"���̰�ȫ�׼�", MB_OK);
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
					MessageBox(NULL, L"�رձ����ɹ���[0x0]", L"���̰�ȫ�׼�", MB_OK);
				}
				if (status==3)
				{
					MessageBox(NULL, L"���������ɹ���[0x0]", L"���̰�ȫ�׼�", MB_OK);
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
					MessageBox(NULL, L"����δ������[0x0]", L"���̰�ȫ�׼�", MB_OK);
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
					MessageBox(NULL, L"����δ������[0x1]", L"���̰�ȫ�׼�", MB_OK);
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
					MessageBox(NULL, L"����δ������[0x2]", L"���̰�ȫ�׼�", MB_OK);
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
	//��ȡHook���API
	HookFunction = (BOOL (__cdecl *)(ULONG_PTR, ULONG_PTR))GetProcAddress(hHookEngineDll, "HookFunction");
	UnhookFunction = (VOID (__cdecl *)(ULONG_PTR))GetProcAddress(hHookEngineDll, "UnhookFunction");
	GetOriginalFunction = (ULONG_PTR (__cdecl *)(ULONG_PTR))GetProcAddress(hHookEngineDll, "GetOriginalFunction");

	//��ȡʧ��
	if (HookFunction == NULL || UnhookFunction == NULL || GetOriginalFunction == NULL)
	{
		MessageBox(NULL, L"���ر���ʧ�ܣ�[0x6]", L"���̰�ȫ�׼�", MB_OK);
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