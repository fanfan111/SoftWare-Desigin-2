#include "stdafx.h"
#include "DriverLoader.h"

DriverLoader::DriverLoader()
{
	m_pSysPath = NULL;
	m_pServiceName = NULL;
	m_pDisplayName = NULL;
	m_hSCManager = NULL;
	m_hService = NULL;
}
DriverLoader::~DriverLoader()
{
	if (m_hService) CloseServiceHandle(m_hService);
	if (m_hSCManager) CloseServiceHandle(m_hSCManager);
}

BOOL DriverLoader::GetSvcHandle(PCHAR pServiceName)
{
	m_pServiceName = pServiceName;
	m_hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == m_hSCManager)
	{
		m_dwLastError = GetLastError();
		return FALSE;
	}
	m_hService = OpenServiceA(m_hSCManager, m_pServiceName, SERVICE_ALL_ACCESS);
	if (NULL == m_hService)
	{
		CloseServiceHandle(m_hSCManager);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL DriverLoader::Install(PCHAR pSysPath, PCHAR pServiceName, PCHAR pDisplayName)
{
	m_pSysPath = pSysPath;
	m_pServiceName = pServiceName;
	m_pDisplayName = pDisplayName;
	m_hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == m_hSCManager)
	{
		m_dwLastError = GetLastError();
		return FALSE;
	}
	m_hService = CreateServiceA(m_hSCManager, m_pServiceName, m_pDisplayName,
		SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		m_pSysPath, NULL, NULL, NULL, NULL, NULL);
	if (NULL == m_hService)
	{
		m_dwLastError = GetLastError();
		if (ERROR_SERVICE_EXISTS == m_dwLastError)
		{
			m_hService = OpenServiceA(m_hSCManager, m_pServiceName, SERVICE_ALL_ACCESS);
			if (NULL == m_hService)
			{
				CloseServiceHandle(m_hSCManager);
				return FALSE;
			}
		}
		else
		{
			CloseServiceHandle(m_hSCManager);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL DriverLoader::Start()
{
	if (!StartServiceA(m_hService, NULL, NULL))
	{
		m_dwLastError = GetLastError();
		return FALSE;
	}
	return TRUE;
}

BOOL DriverLoader::Stop(PCHAR pServiceName)
{
	SERVICE_STATUS ss;
	GetSvcHandle(pServiceName);
	if (!ControlService(m_hService,SERVICE_CONTROL_STOP,&ss))
	{
		m_dwLastError = GetLastError();
		return FALSE;
	}
	return TRUE;
}

BOOL DriverLoader::Remove()
{
	GetSvcHandle(m_pServiceName);
	if (!DeleteService(m_hService))
	{
		m_dwLastError = GetLastError();
		return FALSE;
	}

	return TRUE;
}

BOOL DriverLoader::isDriverLoaded()
{
	HANDLE hDevice = CreateFileW(DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		CloseHandle(hDevice);
		return true;
	}
}

BOOL DriverLoader::isDriverLoadedR0()
{
	HANDLE hDevice = CreateFileW(DEVICE_NAME_R0, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		CloseHandle(hDevice);
		return true;
	}
}