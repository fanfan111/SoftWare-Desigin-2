#pragma once

// DUI�¼�������
class HookGet 
{
public:
    HookGet(void);
    virtual ~HookGet(void);

    virtual void OnInit(CDlgBase* m_pDlg);

    LRESULT OnDuiBtnShowAbout(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);

    virtual void OnTimer(UINT uTimerID, CString strTimerName);

    UINT m_uTimerAni;   // ������ʱ��
    int m_nAniIndex;    // ��������
	

    // ��Ϣ������
    DUI_DECLARE_MESSAGE_BEGIN(CDuiHandlerControl)
        DUI_CONTROL_NAMEMSG_MESSAGE(L"button.about", MSG_BUTTON_UP, OnDuiBtnShowAbout)
    DUI_DECLARE_MESSAGE_END()
};