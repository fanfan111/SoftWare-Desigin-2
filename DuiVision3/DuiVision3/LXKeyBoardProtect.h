#pragma once

#include <tlhelp32.h>


//VT�ں˱�������
#define MUTEX_NAME _T("lx_test_lock")
#define SHM_NAME   _T("lx_test_memory")

//VT��������·��
#define INJECT_X86_EXE_PATH   _T("C:\\KeyboardSecurityTool\\InjectDll32.exe")
#define INJECT_X86_DLL_PATH   _T("C:\\KeyboardSecurityTool\\HookDll32.dll")
#define INJECT_X64_EXE_PATH   _T("C:\\KeyboardSecurityTool\\InjectDll64.exe")
#define INJECT_X64_DLL_PATH   _T("C:\\KeyboardSecurityTool\\HookDll64.dll")

//VT���������ڴ�
#define MAX_CLIENT 1024
#define MAX_SIZE   2049

map<DWORD, DWORD> all_thread();

bool judgeProtect();
void initProtect();
void startProtect();
void stopProtect();

