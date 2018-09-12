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
#include "ConfigImpl.h"
#include "strutils.h"
#include "DataSrcCpMgr.h"
#include <sstream>
#include <iomanip>

#include "OfferLoginMgr.h"


#include "Logger.h"
#include "ConfigImpl.h"
#include "BroadcastPacket.h"
#include "ProtocolConnectPoint.h"
#include "ProcessInterfaceMC.h"
#include "ProcessInterfaceMS.h"
#include "ProcessInterfaceH1C.h"
#include "ProcessInterfaceH2C.h"
#include "LinePacket.h"
#include "GessTimerMgrPosix.h"
#include "AbsTimerMgrWin32.h"
#include "NetMgrModule.h"
#include "MibConstant.h"

#include "STKDRV.H"

using namespace MibConst;

//Դ�ӿ�+������ ����·�����ñ�
CDataSrcCpMgr::IfRouterCfg CDataSrcCpMgr::m_tblIfRouterCfg[] = 
{
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


	//from EnumKeyIfMC
	//EnumKeyIfMC To default ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyTraderLoginMgr,	EnumKeyIfMC,    			 gc_sDefaultCmdID},

	//from EnumKeyTraderLoginMgr
	//EnumKeyTraderLoginMgr To default ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfMC,				EnumKeyTraderLoginMgr,    	 "StateInfo"},
	{0,    EnumKeyIfMS,				EnumKeyTraderLoginMgr,    	 "StateRsp"},


	//from EnumKeyIfMS
	//EnumKeyIfMS To default ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyTraderLoginMgr,	EnumKeyIfMS,    			 gc_sDefaultCmdID},


	//from IFCMD
	//IFCMD To default ȱʡ·��
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyCmdHandler,		EnumKeyIfCmd,    			 gc_sDefaultCmdID}
};



CDataSrcCpMgr::CDataSrcCpMgr()
:m_pCpInterfaceMC(0)
,m_pCpInterfaceMS(0)
,m_pOfferLoginMgr(0)
,m_pCpInterfaceH1(0)
,m_pCpInterfaceH2(0)
,m_pNetMagModule(0)
,m_uiNodeID(0)
,m_uiNodeType(0)
,m_bFwdStop(false)
,m_hwndOwner(0)
,m_uiDelayLess0s(0)
,m_uiDelayLess1s(0)
,m_uiDelayLess2s(0)
,m_uiDelayLess3s(0)
,m_uiDelayLess5s(0)
,m_uiDelayLess10s(0)
,m_uiDelayLess30s(0)
,m_uiDelayLess60s(0)
,m_uiDelayLess120s(0)
,m_uiDelayMore120s(0)
,m_uiDelayMin(0xFFFFFFFF)
,m_uiDelayMax(0)
,m_uiFwdTotal(0)
{
	m_pConfig = new CConfigImpl();
}

CDataSrcCpMgr::~CDataSrcCpMgr(void)
{
}

int CDataSrcCpMgr::Bind(long ulHwnd)
{
	m_hwndOwner = ulHwnd;
	return 0;
}

//���̻����ӵ��������ʼ��
int CDataSrcCpMgr::Init(const string& sProcName)
{

	cout << "���������ļ�..." << endl;

	std::string sCfgFilename;
	sCfgFilename = DEFUALT_CONF_PATH PATH_SLASH;
	sCfgFilename = sCfgFilename + "DataSrc_wjf";//m_sProcName;
	sCfgFilename = sCfgFilename + ".cfg";
	if (m_pConfig->Load(sCfgFilename) != 0) 
	{
		return -1;
	}

	//cout << "��ʼ����־..." << endl;

	// ��ʼ����־
	if (CLogger::Instance()->Initial(m_pConfig->GetProperties("logger")) != 0)
	{
		return -1;
	}

	//cout << "������־..." << endl;

	// ������־
	if (CLogger::Instance()->Start() != 0)
	{
		return -1;
	}

	string sTmp = "";
	if (0 == m_pConfig->GetProperty("node_id",sTmp))
		m_uiNodeID = FromString<unsigned int>(sTmp);

	if (0 == m_pConfig->GetProperty("node_type",sTmp))
		m_uiNodeType = FromString<unsigned int>(sTmp);
	
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

	CGessTimerMgr* pTimerMgr = CGessTimerMgrImp::Instance();
	pTimerMgr->Init(2);

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
	pTimerMgr->CreateTimer(&m_oIfkTimer,nInterval,"KHello");


	//��ʱ�������� "0,12:00:00"
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
	string sTblPrefix = "datasrc";
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

	//
	m_oNmoModule.Bind(this);

	unsigned int uiXQueNum = 2;
	if (0 == m_pConfig->GetProperty("XQUE_NUM", sTmp))
	{
		uiXQueNum = strutils::FromString<unsigned int>(sTmp);
		if (uiXQueNum > 10)
			uiXQueNum = 2;
	}

	for (unsigned int uiIndex = 1; uiIndex <= uiXQueNum; uiIndex++)
	{
		string sCfgName = "XQUE" + strutils::ToString<unsigned int>(uiIndex);

		CConfig *pCfgWriter;
		pCfgWriter = m_pConfig->GetProperties(sCfgName);
		if (0 != pCfgWriter && !pCfgWriter->IsEmpty())
		{
		}
		else
		{
			pCfgWriter = m_pConfig;
		}
		CRLog(E_APPINFO,"��ʼ��[%s]������", sCfgName.c_str());
		CXQueueIo<QUOTATION>* pWriter = new CXQueueIo<QUOTATION>();
		pWriter->Init(pCfgWriter);
		m_vecQueueIo.push_back(pWriter);

		CNetMgr::Instance()->Register(&m_oNmoModule,mibQueNum,mibQueNum+"." + sCfgName + "д�������");
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueFree,gc_sMemQueFree+"." + sCfgName);
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueUsed,gc_sMemQueUsed+"." + sCfgName);
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueTotal,gc_sMemQueTotal+"." + sCfgName);
	}


	if (0 == m_pConfig->GetProperty("wjf_name",sTmp))
		m_sWjfName = trim(sTmp);




	//��¼����ģ�鴴������ʼ��
	CRLog(E_NOTICE,"��ʼ��������¼����ģ��");
	m_pOfferLoginMgr = new CLoginMgr();
	CConfig *pCfgTraderLoginMgr = 0;
	pCfgTraderLoginMgr = m_pConfig->GetProperties(gc_sCfgTraderLoginMgr);
	if (0 == pCfgTraderLoginMgr)
		pCfgTraderLoginMgr = m_pConfig;
	m_pOfferLoginMgr->Bind(this,EnumKeyTraderLoginMgr);
	m_pOfferLoginMgr->Init(pCfgTraderLoginMgr);



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

	//MC��MS�ӿ�	
	CConfig *pCfgIfMS;
	pCfgIfMS = m_pConfig->GetProperties(gc_sCfgIfMS);
	if (0 != pCfgIfMS && !pCfgIfMS->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�MS");
		m_pCpInterfaceMS = new CProtocolCpSvr<CProcessInterfaceMS>();
		m_pCpInterfaceMS->Bind(this,EnumKeyIfMS);
		m_pCpInterfaceMS->Init(pCfgIfMS);
	}


	CConfig *pCfgIfMC = 0;
	pCfgIfMC = m_pConfig->GetProperties(gc_sCfgIfMC);
	if (0 != pCfgIfMC && !pCfgIfMC->IsEmpty())
	{
		CRLog(E_NOTICE,"��ʼ�����ӵ�MC");
		m_pCpInterfaceMC = new CProtocolCpCli<CProcessInterfaceMC>();
		m_pCpInterfaceMC->Bind(this,EnumKeyIfMC);
		m_pCpInterfaceMC->Init(pCfgIfMC);
	}	


	//��ʼ������·�ɱ�
	InitRouterTbl();

	
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sSamplerInPktTotal,gc_sSamplerInPktTotal +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sSamplerFwdPktTotal,gc_sSamplerFwdPktTotal +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayMin,gc_sDelayMin +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayMax,gc_sDelayMax +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess0s,gc_sDelayLess0s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess1s,gc_sDelayLess1s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess2s,gc_sDelayLess2s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess3s,gc_sDelayLess3s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess5s,gc_sDelayLess5s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess10s,gc_sDelayLess10s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess30s,gc_sDelayLess30s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess60s,gc_sDelayLess60s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess120s,gc_sDelayLess120s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayMore120s,gc_sDelayMore120s +".0");

    // ���Ӷ�ȡ��������, Jerry Lee, 2012-3-28
    // begin
    char buf[16] = {0};
    string saTmp;
    for (int i = 0; i < 7; i++)
    {   
        sprintf(buf, "FILTER.week%d", i+1);
        if (0 == m_pConfig->GetProperty(buf,saTmp))
        {
            m_dataFilter.timeFilters[i].set(saTmp);
        }
    }

    sprintf(buf, "FILTER.holiday");
    if (0 == m_pConfig->GetProperty(buf,saTmp))
    {
        m_dataFilter.setHoliday(saTmp);
    }

    // end


	return 0;
}

//�����ӵ�����
int CDataSrcCpMgr::Start()
{
	CRLog(E_NOTICE,"������ʱ��������");
	CGessTimerMgrImp::Instance()->Start();
	CRLog(E_NOTICE,"������ʱ��������");
	CAbsTimerMgrWin32::Instance()->Start();

	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>*  >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			CRLog(E_APPINFO,"����[XQUE%d]", nCount);
			(*it)->Start();
		}
	}


	int nRtn = 0;
	//������������ģ��
	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"�������ܴ���ģ��");
		m_pNetMagModule->Start();
	}


	
	if (0 != m_pOfferLoginMgr)
	{
		CRLog(E_NOTICE,"������������¼����ģ��");
		m_pOfferLoginMgr->Start();
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

	if (0 != m_pCpInterfaceMS)
	{
		CRLog(E_NOTICE,"�������ӵ�MS");
		int nRtn = m_pCpInterfaceMS->Start();

		if (0 != nRtn)
		{
			return -1;
		}

		CConfig *pCfgIfMS = m_pConfig->GetProperties(gc_sCfgIfMS);
		string sWaitAfterListen("2");
		unsigned int nWaitAfterListen = 2;
		if (0 == pCfgIfMS->GetProperty("wait_after_listen",sWaitAfterListen))
			nWaitAfterListen = FromString<unsigned int>(sWaitAfterListen);

		CRLog(E_DEBUG,"Listen������ʱ��:%d",nWaitAfterListen);
		msleep(nWaitAfterListen);
	}

	if (0 != m_pCpInterfaceMC)
	{
		CRLog(E_NOTICE,"�������ӵ�MC");
		m_pCpInterfaceMC->Start();
	}


	return 0;
}

//ֹͣ�����ӵ�
void CDataSrcCpMgr::Stop()
{
	CRLog(E_NOTICE,"ֹͣ��ʱ��������");
	CGessTimerMgr* pTimerMgr = CGessTimerMgrImp::Instance();
	pTimerMgr->Stop();
	CAbsTimerMgrWin32::Instance()->Stop();

	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>*  >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			CRLog(E_APPINFO,"ֹͣ[XQUE%d]", nCount);
			(*it)->Stop();
		}
	}


	if (0 != m_pCpInterfaceMC)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�MC");
		m_pCpInterfaceMC->Stop();
	}

	if (0 != m_pCpInterfaceMS)
	{
		CRLog(E_NOTICE,"ֹͣ���ӵ�MS");
		m_pCpInterfaceMS->Stop();
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

	if (0 != m_pOfferLoginMgr)
	{
		CRLog(E_NOTICE,"ֹͣ��������¼����ģ��");
		m_pOfferLoginMgr->Stop();
	}
}

//��������
void CDataSrcCpMgr::Finish()
{
	CGessTimerMgr* pTimerMgr = CGessTimerMgrImp::Instance();
	pTimerMgr->Finish();

	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>*  >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			CRLog(E_APPINFO,"����[XQUE%d]", nCount);
			(*it)->Finish();
			delete (*it);
		}
	}
	m_vecQueueIo.clear();

	if (0 != m_pCpInterfaceMC)
	{
		CRLog(E_NOTICE,"�������ӵ�MC");
		m_pCpInterfaceMC->Finish();
		delete m_pCpInterfaceMC;
		m_pCpInterfaceMC = 0;
	}

	if (0 != m_pCpInterfaceMS)
	{
		CRLog(E_NOTICE,"�������ӵ�MS");
		m_pCpInterfaceMS->Finish();
		delete m_pCpInterfaceMS;
		m_pCpInterfaceMS = 0;
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

	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"�������ܴ���ģ��");
		m_pNetMagModule->Finish();
		delete m_pNetMagModule;
		m_pNetMagModule=0;
	}

	if (0 != m_pOfferLoginMgr)
	{
		CRLog(E_NOTICE,"����������¼����ģ��");
		m_pOfferLoginMgr->Finish();
	}

	CLogger::Instance()->Finish();
	delete m_pConfig;
	m_pConfig = 0;

	CNetMgr::Instance()->NmFinish();
	CNetMgr::Instance()->UnRegisterModule(&m_oNmoModule);
	
	//
	m_oMemShareAlive.UnMap();
}


//����wfj�����
int CDataSrcCpMgr::OnRecvQuotation(QUOTATION& stQuotation, int nDelay)
{
    // ��ʱ�汾�������������Ͼŵ�������һ�����֮������ݲ�ת��, Jerry Lee, 2012-3-28
    /*
    �������ļ�������������Ϣ
    FILTER.week1=12:00-15:59;21:00-23:59;00:00-5:59
    FILTER.week2=12:00-15:59;21:00-23:59;00:00-5:59
    FILTER.week3=12:00-15:59;21:00-23:59;00:00-5:59
    FILTER.week4=21:00-23:59;00:00-5:59;12:00-15:11
    FILTER.week5=12:00-15:59;21:00-23:59;00:00-5:59
    FILTER.week6=12:00-15:59;21:00-23:59;00:00-5:59
    FILTER.week7=12:00-15:59;21:00-23:59;00:00-5:59
    FILTER.holiday=20120307,20120328,20120329
    */
    SYSTEMTIME stNow;
    GetLocalTime(&stNow);

    if (m_dataFilter.isFilter(stNow.wYear, stNow.wMonth, stNow.wDay, 
        stNow.wDayOfWeek, stNow.wHour, stNow.wMinute))
    {
        CRLog(E_APPINFO,"�����ǽ���ʱ�����������!");

        return -1;
    }
    /*
    if ((stNow.wDayOfWeek == 0 && stNow.wHour >= 21 && stNow.wHour <= 23)
    || (stNow.wDayOfWeek == 1 && stNow.wHour >= 0 && stNow.wHour < 6))
    {
    CRLog(E_APPINFO,"�����ǽ���ʱ�����������, �г�����Ϊ: %d!", uiMarketType);

    return -1;
    }
    */
    //

    if (nDelay < 0)
	{
		m_uiDelayLess0s++;
	}
	else if (nDelay <= 1)
	{
		m_uiDelayLess1s++;
	}
	else if (nDelay <= 2)
	{
		m_uiDelayLess2s++;
	}
	else if (nDelay <= 3)
	{
		m_uiDelayLess3s++;
	}
	else if (nDelay <= 5)
	{
		m_uiDelayLess5s++;
	}
	else if (nDelay <= 10)
	{
		m_uiDelayLess10s++;
	}
	else if (nDelay <= 30)
	{
		m_uiDelayLess30s++;
	}
	else if (nDelay <= 60)
	{
		m_uiDelayLess60s++;
	}
	else if (nDelay <= 120)
	{
		m_uiDelayLess120s++;
	}
	else
	{
		m_uiDelayMore120s++;
	}

	if (nDelay >= 0 && nDelay < m_uiDelayMin)
	{
		m_uiDelayMin = nDelay;
	}
	if (nDelay > 0 && nDelay > m_uiDelayMax)
	{
		m_uiDelayMax = nDelay;
	}
	
	//
	m_uiFwdTotal++;

	//�ַ�����Ӧ���д���
	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>*  >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			(*it)->Enque(stQuotation);
		}
	}
	return 0;
}






//�ͻ���Э�����ӵ����ӳɹ���ص�
int CDataSrcCpMgr::OnConnect(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;


	//֪ͨ������¼����ģ�����ӶԷ����
	if (EnumKeyIfMC == ulKey)
	{
		m_pOfferLoginMgr->ConnectNtf(nFlag);
	}

	return 0;
}

//�����Э�����ӵ���յ����Ӻ�ص�
int CDataSrcCpMgr::OnAccept(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;

	return 0;
}

int CDataSrcCpMgr::OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	return 0;
}

int CDataSrcCpMgr::OnClose(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;

	//֪ͨ������¼����ģ�����ӶԷ����
	if (EnumKeyIfMC == ulKey)
	{
		//CRLog
		m_pOfferLoginMgr->DisconnectNtf();
	}

	return 0;
}

//��ʼ��·�ɱ�
int CDataSrcCpMgr::InitRouterTbl()
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
		case EnumKeyIfH1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceH1));
			break;
		case EnumKeyIfH2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceH2));
			break;
		case EnumKeyIfMC:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceMC));
			break;
		case EnumKeyIfMS:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceMS));
			break;
		case EnumKeyTraderLoginMgr:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pOfferLoginMgr));
			break;
		case EnumNetMagModule:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pNetMagModule));
		default:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, 0));
			break;
		}
	}

	return 0;
}


void CDataSrcCpMgr::NotifyEvent(const string& sEvt, int nGrade)
{
	//�¼�֪ͨ
	CEventSimple e;
	e.m_nEvtCategory = 0;
	e.m_nEvtType = 0;
	e.m_nGrade = nGrade;
	e.m_sDateTime = CGessDate::NowToString("-") + " " + CGessTime::NowToString(":");		
	e.m_sEvtContent = sEvt;
	CNetMgr::Instance()->OnEvtNotify(e);
}

//����ת������ ����ֵ-2��ʾ��·��
int CDataSrcCpMgr::Forward(CPacket &pkt,const unsigned long& ulKey)
{
	try
	{	
		int nRtn = -2;
		assert(EnumKeyUnknown > ulKey);
		if (EnumKeyUnknown <= ulKey)
			return -1;

		if (m_bFwdStop)
			return 0;

		std::string sCmdID = pkt.GetCmdID();

		bool blFound = false;
		MMAP_IT it;
		RANGE_CP range = m_tblIfRouter[ulKey].mmapCmds.equal_range(sCmdID);
		for (it = range.first; it != range.second; ++it)
		{
			if (0 != (*it).second)
			{
				(*it).second->SendPacket(pkt);
				nRtn = 0;
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
					(*it).second->SendPacket(pkt);
					nRtn = 0;
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



bool CDataSrcCpMgr::Logout()
{
	string sEvt = "����һ��ʱ��û���յ�ҵ����,������������,ָʾ�ǳ�!";
	NotifyEvent(sEvt,0);
	CRLog(E_APPINFO, "%s", sEvt.c_str());

	bool bRet = m_pOfferLoginMgr->LogoutInd();

	Stock_Dll_Release();
	::PostMessage((HWND)m_hwndOwner, WM_CLOSE, 0, 0);

	return bRet;
}

bool CDataSrcCpMgr::Stock_Dll_Init()
{
	NotifyEvent("wjf Stock_Dll_Init", 0);
	CRLog(E_APPINFO, "wjf Stock_Dll_Init");

	::PostMessage((HWND)m_hwndOwner, WM_MSG_STOCKDLL_INIT, 0, 0);
	return true;
}
bool CDataSrcCpMgr::Stock_Dll_Release()
{
	NotifyEvent("wjf Stock_Dll_Release", 1);
	CRLog(E_APPINFO, "wjf Stock_Dll_Release");

	::PostMessage((HWND)m_hwndOwner, WM_MSG_STOCKDLL_RELEASE, 0, 0);
	return true;
}


//K�ӿ�������ʱ���ص��ӿ�
int CDataSrcCpMgr::OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan)
{
	m_oMemShareAlive.IamAlive();	
	if (m_oMemShareAlive.IsIQuitCmd())
	{
		//�¼�֪ͨ
		string sEvtContent = "��ʱʱ�䵽,�Ժ�ʼ����!";

		CRLog(E_NOTICE,sEvtContent.c_str());
		NotifyEvent(sEvtContent);
		Stock_Dll_Release();
		::PostMessage((HWND)m_hwndOwner, WM_CLOSE, 0, 0);

		//
		return -1;
	}

	static unsigned int guiTimerCount = 0;
	if (guiTimerCount % 5 == 0)
	{
		size_t nFree = 0;
		size_t nUsed = 0;
		size_t nTotal = 0;
		unsigned int uiIndex = 1;
		vector<CNMO> vNmo;
		for (vector< CXQueueIo<QUOTATION>* >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
		{
			CNMO oNmo;
			if (0 == (*it)->GetBlockInf(nFree, nUsed, nTotal))			
			{
				oNmo.m_sOid=gc_sMemQueFree;
				oNmo.m_sOidIns = gc_sMemQueFree+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex);
				oNmo.m_sValue=ToString(nFree);
				vNmo.push_back(oNmo);

				oNmo.m_sOid=gc_sMemQueUsed;
				oNmo.m_sOidIns = gc_sMemQueUsed+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex);
				oNmo.m_sValue=ToString(nUsed);
				vNmo.push_back(oNmo);


				oNmo.m_sOid=gc_sMemQueTotal;
				oNmo.m_sOidIns = gc_sMemQueTotal+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex);
				oNmo.m_sValue=ToString(nTotal);
				vNmo.push_back(oNmo);

				CNetMgr::Instance()->Report(vNmo);
			}			
			uiIndex++;
		}
	}
	guiTimerCount++;
	return 0;
}

//��ʱ������ʱ���ص��ӿ�
int CDataSrcCpMgr::OnResetTimeout(const string& sTmKey)
{
	//�¼�֪ͨ
	string sEvtContent = "��ʱʱ�䵽,�Ժ�ʼ����!";

	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);

	//
	return 0;
}


int CDataSrcCpMgr::Query(CNMO& oNmo)
{
	int nRtn = -1;
	oNmo.m_nQuality=gc_nQuolityGood;
	//oNmo.m_sTimeStamp=CGessDateTime::NowToString()+" "+CGessTime::NowToString(":");

	if (oNmo.m_sOid == gc_sDelayMin)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayMin);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayMax)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayMax);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess0s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess0s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess1s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess1s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess2s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess2s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess3s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess3s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess5s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess5s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess10s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess10s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess30s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess30s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess60s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess60s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess120s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess120s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayMore120s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayMore120s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sSamplerInPktTotal)
	{
		oNmo.m_sValue=ToString<unsigned int>(0);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sSamplerFwdPktTotal)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiFwdTotal);
		nRtn = 0;
		return nRtn;
	}

	size_t nFree = 0;
	size_t nUsed = 0;
	size_t nTotal = 0;
	unsigned int uiIndex = 1;
	for (vector< CXQueueIo<QUOTATION>* >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		if(oNmo.m_sOidIns==mibQueNum+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex) + "д�������")
		{
			oNmo.m_sValue=ToString((*it)-> QueueLen());
			nRtn = 0;
			break;
		}
		else if (oNmo.m_sOidIns==gc_sMemQueFree+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex))
		{
			if (0 == (*it)->GetBlockInf(nFree, nUsed, nTotal))
			{
				oNmo.m_sValue=ToString(nFree);
				nRtn = 0;
			}
			break;
		}
		else if (oNmo.m_sOidIns==gc_sMemQueUsed+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex))
		{
			if (0 == (*it)->GetBlockInf(nFree, nUsed, nTotal))
			{
				oNmo.m_sValue=ToString(nUsed);
				nRtn = 0;
			}
			break;
		}
		else if (oNmo.m_sOidIns==gc_sMemQueTotal+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex))
		{
			if (0 == (*it)->GetBlockInf(nFree, nUsed, nTotal))
			{
				oNmo.m_sValue=ToString(nTotal);
				nRtn = 0;
			}
			break;
		}
		uiIndex++;
	}
	return nRtn;
}
