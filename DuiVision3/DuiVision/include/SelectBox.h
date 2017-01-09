// ���ؼ�
#pragma once
#include "ControlBase.h"
#include <vector>

using namespace  std;

// ѡ�������Ϣ
struct SelectInfo
{
	int nType;
	Color clr;
	UINT uIndex;
};

class CSelectBox : public CControlBase
{
	DUIOBJ_DECLARE_CLASS_NAME(CSelectBox, _T("selectbox"))
public:
	CSelectBox(HWND hWnd, CDuiObject* pDuiObject);
	CSelectBox(HWND hWnd, CDuiObject* pDuiObject, UINT uControlID, CRect rc, int nXCount, int nYCount, BOOL bImage,  
		Color clrFrame = Color(254, 0, 0, 0), Color clrHover = Color(128, 0, 0, 0), Color clrSelect = Color(254, 255, 255, 255), BOOL bIsVisible = TRUE);
	virtual ~CSelectBox(void);

	BOOL SetBitmap(UINT nResourceID, int nIndex = -1, CString strType= TEXT("PNG"));
	BOOL SetBitmap(CString strImage, int nIndex = -1);
	BOOL SetColor(Color clr, int nIndex = -1);
	BOOL SetColor(Color clr[], int nColorCount);

	int  GetSelectIndex() { return m_nSelectIndex; }
	Color GetSelectColor() { return m_clrSelect; }

	virtual BOOL Load(DuiXmlNode pXmlElem, BOOL bLoadSubControl = TRUE);

protected:
	virtual void DrawControl(CDC &dc, CRect rcUpdate);

	virtual BOOL OnControlMouseMove(UINT nFlags, CPoint point);
	virtual BOOL OnControlLButtonDown(UINT nFlags, CPoint point);

public:
	BOOL			m_bImage;		// �Ƿ�ͼƬ����
	vector<Image*>	m_vecpImage;	// ͼƬ�б�
	vector<CSize>	m_vecsizeImage;	// ͼƬ��С�б�
	vector<Color>	m_vecclr;		// ��ɫ�б�
	int				m_nXCount;		// ����
	int				m_nYCount;		// ����
	Color			m_clrFrame;		// �߿���ɫ
	Color			m_clrHover;		// �ȵ���ɫ
	Color			m_clrSelect;	// ѡ������ɫ

	int				m_nXHover;		// �ȵ���
	int				m_nYHover;		// �ȵ���
	int				m_nXSelect;		// ѡ����
	int				m_nYSelect;		// ѡ����
	int				m_nSelectIndex;	// ��ǰѡ����������
	Color			m_clrCurSelect;	// ��ǰѡ�����ɫ

	DUI_DECLARE_ATTRIBUTES_BEGIN()
		DUI_INT_ATTRIBUTE("image", m_bImage, FALSE)
		DUI_COLOR_ATTRIBUTE("crframe", m_clrFrame, FALSE)
		DUI_COLOR_ATTRIBUTE("crhover", m_clrHover, FALSE)
		DUI_COLOR_ATTRIBUTE("crselect", m_clrSelect, FALSE)
		DUI_INT_ATTRIBUTE("col", m_nXCount, FALSE)
		DUI_INT_ATTRIBUTE("row", m_nYCount, FALSE)
    DUI_DECLARE_ATTRIBUTES_END()
};
