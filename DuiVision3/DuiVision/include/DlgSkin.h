#pragma once

#include "DlgPopup.h"

#define	MSG_SKIN					1001
#define	MSG_SELECT_SKIN				1002

#define	MSG_GET_SKIN_TYPE			1
#define	MSG_GET_SKIN_VALUE			2
#define	MSG_SET_SKIN_VALUE			3

// ���ڱ���ģʽ
enum
{
	BKTYPE_IMAGE_RESOURCE	= 0,	// ͼƬ��Դ
	BKTYPE_COLOR,					// ָ����ɫ
	BKTYPE_IMAGE_FILE,				// ͼƬ�ļ�
};

class CDlgSkin : public CDlgPopup
{
	BOOL m_bImage;
public:
	CDlgSkin(void);
	virtual ~CDlgSkin(void);

	virtual void DrawWindowEx(CDC &dc, CRect rcClient);
	virtual void InitUI(CRect rcClient);

	// ��Ϣ��Ӧ
	virtual LRESULT OnMessage(UINT	uID, UINT Msg, WPARAM wParam, LPARAM lParam);
};
