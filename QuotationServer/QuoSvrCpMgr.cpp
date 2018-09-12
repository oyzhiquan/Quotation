/******************************************************************************

��    Ȩ:��������������ϵͳ���޹�˾.
ģ������:QuoSvrCpMgr.cpp
������	:ղ���
��������:2012.04.18
��    ��:1.0				
ģ������:�������ص���ģ��
��Ҫ����:Init(...)��ʼ������
         Finish() ��������
         Run()�����̺߳���
�޸ļ�¼:
******************************************************************************/

#include <iostream>
#include "Logger.h"
#include "ConfigImpl.h"
#include "QuoSvrCpMgr.h"
#include "HisDataHandler.h"
#include "ServiceHandler.h"
#include "DeliverMgr.h"
#include "IfSvAgent.h"
#include "ProtocolConnectPoint.h"
#include "ProcessInterfaceZC.h"
#include "ProcessInterfaceZS.h"
#include "ProcessInterfaceH1C.h"
#include "ProcessInterfaceH2C.h"
#include "ProcessInterfaceKC.h"
#include "ProcessInterfaceCmd.h"
#include "LinePacket.h"
#include "GessTimerMgrPosix.h"
#include "AbsTimerMgrWin32.h"
#include "NetMgrModule.h"
#include <sstream>
#include <iomanip>


//Դ�ӿ�+������ ����·�����ñ�
CQuoSvrCpMgr::IfRouterCfg CQuoSvrCpMgr::m_tblIfRouterCfg[] = 
{
	//from EnumKeyIfZC1
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyDeliverMgr,		EnumKeyIfZC1,     "00000006"},
	{0,		   EnumKeyHisDataHandler,	EnumKeyIfZC1,     "00000006"},

	{0,    EnumKeyServiceHandler1,		EnumKeyIfZC1,     "80000003"},

	//���ĳɹ���Ҫ����
	{0,    	EnumKeySvAgent,				EnumKeyIfZC1,     "80000003"},

	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler1,		EnumKeyIfZC1,     gc_sDefaultCmdID},

	//��Ϊ���ж�,�ϼ��ڵ�ļ�ز�ѯ���� ֻ���ܴ��ϼ�Quotation Server�յ���SV_REQ����
	{0,		   EnumKeySvAgent,			EnumKeyIfZC1,     "00000011"},
	//��Ϊ���ж�,�ϼ��ڵ�ļ�ز�ѯӦ�� Ӧ������ת��
	{0,		   EnumKeyIfZC1,			EnumKeySvAgent,   "80000011"},
	//��Ϊ���ж�,��ظ澯�¼� �¼�����ת��
	{0,		   EnumKeyIfZC1,			EnumKeySvAgent,   "00000012"},
	//��Ϊ���ж�,�����¼�����ת��
	{0,		   EnumKeyIfZC1,			EnumKeySvAgent,   "00000013"},
	//��Ϊ���ж�,�˶��¼�����ת��
	{0,		   EnumKeyIfZC1,			EnumKeySvAgent,   "00000014"},
	//��Ϊ���ж�,��ز�ѯ���¼��ڵ�ת�� ��������ת��
	{0,		   EnumKeyIfZS1,				EnumKeySvAgent,   "00000011"},
	{0,		   EnumKeyIfZS1,				EnumKeySvAgent,   "00000011"},

	//�¼��ڵ����ļ�����ѯӦ��SV_RSP
	{0,		   EnumKeySvAgent,			EnumKeyIfZS1,		"80000011"},
	{0,		   EnumKeySvAgent,			EnumKeyIfZS2,		"80000011"},
	//�¼��ڵ����ļ���¼��澯SV_NTF
	{0,		   EnumKeySvAgent,			EnumKeyIfZS1,		"00000012"},
	{0,		   EnumKeySvAgent,			EnumKeyIfZS2,		"00000012"},
	//�¼��ڵ����Ķ���֪ͨ
	{0,		   EnumKeySvAgent,			EnumKeyIfZS1,		"00000013"},
	{0,		   EnumKeySvAgent,			EnumKeyIfZS2,		"00000013"},


	//from EnumKeyServiceHandler1
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC1,				EnumKeyServiceHandler1,     gc_sDefaultCmdID},

	//from EnumKeyIfZC2
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyDeliverMgr,		EnumKeyIfZC2,     "00000006"},
	{0,		   EnumKeyHisDataHandler,	EnumKeyIfZC2,     "00000006"},
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler2,		EnumKeyIfZC2,     gc_sDefaultCmdID},

	//from EnumKeyServiceHandler2
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC2,				EnumKeyServiceHandler2,     gc_sDefaultCmdID},


	//from EnumKeyDeliverMgr
	//EnumKeyDeliverMgr To EnumKeyIfZS1 ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZS1,				EnumKeyDeliverMgr,     gc_sDefaultCmdID},
	{0,    EnumKeyIfZS2,				EnumKeyDeliverMgr,     gc_sDefaultCmdID},
	//from EnumKeyIfZS
	//EnumKeyIfZS To EnumKeyDeliverMgr 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////	
	{0,    EnumKeyDeliverMgr,			EnumKeyIfZS1,     "00000003"},
	{0,    EnumKeyDeliverMgr,			EnumKeyIfZS1,     "00000004"},
	{0,    EnumKeyDeliverMgr,			EnumKeyIfZS2,     "00000003"},
	{0,    EnumKeyDeliverMgr,			EnumKeyIfZS2,     "00000004"},


	//����/�˶�
	{0,    EnumKeySvAgent,				EnumKeyIfZS1,     "00000003"},
	{0,    EnumKeySvAgent,				EnumKeyIfZS1,     "00000004"},
	{0,    EnumKeySvAgent,				EnumKeyIfZS2,     "00000003"},
	{0,    EnumKeySvAgent,				EnumKeyIfZS2,     "00000004"},

	//EnumKeyIfZC1 To EnumKeyTranslator ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	//{0,		EnumKeyIfZC1,			EnumKeyIfZS,     "00000005"},
	{0,		EnumKeyServiceHandlerSvr1,	EnumKeyIfZS1,     gc_sDefaultCmdID},
	{0,		EnumKeyServiceHandlerSvr2,	EnumKeyIfZS2,     gc_sDefaultCmdID},

	//from EnumKeyIfZS
	//EnumKeyIfZC1 To EnumKeyTranslator ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,		   EnumKeyIfZS1,		EnumKeyServiceHandlerSvr1,     gc_sDefaultCmdID},
	{0,		   EnumKeyIfZS2,		EnumKeyServiceHandlerSvr2,     gc_sDefaultCmdID},
	//{0,		   EnumKeyIfZC1,	EnumKeyServiceHandlerSvr,     "00000005"}, ���� 20120418

	//from H1	
	//H1 To NetMgrModule
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From                    CmdID		///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeySvAgent,			EnumKeyIfH1,            "1921"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,			"1922"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,			"1923"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,			"1924"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,		    "1925"}, 


	//from NetMgrModule	
	//NetMgrModule To H1 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From                    CmdID		//
	{0,    EnumKeySvAgent,				EnumNetMagModule,       "1921"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1922"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1923"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1924"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1925"}, 


	//NetMgrModule To H2
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From					CmdID		///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"onEventNotify"},   //�¼��㲥�౨��
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"onAlarmNotify"},   //�澯�㲥�౨��
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"onNodeMibTblChg"}, //��¼�仯�㲥����

	//from IFCMD
	//IFCMD To default ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyCmdHandler,		EnumKeyIfCmd,    			 gc_sDefaultCmdID}
};


//Telnet or Console CommandLine ��Ӧ���������ñ�
CQuoSvrCpMgr::CmdLine2Api CQuoSvrCpMgr::m_CmdLine2Api[] = 
{
	//������			��д			�������ָ��					˵��
	{"quit",			"q",			&CQuoSvrCpMgr::OnCmdLineQuit,			"quit the system"},
	{"buf",				"b",			&CQuoSvrCpMgr::OnCmdLineBuffer,		"list the quotation buffer"},
	{"mem",				"m",			&CQuoSvrCpMgr::OnCmdLineMem,			"show mem bytes"},
	{"evt",				"e",			&CQuoSvrCpMgr::OnCmdLineEvtTest,		"test evt notify"},
	{"info",			"i",			&CQuoSvrCpMgr::OnCmdLineSysInfo,		"show SysInfo"},
	{"que",			   "que",			&CQuoSvrCpMgr::OnCmdLineQue,			"for que"},
	{"?",				"",				&CQuoSvrCpMgr::OnCmdLineHelp,			"for help"},
	{"help",			"h",			&CQuoSvrCpMgr::OnCmdLineHelp,			"for help"},	
	{"settime",        "st",            &CQuoSvrCpMgr::OnCmdLineSetTime,      "set time"}
};


CQuoSvrCpMgr::CQuoSvrCpMgr()
:m_sProcName("QuotationServer")
,m_pCpInterfaceCmd(0)
,m_pCpInterfaceH1(0)
,m_pCpInterfaceH2(0)
,m_pCpInterfaceZS1(0)
,m_pCpInterfaceZS2(0)
,m_pCpInterfaceZC1(0)
,m_pCpInterfaceZC2(0)
,m_pHisDataHandler(0)
,m_pServiceHandler1(0)
,m_pServiceHandler2(0)
,m_pServiceHandlerSvr1(0)
,m_pServiceHandlerSvr2(0)
,m_pDeliverMgr(0)
,m_pNetMagModule(0)
,m_pSvAgent(0)
,m_pGessTimerMgr(0)
,m_uiNodeID(0)
,m_uiNodeType(0)
,m_bStop(false)
{
	m_pConfig = new CConfigImpl();

	for (int i = 0; i < EnumKeyUnknown; i++)
	{
		m_nConNumIf[i] = 0;
	}
}

CQuoSvrCpMgr::~CQuoSvrCpMgr(void)
{
	m_deqTelnets.clear();
	m_vResetTime.clear();
}

//�ͻ���Э�����ӵ����ӳɹ���ص�
int CQuoSvrCpMgr::OnConnect(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;

	m_csConNum.Lock();
	if (0 == nFlag)
	{
		m_nConNumIf[ulKey]++;
	}
	m_csConNum.Unlock();
	return 0;
}

//�����Э�����ӵ���յ����Ӻ�ص�
int CQuoSvrCpMgr::OnAccept(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;
	
	return 0;
}

int CQuoSvrCpMgr::OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	return 0;
}

int CQuoSvrCpMgr::OnClose(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;


	bool blNeedLogout = false;
	m_csConNum.Lock();
	if (m_nConNumIf[ulKey] > 0)
		m_nConNumIf[ulKey]--;

	m_csConNum.Unlock();
	return 0;
}
//��ʼ��·�ɱ�
int CQuoSvrCpMgr::InitRouterTbl()
{
	//���ñ�
	int nSize = sizeof(m_tblIfRouterCfg)/sizeof(IfRouterCfg);
	//����·�����ñ��ʼ���ڴ�·�ɱ�
	for ( int m = 0; m < nSize; m++ )
	{
		unsigned long ulRow = m_tblIfRouterCfg[m].ulIfFrom;
		m_tblIfRouter[ulRow].ulIfFrom = m_tblIfRouterCfg[m].ulIfFrom;
		string sCmdID = m_tblIfRouterCfg[m].sCmdID;

		switch(m_tblIfRouterCfg[m].ulIfTo)
		{
		case EnumKeyDeliverMgr:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pDeliverMgr));
			break;
		case EnumKeyHisDataHandler:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pHisDataHandler));
			break;
		case EnumKeyIfZS1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZS1));
			break;
		case EnumKeyIfZS2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZS2));
			break;
		case EnumKeyServiceHandlerSvr1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandlerSvr1));
			break;
		case EnumKeyServiceHandlerSvr2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandlerSvr2));
			break;
		case EnumKeyServiceHandler1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler1));
			break;
		case EnumKeyServiceHandler2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler2));
			break;
		case EnumKeyIfZC1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC1));
			break;
		case EnumKeyIfZC2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC2));
			break;
		case EnumKeyIfH1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceH1));
			break;
		case EnumKeyIfH2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceH2));
			break;
		case EnumKeyIfCmd:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceCmd));
			break;
		case EnumKeyCmdHandler:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, &m_oCpCmdHandler));
			break;
		case EnumNetMagModule:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pNetMagModule));
			break;
		case EnumKeySvAgent:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pSvAgent));
			break;
		default:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, 0));
			break;
		}
	}

	return 0;
}

//Convergence���ӵ��������ʼ��
int CQuoSvrCpMgr::Init()
{

	cout << "���������ļ�..." << endl;

	std::string sCfgFilename;
	sCfgFilename = DEFUALT_CONF_PATH PATH_SLASH;
	sCfgFilename = sCfgFilename + m_sProcName;
	sCfgFilename = sCfgFilename + ".cfg";
	if (m_pConfig->Load(sCfgFilename) != 0)
	{
		cout << "���������ļ�[" << sCfgFilename << "]ʧ��!" << endl;
		msleep(3);
		return -1;
	}

	cout << "��ʼ����־..." << endl;

	// ��ʼ����־
	if (CLogger::Instance()->Initial(m_pConfig->GetProperties("logger")) != 0)
	{
		cout << "Init Log [" << m_sProcName << "] failure!" << endl;
		msleep(3);
		return -1;
	}

	cout << "������־..." << endl;

	// ������־
	if (CLogger::Instance()->Start() != 0)
	{
		cout << "Log start failure!" << endl;
		msleep(3);
		return -1;
	}

	string sTmp = "";
	if (0 == m_pConfig->GetProperty("node_id",sTmp))
		m_uiNodeID = FromString<unsigned int>(sTmp);

	if (0 == m_pConfig->GetProperty("node_type",sTmp))
		m_uiNodeType = FromString<unsigned int>(sTmp);


	CRLog(E_NOTICE,"[%s]�������...",m_sProcName.c_str());

	//
	char szFileName[_MAX_PATH];
	::GetModuleFileName(0,szFileName, _MAX_PATH);
	sTmp = szFileName;
	sTmp = strutils::LeftSubRight(sTmp, '.');
	m_oMemShareAlive.Bind(E_PROCESS_APP);
	if (FALSE == m_oMemShareAlive.Create(sTmp.c_str()))
	{
		CRLog(E_ERROR, "m_oMemShareAlive.Create fail");
		return -1;
	}
	unsigned int uiProcessID = ::GetCurrentProcessId();
	m_oMemShareAlive.IamAlive(uiProcessID);
	m_oMemShareAlive.SetNodeID(m_uiNodeID);



	//
	CAbsTimerMgrWin32::Instance()->Init();

	//������ʱ��������
	m_pGessTimerMgr = CGessTimerMgrImp::Instance();
	m_pGessTimerMgr->Init(2);
	
	int nInterval = 4;
	string sInterval("4");
	if (0 == m_pConfig->GetProperty("hello_interval",sInterval))
	{
		nInterval = FromString<int>(sInterval);
	}
	if (nInterval > 10)
		nInterval = 10;
	if (nInterval < 2)
		nInterval = 2;

	m_oIfkTimer.Bind(this);
	m_pGessTimerMgr->CreateTimer(&m_oIfkTimer,nInterval,"KHello");

	// added by zyb ͬ��ʱ�䶨ʱ�� Ĭ��ÿ��2������һ��ʱ��ͬ��
	int nSyncInterval = 2;
	string sSyncInterval("2");
	if (0 == m_pConfig->GetProperty("sync_interval",sSyncInterval))
	{
		nSyncInterval = FromString<int>(sSyncInterval);
	}

	m_bAcceptSyncTime = true;
	string sAcceptSyncTime("1");
	if (0 == m_pConfig->GetProperty("accept_sync_time",sAcceptSyncTime))
	{
		if (sAcceptSyncTime == "0")
			m_bAcceptSyncTime = false;
	}
	// end add


	//��ʱ��������
	string sResetTimes = "";
	if (0 == m_pConfig->GetProperty("reset_time",sResetTimes))
	{
		bool blPara = false;
		vector<string> vWeekDayTm = explodeQuoted(",",sResetTimes);
		if (vWeekDayTm.size() == 2)
		{
			int nWeekDay = strutils::FromString<int>(vWeekDayTm[0]);
			if (nWeekDay >= 0 && nWeekDay <= 6)
			{
				CGessTime oTm;
				if (oTm.FromString(trim(vWeekDayTm[1])))
				{
					m_oResetTimer.Bind(this);
					CAbsTimerMgrWin32::Instance()->CreateWeekTimer(&m_oResetTimer,nWeekDay, oTm, "reset_timer_key");
					blPara = true;
				}
			}
		}
		else if (vWeekDayTm.size() == 1)
		{
			CGessTime oTm;
			if (oTm.FromString(trim(vWeekDayTm[0])))
			{
				m_oResetTimer.Bind(this);
				CAbsTimerMgrWin32::Instance()->CreateDayTimer(&m_oResetTimer, oTm, "reset_timer_key");
				blPara = true;
			}
		}
		
		if (!blPara)
		{
			 CRLog(E_APPINFO,"%s", "�Զ�����ʱ������ó���");
		}
	}

	CRLog(E_NOTICE,"��ʼ�����ܴ���");
	string sTblPrefix = "Quotation Server";
	CConfig *pCfgNetMagModule = m_pConfig->GetProperties(gc_sCfgNetMagModule);
	if (0 != pCfgNetMagModule)
	{
		if (0 == pCfgNetMagModule->GetProperty("tbl_prefix",sTmp))
			sTblPrefix = sTmp;
	}
	m_pNetMagModule = new CNetMgrModule();
	CNetMgr::Instance()->NmInit(m_pNetMagModule,sTblPrefix);
	

	//��ʼ�����ܴ���ģ��
	m_pNetMagModule->Bind(this,EnumNetMagModule);
	m_pNetMagModule->Init(pCfgNetMagModule);

	CConfig *pCfgHisData;
	pCfgHisData = m_pConfig->GetProperties(gc_sCfgHisData);
	if (0 != pCfgHisData && !pCfgHisData->IsEmpty())
	{
	}
	else
	{
		pCfgHisData = m_pConfig;
	}

	// ��д��ʷ���ݴ���������Ϊҵ������
	CRLog(E_NOTICE,"��ʼ�����ӵ�HisDataHandler");
	m_pHisDataHandler = new CHisDataHandler();
	m_pHisDataHandler->Bind(this,EnumKeyHisDataHandler);
	m_pHisDataHandler->Init(pCfgHisData);

	CConfig *pCfgSvrHdl_Ask;
	pCfgSvrHdl_Ask = m_pConfig->GetProperties(gc_sCfgService_Ask);
	if (0 != pCfgSvrHdl_Ask && !pCfgSvrHdl_Ask->IsEmpty())
	{
	}
	else
	{
		pCfgSvrHdl_Ask = m_pConfig;
	}
	
	CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandlerSvr1");
	m_pServiceHandlerSvr1 = new CServiceHandler();
	m_pServiceHandlerSvr1->Bind(this,EnumKeyServiceHandlerSvr1);
	m_pServiceHandlerSvr1->Init(pCfgSvrHdl_Ask);
	

	CConfig *pCfgSvrHdl_Push;
	pCfgSvrHdl_Push = m_pConfig->GetProperties(gc_sCfgService_Push);
	if (0 != pCfgSvrHdl_Push && !pCfgSvrHdl_Push->IsEmpty())
	{
	}
	else
	{
		pCfgSvrHdl_Push = m_pConfig;
	}

	CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandlerSvr1");
	m_pServiceHandlerSvr2 = new CServiceHandler();
	m_pServiceHandlerSvr2->Bind(this,EnumKeyServiceHandlerSvr2);
	m_pServiceHandlerSvr2->Init(pCfgSvrHdl_Push);

	
	CConfig *pCfgZS1;
	pCfgZS1 = m_pConfig->GetProperties(gc_sCfgIfZS1);
	if (0 != pCfgZS1 && !pCfgZS1->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZS1");
		m_pCpInterfaceZS1 = new CProtocolCpSvr<CProcessInterfaceZS>();
		m_pCpInterfaceZS1->Bind(this,EnumKeyIfZS1);
		m_pCpInterfaceZS1->Init(pCfgZS1);
	}

	CConfig *pCfgZS2;
	pCfgZS2 = m_pConfig->GetProperties(gc_sCfgIfZS2);
	if (0 != pCfgZS2 && !pCfgZS2->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZS2");
		m_pCpInterfaceZS2 = new CProtocolCpSvr<CProcessInterfaceZS>();
		m_pCpInterfaceZS2->Bind(this,EnumKeyIfZS2);
		m_pCpInterfaceZS2->Init(pCfgZS2);
	}

	CConfig *pCfgDeliverMgr;
	pCfgDeliverMgr = m_pConfig->GetProperties(gc_sCfgDeliver);
	if (0 != pCfgDeliverMgr && !pCfgDeliverMgr->IsEmpty())
	{
	}
	else
	{
		pCfgDeliverMgr = m_pConfig;
	}
	CRLog(E_NOTICE,"��ʼ�����ӵ�DeliverMgr");
	m_pDeliverMgr = new CDeliverMgr();
	m_pDeliverMgr->Bind(this,EnumKeyDeliverMgr);
	m_pDeliverMgr->Init(pCfgDeliverMgr);

	CConfig *pCfgZC1;
	pCfgZC1 = m_pConfig->GetProperties(gc_sCfgIfZC1);
	if (0 != pCfgZC1 && !pCfgZC1->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC1");
		m_pCpInterfaceZC1 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC1->Bind(this,EnumKeyIfZC1);
		m_pCpInterfaceZC1->Init(pCfgZC1);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler1");
		m_pServiceHandler1 = new CServiceHandler();
		m_pServiceHandler1->Bind(this,EnumKeyServiceHandler1);
		m_pServiceHandler1->Init(pCfgSvrHdl_Ask);
	}

	CConfig *pCfgZC2;
	pCfgZC2 = m_pConfig->GetProperties(gc_sCfgIfZC2);
	if (0 != pCfgZC2 && !pCfgZC2->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC2");
		m_pCpInterfaceZC2 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC2->Bind(this,EnumKeyIfZC2);
		m_pCpInterfaceZC2->Init(pCfgZC2);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler2");
		m_pServiceHandler2 = new CServiceHandler();
		m_pServiceHandler2->Bind(this,EnumKeyServiceHandler2);
		m_pServiceHandler2->Init(pCfgSvrHdl_Ask);
	}
	//H1�ӿں�H2�ӿ�
	CConfig *pCfgH1;
	pCfgH1 = m_pConfig->GetProperties(gc_sCfgIfH1);
	if (0 != pCfgH1 && !pCfgH1->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�H1");
		m_pCpInterfaceH1 = new CProtocolCpCli<CProcessInterfaceH1C>();
		m_pCpInterfaceH1->Bind(this,EnumKeyIfH1);
		m_pCpInterfaceH1->Init(pCfgH1);
	}
	
	CConfig *pCfgH2;
	pCfgH2 = m_pConfig->GetProperties(gc_sCfgIfH2);
	if (0 != pCfgH2 && !pCfgH2->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�H2");
		m_pCpInterfaceH2 = new CProtocolCpCli<CProcessInterfaceH2C>();
		m_pCpInterfaceH2->Bind(this,EnumKeyIfH2);
		m_pCpInterfaceH2->Init(pCfgH2);
	}
	
	CConfig *pCfgSvAgent;
	pCfgSvAgent = m_pConfig->GetProperties(gc_sCfgSvAgent);
	if (0 == pCfgSvAgent || pCfgSvAgent->IsEmpty())
	{
		pCfgSvAgent = m_pConfig;
	}
	m_pSvAgent = new CIfSvAgent();
	m_pSvAgent->Bind(this,EnumKeySvAgent);
	m_pSvAgent->Init(pCfgSvAgent);
	m_pSvAgent->SetObj(m_pNetMagModule, m_pCpInterfaceH1, m_pCpInterfaceH2);

	//��ʼ������·�ɱ�
	InitRouterTbl();
	return 0;
}

//�����ӵ�����
int CQuoSvrCpMgr::Start()
{
	m_pGessTimerMgr->Start();
	CAbsTimerMgrWin32::Instance()->Start();

	if (0 != m_pSvAgent)
	{
		//CRLog(E_NOTICE,"����SvAgentģ��");
		m_pSvAgent->Start();
	}

	//������������ģ��
	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"�������ܴ���ģ��");
		m_pNetMagModule->Start();
	}

	if (0 != m_pDeliverMgr)
	{
		CRLog(E_NOTICE,"����DeliverMgr");
		m_pDeliverMgr->Start();
	}

	if (0 != m_pHisDataHandler)
	{
		CRLog(E_NOTICE,"����HisDataHandler");
		m_pHisDataHandler->Start();
	}

	if (0 != m_pServiceHandler1)
	{
		CRLog(E_NOTICE,"����ServiceHandler1");
		m_pServiceHandler1->Start();
	}

	if (0 != m_pServiceHandler2)
	{
		CRLog(E_NOTICE,"����ServiceHandler2");
		m_pServiceHandler2->Start();
	}

	if (0 != m_pServiceHandlerSvr1)
	{
		CRLog(E_NOTICE,"����ServiceHandlerSvr1");
		m_pServiceHandlerSvr1->Start();
	}

	if (0 != m_pServiceHandlerSvr2)
	{
		CRLog(E_NOTICE,"����ServiceHandlerSvr2");
		m_pServiceHandlerSvr2->Start();
	}

	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC1");
		m_pCpInterfaceZC1->Start();
	}

	if (0 != m_pCpInterfaceZC2)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC2");
		m_pCpInterfaceZC2->Start();
	}

	if (0 != m_pCpInterfaceZS1)
	{
		CRLog(E_NOTICE,"�������ӵ�ZS1");
		m_pCpInterfaceZS1->Start();
	}

	if (0 != m_pCpInterfaceZS2)
	{
		CRLog(E_NOTICE,"�������ӵ�ZS2");
		m_pCpInterfaceZS2->Start();
	}

	if (0 != m_pCpInterfaceH1)
	{
		CRLog(E_NOTICE,"�������ӵ�H1");
		m_pCpInterfaceH1->Start();
	}

	if (0 != m_pCpInterfaceH2)
	{
		CRLog(E_NOTICE,"�������ӵ�H2");
		m_pCpInterfaceH2->Start();
	}

	return 0;
}

//ֹͣ�����ӵ�
void CQuoSvrCpMgr::Stop()
{
	//ֹͣ��ʱ��������
	CRLog(E_NOTICE,"ֹͣ��ʱ��������");
	m_pGessTimerMgr->Stop();
	CAbsTimerMgrWin32::Instance()->Stop();

	if (0 != m_pDeliverMgr)
	{
		CRLog(E_NOTICE,"ֹͣDeliverMgr");
		m_pDeliverMgr->Stop();
	}

	if (0 != m_pHisDataHandler)
	{
		CRLog(E_NOTICE,"ֹͣHisDataHandler");
		m_pHisDataHandler->Stop();
	}

	if (0 != m_pServiceHandler1)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler1");
		m_pServiceHandler1->Stop();
	}

	if (0 != m_pServiceHandler2)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler2");
		m_pServiceHandler2->Stop();
	}

	if (0 != m_pServiceHandlerSvr1)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandlerSvr1");
		m_pServiceHandlerSvr1->Stop();
	}

	if (0 != m_pServiceHandlerSvr2)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandlerSvr2");
		m_pServiceHandlerSvr2->Stop();
	}

	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC1");
		m_pCpInterfaceZC1->Stop();
	}

	if (0 != m_pCpInterfaceZC2)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC2");
		m_pCpInterfaceZC2->Stop();
	}

	if (0 != m_pCpInterfaceZS1)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZS1");
		m_pCpInterfaceZS1->Stop();
	}

	if (0 != m_pCpInterfaceZS2)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZS2");
		m_pCpInterfaceZS2->Stop();
	}

	if (0 != m_pCpInterfaceH1)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�H1");
		m_pCpInterfaceH1->Stop();
	}

	if (0 != m_pCpInterfaceH2)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�H2");
		m_pCpInterfaceH2->Stop();
	}


	//ֹͣ��������ģ��
	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"ֹͣ���ܴ���ģ��");
		m_pNetMagModule->Stop();
	}

	if (0 != m_pSvAgent)
	{
		m_pSvAgent->Stop();
	}
}

//��������
void CQuoSvrCpMgr::Finish()
{
	m_pGessTimerMgr->Finish();
	m_pGessTimerMgr=0;
	CAbsTimerMgrWin32::Instance()->Finish();

	if (0 != m_pDeliverMgr)
	{
		CRLog(E_NOTICE,"����DeliverMgr");
		m_pDeliverMgr->Finish();
		delete m_pDeliverMgr;
		m_pDeliverMgr = 0;
	}

	if (0 != m_pHisDataHandler)
	{
		CRLog(E_NOTICE,"����HisDataHandler");
		m_pHisDataHandler->Finish();
		delete m_pHisDataHandler;
		m_pHisDataHandler = 0;
	}

	if (0 != m_pServiceHandler1)
	{
		CRLog(E_NOTICE,"����ServiceHandler1");
		m_pServiceHandler1->Finish();
		delete m_pServiceHandler1;
		m_pServiceHandler1 = 0;
	}

	if (0 != m_pServiceHandler2)
	{
		CRLog(E_NOTICE,"����ServiceHandler2");
		m_pServiceHandler2->Finish();
		delete m_pServiceHandler2;
		m_pServiceHandler2 = 0;
	}
	
	if (0 != m_pServiceHandlerSvr1)
	{
		CRLog(E_NOTICE,"����ServiceHandlerSvr1");
		m_pServiceHandlerSvr1->Finish();
		delete m_pServiceHandlerSvr1;
		m_pServiceHandlerSvr1 = 0;
	}

	if (0 != m_pServiceHandlerSvr2)
	{
		CRLog(E_NOTICE,"����ServiceHandlerSvr2");
		m_pServiceHandlerSvr2->Finish();
		delete m_pServiceHandlerSvr2;
		m_pServiceHandlerSvr2 = 0;
	}

	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC1");
		m_pCpInterfaceZC1->Finish();
		delete m_pCpInterfaceZC1;
		m_pCpInterfaceZC1 = 0;
	}

	if (0 != m_pCpInterfaceZC2)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC2");
		m_pCpInterfaceZC2->Finish();
		delete m_pCpInterfaceZC2;
		m_pCpInterfaceZC2 = 0;
	}

	if (0 != m_pCpInterfaceZS1)
	{
		CRLog(E_NOTICE,"�������ӵ�ZS1");
		m_pCpInterfaceZS1->Finish();	
		delete m_pCpInterfaceZS1;
		m_pCpInterfaceZS1 = 0;
	}

	if (0 != m_pCpInterfaceZS2)
	{
		CRLog(E_NOTICE,"�������ӵ�ZS2");
		m_pCpInterfaceZS2->Finish();	
		delete m_pCpInterfaceZS2;
		m_pCpInterfaceZS2 = 0;
	}

	if (0 != m_pCpInterfaceH1)
	{
		CRLog(E_NOTICE,"�������ӵ�H1");
		m_pCpInterfaceH1->Finish();
		delete m_pCpInterfaceH1;
		m_pCpInterfaceH1 = 0;
	}

	if (0 != m_pCpInterfaceH2)
	{
		CRLog(E_NOTICE,"�������ӵ�H2");
		m_pCpInterfaceH2->Finish();	
		delete m_pCpInterfaceH2;
		m_pCpInterfaceH2 = 0;
	}
	

	//
	m_oCpCmdHandler.Finish();

	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"�������ܴ���ģ��");
		m_pNetMagModule->Finish();
		delete m_pNetMagModule;
		m_pNetMagModule=0;
	}

	if (0 != m_pSvAgent)
	{
		m_pSvAgent->Finish();
		delete m_pSvAgent;
		m_pSvAgent = 0;
	}
	
	CLogger::Instance()->Finish();
	delete m_pConfig;

	CNetMgr::Instance()->NmFinish();
	
	//
	m_oMemShareAlive.UnMap();
}

//�����߳�
int CQuoSvrCpMgr::StartMe()
{
	//CRLog(E_NOTICE,"������ʱ������ⶨʱ��");
	//m_oResetTimer.Bind(this);
	//m_pGessTimerMgr->CreateTimer(&m_oResetTimer,4,"reset_timer");

	//telnet ���Ĵ������ӵ��
	m_oCpCmdHandler.Bind(this,EnumKeyCmdHandler);
	//������־�ص������
	m_oNetLogHost.Bind(this);
	//������־�����̰߳�
	m_oNetLogThread.Bind(this);
	//�����д����̰߳�
	m_oCmdLineThread.Bind(this);


	//
	m_oCpCmdHandler.Start();

	//telnet ���������ӵ��ʼ������
	CConfig *pCfgCmd = m_pConfig->GetProperties(gc_sCfgIfCmd);
	if (0 != pCfgCmd && !pCfgCmd->IsEmpty())
	{
		m_pCpInterfaceCmd = new CProtocolCpSvr<CProcessInterfaceCmd>();
		CRLog(E_NOTICE,"��ʼ�����ӵ�Telnet Cmd");
		m_pCpInterfaceCmd->Init(pCfgCmd);
		m_pCpInterfaceCmd->Bind(this,EnumKeyIfCmd);

		CRLog(E_NOTICE,"�������ӵ�Telnet Cmd");
		m_pCpInterfaceCmd->Start();
	}

	string sCmdFlag("0");
	int nCmdFlag = 0;
	if (0 == m_pConfig->GetProperty("cmdline",sCmdFlag))
	{
		nCmdFlag = FromString<int>(sCmdFlag);
	}
	if (1 == nCmdFlag)
	{
		//���������д����߳�
		m_oCmdLineThread.BeginThread();
	}

	//����������־�����߳�
	m_oNetLogThread.BeginThread();
	return 0;
}

//ֹͣ�߳�
void CQuoSvrCpMgr::StopMe()
{
	//CRLog(E_NOTICE,"ֹͣ��ʱ������ⶨʱ��");
	//m_pGessTimerMgr->DestroyTimer(&m_oResetTimer,"reset_timer");

	m_oNetLogThread.EndThread();
	//m_oCmdLineThread.EndThread();

	//
	m_oCpCmdHandler.Stop();
	m_oCpCmdHandler.Finish();

	if (0 != m_pCpInterfaceCmd)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�Telnet Cmd");
		m_pCpInterfaceCmd->Stop();

		CRLog(E_NOTICE,"�������ӵ�Telnet Cmd");
		m_pCpInterfaceCmd->Finish();
		delete m_pCpInterfaceCmd;
		m_pCpInterfaceCmd = 0;
	}
}

int CQuoSvrCpMgr::HandleCmdSpecial(const string& sCmdID, CPacket &pkt)
{
	int nRtn = -1;
	try
	{
		// added by Ben 2011-05-29 �����˳���������
		if (strutils::ToHexString<unsigned int>(YL_QUITMSG) == sCmdID)
		{
			CSamplerPacket &sPkt = dynamic_cast<CSamplerPacket &>( pkt);
			unsigned int uiNodeidDest = 0;
			CMessageImpl& msg = dynamic_cast<CMessageImpl&>(sPkt.GetMsg());
			msg.GetField(MSG_BODY_NODEID, uiNodeidDest);

			if (uiNodeidDest == m_uiNodeID)
			{
				m_bStop = true;
				m_deqCondMutex.Signal();
			}
			nRtn = 0;
		}
		else if (strutils::ToHexString<unsigned int>(YL_SYNC_TIME) == sCmdID)
		{
			CSamplerPacket &sPkt = dynamic_cast<CSamplerPacket &>( pkt);
			unsigned int uiNodeidDest = 0;
			unsigned int uiNodeid = 0;
			CMessageImpl& msg = dynamic_cast<CMessageImpl&>(sPkt.GetMsg());
			msg.GetField(MSG_NODE_ID, uiNodeid);
			msg.GetField(MSG_BODY_NODEID, uiNodeidDest);

			unsigned int uiDate = 0;
			unsigned int uiTime = 0;
			msg.GetField(MSG_DATE, uiDate);
			msg.GetField(MSG_TIME, uiTime);

			if (uiNodeid == m_uiNodeID)
			{
				if (uiNodeidDest == m_uiNodeID)
				{
					if (m_bAcceptSyncTime)
					{
						SYSTEMTIME st;
						st.wYear = uiDate / 100000;
						st.wMonth = (uiDate % 100000) / 1000;
						st.wDay = (uiDate % 1000) / 10;
						st.wDayOfWeek = uiDate % 10;

						st.wHour = uiTime / 10000000;
						st.wMinute = (uiTime % 10000000) / 100000;
						st.wSecond = (uiTime % 100000) / 1000;
						st.wMilliseconds = uiTime % 1000;

						SYSTEMTIME stLocal;
						GetLocalTime(&stLocal);

						if (st.wYear != stLocal.wYear || st.wMonth != stLocal.wMonth ||
							st.wDay != stLocal.wDay ||	st.wDayOfWeek != stLocal.wDayOfWeek ||
							st.wHour != stLocal.wHour || st.wMinute != stLocal.wMinute ||
							abs(st.wSecond - stLocal.wSecond) > 2 )
						{
							::SetLocalTime(&st);
							string sDate = ToString<unsigned int>(uiDate);
							string sTime = ToString<unsigned int>(uiTime);
							string sEvtContent = "����ʱ��ͬ���� ";
							
							sEvtContent = sEvtContent + sDate + " ";
							sEvtContent = sEvtContent + sTime;
							CRLog(E_NOTICE,sEvtContent.c_str());
							NotifyEvent(sEvtContent);
						}
					}
				}
			}

			nRtn = 0;
		}
		// end add
		return nRtn;
	}
	catch(...)
	{
		return -1;
	}
}

//����ת������ ����ֵ-2��ʾ��·��
int CQuoSvrCpMgr::Forward(CPacket &pkt,const unsigned long& ulKey)
{
	std::string sCmdID = "";
	try
	{	
		int nRtn = -2;
		assert(EnumKeyUnknown > ulKey);
		if (EnumKeyUnknown <= ulKey)
			return -1;

		if (m_bStop)
			return 0;

		sCmdID = pkt.GetCmdID();

		//���������
		if (0 == HandleCmdSpecial(sCmdID, pkt))
			return 0;
		
		bool blFound = false;
		MMAP_IT it;
		RANGE_CP range = m_tblIfRouter[ulKey].mmapCmds.equal_range(sCmdID);
		for (it = range.first; it != range.second; ++it)
		{
			if (0 != (*it).second)
			{
				nRtn = (*it).second->SendPacket(pkt);
			}
			blFound = true;
		}

		if (!blFound)
		{
			it = m_tblIfRouter[ulKey].mmapCmds.find(gc_sDefaultCmdID);
			if (it != m_tblIfRouter[ulKey].mmapCmds.end())
			{
				if (0 != (*it).second)
				{
					nRtn = (*it).second->SendPacket(pkt);
				}
			}
		}
		return nRtn;
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"from %u,%s packet error!", ulKey, sCmdID.c_str());
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"from %u,%s exception:%s", ulKey, sCmdID.c_str(), e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"from %u,%s Unknown exception", ulKey, sCmdID.c_str());
		return -1;
	}
}

//�������̺߳��� �����߳��˳�������˳�
int CQuoSvrCpMgr::Run()
{
	try
	{
		while ( !m_bStop )
		{
			m_deqCondMutex.Lock();
			while(!m_bStop)
				m_deqCondMutex.Wait();
			
			if (m_bStop)
			{
				m_deqCondMutex.Unlock();
				break;
			}		

		}

		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		return -1;
	}
}

//CmdLine����ƥ�䴦��
string CQuoSvrCpMgr::OnCmd(const string& sCmdLine, const vector<string>& vecPara)
{
	try
	{
		std::string sCmdID = trim(sCmdLine);
	
		int nSize = sizeof(m_CmdLine2Api)/sizeof(CmdLine2Api);
		for ( int i = 0 ; i < nSize ; i++ )
		{
			if ( m_CmdLine2Api[i].sCmdName == sCmdID || (sCmdID != "" && m_CmdLine2Api[i].sCmdAbbr == sCmdID) )
			{
				if (m_CmdLine2Api[i].pMemberFunc == 0)
					break;

				return (this->*(m_CmdLine2Api[i].pMemberFunc))(sCmdLine, vecPara);				
			}
		}
		
		string sRtn("");
		if (sCmdID != "")
		{
			sRtn += "err command!\r\n";
		}
		sRtn += "Quotation Server->";
		return sRtn;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		string sRtn = "\r\nQuotation Server->";
		return sRtn;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		string sRtn = "\r\nQuotation Server->";
		return sRtn;
	}
}

//telnet�ն������
int CQuoSvrCpMgr::OnPacketCmd(CPacket& pkt)
{
	try
	{
		CPacketLineReq & pktLine = dynamic_cast<CPacketLineReq&>(pkt);
		string sRouteKey = pktLine.RouteKey();
		string sCmd = trim(pktLine.GetCmdID());

		vector<string> vecPara;
		vecPara.clear();
		pktLine.GetPara(vecPara);
		
		string sRsp("");
		if ("q" == sCmd || "quit" == sCmd)
		{
			sRsp += "Quotation Server->";
		}
		else if (sCmd == "debug")
		{
			string sPara("");
			if (vecPara.size() > 0)
				sPara = vecPara[0];
			
			if (trim(sPara) == "on")
			{
				m_csTelnets.Lock();
				m_deqTelnets[sRouteKey] = sRouteKey;
				if (m_deqTelnets.size() == 1)
					CLogger::Instance()->RegisterNetLog(&m_oNetLogHost);
				m_csTelnets.Unlock();

				sRsp += "Quotation Server->";
			}
			else if (trim(sPara) == "off")
			{
				map<string,string>::iterator itTel;	
				m_csTelnets.Lock();
				itTel = m_deqTelnets.find(sRouteKey);
				if (itTel != m_deqTelnets.end())
					m_deqTelnets.erase( itTel);

				if (m_deqTelnets.size() == 0)
					CLogger::Instance()->UnRegisterNetLog(&m_oNetLogHost);
				m_csTelnets.Unlock();

				sRsp += "Quotation Server->";
			}
			else
			{
				sRsp = "Parameter err!";
				sRsp += "\r\n";
				sRsp += "Quotation Server->";
			}
		}
		else
		{
			sRsp = OnCmd(sCmd, vecPara);
		}

		CPacketLineRsp pktRsp(sRsp);
		pktRsp.AddRouteKey(sRouteKey);
		return m_pCpInterfaceCmd->SendPacket(pktRsp);
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"packet error!");
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"Unknown exception!");
		return -1;
	}
}

//��־��Ϣ���̶߳���
void CQuoSvrCpMgr::OnNetLogMsg(const string& sMsg)
{
	m_oNetLogThread.Enque(sMsg);
}

//��־����
int CQuoSvrCpMgr::HandleNetLogMsg(const string & sNetLogMsg)
{
	string sMsg = sNetLogMsg;
	sMsg += "\r\n";
	map<string,string>::iterator itTel;	
	m_csTelnets.Lock();
	for (itTel = m_deqTelnets.begin(); itTel != m_deqTelnets.end(); ++itTel)
	{
		CPacketLineRsp pktRsp(sMsg);
		pktRsp.AddRouteKey((*itTel).first);
		if (0 != m_pCpInterfaceCmd->SendPacket(pktRsp))
		{
			string sKey = (*itTel).first;
			m_deqTelnets.erase(itTel);
			if (0 == m_deqTelnets.size())
			{
				cout << "UnRegisterNetLog(" << sKey << ")!" << endl;
				CLogger::Instance()->UnRegisterNetLog(&m_oNetLogHost);
			}

			break;
		}
	}
	m_csTelnets.Unlock();

	return 0;
}

//�������̴߳����� ������
int CQuoSvrCpMgr::HandleCmdLine(string& sIn)
{
	try
	{
		char cIn = 0;
		cin.get(cIn);
		if (cIn == '\n')
		{				
			string sCmd("");

			sIn = trim(sIn);
			vector<string> vPara;
			vPara = explodeQuoted(" ", sIn);

			if (vPara.size() > 1)
			{
				sCmd = vPara[0];
				vPara.erase(vPara.begin());
			}
			else
			{
				sCmd = sIn;
				vPara.clear();
			}

			string sOut = OnCmd(sCmd, vPara);
			cout << sOut.c_str();
			sIn.clear();
		}
		else
		{
			sIn.append(1,cIn);
		}

		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		return -1;
	}
}

//K�ӿ�������ʱ���ص��ӿ�
int CQuoSvrCpMgr::OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan)
{
	m_oMemShareAlive.IamAlive();
	if (m_oMemShareAlive.IsIQuitCmd())
	{
		//�¼�֪ͨ
		string sEvtContent = "��ʱʱ�䵽,�Ժ�ʼ����!";

		CRLog(E_NOTICE,sEvtContent.c_str());
		NotifyEvent(sEvtContent);

		m_bStop = true;
		m_deqCondMutex.Signal();
		return -1;
	}
	return 0;
}


//��ʱ������ʱ���ص��ӿ�
int CQuoSvrCpMgr::OnResetTimeout(const string& sTmKey)
{
	//�¼�֪ͨ
	string sEvtContent = "��ʱʱ�䵽,�Ժ�ʼ����!";

	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);

	m_bStop = true;
	m_deqCondMutex.Signal();
	return 0;
}

//������Ϣ
string CQuoSvrCpMgr::OnCmdLineHelp(const string& sCmdLine, const vector<string>& vecPara)
{
	string sRtn("");
	int nSize = sizeof(m_CmdLine2Api)/sizeof(CmdLine2Api);
	for ( int i = 0 ; i < (nSize - 2); i++ ) // modiedified by Ben 2011-05-29 ��ֹquitnode�������ʾ
	{
		sRtn += m_CmdLine2Api[i].sCmdName;
		if (m_CmdLine2Api[i].sCmdAbbr != "")
		{
			sRtn += "/";
			sRtn += m_CmdLine2Api[i].sCmdAbbr;
		}
		sRtn += ":";
		sRtn += m_CmdLine2Api[i].sHelp;
		sRtn += "\r\n";
	}
	sRtn += "Quotation Server->";

	return sRtn;
}

//quit�����
string CQuoSvrCpMgr::OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara)
{
	//�¼�֪ͨ
	string sEvtContent = "�����з���Quit�˳�ָ��,��Լ3����˳�!";
	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);
		
	CancelOrder();
	msleep(3);

	m_bStop = true;
	m_deqCondMutex.Signal();

	string sRtn = "Quotation Server->";
	return sRtn;
}

//����������ʾ
string CQuoSvrCpMgr::OnCmdLineBuffer(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn;
	string sInstID;

	if (vecPara.size() >  0)
	{
		sInstID = vecPara[0];
	}

	map<string,QUOTATION> mapQuotation;
	CMemData::Instance()->GetQuotationTbl().GetRecs(mapQuotation);
	if (sInstID.empty())
	{
		for (map<string,QUOTATION>::iterator it = mapQuotation.begin(); it != mapQuotation.end(); ++it)
		{
			sRtn += ToHexString<unsigned int>((*it).second.m_CodeInfo.m_usMarketType);
			sRtn += "\t";
			sRtn += (*it).second.m_CodeInfo.m_acCode;
			sRtn += "\t";
			sRtn += (*it).second.m_CodeInfo.m_acName;
			sRtn += "\t";
			sRtn += ToString<unsigned int>((*it).second.m_CodeInfo.m_uiUnit);
			sRtn += "\r\n";
		}
		sRtn += "Total ";
		sRtn += ToString<unsigned int>(mapQuotation.size());
		sRtn += "\r\n";
	}
	else
	{
		for (map<string,QUOTATION>::iterator it = mapQuotation.begin(); it != mapQuotation.end(); ++it)
		{
			string sTmpID = (*it).second.m_CodeInfo.m_acCode;
			if (sTmpID == sInstID)
			{
				sRtn += ToHexString<unsigned int>((*it).second.m_CodeInfo.m_usMarketType);
				sRtn += "\t";
				sRtn += (*it).second.m_CodeInfo.m_acCode;
				sRtn += "\t";
				sRtn += (*it).second.m_CodeInfo.m_acName;
				sRtn += "\t";
				sRtn += ToString<unsigned int>((*it).second.m_CodeInfo.m_uiUnit);
				sRtn += "\r\n";
				
				sRtn += "m_uiSeqNo:";        
				sRtn += ToString<unsigned int>((*it).second.m_uiSeqNo);
				sRtn += "\r\n";
				sRtn += "m_uiDate:";            
				sRtn += ToString<unsigned int>((*it).second.m_uiDate);
				sRtn += "\r\n";
				sRtn += "m_uiTime:";            
				sRtn += ToString<unsigned int>((*it).second.m_uiTime);
				sRtn += "\r\n";
				sRtn += "m_uilastClose:";    
				sRtn += ToString<unsigned int>((*it).second.m_uilastClose);
				sRtn += "\r\n";
				sRtn += "m_uiLastSettle:"; 
				sRtn += ToString<unsigned int>((*it).second.m_uiLastSettle);
				sRtn += "\r\n";
				sRtn += "m_uiSettle:";   
				sRtn += ToString<unsigned int>((*it).second.m_uiSettle);   
				sRtn += "\r\n";
				sRtn += "m_uiOpenPrice:";    
				sRtn += ToString<unsigned int>((*it).second.m_uiOpenPrice);
				sRtn += "\r\n";
				sRtn += "m_uiHigh:";            
				sRtn += ToString<unsigned int>((*it).second.m_uiHigh);
				sRtn += "\r\n";
				sRtn += "m_uiLow:";             
				sRtn += ToString<unsigned int>((*it).second.m_uiLow);
				sRtn += "\r\n";
				sRtn += "m_uiClose:";           
				sRtn += ToString<unsigned int>((*it).second.m_uiClose);
				sRtn += "\r\n";
				sRtn += "m_uiHighLimit:";       
				sRtn += ToString<unsigned int>((*it).second.m_uiHighLimit);
				sRtn += "\r\n";
				sRtn += "m_uiLowLimit:";	       
				sRtn += ToString<unsigned int>((*it).second.m_uiLowLimit);
				sRtn += "\r\n";
				sRtn += "m_uiLast:";            
				sRtn += ToString<unsigned int>((*it).second.m_uiLast);
				sRtn += "\r\n";
				sRtn += "m_uiAverage:";        
				sRtn += ToString<unsigned int>((*it).second.m_uiAverage);
				sRtn += "\r\n";
				sRtn += "m_uiVolume:";          
				sRtn += ToString<unsigned int>((*it).second.m_uiVolume);
				sRtn += "\r\n";
				sRtn += "m_uiTurnOver:";        
				sRtn += ToString<unsigned int>((*it).second.m_uiTurnOver);
				sRtn += "\r\n";
				sRtn += "m_uiChiCangLiang:";    
				sRtn += ToString<unsigned int>((*it).second.m_uiChiCangLiang);
				sRtn += "\r\n";
				sRtn += "m_uiLastChiCangLiang:";
				sRtn += ToString<unsigned int>((*it).second.m_uiLastChiCangLiang);
				for (int nIndex = 0; nIndex < 5; nIndex++)
				{
					sRtn += "\r\n";
					sRtn += "Bid" + ToString<int>(nIndex+1) + ":";
					sRtn += ToString<unsigned int>((*it).second.m_Bid[nIndex].m_uiPrice);
					sRtn += "\t";
					sRtn += "Bidlot" + ToString<int>(nIndex+1) + ":";
					sRtn += ToString<unsigned int>((*it).second.m_Bid[nIndex].m_uiVol);

					sRtn += "\r\n";
					sRtn += "Ask" + ToString<int>(nIndex+1) + ":";
					sRtn += ToString<unsigned int>((*it).second.m_Ask[nIndex].m_uiPrice);
					sRtn += "\t";
					sRtn += "Asklot" + ToString<int>(nIndex+1) + ":";
					sRtn += ToString<unsigned int>((*it).second.m_Ask[nIndex].m_uiVol);
				}
				sRtn += "\r\n";
				sRtn += "\r\n";
			}
		}
	}
	return sRtn;
}

string CQuoSvrCpMgr::OnCmdLineSysInfo(const string& sCmdLine, const vector<string>& vecPara)
{
	string sRtn = CSelectorIo::Instance()->ToString();
	sRtn += "\r\n";
	sRtn += CSelectorListen::Instance()->ToString();
	sRtn += "\r\n";
	sRtn += CGessTimerMgrImp::Instance()->ToString();
	sRtn += "Quotation Server->";
	return sRtn;
}

//��ʾ�ڴ�
string CQuoSvrCpMgr::OnCmdLineMem(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Quotation Server->\r\n";

	bool blFalg = true;
	unsigned long ulMemAddr = 0x00;
	unsigned long ulLen = 16;
	std::stringstream ss1;
	if (vecPara.size() == 2)
	{
		ss1 << hex << vecPara[0];
		ss1 >> ulMemAddr;
		ss1 << hex << vecPara[1];
		ss1 >> ulLen;
		if (ulLen > 1024)
			ulLen = 1024;
	}
	else if (vecPara.size() == 1)
	{
		ss1 << vecPara[0];
		ss1 >> ulMemAddr;
	}
	else
	{
		blFalg = false;
	}

	
	if (blFalg)
	{
		try
		{
			std::stringstream ss2;
			const unsigned char * pMemAddr = reinterpret_cast<const unsigned char*>(ulMemAddr);
			for (unsigned long ulIndex = 0; ulIndex < ulLen; ulIndex++,pMemAddr++)
			{
				unsigned int uiVal = static_cast<unsigned int>(*pMemAddr);			
				if (ulIndex != 0 && ulIndex % 4 == 0)
				{
					ss2 << " ";
				}
				ss2 << setfill('0') << setw(2) << hex << uppercase << uiVal;
			}
			sRtn = ss2.str();
		}
		catch(...)
		{
			CRLog(E_ERROR,"��ȡ��ַ������Χ");
		}
	}

	return sRtn;
}

// K�ӿ�[onRecvQuit] ҵ����Ӧ�����
int CQuoSvrCpMgr::OnRecvQuit(CIpcPacket& pkt)
{
	//�¼�֪ͨ
	string sEvtContent = "Convergence���յ��˳�ָ��,��Լ3����˳�!";
	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);
	//msleep(3);

	CancelOrder();
	msleep(1);

	m_bStop = true;
	m_deqCondMutex.Signal();

	return 0;
}

int CQuoSvrCpMgr::HandleConsolMsg(unsigned int uiMsg)
{
	
	//�¼�֪ͨ
	string sEvtContent = "";
	switch (uiMsg)
	{
	case CTRL_CLOSE_EVENT:
		sEvtContent = "Convergence����̨���ڱ�ǿ�ƹر�,�����˳�Ӧ��!";
		break;
	case CTRL_SHUTDOWN_EVENT:
		sEvtContent = "Convergence������ػ�,�����˳�Ӧ��!";
		break;
	default:
		return 0;
	}
	
	CancelOrder();

	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);

	m_bStop = true;
	m_deqCondMutex.Signal();
	return 0;
}

void CQuoSvrCpMgr::NotifyEvent(const string& sEvt)
{
	//�¼�֪ͨ
	CEventSimple e;
	e.m_nEvtCategory = 0;
	e.m_nEvtType = 0;
	e.m_nGrade = 1;
	e.m_sDateTime = CGessDate::NowToString("-") + " " + CGessTime::NowToString(":");		
	e.m_sEvtContent = sEvt;
	CNetMgr::Instance()->OnEvtNotify(e);
}

string CQuoSvrCpMgr::OnCmdLineEvtTest(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Quotation Server->";
		
	//�¼�֪ͨ
	if (vecPara.size() == 0)
	{
		string sEvtContent = "�¼�����!";
		NotifyEvent(sEvtContent);
		return sRtn;
	}

	string sType = "1";
	string sNodeID = "1308";
	if (vecPara.size() >= 1)
	{
		sNodeID = vecPara[0];
	}
	if (vecPara.size() >= 2)
	{
		sType = vecPara[1];
	}

	CTradePacket oPkt;
	HEADER_REQ stHeaderReq;
	QueryItemReq stBodyReq;
	memset(&stHeaderReq, 0x00, sizeof(stHeaderReq));
	strcpy(stHeaderReq.seq_no, "12345678");
	strcpy(stHeaderReq.msg_type, "1");
	strcpy(stHeaderReq.msg_flag, "1");
	strcpy(stHeaderReq.term_type, "13");
	strcpy(stHeaderReq.user_type, "1");
	strcpy(stHeaderReq.user_id, "admin");
	
	strcpy(stHeaderReq.exch_code, "1922");
	if (sType == "1")
	{
		strcpy(stHeaderReq.exch_code, "1922");
	}
	else if (sType == "2")
	{
		strcpy(stHeaderReq.exch_code, "1923");
	}
	else if (sType == "3")
	{
		strcpy(stHeaderReq.exch_code, "1924");
	}
	oPkt.SetHeader(stHeaderReq);

	stBodyReq.host_id  = "13";
	stBodyReq.oper_flag = 1;
	stBodyReq.node_id = sNodeID;

	CPacketStructTradeNm::Struct2Packet(stBodyReq, oPkt);
	Forward(oPkt, EnumKeyIfH1);
	return sRtn;
}

//���г���
string CQuoSvrCpMgr::OnCmdLineQue(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Quotation Server->";
	
	return sRtn;
}

int CQuoSvrCpMgr::CancelOrder()
{
	CMessageImpl oMsgUnsub;
	
	unsigned int uiVal = 0xFFFFFFFF;
	string sItems;
	sItems.assign((char*)(&uiVal),sizeof(unsigned int));
	oMsgUnsub.SetBinaryField(MSG_SUBSCRIP_RECS,sItems);

	CSamplerPacket oPktUnsub(oMsgUnsub,YL_UNSUBSCRIP);
	if (0 != m_pCpInterfaceZC1)
	{
		m_pCpInterfaceZC1->SendPacket(oPktUnsub);
	}

	if (0 != m_pCpInterfaceZC2)
	{
		m_pCpInterfaceZC2->SendPacket(oPktUnsub);		
	}

	return 0;
}




string CQuoSvrCpMgr::OnCmdLineSetTime(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn;
	string sNodeIDHead;
	string sNodeIDDestination;
	if (vecPara.size() <  2)
	{
		sRtn = "��������ȷ����������յĽڵ�ź�Ŀ�Ľڵ�š�";
		return sRtn;
	}

	sNodeIDHead = vecPara[0];
	sNodeIDDestination = vecPara[1];
	unsigned int uiNodeID = FromString<unsigned int>(sNodeIDHead);
	unsigned int uiNodeIDDest = FromString<unsigned int>(sNodeIDDestination);

	SYSTEMTIME st;
	::GetLocalTime(&st);
	unsigned int uiDate = st.wYear * 100000 + st.wMonth * 1000 + st.wDay * 10 + st.wDayOfWeek;
	unsigned int uiTime = st.wHour * 10000000 + st.wMinute * 100000 + st.wSecond * 1000 + st.wMilliseconds;
	
	CSamplerPacket oPacketTime(YL_SYNC_TIME);
	CMessage &  msg = oPacketTime.GetMsg();
	msg.SetField(MSG_SEQ_ID, static_cast<unsigned int>(0));
	msg.SetField(MSG_NODE_ID, uiNodeID);
	msg.SetField(MSG_BODY_NODEID, uiNodeIDDest);
	msg.SetField(MSG_DATE, uiDate);
	msg.SetField(MSG_TIME, uiTime);
	if (0 != m_pCpInterfaceZS2)
	{
		m_pCpInterfaceZS2->SendPacket(oPacketTime);
	}
	return sRtn;
}
