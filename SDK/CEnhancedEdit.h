
// CEnhancedEdit.h : 头文件
//

#pragma once
#include "afxwin.h"

//自定义消息
#define WM_EDIT_GET_KEY  WM_USER + 100

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

//互斥量
#define MUTEX_NAME L"lx_test_lock"

//共享内存
#define MAX_CLIENT 1024
#define MAX_SIZE   2049
#define SHM_NAME L"lx_test_memory"


class CEnhancedEdit : public CEdit
{
public:
    CEnhancedEdit(CWnd* pParent=NULL);
    ~CEnhancedEdit();

public:
    DECLARE_MESSAGE_MAP()
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg LRESULT OnEditGetKey(WPARAM wParam, LPARAM lParam);

public:
	VOID is_global_pause();
	VOID fix_data();
	BOOL set_intercept(DWORD Flag);
	BYTE try_get_key();

public:
	static VOID WINAPI ThreadFunc(LPVOID lpParam);

public:
	void (*m_startCallback)(CWnd* pWnd);
	void (*m_stopCallback)(CWnd* pWnd);
	void (*m_errorCallback)(CWnd* pWnd);
	void (*m_normalCallback)(CWnd* pWnd);

public:
	CWnd* m_pWnd;

public:
	BOOL m_isFailedInit;
	BOOL m_isProtect;
	BOOL m_running;
	BOOL m_isGloalPause;

public:
	HANDLE m_hMutex;
	HANDLE m_hMap;
	DWORD* m_intercept;
};
