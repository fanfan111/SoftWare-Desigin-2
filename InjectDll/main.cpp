#include <stdio.h>
#include <Windows.h>
#include "main.h"

#pragma comment(linker,"/subsystem:\"Windows\" /entry:\"wmainCRTStartup\"")

typedef long (__fastcall *RTLADJUSTPRIVILEGE64)(ULONG,ULONG,ULONG,PVOID);
RTLADJUSTPRIVILEGE64 RtlAdjustPrivilege;

int wmain(int argc, wchar_t* argv[])
{
	wchar_t* pidStr = argv[1];
	wchar_t* pathStr = argv[2];
	WCHAR dllname[MAX_PATH]={0};
	DWORD dwPID=0,dwRetVal=0;
	RtlAdjustPrivilege=(RTLADJUSTPRIVILEGE64)GetProcAddress(LoadLibraryW(L"ntdll.dll"),"RtlAdjustPrivilege");
	RtlAdjustPrivilege(20,1,0,&dwRetVal);//debug
	swscanf(pidStr, L"%ld", &dwPID);
	InjectDllExW(dwPID,pathStr);
	return 0;
}