#ifndef _DATA_PROVIDER_CP_MGR_H
#define _DATA_PROVIDER_CP_MGR_H
#include "netlogdev.h"
#include "IpcPacket.h"
#include "Comm.h"
#include "WorkThreadNm.h"
#include "SamplerPacket.h"
#include "XQueueIo.h"
#include <vector>
#include <string>
#include <iostream>
#include "ConfigImpl.h"
#include "MemShareAlive.h"
#include "PktBinBlock.h"
#include "BasicInfoMemFile.h"

using namespace std;

//���ӵ�key����
typedef enum tagEnumKey
{
	EnumKeyDeliverMgr,
	EnumKeyServiceHandlerSvr,
	EnumKeyIfZS,
	EnumKeyServiceHandlerCln1,
	EnumKeyIfZC1,
	EnumKeyIfH1,
	EnumKeyIfH2,
	EnumKeyIfCmd,
	EnumKeyCmdHandler,
	EnumNetMagModule,
	EnumKeySelfIpc,
	EnumKeyUnknown
} EnumKeyIf;

//�������ӵ���������
const string gc_sCfgWriter = "XQUEUE_SVR";
const string gc_sCfgReader = "XQUEUE_QUO";
const string gc_sCfgDeliver = "IFDELIVER";
const string gc_sCfgService = "IFSERVICE";
const string gc_sCfgHisData = "IFHISDATA";
const string gc_sCfgHisDataFx = "IFHISDATAFX";
const string gc_sCfgIfZS = "IFZS";
const string gc_sCfgIfZC1 = "IFZC1";
const string gc_sCfgIfH1 = "IFH1";
const string gc_sCfgIfH2 = "IFH2";
const string gc_sCfgIfCmd = "IFCMD";
const string gc_sCfgNetMagModule = "net_mgr";

//ȱʡ������ƥ��
const string gc_sDefaultCmdID = "#";


//���ӽ�������  
typedef struct
{
	char			instID[16];				//��Լ����
	unsigned int	bidLot;					//����
	unsigned int	askLot;					//����
	unsigned int	midBidLot;				//����������
	unsigned int	midAskLot;				//����������
} DeliveryQuotation, *PDeliveryQuotation;

//����֪ͨ
typedef struct
{
	char  StartDate[12];				//��������
	char  StartTime[12];					//����ʱ��
} START_NTF,*PSTART_NTF;

//������Ϣ
typedef struct
{
	char  memberID[6];				//��ԱID
	char  traderID[10];					//����ԱID
	char  exchDate[12];
	char  ts[16];
} BASIC_INFO,*PBASIC_INFO;


//��Լ����״̬��Ϣ  
typedef struct
{
	char  instID[12];				//��Լ����
	char  marketID;					//�г�
	char  tradeState;				//��Լ����״̬
} INST_STATE,*PINST_STATE;

typedef struct
{
	char  ts[12];					//ʱ���
} LOGIN,*PLOGIN;

#define	PKT_INST_STATE				0x00000001
#define	PKT_QUOTATION				0x00000002
#define	PKT_DELIVERY_QUOTATION		0x00000003

#define	PKT_START_NTF				0x0000000A
#define	PKT_BASIC_INFO				0x0000000B

#define	PKT_LOGIN					0x80000001


class CGessTimerMgr;
class CConfigImpl;
class CNetMgrModule;
class CHisDataHandler;
class CHisDataHandlerFx;
class CDeliverMgr;
class CProviderCpMgr:public CProtocolCpMgr, public CXQueueCallback<CBinBlockPkt>
{
private:
	int Query(CNMO& oNmo) ;
	class CCpMgrNm: public CNmoModule
	{
	public:
		CCpMgrNm():m_pParent(0){}
		virtual ~CCpMgrNm(){}
		void Bind(CProviderCpMgr* pParent){m_pParent = pParent;}
		//������ѯ�ӿ�
		int Query(CNMO& oNmo) const
		{
			if (0 != m_pParent)
				return m_pParent->Query(oNmo);
			return -1;
		}

		//������ѯ�ӿ�
		int Query(vector< CNMO > & vNmo) const
		{
			for (vector< CNMO >::iterator it = vNmo.begin(); it != vNmo.end(); ++it)
			{
				Query(*it);
			}
			return 0;
		}

		//���ƽӿ�
		int Operate(const string &sOid, int nflag, const string &sDstValue, const string &sParamer) {return -1;}
	private:
		CProviderCpMgr * m_pParent;
	};

	//�����д����߳�
	class CCommandLineThread :public CWorkThreadNm
	{
	public:
		CCommandLineThread():m_pParent(0){}
		virtual ~CCommandLineThread(){}
		void Bind(CProviderCpMgr* p){m_pParent = p;}
	private:
		int ThreadEntry()
		{
			string sIn("");	
			cout << "Provider->";
			try
			{
				while(!m_bEndThread)
				{
					try
					{
						if (0 != m_pParent)
						{
							//�ȴ���������,����һֱ����,ֱ��������������߳�/�����˳�
							m_pParent->HandleCmdLine(sIn);
						}
						else
						{
							msleep(2);
						}
					}
					catch(...)
					{
						CRLog(E_ERROR,"Unknown exception!");
						msleep(1);
					}
				}

				CRLog(E_SYSINFO,"Provider_CmdLine_Thread exit!");
				return 0;
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
		int End()
		{
			return 0;
		}
		//�������߳�״̬�Ƿ���Ҫ������
		bool IsNetManaged(string& sKeyName)
		{
			sKeyName = "�����д����߳�";
			return true;
		}
	private:
		CProviderCpMgr* m_pParent;
	};

	//������־����Ϣ�����߳�
	class CNetLogThread :public CWorkThreadNm
	{
	public:
		CNetLogThread():m_pParent(0){}
		virtual ~CNetLogThread(){}
		void Bind(CProviderCpMgr* p){m_pParent = p;}
		int Enque(const string& sMsg)
		{
			m_deqCondMutex.Lock();
			m_deqLog.push_back(sMsg);
			m_deqCondMutex.Unlock();
			m_deqCondMutex.Signal();
			return 0;
		}
	private:
		int ThreadEntry()
		{
			try
			{
				while(!m_bEndThread)
				{
					m_deqCondMutex.Lock();
					while(m_deqLog.empty() && !m_bEndThread)
						m_deqCondMutex.Wait();
					
					if (m_bEndThread)
					{
						m_deqLog.clear();
						m_deqCondMutex.Unlock();
						break;
					}

					if ( !m_deqLog.empty())
					{
						string sMsg = m_deqLog.front();
						m_deqLog.pop_front();
						m_deqCondMutex.Unlock();
						
						if (0 == m_pParent)
							continue;

						try
						{
							m_pParent->HandleNetLogMsg(sMsg);
						}
						catch(std::exception e)
						{
							CRLog(E_ERROR,"exception:%s!",e.what());
						}
						catch(...)
						{
							CRLog(E_ERROR,"Unknown exception!");
						}
					}
				}

				CRLog(E_SYSINFO,"Provider_NetLog Thread exit!");
				return 0;
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
		int End()
		{
			m_deqCondMutex.Lock();
			m_deqCondMutex.Signal();
			m_deqCondMutex.Unlock();
			Wait();
			return 0;
		}
		//�������߳�״̬�Ƿ���Ҫ������
		bool IsNetManaged(string& sKeyName)
		{
			sKeyName = "������־��Ϣ�߳�";
			return true;
		}
	private:
		CProviderCpMgr* m_pParent;
		//��־��Ϣ����
		std::deque<string> m_deqLog;
		CCondMutex	m_deqCondMutex;
	};

	//telnet �����д������ӵ�
	class CConnectPointCmd:public CConnectPointAsyn
	{
	public:
		CConnectPointCmd():m_pMgr(0),m_ulKey(EnumKeyUnknown){}
		~CConnectPointCmd(){}
		int Init(CConfig* pConfig){return 0;}
		int Start(){return 0;}
		void Stop(){}
		void Finish(){}
		int OnRecvPacket(CPacket &GessPacket){return 0;}
		void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
		{
			m_pMgr=dynamic_cast<CProviderCpMgr*>(pCpMgr); 
			m_ulKey=ulKey;
		}
		int SendPacket(CPacket &GessPacket)
		{
			if (0==m_pMgr)
				return -1;
			return m_pMgr->OnPacketCmd(GessPacket);
		}
	private:
		CProviderCpMgr* m_pMgr;
		unsigned long m_ulKey;
	};

	//��־����
	class CNetLogHost:public CSubscriber
	{
	public:
		CNetLogHost():m_pMgr(0){}
		virtual ~CNetLogHost(){}
		void Bind(CProviderCpMgr* p){m_pMgr = p;}
		void OnNotify(const string& sMsg)
		{
			if (0 != m_pMgr)
				m_pMgr->OnNetLogMsg(sMsg);
		}
	private:
		CProviderCpMgr* m_pMgr;
	};

	//dog������ʱ��
	class CDogTimer : public CGessTimer
	{
	public:
		CDogTimer():m_pParent(0){}
		virtual ~CDogTimer(){}
		void Bind(CProviderCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnDogTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CProviderCpMgr* m_pParent;
	};

	//��ʱ������ʱ��
	class CResetTimer: public CGessAbsTimer
	{
	public:
		CResetTimer():m_pParent(0){}
		~CResetTimer(void){}
		void Bind(CProviderCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& sKey)
		{
			if (0 != m_pParent)
				return m_pParent->OnResetTimeout(sKey);
			return -1;
		}
		void TimerCanceled(const string& ulKey){}
	private:
		CProviderCpMgr* m_pParent;
	};
public:
	CProviderCpMgr();
	virtual ~CProviderCpMgr();
	
	int OnConnect(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	int OnAccept(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);	
	int OnClose(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);
	int OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	//int OnLogout
	int Forward(CPacket &GessPacket,const unsigned long& ulKey);

	int Init(const string& sProcName);
	void Finish();
	int Start();
	void Stop();
	int Run();

	int HandleConsolMsg(unsigned int uiMsg);

	int OnXQueuePkt(CBinBlockPkt& oBlockPkt);
	int ToHisData(const string& sQuotation);
	int ToHisDataFx(const string& sQuotation);
private:

	//�����Ա����ָ��
	typedef string (CProviderCpMgr::*MFP_CmdHandleApi)(const string& sCmd, const vector<string>& vecPara);
	//�����������뱨�Ĵ����Ա����ӳ��ṹ
	typedef struct tagCmdLine2Api
	{
		string sCmdName;					//CmdName
		string sCmdAbbr;					//������д
		MFP_CmdHandleApi pMemberFunc;		//���Ĵ�����ָ��
		string sHelp;						//����˵��
	} CmdLine2Api;
	//��������������������Ա����ӳ���
	static CmdLine2Api m_CmdLine2Api[];


	//Դ�ӿ�+�������������Ӧ·�ɽӿ�ӳ��ṹ
	typedef struct tagIfRouterCfg
	{
		CConnectPointAsyn* pCp;
		unsigned long ulIfTo;
		unsigned long ulIfFrom;
		string sCmdID;
	} IfRouterCfg;
	//Դ�ӿ�+�������������Ӧ·�ɽӿ�ӳ���ϵ���ñ�
	static IfRouterCfg m_tblIfRouterCfg[];

	//·�ɵ�
	typedef multimap<string,CConnectPointAsyn*> MMAP_CP;
	typedef MMAP_CP::iterator MMAP_IT;
	typedef pair<MMAP_IT,MMAP_IT> RANGE_CP;
	typedef struct tagIfRouterPoint
	{
		unsigned long ulIfFrom;
		MMAP_CP  mmapCmds;
	} IfRouterPoint;
	//�ڴ�·�ɱ�
	IfRouterPoint m_tblIfRouter[EnumKeyUnknown];

	int InitRouterTbl();
	
protected:

private:
	int HandleNetLogMsg(const string & sNetLogMsg);
	int HandleCmdLine(string& sIn);

	int OnResetTimeout(const string& sTmKey);
	int OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan);
	int OnPacketSelfIpc(CPacket& GessPacket);
	

	//telnet ���������ӵ�ص��ӿ�
	int OnPacketCmd(CPacket& GessPacket);		
	//��־�ص��ӿ�
	void OnNetLogMsg(const string& sMsg);

	//�����¼�֪ͨ
	void NotifyEvent(const string& sEvt);	
private:
	std::string				m_sProcName;			//��������
	
	CConnectPointAsyn*		m_pCpInterfaceCmd;		//telnet����ӿ�
	CConnectPointAsyn*		m_pCpInterfaceH1;		//ϵͳ���H1�ӿ�
	CConnectPointAsyn*		m_pCpInterfaceH2;		//ϵͳ���H2�ӿ�
	CConnectPointAsyn*		m_pCpInterfaceZS;		//�ɼ�����������ӿ�	
	CConnectPointAsyn*		m_pCpInterfaceZC1;		//�ɼ�����������ӿ�
	CHisDataHandler*		m_pHisDataHandler;		//
	CHisDataHandlerFx*		m_pHisDataHandlerFx;		//
	CConnectPointAsyn*		m_pServiceHandlerSvr;		//
	CConnectPointAsyn*		m_pServiceHandlerCln1;		//
	CDeliverMgr*			m_pDeliverMgr;          //���鷢�������߽ӿ�

	CNetMgrModule*			m_pNetMagModule;		//���ܴ���
	CConnectPointCmd		m_oCpCmdHandler;		//Telnet�����д���

	//���Ź������ڴ�
	CMemShareAlive			m_oMemShareAlive;

	//�����д����߳�
	CCommandLineThread		m_oCmdLineThread;
	//������־�����߳�
	CNetLogThread			m_oNetLogThread;

	//��־����
	CNetLogHost				m_oNetLogHost;


	//Զ��Telnet�������ն˶����б�
	map<string,string>  m_deqTelnets;
	CGessMutex			m_csTelnets;
	
	//���
	CCpMgrNm	m_oNmoModule;

	CCondMutex	m_deqCondMutex;

	//��ʱ��������
	CGessTimerMgr* m_pGessTimerMgr;
	//dog��ʱ��
	CDogTimer m_oIfkTimer;

	//��ʱ������ʱ��
	CResetTimer m_oResetTimer;
	//��ʱ����ʱ�������
	vector<CGessTime> m_vResetTime;	

	//�ӿ����ӵ���������
	int m_nConNumIf[EnumKeyUnknown];
	CGessMutex	m_csConNum;

	unsigned int			m_uiNodeID;
	unsigned int			m_uiNodeType;
	CConfigImpl*			m_pConfig;
	volatile bool			m_bStop;

	//��������ģʽ
	int			  m_nQuoDateMode;
	//���ڳ����Ƿ���
	int			  m_nExpDateDiscard;
	//����ʱ��ģʽ
	int			  m_nQuoTmMode;
	//ʱ�ӵ���
	int			  m_nDiff;
	//�������ʱ�ӣ���������
	int			  m_nDelayPermit;
	//����һ��ʱ����Ҫ��ӡ
	int			  m_nDelayPrint;

	//��Լ״̬������Ϣ
	CBasicInfMemFile		m_oBasicInf;
	//��ԼIDת��
	CConfigImpl				m_oNameConvertFile;
	map<string, string>		m_mapNamePair;

	
	CXQueueIo<CBinBlockPkt>*	m_pReader;
	CXQueueIo<CBinBlockPkt>*	m_pIoServiceWriter;
private:
	void MemSendLogin(); 
	int HandleStartNtf(const char* pBuf, unsigned int uiLen);
	int HandleBasicInfo(const char* pBuf, unsigned int uiLen);
	int HandleInstState(const char* pBuf, unsigned int uiLen);
	int HandleQuotation(const char* pBuf, unsigned int uiLen);
	int HandleDeliveryQuotation(const char* pBuf, unsigned int uiLen);
	

	//��ԼIDת��
	void ConvertInstID(string& sInstID);
private:
	//����������ַ�
	string OnCmd(const string& sCmdLine, const vector<string>& vecPara);

	string OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineMem(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineHelp(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineEvtTest(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineSysInfo(const string& sCmdLine, const vector<string>& vecPara);
	string OnCmdLineQue(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineReplayQuotation(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineBuffer(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineLoad(const string& sCmd, const vector<string>& vecPara);
private:

};
#endif