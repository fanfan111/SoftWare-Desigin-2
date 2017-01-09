#pragma once

// DUI事件处理类
class HookGet 
{
public:
    HookGet(void);
    virtual ~HookGet(void);

    virtual void OnInit(CDlgBase* m_pDlg);

    LRESULT OnDuiBtnShowAbout(UINT uID, CString strName, UINT Msg, WPARAM wParam, LPARAM lParam);

    virtual void OnTimer(UINT uTimerID, CString strTimerName);

    UINT m_uTimerAni;   // 动画定时器
    int m_nAniIndex;    // 动画索引
	

    // 消息处理定义
    DUI_DECLARE_MESSAGE_BEGIN(CDuiHandlerControl)
        DUI_CONTROL_NAMEMSG_MESSAGE(L"button.about", MSG_BUTTON_UP, OnDuiBtnShowAbout)
    DUI_DECLARE_MESSAGE_END()
};