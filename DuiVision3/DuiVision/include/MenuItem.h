#pragma once
#include "ControlBase.h"

class CDuiMenu;

class CMenuItem : public CControlBaseFont
{
	DUIOBJ_DECLARE_CLASS_NAME(CMenuItem, _T("menuitem"))
public:
	CMenuItem(HWND hWnd, CDuiObject* pDuiObject);
	CMenuItem(HWND hWnd, CDuiObject* pDuiObject, UINT uControlID, CRect rc, CString strTitle= TEXT(""), int nLeft = 30, BOOL bSelect = false, BOOL bIsVisible = TRUE, BOOL bIsDisable = FALSE, BOOL bIsPressDown = FALSE);
	virtual ~CMenuItem(void);

	// RadioButton����
	CString GetGroupName() { return m_strGroupName; }
	void SetGroupName(CString strGroupName) { m_strGroupName = strGroupName; }
	CString GetValue() { return m_strValue; }
	CString GetGroupValue();

	// ����״̬
	BOOL GetCheck();
	BOOL SetCheck(BOOL bCheck);
	void SetControlCheck(BOOL bCheck);
	BOOL ResetGroupCheck();

	// �Ƿ�ָ���
	BOOL IsSeparator() { return m_bIsSeparator; }
	// �Ƿ񵯳��˵�
	BOOL IsPopup() { return m_bIsPopup; }
	// �Ƿ��ڻ״̬(����ڴ˲˵���)
	BOOL IsHover() { return (m_enButtonState == enBSHover); }

	// ��ȡ���˵�����
	CDuiMenu* GetParentMenu();
	// ���ò˵����Ƕ��XML�ļ���
	void SetMenuXml(CString strMenuXml) { m_strMenuXml = strMenuXml; }
	// ��ȡ�˵����Ƕ��XML�ļ���
	CString GetMenuXml() { return m_strMenuXml; }
	// ��ʾ�����˵�
	void ShowPopupMenu();
	// ��ȡ�����˵�
	CDuiMenu* GetPopupMenu() { return m_pPopupMenu; }
	// ���õ����˵�
	void SetPopupMenu(CDuiMenu* pPopupMenu) { m_pPopupMenu = pPopupMenu; }
	// ���ò˵���ͱ߿�֮��ľ���
	void SetFrameWidth(int nFrameWidth) { m_nFrameWidth = nFrameWidth; }

	HRESULT OnAttributeCheck(const CString& strValue, BOOL bLoading);

	virtual BOOL Load(DuiXmlNode pXmlElem, BOOL bLoadSubControl = TRUE);

protected:
	virtual BOOL OnControlMouseMove(UINT nFlags, CPoint point);
	virtual BOOL OnControlLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnControlLButtonUp(UINT nFlags, CPoint point);

	virtual	void SetControlDisable(BOOL bIsDisable);
	virtual void DrawControl(CDC &dc, CRect rcUpdate);

public:
	enumButtonState		m_enButtonState;
	BOOL				m_bDown;			// ����Ǽ��򣬱�ʾ�Ƿ�ѡ��
	BOOL				m_bMouseDown;
	BOOL				m_bSelect;			// �Ƿ�ѡ��,���������ѡ���
	BOOL				m_bIsSeparator;		// �Ƿ�ָ���
	BOOL				m_bIsPopup;			// �Ƿ񵯳��˵�
	CDuiMenu*			m_pPopupMenu;		// �����˵��Ķ���ָ��
	CString				m_strMenuXml;		// Ƕ�ײ˵���XML�ļ�
	int					m_nLeft;			// �˵������ֵ���߾�
	int					m_nFrameWidth;		// �˵������߿�Ŀ��
	CString				m_strGroupName;		// Radio������
	CString				m_strValue;			// Radioֵ

	Color				m_clrHover;			// ������ɫ(����ƶ����˵���)
	Image*				m_pImageHover;		// ����ͼƬ(����ƶ����˵���)
	CSize				m_sizeHover;		// ����ͼƬ��С
	Image*				m_pImagePopupArrow;	// �����˵���ͷͼƬ
	CSize				m_sizePopupArrow;	// �����˵���ͷͼƬ��С

	DUI_DECLARE_ATTRIBUTES_BEGIN()
		DUI_INT_ATTRIBUTE(_T("select"), m_bSelect, FALSE)
		DUI_INT_ATTRIBUTE(_T("separator"), m_bIsSeparator, FALSE)
		DUI_TSTRING_ATTRIBUTE(_T("menu"), m_strMenuXml, FALSE)
		DUI_CUSTOM_ATTRIBUTE(_T("check"), OnAttributeCheck)
		DUI_TSTRING_ATTRIBUTE(_T("group"), m_strGroupName, FALSE)
		DUI_TSTRING_ATTRIBUTE(_T("value"), m_strValue, FALSE)
		DUI_COLOR_ATTRIBUTE(_T("crhover"), m_clrHover, FALSE)
	DUI_DECLARE_ATTRIBUTES_END()
};