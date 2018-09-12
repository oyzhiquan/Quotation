/******************************************************************************
��    Ȩ:��������������ϵͳ���޹�˾.
ģ������:OfferingMgr.cpp
������	:��ΰ
��������:2008.07.22
��    ��:1.0				
ģ������:�������̻��������ص���ģ��
��Ҫ����:Init(...)��ʼ������
         Finish() ��������
         Run()�����̺߳���
�޸ļ�¼:
******************************************************************************/

#include <iostream>
#include <fstream>
#include "Logger.h"
#include "ConfigImpl.h"
#include "ProviderCpMgr.h"
#include "MibConstant.h"
#include "HisDataHandler.h"
#include "HisDataHandlerFx.h"
#include "ServiceHandler.h"
#include "ServiceHandlerCln.h"
#include "DeliverMgr.h"
#include "XQueueIo.h"
#include "ProtocolConnectPoint.h"
#include "ProcessInterfaceZS.h"
#include "ProcessInterfaceZC.h"
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

using namespace MibConst;

//Դ�ӿ�+������ ����·�����ñ�
CProviderCpMgr::IfRouterCfg CProviderCpMgr::m_tblIfRouterCfg[] = 
{	
	//EnumKeyDeliverMgr To EnumKeyIfZS 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////	
    {0,			EnumKeyIfZS,			EnumKeyDeliverMgr,     gc_sDefaultCmdID},

	//EnumKeyIfZS To EnumKeyDeliverMgr 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////	
	{0,    EnumKeyDeliverMgr,			EnumKeyIfZS,     "00000003"},
	{0,    EnumKeyDeliverMgr,			EnumKeyIfZS,     "00000004"},

	//EnumKeyIfZS To EnumKeyServiceHandlerSvr 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////	
	{0,    EnumKeyServiceHandlerSvr,		EnumKeyIfZS,     gc_sDefaultCmdID},

	//EnumKeyServiceHandlerSvr To EnumKeyIfZS 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////	
	{0,		   EnumKeyIfZS,				EnumKeyServiceHandlerSvr,     gc_sDefaultCmdID},

	
	
	//EnumKeyIfZS To EnumKeyServiceHandlerSvr 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////	
	{0,    EnumKeyServiceHandlerCln1,		EnumKeyIfZC1,     gc_sDefaultCmdID},

	//from H1	
	//H1 To NetMgrModule
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From                    CmdID		///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumNetMagModule,			EnumKeyIfH1,            "1921"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1922"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1923"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1924"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,		    "1925"}, 

	{0,    EnumNetMagModule,			EnumKeyIfH1,            "1911"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1912"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1913"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1914"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,		    "1915"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,		    "1916"}, 

	//from NetMgrModule	
	//NetMgrModule To H1 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From                    CmdID		//
	{0,    EnumKeyIfH1,				EnumNetMagModule,       "1921"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1922"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1923"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1924"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1925"}, 

	{0,    EnumKeyIfH1,				EnumNetMagModule,       "1911"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1912"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1913"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1914"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1915"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1916"}, 

	//NetMgrModule To H2
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From					CmdID		///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfH2,				EnumNetMagModule,		"onEventNotify"},   //�¼��㲥�౨��
	{0,    EnumKeyIfH2,				EnumNetMagModule,		"onAlarmNotify"},   //�澯�㲥�౨��
	{0,    EnumKeyIfH2,				EnumNetMagModule,		"onNodeMibTblChg"}, //��¼�仯�㲥����


	//from IFCMD
	//IFCMD To default ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyCmdHandler,		EnumKeyIfCmd,    			 gc_sDefaultCmdID},


};


//Telnet or Console CommandLine ��Ӧ���������ñ�
CProviderCpMgr::CmdLine2Api CProviderCpMgr::m_CmdLine2Api[] = 
{
	//������			��д			�������ָ��					˵��
	{"replay",			"r",			&CProviderCpMgr::OnCmdLineReplayQuotation,	"replay the quotation by log file!\r\n\t  r �ļ��� �г����� ��Լ ��� ���� , r 2010.dat 6100 AUTD 1000 10"},
	{"buf",				"b",			&CProviderCpMgr::OnCmdLineBuffer,		"list the quotation buffer"},
	{"load",			"ld",			&CProviderCpMgr::OnCmdLineLoad,			"reload the code unit file"},
	{"quit",			"q",			&CProviderCpMgr::OnCmdLineQuit,			"quit the system"},
	{"mem",				"m",			&CProviderCpMgr::OnCmdLineMem,			"show mem bytes"},
	{"evt",				"e",			&CProviderCpMgr::OnCmdLineEvtTest,		"test evt notify"},
	{"info",			"i",			&CProviderCpMgr::OnCmdLineSysInfo,		"show SysInfo"},
	{"que",			   "que",			&CProviderCpMgr::OnCmdLineQue,			"for que"},
	{"?",				"",				&CProviderCpMgr::OnCmdLineHelp,			"for help"},
	{"help",			"h",			&CProviderCpMgr::OnCmdLineHelp,			"for help"}	
};


CProviderCpMgr::CProviderCpMgr()
:m_sProcName("Provider")
,m_pCpInterfaceCmd(0)
,m_pCpInterfaceH1(0)
,m_pCpInterfaceH2(0)
,m_pCpInterfaceZS(0)
,m_pServiceHandlerSvr(0)
,m_pDeliverMgr(0)
,m_pHisDataHandler(0)
,m_pHisDataHandlerFx(0)
,m_pReader(0)
,m_pIoServiceWriter(0)
,m_pNetMagModule(0)
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

	//CGessDate oDate;
	//int nWeekDay = oDate.WeekDay();

	//CGessTime oNowTime;
	//CGessTime oTime1(17,0,0);
	//if (1 <= nWeekDay && nWeekDay <= 5)
	//{
	//	if (oNowTime >= oTime1)
	//	{
	//		oDate++;
	//	}
	//}
	//else if (6 == nWeekDay)
	//{
	//	oDate += 2;
	//}
	//else
	//{
	//	oDate++;
	//}

}

CProviderCpMgr::~CProviderCpMgr(void)
{
	m_deqTelnets.clear();
}

//�ͻ���Э�����ӵ����ӳɹ���ص�
int CProviderCpMgr::OnConnect(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
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
int CProviderCpMgr::OnAccept(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;
	
	return 0;
}

int CProviderCpMgr::OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	return 0;
}

int CProviderCpMgr::OnClose(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
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
int CProviderCpMgr::InitRouterTbl()
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
		case EnumKeyServiceHandlerSvr:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandlerSvr));
			break;
		case EnumKeyServiceHandlerCln1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandlerCln1));
			break;
		case EnumKeyIfZS:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZS));
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
		default:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, 0));
			break;
		}
	}

	return 0;
}

//���̻����ӵ��������ʼ��
int CProviderCpMgr::Init(const string& sProcName)
{
	m_sProcName = sProcName;

	cout << "���������ļ�..." << endl;

	std::string sCfgFilename;
	sCfgFilename = DEFUALT_CONF_PATH PATH_SLASH;
	sCfgFilename = sCfgFilename + "ProviderDMA";//m_sProcName;
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

	//CRLog(E_NOTICE,"[%s]�����ṩ��...",sProcName.c_str());
	//CRLog(E_NOTICE,"��ʼ�����ܴ���");
	//string sTblPrefix = "provider";
	//CConfig *pCfgNetMagModule = m_pConfig->GetProperties(gc_sCfgNetMagModule);
	//if (0 != pCfgNetMagModule)
	//{
	//	if (0 == pCfgNetMagModule->GetProperty("tbl_prefix",sTmp))
	//		sTblPrefix = sTmp;
	//}
	//m_pNetMagModule = new CNetMgrModule();
	//CNetMgr::Instance()->NmInit(m_pNetMagModule,sTblPrefix);
	//
	////
	//m_oNmoModule.Bind(this);
	//
	////��ʼ�����ܴ���ģ��
	//m_pNetMagModule->Bind(this,EnumNetMagModule);
	//m_pNetMagModule->Init(pCfgNetMagModule);

	CConfig *pCfgHisData;
	pCfgHisData = m_pConfig->GetProperties(gc_sCfgHisData);
	if (0 != pCfgHisData && !pCfgHisData->IsEmpty())
	{
	}
	else
	{
		pCfgHisData = m_pConfig;
	}

	CRLog(E_NOTICE,"��ʼ�����ӵ�HisDataHandler");
	m_pHisDataHandler = new CHisDataHandler();
	m_pHisDataHandler->Init(pCfgHisData);

	//
	CConfig *pCfgHisDataFx;
	pCfgHisDataFx = m_pConfig->GetProperties(gc_sCfgHisDataFx);
	if (0 != pCfgHisDataFx && !pCfgHisDataFx->IsEmpty())
	{
	}
	else
	{
		pCfgHisDataFx = m_pConfig;
	}

	CRLog(E_NOTICE,"��ʼ�����ӵ�HisDataHandlerFx");
	m_pHisDataHandlerFx = new CHisDataHandlerFx();
	m_pHisDataHandlerFx->Init(pCfgHisDataFx);

	CConfig *pCfgServiceHandler;
	pCfgServiceHandler = m_pConfig->GetProperties(gc_sCfgService);
	if (0 != pCfgServiceHandler && !pCfgServiceHandler->IsEmpty())
	{
	}
	else
	{
		pCfgServiceHandler = m_pConfig;
	}
	CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler");
	m_pServiceHandlerSvr = new CServiceHandlerSvr();
	m_pServiceHandlerSvr->Bind(this,EnumKeyServiceHandlerSvr);
	m_pServiceHandlerSvr->Init(pCfgServiceHandler);

	
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
	
	CConfig *pCfgZS;
	pCfgZS = m_pConfig->GetProperties(gc_sCfgIfZS);
	if (0 != pCfgZS && !pCfgZS->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZS");
		m_pCpInterfaceZS = new CProtocolCpSvr<CProcessInterfaceZS>();
		m_pCpInterfaceZS->Bind(this,EnumKeyIfZS);
		m_pCpInterfaceZS->Init(pCfgZS);
	}

	CConfig *pCfgZC1;
	pCfgZC1 = m_pConfig->GetProperties(gc_sCfgIfZC1);
	if (0 != pCfgZC1 && !pCfgZC1->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�ZC1");
		m_pCpInterfaceZC1 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC1->Bind(this,EnumKeyIfZC1);
		m_pCpInterfaceZC1->Init(pCfgZC1);

		CRLog(E_NOTICE,"��ʼ�����ӵ�ServiceHandler1");
		m_pServiceHandlerCln1 = new CServiceHandlerCln();
		m_pServiceHandlerCln1->Bind(this,EnumKeyServiceHandlerCln1);
		m_pServiceHandlerCln1->Init(pCfgZC1);
	}


	CConfig *pCfgReader;
	pCfgReader = m_pConfig->GetProperties(gc_sCfgReader);
	if (0 != pCfgReader && !pCfgReader->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ��Reader");
		m_pReader = new CXQueueIo<CBinBlockPkt>(this);
		m_pReader->Init(pCfgReader);
	}

	CConfig *pCfgWriter;
	pCfgWriter = m_pConfig->GetProperties(gc_sCfgWriter);
	if (0 != pCfgWriter && !pCfgWriter->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ��Writer");
		m_pIoServiceWriter = new CXQueueIo<CBinBlockPkt>();
		m_pIoServiceWriter->Init(pCfgWriter);
	}

	std::string sNameConvertFile;
	sNameConvertFile = DEFUALT_CONF_PATH PATH_SLASH;
	sNameConvertFile = sNameConvertFile + "NameConvert";
	sNameConvertFile = sNameConvertFile + ".cfg";
	
	m_oNameConvertFile.Load(sNameConvertFile);
	vector<string> vKeys = m_oNameConvertFile.GetKeys();
	for (vector<string>::iterator it = vKeys.begin(); it != vKeys.end(); ++it)
	{
		string sTmp;
		if (0 == m_oNameConvertFile.GetProperty(*it, sTmp))
		{
			m_mapNamePair[*it] = sTmp;
		}
	}

	std::string sInfoFile;
	sInfoFile = DEFUALT_CONF_PATH PATH_SLASH;
	sInfoFile = sInfoFile + "Info";
	sInfoFile = sInfoFile + ".dat";
	if (TRUE == m_oBasicInf.Create(sInfoFile.c_str()))
	{
		unsigned int uiDate = m_oBasicInf.TradeDate();
		if (0 != m_pHisDataHandler)
		{
			m_pHisDataHandler->SwitchTradeDate(uiDate);
		}
	}

	////H1�ӿں�H2�ӿ�
	//CConfig *pCfgH1;
	//pCfgH1 = m_pConfig->GetProperties(gc_sCfgIfH1);
	//if (0 != pCfgH1 && !pCfgH1->IsEmpty())
	//{
	//	CRLog(E_NOTICE,"��ʼ�����ӵ�H1");
	//	m_pCpInterfaceH1 = new CProtocolCpCli<CProcessInterfaceH1C>();
	//	m_pCpInterfaceH1->Bind(this,EnumKeyIfH1);
	//	m_pCpInterfaceH1->Init(pCfgH1);
	//}
	//
	//CConfig *pCfgH2;
	//pCfgH2 = m_pConfig->GetProperties(gc_sCfgIfH2);
	//if (0 != pCfgH2 && !pCfgH2->IsEmpty())
	//{
	//	CRLog(E_NOTICE,"��ʼ�����ӵ�H2");
	//	m_pCpInterfaceH2 = new CProtocolCpCli<CProcessInterfaceH2C>();
	//	m_pCpInterfaceH2->Bind(this,EnumKeyIfH2);
	//	m_pCpInterfaceH2->Init(pCfgH2);
	//}
	

	//��ʼ������·�ɱ�
	InitRouterTbl();
	return 0;
}

//�����ӵ�����
int CProviderCpMgr::Start()
{
	CRLog(E_NOTICE,"������ʱ��������");
	m_pGessTimerMgr->Start();
	CRLog(E_NOTICE,"������ʱ��������");
	CAbsTimerMgrWin32::Instance()->Start();

	//������������ģ��
	//if (0 != m_pNetMagModule)
	//{
	//	CRLog(E_NOTICE,"�������ܴ���ģ��");
	//	m_pNetMagModule->Start();
	//}

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

	if (0 != m_pHisDataHandlerFx)
	{
		CRLog(E_NOTICE,"����HisDataHandlerFx");
		m_pHisDataHandlerFx->Start();
	}

	if (0 != m_pServiceHandlerSvr)
	{
		CRLog(E_NOTICE,"����ServiceHandler");
		m_pServiceHandlerSvr->Start();
	}

	if (0 != m_pCpInterfaceZS)
	{
		CRLog(E_NOTICE,"�������ӵ�ZS");
		m_pCpInterfaceZS->Start();
	}

	if (0 != m_pServiceHandlerCln1)
	{
		CRLog(E_NOTICE,"����ServiceHandler1");
		m_pServiceHandlerCln1->Start();
	}

	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC1");
		m_pCpInterfaceZC1->Start();
	}

	if (0 != m_pIoServiceWriter)
	{
		CRLog(E_NOTICE,"����Writer");
		m_pIoServiceWriter->Start();
	}

	if (0 != m_pReader)
	{
		CRLog(E_NOTICE,"����Reader");
		m_pReader->Start();
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

//ֹͣ�����ӵ�
void CProviderCpMgr::Stop()
{	
	//ֹͣ��ʱ��������
	CRLog(E_NOTICE,"ֹͣ��ʱ��������");
	m_pGessTimerMgr->Stop();
	CRLog(E_NOTICE,"ֹͣ��ʱ��������abs");
	CAbsTimerMgrWin32::Instance()->Stop();

	m_oNetLogThread.EndThread();
	m_oCmdLineThread.EndThread();

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


	if (0 != m_pIoServiceWriter)
	{
		CRLog(E_NOTICE,"ֹͣWriter");
		m_pIoServiceWriter->Stop();
	}

	if (0 != m_pReader)
	{
		CRLog(E_NOTICE,"ֹͣReader");
		m_pReader->Stop();
	}

	
	if (0 != m_pServiceHandlerCln1)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler1");
		m_pServiceHandlerCln1->Stop();
	}
	
	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZC1");
		m_pCpInterfaceZC1->Stop();
	}

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

	if (0 != m_pHisDataHandlerFx)
	{
		CRLog(E_NOTICE,"ֹͣHisDataHandlerFx");
		m_pHisDataHandlerFx->Stop();
	}

	if (0 != m_pServiceHandlerSvr)
	{
		CRLog(E_NOTICE,"ֹͣServiceHandler");
		m_pServiceHandlerSvr->Stop();
	}
	
	if (0 != m_pCpInterfaceZS)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�ZS");
		m_pCpInterfaceZS->Stop();
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
	//if (0 != m_pNetMagModule)
	//{
	//	CRLog(E_NOTICE,"ֹͣ���ܴ���ģ��");
	//	m_pNetMagModule->Stop();
	//}
}

//��������
void CProviderCpMgr::Finish()
{
	m_pGessTimerMgr->Finish();
	m_pGessTimerMgr=0;
	CAbsTimerMgrWin32::Instance()->Finish();

		if (0 != m_pIoServiceWriter)
	{
		CRLog(E_NOTICE,"����Writer");
		m_pIoServiceWriter->Finish();
	}

	if (0 != m_pReader)
	{
		CRLog(E_NOTICE,"����Reader");
		m_pReader->Finish();
		delete m_pReader;
		m_pReader = 0;
	}

	if (0 != m_pServiceHandlerCln1)
	{
		CRLog(E_NOTICE,"����ServiceHandler1");
		m_pServiceHandlerCln1->Finish();
		delete m_pServiceHandlerCln1;
		m_pServiceHandlerCln1 = 0;
	}
	
	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"�������ӵ�ZC1");
		m_pCpInterfaceZC1->Finish();
		delete m_pCpInterfaceZC1;
		m_pCpInterfaceZC1 = 0;
	}

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

	if (0 != m_pHisDataHandlerFx)
	{
		CRLog(E_NOTICE,"����HisDataHandlerFx");
		m_pHisDataHandlerFx->Finish();
		delete m_pHisDataHandlerFx;
		m_pHisDataHandlerFx = 0;
	}

	if (0 != m_pServiceHandlerSvr)
	{
		CRLog(E_NOTICE,"����ServiceHandler");
		m_pServiceHandlerSvr->Finish();
		delete m_pServiceHandlerSvr;
		m_pServiceHandlerSvr = 0;
	}


	if (0 != m_pCpInterfaceZS)
	{
		CRLog(E_NOTICE,"�������ӵ�ZS");
		m_pCpInterfaceZS->Finish();	
		delete m_pCpInterfaceZS;
		m_pCpInterfaceZS = 0;
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

	//if (0 != m_pNetMagModule)
	//{
	//	CRLog(E_NOTICE,"�������ܴ���ģ��");
	//	m_pNetMagModule->Finish();
	//	delete m_pNetMagModule;
	//	m_pNetMagModule=0;
	//}
	
	CLogger::Instance()->Finish();
	delete m_pConfig;

	//CNetMgr::Instance()->NmFinish();

	//
	m_oMemShareAlive.UnMap();

	m_oBasicInf.Close();
}

//����ת������ ����ֵ-2��ʾ��·��
int CProviderCpMgr::Forward(CPacket &pkt,const unsigned long& ulKey)
{
	try
	{	
		int nRtn = -2;
		assert(EnumKeyUnknown > ulKey);
		if (EnumKeyUnknown <= ulKey)
			return -1;

		if (m_bStop)
			return 0;

		std::string sCmdID = pkt.GetCmdID();

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
		CRLog(E_ERROR,"packet error!");
		return -1;
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


//�������̺߳��� �����߳��˳�������˳�
int CProviderCpMgr::Run()
{
	try
	{
		MemSendLogin();

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
string CProviderCpMgr::OnCmd(const string& sCmdLine, const vector<string>& vecPara)
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
		sRtn += "Provider->";
		return sRtn;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		string sRtn = "\r\nProvider->";
		return sRtn;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		string sRtn = "\r\nProvider->";
		return sRtn;
	}
}

//telnet�ն������
int CProviderCpMgr::OnPacketCmd(CPacket& pkt)
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
			sRsp += "Provider->";
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

				sRsp += "Provider->";
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

				sRsp += "Provider->";
			}
			else
			{
				sRsp = "Parameter err!";
				sRsp += "\r\n";
				sRsp += "Provider->";
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
void CProviderCpMgr::OnNetLogMsg(const string& sMsg)
{
	m_oNetLogThread.Enque(sMsg);
}

//��־����
int CProviderCpMgr::HandleNetLogMsg(const string & sNetLogMsg)
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
int CProviderCpMgr::HandleCmdLine(string& sIn)
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
int CProviderCpMgr::OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan)
{
	m_oMemShareAlive.IamAlive();
	if (m_oMemShareAlive.IsIQuitCmd())
	{
		//�¼�֪ͨ
		string sEvtContent = "֪ͨ�˳�,�Ժ�ʼ����!";

		CRLog(E_NOTICE,sEvtContent.c_str());
		NotifyEvent(sEvtContent);

		m_bStop = true;
		m_deqCondMutex.Signal();
		return -1;
	}
	return 0;
}

//��ʱ������ʱ���ص��ӿ�
int CProviderCpMgr::OnResetTimeout(const string& sTmKey)
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
string CProviderCpMgr::OnCmdLineHelp(const string& sCmdLine, const vector<string>& vecPara)
{
	string sRtn("");
	int nSize = sizeof(m_CmdLine2Api)/sizeof(CmdLine2Api);
	for ( int i = 0 ; i < nSize ; i++ )
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
	sRtn += "Provider->";

	return sRtn;
}

string CProviderCpMgr::OnCmdLineReplayQuotation(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Provider->\r\n";
	if (0 != m_pDeliverMgr)
		sRtn = m_pDeliverMgr->HandleCmdLine(sCmd, vecPara);

	return sRtn;
}

//����������ʾ
string CProviderCpMgr::OnCmdLineBuffer(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Cvg->";
	if (0 != m_pDeliverMgr)
		sRtn = m_pDeliverMgr->HandleCmdLine(sCmd, vecPara);

	return sRtn;
}

//���¼��غ�Լÿ�ֵ�λ�����ļ�
string CProviderCpMgr::OnCmdLineLoad(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Cvg->";
	if (0 != m_pDeliverMgr)
		sRtn = m_pDeliverMgr->HandleCmdLine(sCmd, vecPara);

	return sRtn;
}

//quit�����
string CProviderCpMgr::OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara)
{
	//�¼�֪ͨ
	string sEvtContent = "�����з���Quit�˳�ָ��,��Լ3����˳�!";
	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);
	msleep(3);

	m_bStop = true;
	m_deqCondMutex.Signal();

	string sRtn = "Provider->";
	return sRtn;
}

string CProviderCpMgr::OnCmdLineSysInfo(const string& sCmdLine, const vector<string>& vecPara)
{
	string sRtn = CSelectorIo::Instance()->ToString();
	sRtn += "\r\n";
	sRtn += CSelectorListen::Instance()->ToString();
	sRtn += "\r\n";
	sRtn += CGessTimerMgrImp::Instance()->ToString();
	sRtn += "Provider->";
	return sRtn;
}

//��ʾ�ڴ�
string CProviderCpMgr::OnCmdLineMem(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Provider->\r\n";

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

int CProviderCpMgr::HandleConsolMsg(unsigned int uiMsg)
{
	
	//�¼�֪ͨ
	string sEvtContent = "";
	switch (uiMsg)
	{
	case CTRL_CLOSE_EVENT:
		sEvtContent = "Provider����̨���ڱ�ǿ�ƹر�,�����˳�Ӧ��!";
		break;
	case CTRL_SHUTDOWN_EVENT:
		sEvtContent = "Provider������ػ�,�����˳�Ӧ��!";
		break;
	default:
		return 0;
	}
	
	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);

	m_bStop = true;
	m_deqCondMutex.Signal();
	return 0;
}

void CProviderCpMgr::NotifyEvent(const string& sEvt)
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

string CProviderCpMgr::OnCmdLineEvtTest(const string& sCmd, const vector<string>& vecPara)
{
	//�¼�֪ͨ
	string sEvtContent = "�¼�����!";
	NotifyEvent(sEvtContent);

	string sRtn = "Provider->";
	return sRtn;
}

//���г���
string CProviderCpMgr::OnCmdLineQue(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Provider->";
	
	return sRtn;
}

int CProviderCpMgr::HandleStartNtf(const char* pBuf, unsigned int uiLen)
{
	PSTART_NTF pStartNtf = (PSTART_NTF)pBuf;
	//cout << pStartNtf->StartDate << " " << pStartNtf->StartTime << endl;
	CRLog(E_APPINFO, "OffStartNtf:%s,%s", pStartNtf->StartDate, pStartNtf->StartTime);
	
	MemSendLogin();
	return 0;
}

int CProviderCpMgr::HandleBasicInfo(const char* pBuf, unsigned int uiLen)
{
	PBASIC_INFO pInfo = (PBASIC_INFO)pBuf;
	//cout << pInfo->memberID << " " << pInfo->traderID << endl;
	CRLog(E_APPINFO, "BasicInfo:%s,%s,%s,%s", pInfo->memberID, pInfo->traderID, pInfo->exchDate, pInfo->ts);

	if (0 != pInfo->exchDate[0])
	{
		unsigned int uiDate = atoi(pInfo->exchDate);
		int nRtn = m_oBasicInf.SwitchTradeDate(uiDate);
		if(0 == nRtn)
		{
			if (0 != m_pDeliverMgr)
			{
				m_pDeliverMgr->MarketClose();
			}
			
			if (0 != m_pHisDataHandler)
			{
				m_pHisDataHandler->SwitchTradeDate(uiDate);
			}
		}
	}
	return 0;
}

int CProviderCpMgr::HandleInstState(const char* pBuf, unsigned int uiLen)
{
	PINST_STATE pInstState = (PINST_STATE)(pBuf);
	CRLog(E_APPINFO, "InstState: InstID=%s,Market=%c,state=%c", pInstState->instID, pInstState->marketID, pInstState->tradeState);

	m_oBasicInf.SetInstState(pInstState->instID, pInstState->tradeState, pInstState->marketID);
	if (I_END == pInstState->tradeState && 0 != m_pDeliverMgr)
	{
		m_pDeliverMgr->MarketClose();
	}
	return 0;
}

int CProviderCpMgr::HandleQuotation(const char* pBuf, unsigned int uiLen)
{
	QUOTATION stQuotation = *(QUOTATION*)(pBuf);
	string sInstID = stQuotation.m_CodeInfo.m_acCode;
	int nRtn = m_oBasicInf.IsSeqNo(sInstID, stQuotation.m_uiSeqNo, stQuotation.m_uiClose);
	if (0 != nRtn)
	{
		if (-3 == nRtn)
		{
			stQuotation.m_uiLast = stQuotation.m_uiClose;
			CRLog(E_DEBUG, "Close Inst=%s, Last=%u, Close=%u, Settle=%u, Avg=%u, LastClose=%u, LastSettle=%u, seqno=%u", sInstID.c_str(), stQuotation.m_uiLast, stQuotation.m_uiClose, stQuotation.m_uiSettle, stQuotation.m_uiAverage, stQuotation.m_uilastClose, stQuotation.m_uiLastSettle, stQuotation.m_uiSeqNo);
			CRLog(E_DEBUG, "%u\t%u\r\n%u\t%u\r\n%u\t%u\r\n%u\t%u\r\n%u\t%u\r\n-----------\r\n%u\t%u\r\n%u\t%u\r\n%u\t%u\r\n%u\t%u\r\n%u\t%u", 
				stQuotation.m_Ask[4].m_uiPrice, stQuotation.m_Ask[4].m_uiVol,
				stQuotation.m_Ask[3].m_uiPrice, stQuotation.m_Ask[3].m_uiVol,
				stQuotation.m_Ask[2].m_uiPrice, stQuotation.m_Ask[2].m_uiVol,
				stQuotation.m_Ask[1].m_uiPrice, stQuotation.m_Ask[1].m_uiVol,
				stQuotation.m_Ask[0].m_uiPrice, stQuotation.m_Ask[0].m_uiVol,
				stQuotation.m_Bid[0].m_uiPrice, stQuotation.m_Bid[0].m_uiVol,
				stQuotation.m_Bid[1].m_uiPrice, stQuotation.m_Bid[1].m_uiVol,
				stQuotation.m_Bid[2].m_uiPrice, stQuotation.m_Bid[2].m_uiVol,
				stQuotation.m_Bid[3].m_uiPrice, stQuotation.m_Bid[3].m_uiVol,
				stQuotation.m_Bid[4].m_uiPrice, stQuotation.m_Bid[4].m_uiVol);
		}
		else
		{
			if (-2 == nRtn)
			{
				if (sInstID == "Ag(T+D)" || sInstID == "Au(T+D)")
				{
					CRLog(E_DEBUG, "Inst=%s, Last=%u, Close=%u, Settle=%u, Avg=%u, LastClose=%u, LastSettle=%u, seqno=%u", sInstID.c_str(), stQuotation.m_uiLast, stQuotation.m_uiClose, stQuotation.m_uiSettle, stQuotation.m_uiAverage, stQuotation.m_uilastClose, stQuotation.m_uiLastSettle, stQuotation.m_uiSeqNo);
				
					int nHour = (stQuotation.m_uiTime/1000) / 10000;
					if (nHour > 15 && nHour < 16)
					{
						CRLog(E_DEBUG, "%u\t%u\r\n%u\t%u\r\n%u\t%u\r\n%u\t%u\r\n%u\t%u\r\n-----------\r\n%u\t%u\r\n%u\t%u\r\n%u\t%u\r\n%u\t%u\r\n%u\t%u", 
							stQuotation.m_Ask[4].m_uiPrice, stQuotation.m_Ask[4].m_uiVol,
							stQuotation.m_Ask[3].m_uiPrice, stQuotation.m_Ask[3].m_uiVol,
							stQuotation.m_Ask[2].m_uiPrice, stQuotation.m_Ask[2].m_uiVol,
							stQuotation.m_Ask[1].m_uiPrice, stQuotation.m_Ask[1].m_uiVol,
							stQuotation.m_Ask[0].m_uiPrice, stQuotation.m_Ask[0].m_uiVol,
							stQuotation.m_Bid[0].m_uiPrice, stQuotation.m_Bid[0].m_uiVol,
							stQuotation.m_Bid[1].m_uiPrice, stQuotation.m_Bid[1].m_uiVol,
							stQuotation.m_Bid[2].m_uiPrice, stQuotation.m_Bid[2].m_uiVol,
							stQuotation.m_Bid[3].m_uiPrice, stQuotation.m_Bid[3].m_uiVol,
							stQuotation.m_Bid[4].m_uiPrice, stQuotation.m_Bid[4].m_uiVol);
					}
				}
			}
			return 0;
		}
	}

	//��ԼIDת��
	ConvertInstID(sInstID);
	memset(stQuotation.m_CodeInfo.m_acCode, 0x00, sizeof(stQuotation.m_CodeInfo.m_acCode));
	memcpy(stQuotation.m_CodeInfo.m_acCode, sInstID.c_str(), sInstID.length());

	//�ַ���ʵʱ���鴦��
	if (0 != m_pDeliverMgr)
	{
		m_pDeliverMgr->Enque(stQuotation);	
	}
	return 0;
}

int CProviderCpMgr::HandleDeliveryQuotation(const char* pBuf, unsigned int uiLen)
{
	DeliveryQuotation* pstQuotation = (DeliveryQuotation*)pBuf;
	return 0;
}

//����wfj�����
int CProviderCpMgr::OnXQueuePkt(CBinBlockPkt& oBlockPkt)
{
	const char* pBuf = oBlockPkt.Buffer();
	if (0 == pBuf)
		return -1;

	unsigned int uiCmdID = oBlockPkt.GetCmdID();
	if (PKT_INST_STATE == uiCmdID)
	{
		HandleInstState(pBuf, oBlockPkt.BufLength());
		return 0;
	}
	else if (PKT_QUOTATION == uiCmdID)
	{
		HandleQuotation(pBuf, oBlockPkt.BufLength());
		return 0;
	}
	else if (PKT_DELIVERY_QUOTATION == uiCmdID)
	{
		HandleDeliveryQuotation(pBuf, oBlockPkt.BufLength());
		return 0;
	}
	else if (PKT_START_NTF == uiCmdID)
	{
		HandleStartNtf(pBuf, oBlockPkt.BufLength());
		return 0;
	}
	else if (PKT_BASIC_INFO == uiCmdID)
	{
		HandleBasicInfo(pBuf, oBlockPkt.BufLength());
		return 0;
	}
	return 0;
}

int CProviderCpMgr::ToHisData(const string& sQuotation)
{
	if (0 != m_pHisDataHandler)
		m_pHisDataHandler->Enque(sQuotation);

	return 0;
}

int CProviderCpMgr::ToHisDataFx(const string& sQuotation)
{
	if (0 != m_pHisDataHandlerFx)
		m_pHisDataHandlerFx->Enque(sQuotation);

	return 0;
}
void CProviderCpMgr::MemSendLogin()
{	
	LOGIN stLogin;
	unsigned int uiLen = sizeof(stLogin);
	memset(&stLogin, 0x00, uiLen);

	CBinBlockPkt oPkt(PKT_LOGIN);
	oPkt.FillBuf((char*)(&stLogin), uiLen);
		
	if (0 != m_pIoServiceWriter)
	{
		m_pIoServiceWriter->Enque(oPkt);
	}
}


//��ԼIDת��
void CProviderCpMgr::ConvertInstID(string& sInstID)
{
	map<string, string>::iterator it = m_mapNamePair.find(sInstID);
	if (it != m_mapNamePair.end())
	{
		sInstID = (*it).second;
	}
	else
	{
		if (sInstID.length() > 8)
		{
			sInstID = sInstID.substr(0,8);
		}
	}
}

int CProviderCpMgr::Query(CNMO& oNmo)
{
	int nRtn = -1;
	oNmo.m_nQuality=gc_nQuolityGood;

	return nRtn;
}