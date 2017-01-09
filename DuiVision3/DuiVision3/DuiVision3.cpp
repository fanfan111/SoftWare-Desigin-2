// DuiVision3.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "DuiVision3.h"
#include "DuiHandlerMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// SHA���ƣ�OnBaseMessage������д
LRESULT CDlgBaseSHA::OnBaseMessage(UINT uID, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CControlBase* pControlBase = GetBaseControl(uID);
	CControlBase* pControl = GetControl(uID);
	if((pControlBase == NULL) && (pControl == NULL))
	{
		return 0L;
	}

	if(pControlBase != NULL && pControlBase->IsThisObject(BT_MIN, NAME_BT_MIN))
	{
		ShowWindow(SW_HIDE);
		return 0L;
	}

	return CDlgBase::OnBaseMessage(uID, uMsg, wParam, lParam);
}


// SHA���ƣ������Ի���
static CDlgBase* CreateDuiDialogSHA(LPCTSTR lpszXmlTemplate, CDuiObject* pParentObject, CString strName = _T(""), BOOL bModule = TRUE, UINT nIDTemplate = 0, BOOL bAdd = TRUE)
{
	// ����XMLģ���ļ�������
	CString strXmlContent = lpszXmlTemplate;
	CString strXmlFile = _T("");
	if(strXmlContent.Find(_T("<?xml")) != 0)
	{
		strXmlContent = _T("");
		strXmlFile = lpszXmlTemplate;
		if(strXmlFile.Find(_T(".xml")) == -1)
		{
			strXmlFile = DuiSystem::Instance()->GetXmlFile(strXmlFile);
		}else
		if(strXmlFile.Find(_T(":")) == -1)
		{
			strXmlFile = _T("xml:") + strXmlFile;
		}
		if(strXmlFile.IsEmpty())
		{
			return NULL;
		}
	}

	CDlgBase* pParentDlg = NULL;
	if(pParentObject && pParentObject->IsClass(_T("dlg")))
	{
		pParentDlg = ((CDlgBase*)pParentObject);
		if(nIDTemplate == 0)
		{
			nIDTemplate = pParentDlg->GetIDTemplate();
		}
	}

	//SHA���ƣ����������Ĵ���ȷ����
	nIDTemplate = IDD_DUIVISIONAPP_DIALOG;

	CDlgBase* pDlg = new CDlgBaseSHA(nIDTemplate, pParentDlg);
	pDlg->SetParent(pParentObject);
	pDlg->SetXmlFile(strXmlFile);
	pDlg->SetXmlContent(strXmlContent);

	if(!strName.IsEmpty())
	{
		// ����name
		pDlg->SetName(strName);
	}

	if(bAdd)
	{
		// ��ӵ��Ի����б���
		DuiSystem::Instance()->AddDuiDialog(pDlg);
	}

	if(!bModule)
	{
		// ����Ƿ�ģʽ�Ի���,��ֱ�Ӵ�������
		pDlg->Create(pDlg->GetIDTemplate(), NULL);
	}

	return pDlg;
}

BOOL VTLoad()
{
	//�ж������Ƿ��Ѿ�����
	if (DriverLoader::isDriverLoaded())
	{
		return TRUE;
	}

	//��������

	char szSysFile[MAX_PATH] = {0};
	char szSvcLnkName[] = "KeyboardVT";

	GetModuleFileNameA(0, szSysFile, MAX_PATH);
	for (SIZE_T i = strlen(szSysFile) - 1; i >= 0; i--)
	{
		if (szSysFile[i] == '\\')
		{
			szSysFile[i+1] = '\0';
			break;
		}
	}
	strcat_s(szSysFile, "KeyboardVT.sys");

	DriverLoader dl;

	if (!dl.Install(szSysFile, szSvcLnkName, szSvcLnkName))
	{
		dl.Remove();
		return FALSE;
	}

	if (!dl.Start())
	{
		dl.Remove();
		return FALSE;
	}

	if (!DriverLoader::isDriverLoaded())
	{
		dl.Remove();
		return FALSE;
	}

	dl.Remove();

	return TRUE;
}

BOOL R0Load()
{
	//�ж������Ƿ��Ѿ�����
	if (DriverLoader::isDriverLoadedR0())
	{
		return TRUE;
	}

	//��������

	char szSysFile[MAX_PATH] = {0};
	char szSvcLnkName[] = "KeyboardFilter";

	GetModuleFileNameA(0, szSysFile, MAX_PATH);
	for (SIZE_T i = strlen(szSysFile) - 1; i >= 0; i--)
	{
		if (szSysFile[i] == '\\')
		{
			szSysFile[i+1] = '\0';
			break;
		}
	}
	strcat_s(szSysFile, "KeyboardFilter.sys");

	DriverLoader dl;

	if (!dl.Install(szSysFile, szSvcLnkName, szSvcLnkName))
	{
		dl.Remove();
		return FALSE;
	}

	if (!dl.Start())
	{
		dl.Remove();
		return FALSE;
	}

	if (!DriverLoader::isDriverLoaded())
	{
		dl.Remove();
		return FALSE;
	}

	dl.Remove();

	return TRUE;
}


// CDuiVision3App

BEGIN_MESSAGE_MAP(CDuiVision3App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDuiVisionTutorial1App ����

CDuiVision3App::CDuiVision3App()
{
    // TODO: �ڴ˴���ӹ�����룬
    // ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CDuiVisionTutorial1App ����

CDuiVision3App theApp;


// CDuiVisionTutorial1App ��ʼ��

BOOL CDuiVision3App::InitInstance()
{
    CWinApp::InitInstance();

    AfxEnableControlContainer();

    // TODO: Ӧ�ʵ��޸ĸ��ַ�����
    // �����޸�Ϊ��˾����֯��
    SetRegistryKey(_T("ԭ�Ͱ汾"));

    // ��ʼ��DuiVision�����,����ָ������,dwLangIDΪ0��ʾ�Զ��жϵ�ǰ����
    // 11160��Ӧ�ó���ID,ÿ��DUIӦ�ó���Ӧ��ʹ�ò�ͬ��ID,ID��Ҫ���ڽ��̼�ͨ�Ŵ���������ʱ������Ӧ��
    DWORD dwLangID = 0;
    new DuiSystem(m_hInstance, dwLangID, _T("DuiVisionTutorial1.ui"), 11160, IDD_DUIVISIONAPP_DIALOG, _T(""));

    // ����Ƿ��Ѿ��н���������
    CString strAppMutex = DuiSystem::Instance()->GetConfig(_T("appMutex")); // �������ļ��л�ȡ����������
    if(!strAppMutex.IsEmpty())
    {
        ::CreateMutex(NULL,TRUE, _T("Global\\") + strAppMutex);
        if(ERROR_ALREADY_EXISTS == GetLastError() || ERROR_ACCESS_DENIED == GetLastError())
        {
            // ��ȡ�����в���,�������Ҫ���������п���ֱ���˳�
            CString strCmd = L"";
            if(__argc > 0)
            {
                strCmd = __targv[0];
                DuiSystem::LogEvent(LOG_LEVEL_DEBUG, L"Command line:%s", strCmd);
            }

            // ���ͽ��̼���Ϣ(lParamΪ1��ʾ����ʾ����,appMutex��ΪӦ����,��Ϣ�������������в���)
            CString strAppName = DuiSystem::Instance()->GetConfig(_T("appMutex"));
            DuiSystem::Instance()->SendInterprocessMessage(0, DuiSystem::Instance()->GetAppID(), 1, strAppName, strCmd);

            return FALSE; // Here we quit this application
        }
    }


	//VT��ʼ��
	if (!VTLoad())
	{
		DuiSystem::DuiMessageBox(NULL, _T("VT��ʼ��ʧ��"), _T("��ʼ������"));
		return FALSE;
	}

	//R0��ʼ��
	if (!R0Load())
	{
		DuiSystem::DuiMessageBox(NULL, _T("R0��ʼ��ʧ��"), _T("��ʼ������"));
		return FALSE;
	}

    // ����������: SHA���ƹ��ģ�new����������
    CDlgBase* pMainDlg = CreateDuiDialogSHA(_T("dlg_main"), NULL, _T(""), TRUE);
    // ��������ע���¼��������

    CDuiHandlerMain* pHandler = new CDuiHandlerMain();
    pHandler->SetDialog(pMainDlg);
	// ��ָ����DUI����ע���¼��������    ���ҳ�ʼ��
    DuiSystem::RegisterHandler(pMainDlg, pHandler);

    // ��ʼ����ʾ��Ϣ����
    DuiSystem::Instance()->CreateNotifyMsgBox(_T("dlg_notifymsg"));

    // ���շ�ģʽ�Ի��򴴽�������,����Ĭ������
    pMainDlg->Create(pMainDlg->GetIDTemplate(), NULL);




	//��ʼ��VT����
	initProtect();

	//�ж�VT��������״̬
	if (judgeProtect())
	{
		startProtect();
		((CDuiTabCtrl*)(pMainDlg->GetControl(_T("button.systemStart"))))->SetControlTitle(L"VT�����ѿ���");
	}



    //pMainDlg->ShowWindow(SW_HIDE);
    INT_PTR nResponse = pMainDlg->RunModalLoop();

    // ����ǰ���ģʽ�Ի�������������,ֻҪ��Ϊ���´���Ϳ���
    //INT_PTR nResponse = pMainDlg->DoModal();

    // �ͷ�DuiVision��������Դ
    DuiSystem::Release();

    // ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
    //  ����������Ӧ�ó������Ϣ�á�
    return FALSE;
}