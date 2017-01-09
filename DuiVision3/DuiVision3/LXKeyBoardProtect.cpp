#include "stdafx.h"

#include "LXKeyBoardProtect.h"
#include "DriverLoader.h"

//VT����������־
BOOL g_isVTProtected = FALSE;  //���ֵҪ��һЩ��ʼ����ȡ

//VT����������
HANDLE hMutex = NULL;

//VT���������ڴ�
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
			//���������
			HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (hDevice == INVALID_HANDLE_VALUE)
			{
				ReleaseMutex(hMutex);
				DuiSystem::DuiMessageBox(NULL, _T("��������رգ�"), _T("����ʱ����"));
				exit(0);
			}

			DWORD i;

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

			//����ͻ�������������ͣ��
			int pause_count = 0;
			for (i=0; i<g_intercept[MAX_SIZE]; i+=2)
			{
				if (g_intercept[i+1]==-1)
					pause_count++;
			}
			char tmp1[100];
			sprintf(tmp1, "%d|%d", g_intercept[MAX_SIZE]/2, pause_count);

			//��ȡVT����״̬
			BYTE keyData = 0;
			DWORD size = sizeof(BYTE);
			DeviceIoControl(hDevice, IOCTL_CMD_STATUS, NULL, 0, &keyData, size, &size, NULL);
			char tmp2[100];
			sprintf(tmp2, "u: %d", keyData);

			//�ر��������
			CloseHandle(hDevice);

			ReleaseMutex(hMutex);

			//�������ͷ���,�ٸ���UI

			//tmp1: �ͻ���������|��ͣ��
			//tmp2: VT����״̬
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
			//�޸�����
			fix_data();
			ReleaseMutex(hMutex);
			continue;
		}
		else if (status==WAIT_OBJECT_0)
		{
			//�ж�VT������־�ر�,�����߳�
			if (!g_isVTProtected)
			{
				ReleaseMutex(hMutex);
				return;
			}

			//���������
			HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (hDevice == INVALID_HANDLE_VALUE)
			{
				ReleaseMutex(hMutex);
				DuiSystem::DuiMessageBox(NULL, _T("��������رգ�"), _T("����ʱ����"));
				exit(0);
			}

			//��ͣ״̬��ѯ
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

					//��ͣ/�ָ����̵�ȫ��Hook�߳�

					//ö�ٽ��������߳�
					HANDLE hThreads;
					THREADENTRY32 te32;
					hThreads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, processId);
					if( hThreads == INVALID_HANDLE_VALUE )
					{
						ReleaseMutex(hMutex);
						DuiSystem::DuiMessageBox(NULL, _T("����ö���߳�ʧ�ܣ�"), _T("����ʱ����"));
						exit(0);
						continue;
					}
					te32.dwSize = sizeof(THREADENTRY32);

					//DLL�Ƿ���ع�
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

					//DLL�Ѿ����ع�����ת״̬
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
									//��ת״̬
									g_intercept[i+1] = isProcessPause ? 0 : -1;
								}
							}
							anyMore = Thread32Next(hThreads, &te32);
						}
					}

					CloseHandle(hThreads);

					//DLLȫ��ע��
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

			//�ر��������
			CloseHandle(hDevice);

			ReleaseMutex(hMutex);
		}
		Sleep(10);
	}
}

bool judgeProtect()
{
	//VT������ȡ����״̬
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		DuiSystem::DuiMessageBox(NULL, _T("����δ������"), _T("��ʼ������"));
		exit(0);
	}
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_STATUS, NULL, 0, &keyData, size, &size, NULL);
	CloseHandle(hDevice);

	//����VT����״̬
	if (keyData==0 || keyData==1)
		return true;
	else
		return false;
}

void stopProtect()
{
	WaitForSingleObject(hMutex, INFINITE);

	//�޸�����
	fix_data();

	//ȡ��VT����״̬
	g_isVTProtected = FALSE;

	//�����ֹpause״̬��ȡ�߳�

	//VT��������ֹͣ����
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		ReleaseMutex(hMutex);
		DuiSystem::DuiMessageBox(NULL, _T("��������رգ�"), _T("����ʱ����"));
		exit(0);
	}
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_GLOBAL_STOP, NULL, 0, &keyData, size, &size, NULL);
	CloseHandle(hDevice);

	//��ͣȫ���ͻ��߳�
	for (DWORD i=0; i<g_intercept[MAX_SIZE]; i+=2)
	{
		g_intercept[i+1] = -1;
	}

	ReleaseMutex(hMutex);
}

void initProtect()
{
	//�����������
	hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
	if (hMutex==NULL)
	{
		DuiSystem::DuiMessageBox(NULL, _T("��ʼ��������ʧ�ܣ�"), _T("��ʼ������"));
		exit(0);
	}

	//ȫ�»�����
	if (GetLastError()!=ERROR_ALREADY_EXISTS) 
	{
		//��ʼ�������ڴ�
		HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_SIZE*sizeof(DWORD), SHM_NAME);
		if (hMap==NULL)
		{
			CloseHandle(hMutex);
			DuiSystem::DuiMessageBox(NULL, _T("��ʼ�������ڴ�ʧ�ܣ�"), _T("��ʼ������"));
			exit(0);
		}
		//ӳ�乲���ڴ�
		g_intercept = (DWORD*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (g_intercept==NULL)
		{
			CloseHandle(hMutex);
			CloseHandle(hMap);
			DuiSystem::DuiMessageBox(NULL, _T("ӳ�乲���ڴ�ʧ�ܣ�"), _T("��ʼ������"));
			exit(0);
		}
		//�����ڴ�����
		memset(g_intercept, 0, MAX_SIZE*sizeof(DWORD));
	}
	//�Ѵ��ڵĻ�����
	else
	{
		//�򿪹����ڴ�
		HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, SHM_NAME);
		if (hMap==NULL)
		{
			CloseHandle(hMutex);
			DuiSystem::DuiMessageBox(NULL, _T("�򿪹����ڴ�ʧ�ܣ�"), _T("��ʼ������"));
			exit(0);
		}
		//ӳ�乲���ڴ�
		g_intercept = (DWORD*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (g_intercept==NULL)
		{
			CloseHandle(hMutex);
			CloseHandle(hMap);
			DuiSystem::DuiMessageBox(NULL, _T("ӳ�乲���ڴ�ʧ�ܣ�"), _T("��ʼ������"));
			exit(0);
		}
	}

	//�����ػ��߳�
	DWORD dwReadThreadID;
	HANDLE hGuardThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)guard_proc, NULL, 0, &dwReadThreadID);
	CloseHandle(hGuardThread);
}

void startProtect()
{
	WaitForSingleObject(hMutex, INFINITE);

	//�޸�����
	fix_data();

	//VT������������
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		ReleaseMutex(hMutex);
		DuiSystem::DuiMessageBox(NULL, _T("��������رգ�"), _T("����ʱ����"));
		exit(0);
	}
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_GLOBAL_RUN, NULL, 0, &keyData, size, &size, NULL);
	CloseHandle(hDevice);

	//�ָ�ȫ���ͻ��߳�
	for (DWORD i=0; i<g_intercept[MAX_SIZE]; i+=2)
	{
		g_intercept[i+1] = 0;
	}

	//VT�������ÿ�������
	g_isVTProtected = TRUE;

	//����pause״̬��ȡ�߳�
	DWORD dwReadThreadID;
	HANDLE hPauseThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pause_proc, 0, 0, &dwReadThreadID);
	CloseHandle(hPauseThread);

	ReleaseMutex(hMutex);
}