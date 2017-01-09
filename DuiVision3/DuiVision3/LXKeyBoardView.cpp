#include "stdafx.h"
#include "DuiHandlerControl.h"
#include "LXKeyBoardView.h"

//--------------------------------------------------------------------------------------------
#include <strsafe.h>
void ErrorExit(LPTSTR lpszFunction = _T("")) 
{ 
    // Retrieve the system error message for the last-error code
 
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 
 
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
 
    // Display the error message and exit the process
 
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

	DuiSystem::DuiMessageBox(NULL, (LPCTSTR)lpDisplayBuf, _T("运行时错误"));
 
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}
//--------------------------------------------------------------------------------------------


//UI按键处理回调函数
KEY_CALLBACK g_callback = CDuiHandlerControl::HookInthere;


//---------------------------------------------------------------------------------------------------------------------


//VT记录开启标志
BOOL g_isVTRecord = FALSE;

//VT记录文件指针
FILE* g_VTFilePoint = NULL;

VOID WINAPI VTReadKeyProc(LPVOID lpParam)
{
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		DuiSystem::DuiMessageBox(NULL, _T("驱动意外关闭！"), _T("运行时错误"));
		exit(0);
	}

	//打开VT记录文件
	g_VTFilePoint = fopen(VT_RECORD_FILE, "a+");

	while (g_isVTRecord)
	{
		BYTE keyData = 0;
		DWORD size = sizeof(BYTE);
		if (DeviceIoControl(hDevice, IOCTL_CMD_READ_KEY, NULL, 0, &keyData, size, &size, NULL))
		{
			if (keyData != 0)
			{
				g_callback(keyData, L"");
				fprintf(g_VTFilePoint, "%02X ", keyData);
				fflush(g_VTFilePoint);
			}
		}
		Sleep(10);
	}

	//关闭VT记录文件
	fclose(g_VTFilePoint);

	CloseHandle(hDevice);
}

VOID startVTRecord()
{
	//VT记录开启标志打开
	g_isVTRecord = TRUE;

	//清空VT缓冲区
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		DuiSystem::DuiMessageBox(NULL, _T("驱动未开启！"), _T("初始化错误"));
		exit(0);
	}
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_CLEAN_MONITOR, NULL, 0, &keyData, size, &size, NULL);
	CloseHandle(hDevice);

	//开启VT记录读取线程
	DWORD dwReadThreadID;
	HANDLE hReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)VTReadKeyProc, NULL, 0, &dwReadThreadID);
	CloseHandle(hReadThread);
}

VOID stopVTRecord()
{
	//VT记录开启标志关闭
	g_isVTRecord = FALSE;

	//间接停止VT记录读取线程
}








//------------------------------------------------------------------------------------------------------------------------








//R3记录开启标志
BOOL g_isR3Record = FALSE;

//R3记录文件指针
FILE* g_R3FilePoint = NULL;

LRESULT CALLBACK R3ReadKeyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UINT dwSize;
	RAWINPUTDEVICE rid;
	RAWINPUT *buffer;

	switch(msg)
	{
	case WM_CREATE:
		// 注册原始输入设备，捕获键盘记录
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x06;
		rid.dwFlags = RIDEV_INPUTSINK;
		rid.hwndTarget = hwnd;

		// 注册失败
		if(!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
		{
			DuiSystem::DuiMessageBox(NULL, _T("R3记录初始化失败！"), _T("运行时错误"));
			exit(0);
		}
		break;

	case WM_INPUT:
		// 获取原始输入缓冲区大小
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

		// 申请缓冲区内存
		buffer = (RAWINPUT*)HeapAlloc(GetProcessHeap(), 0, dwSize);

		// 获取原始键盘输入
		if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &dwSize, sizeof(RAWINPUTHEADER)))
		{
			// 有键按下
			if(buffer->header.dwType==RIM_TYPEKEYBOARD && buffer->data.keyboard.Message==WM_KEYDOWN)
			{
				if (buffer->data.keyboard.Flags == 0)
				{
					g_callback(buffer->data.keyboard.MakeCode, L".r3");
					fprintf(g_R3FilePoint, "%02X ", buffer->data.keyboard.MakeCode);
					fflush(g_R3FilePoint);
				}
				else if (buffer->data.keyboard.Flags == 2)
				{
					g_callback(0xE0, L".r3");
					g_callback(buffer->data.keyboard.MakeCode, L".r3");
					fprintf(g_R3FilePoint, "E0 %02X ", buffer->data.keyboard.MakeCode);
					fflush(g_R3FilePoint);
				}
				else if (buffer->data.keyboard.Flags == 4)
				{
					g_callback(0xE1, L".r3");
					g_callback(buffer->data.keyboard.MakeCode, L".r3");
					fprintf(g_R3FilePoint, "E1 %02X ", buffer->data.keyboard.MakeCode);
					fflush(g_R3FilePoint);
				}
			}
			// 有键弹起
			if (buffer->header.dwType==RIM_TYPEKEYBOARD && buffer->data.keyboard.Message==WM_KEYUP)
			{
				if (buffer->data.keyboard.Flags == 1)
				{
					g_callback(buffer->data.keyboard.MakeCode+0x80, L".r3");
					fprintf(g_R3FilePoint, "%02X ", buffer->data.keyboard.MakeCode+0x80);
					fflush(g_R3FilePoint);
				}
				else if (buffer->data.keyboard.Flags == 3)
				{
					g_callback(0xE0, L".r3");
					g_callback(buffer->data.keyboard.MakeCode+0x80, L".r3");
					fprintf(g_R3FilePoint, "E0 %02X ", buffer->data.keyboard.MakeCode+0x80);
					fflush(g_R3FilePoint);
				}
				else if (buffer->data.keyboard.Flags == 5)
				{
					g_callback(0xE1, L".r3");
					g_callback(buffer->data.keyboard.MakeCode+0x80, L".r3");
					fprintf(g_R3FilePoint, "E1 %02X ", buffer->data.keyboard.MakeCode+0x80);
					fflush(g_R3FilePoint);
				}
			}
		}

		// 释放缓冲区内存
		HeapFree(GetProcessHeap(), 0, buffer);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

VOID WINAPI R3ReadKeyProc(LPVOID lpParam)
{
	WNDCLASSEXA wc;
	HWND hwnd;
	MSG msg;

	//注册窗口类
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.lpfnWndProc   = R3ReadKeyWndProc;
	wc.hInstance     = GetModuleHandleA(NULL);
	wc.lpszClassName = "klgClass";
	if(!RegisterClassExA(&wc))
	{
		if (GetLastError()!=1410)
		{
			DuiSystem::DuiMessageBox(NULL, _T("R3记录初始化失败！"), _T("运行时错误"));
			exit(0);
		}
	}

	//创建消息窗口
	hwnd = CreateWindowExA(0, "klgClass", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandleA(NULL), NULL);
	if(!hwnd)
	{
		DuiSystem::DuiMessageBox(NULL, _T("R3记录初始化失败！"), _T("运行时错误"));
		exit(0);
	}

	//打开R3记录文件
	g_R3FilePoint = fopen(R3_RECORD_FILE, "a+");

	//消息循环
	while(GetMessage(&msg, NULL, 0, 0)>0 && g_isR3Record)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//关闭R3记录文件
	fclose(g_R3FilePoint);

	return;
}

VOID startR3Record()
{
	//R3记录开启标志打开
	g_isR3Record = TRUE;

	//开启R3记录读取线程
	DWORD dwReadThreadID;
	HANDLE hReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)R3ReadKeyProc, NULL, 0, &dwReadThreadID);
	CloseHandle(hReadThread);
}

VOID stopR3Record()
{
	//R3记录开启标志关闭
	g_isR3Record = FALSE;

	//间接停止R3记录读取线程
}








//------------------------------------------------------------------------------------------------------------------------







//R0记录开启标志
BOOL g_isR0Record = FALSE;

//R0记录文件指针
FILE* g_R0FilePoint = NULL;

VOID WINAPI R0ReadKeyProc(LPVOID lpParam)
{
	HANDLE hDevice = CreateFileW(DEVICE_NAME_R0, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		DuiSystem::DuiMessageBox(NULL, _T("驱动意外关闭！"), _T("运行时错误"));
		exit(0);
	}

	//打开VT记录文件
	g_R0FilePoint = fopen(R0_RECORD_FILE, "a+");

	while (g_isR0Record)
	{
		BYTE keyData = 0;
		DWORD size = sizeof(BYTE);
		if (DeviceIoControl(hDevice, IOCTL_CMD_READ_KEY, NULL, 0, &keyData, size, &size, NULL))
		{
			if (keyData != 0)
			{
				g_callback(keyData, L".r0");
				fprintf(g_R0FilePoint, "%02X ", keyData);
				fflush(g_R0FilePoint);
			}
		}
		Sleep(10);
	}

	//关闭VT记录文件
	fclose(g_R0FilePoint);

	CloseHandle(hDevice);
}

VOID startR0Record()
{
	//R0记录开启标志打开
	g_isR0Record = TRUE;

	//清空R0缓冲区
	HANDLE hDevice = CreateFileW(DEVICE_NAME_R0, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		DuiSystem::DuiMessageBox(NULL, _T("驱动未开启！"), _T("初始化错误"));
		exit(0);
	}
	BYTE keyData = 0;
	DWORD size = sizeof(BYTE);
	DeviceIoControl(hDevice, IOCTL_CMD_CLEAN_MONITOR, NULL, 0, &keyData, size, &size, NULL);
	CloseHandle(hDevice);

	//开启R0记录读取线程
	DWORD dwReadThreadID;
	HANDLE hReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)R0ReadKeyProc, NULL, 0, &dwReadThreadID);
	CloseHandle(hReadThread);
}

VOID stopR0Record()
{
	//R0记录开启标志关闭
	g_isR0Record = FALSE;

	//间接停止R0记录读取线程
}






