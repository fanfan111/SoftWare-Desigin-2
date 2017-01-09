// DuiVision3.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "DuiVision3.h"
#include "DuiHandlerMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// SHA定制：OnBaseMessage方法重写
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


// SHA定制：创建对话框
static CDlgBase* CreateDuiDialogSHA(LPCTSTR lpszXmlTemplate, CDuiObject* pParentObject, CString strName = _T(""), BOOL bModule = TRUE, UINT nIDTemplate = 0, BOOL bAdd = TRUE)
{
	// 解析XML模版文件或内容
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

	//SHA定制：根据上下文代码确定的
	nIDTemplate = IDD_DUIVISIONAPP_DIALOG;

	CDlgBase* pDlg = new CDlgBaseSHA(nIDTemplate, pParentDlg);
	pDlg->SetParent(pParentObject);
	pDlg->SetXmlFile(strXmlFile);
	pDlg->SetXmlContent(strXmlContent);

	if(!strName.IsEmpty())
	{
		// 设置name
		pDlg->SetName(strName);
	}

	if(bAdd)
	{
		// 添加到对话框列表中
		DuiSystem::Instance()->AddDuiDialog(pDlg);
	}

	if(!bModule)
	{
		// 如果是非模式对话框,则直接创建窗口
		pDlg->Create(pDlg->GetIDTemplate(), NULL);
	}

	return pDlg;
}

BOOL VTLoad()
{
	//判断驱动是否已经加载
	if (DriverLoader::isDriverLoaded())
	{
		return TRUE;
	}

	//加载驱动

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
	//判断驱动是否已经加载
	if (DriverLoader::isDriverLoadedR0())
	{
		return TRUE;
	}

	//加载驱动

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


// CDuiVisionTutorial1App 构造

CDuiVision3App::CDuiVision3App()
{
    // TODO: 在此处添加构造代码，
    // 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CDuiVisionTutorial1App 对象

CDuiVision3App theApp;


// CDuiVisionTutorial1App 初始化

BOOL CDuiVision3App::InitInstance()
{
    CWinApp::InitInstance();

    AfxEnableControlContainer();

    // TODO: 应适当修改该字符串，
    // 例如修改为公司或组织名
    SetRegistryKey(_T("原型版本"));

    // 初始化DuiVision界面库,可以指定语言,dwLangID为0表示自动判断当前语言
    // 11160是应用程序ID,每个DUI应用程序应该使用不同的ID,ID主要用于进程间通信传递命令行时候区分应用
    DWORD dwLangID = 0;
    new DuiSystem(m_hInstance, dwLangID, _T("DuiVisionTutorial1.ui"), 11160, IDD_DUIVISIONAPP_DIALOG, _T(""));

    // 检查是否已经有进程在运行
    CString strAppMutex = DuiSystem::Instance()->GetConfig(_T("appMutex")); // 从配置文件中获取互斥量名字
    if(!strAppMutex.IsEmpty())
    {
        ::CreateMutex(NULL,TRUE, _T("Global\\") + strAppMutex);
        if(ERROR_ALREADY_EXISTS == GetLastError() || ERROR_ACCESS_DENIED == GetLastError())
        {
            // 读取命令行参数,如果不需要处理命令行可以直接退出
            CString strCmd = L"";
            if(__argc > 0)
            {
                strCmd = __targv[0];
                DuiSystem::LogEvent(LOG_LEVEL_DEBUG, L"Command line:%s", strCmd);
            }

            // 发送进程间消息(lParam为1表示不显示界面,appMutex作为应用名,信息参数传递命令行参数)
            CString strAppName = DuiSystem::Instance()->GetConfig(_T("appMutex"));
            DuiSystem::Instance()->SendInterprocessMessage(0, DuiSystem::Instance()->GetAppID(), 1, strAppName, strCmd);

            return FALSE; // Here we quit this application
        }
    }


	//VT初始化
	if (!VTLoad())
	{
		DuiSystem::DuiMessageBox(NULL, _T("VT初始化失败"), _T("初始化错误"));
		return FALSE;
	}

	//R0初始化
	if (!R0Load())
	{
		DuiSystem::DuiMessageBox(NULL, _T("R0初始化失败"), _T("初始化错误"));
		return FALSE;
	}

    // 创建主窗口: SHA定制过的，new的是派生类
    CDlgBase* pMainDlg = CreateDuiDialogSHA(_T("dlg_main"), NULL, _T(""), TRUE);
    // 给主窗口注册事件处理对象

    CDuiHandlerMain* pHandler = new CDuiHandlerMain();
    pHandler->SetDialog(pMainDlg);
	// 给指定的DUI对象注册事件处理对象    并且初始化
    DuiSystem::RegisterHandler(pMainDlg, pHandler);

    // 初始化提示信息窗口
    DuiSystem::Instance()->CreateNotifyMsgBox(_T("dlg_notifymsg"));

    // 按照非模式对话框创建主窗口,可以默认隐藏
    pMainDlg->Create(pMainDlg->GetIDTemplate(), NULL);




	//初始化VT保护
	initProtect();

	//判断VT保护运行状态
	if (judgeProtect())
	{
		startProtect();
		((CDuiTabCtrl*)(pMainDlg->GetControl(_T("button.systemStart"))))->SetControlTitle(L"VT保护已开启");
	}



    //pMainDlg->ShowWindow(SW_HIDE);
    INT_PTR nResponse = pMainDlg->RunModalLoop();

    // 如果是按照模式对话框运行主窗口,只要改为如下代码就可以
    //INT_PTR nResponse = pMainDlg->DoModal();

    // 释放DuiVision界面库的资源
    DuiSystem::Release();

    // 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
    //  而不是启动应用程序的消息泵。
    return FALSE;
}