#ifndef _SAMPLER_CP_MGR_H
#define _SAMPLER_CP_MGR_H

#include "netlogdev.h"
#include "Comm.h"
#include "WorkThreadNm.h"
#include <string>
#include <iostream>
#include "XQueueIo.h"
#include "SamplerPacket.h"
#include "MemShareAlive.h"

using namespace std;

//���ӵ�key����
typedef enum tagEnumKey
{
	EnumKeyTranslator,
	EnumKeyServiceHandler1,
	EnumKeyServiceHandler2,
	EnumKeyIfZC1,
	EnumKeyIfZC2,
	EnumKeyIfH1,
	EnumKeyIfH2,
	EnumKeyIfCmd,
	EnumKeyCmdHandler,
	EnumNetMagModule,
	EnumKeySvAgent,
	EnumKeyUnknown
} EnumKeyIf;

//�������ӵ���������
const string gc_sCfgXQueue = "XQUEUE";
const string gc_sCfgTranslator = "IFTRANSLATOR";
const string gc_sCfgService = "IFSERVICE";
const string gc_sCfgIfZS = "IFZS";
const string gc_sCfgIfZC1 = "IFZC1";
const string gc_sCfgIfZC2 = "IFZC2";
const string gc_sCfgIfH1 = "IFH1";
const string gc_sCfgIfH2 = "IFH2";
const string gc_sCfgIfCmd = "IFCMD";
const string gc_sCfgNetMagModule = "net_mgr";
const string gc_sCfgSvAgent = "sv_agent";

// added by Jerry Lee, 2010-12-24, ��ʷ����
const string gc_sCfgXQueueHis = "XQUEUE_HIS";


//ȱʡ������ƥ��
const string gc_sDefaultCmdID = "#";

class CGessTimerMgr;
class CConfigImpl;
class CNetMgrModule;
class CIfSvAgent;
class CTranslator;
class CSamplerCpMgr:public CProtocolCpMgr
{
private:
	//ϵͳ���ע�����
	class CSamplerMgrModule: public CNmoModule
	{
	public:
		CSamplerMgrModule():m_pParent(0){}
		virtual ~CSamplerMgrModule(){}
		void Bind(CSamplerCpMgr* p){m_pParent = p;}
		//���ܵ�����ѯ�ӿ�
		int Query(CNMO& oNmo) const
		{
			if (0 == m_pParent)
				return -1;

			return m_pParent->Query(oNmo);
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
		CSamplerCpMgr* m_pParent;
	};

	//�����д����߳�
	class CCommandLineThread :public CWorkThreadNm
	{
	public:
		CCommandLineThread():m_pParent(0){}
		virtual ~CCommandLineThread(){}
		void Bind(CSamplerCpMgr* p){m_pParent = p;}
	private:
		int ThreadEntry()
		{
			string sIn("");	
			cout << "Sampler->";
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

				CRLog(E_SYSINFO,"Sampler_CmdLine_Thread exit!");
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
		CSamplerCpMgr* m_pParent;
	};

	//������־����Ϣ�����߳�
	class CNetLogThread :public CWorkThreadNm
	{
	public:
		CNetLogThread():m_pParent(0){}
		virtual ~CNetLogThread(){}
		void Bind(CSamplerCpMgr* p){m_pParent = p;}
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

				CRLog(E_SYSINFO,"Offering_NetLog Thread exit!");
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
		CSamplerCpMgr* m_pParent;
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
			m_pMgr=dynamic_cast<CSamplerCpMgr*>(pCpMgr); 
			m_ulKey=ulKey;
		}
		int SendPacket(CPacket &GessPacket)
		{
			if (0==m_pMgr)
				return -1;
			return m_pMgr->OnPacketCmd(GessPacket);
		}
	private:
		CSamplerCpMgr* m_pMgr;
		unsigned long m_ulKey;
	};

	//��־����
	class CNetLogHost:public CSubscriber
	{
	public:
		CNetLogHost():m_pMgr(0){}
		virtual ~CNetLogHost(){}
		void Bind(CSamplerCpMgr* p){m_pMgr = p;}
		void OnNotify(const string& sMsg)
		{
			if (0 != m_pMgr)
				m_pMgr->OnNetLogMsg(sMsg);
		}
	private:
		CSamplerCpMgr* m_pMgr;
	};

	//������ʱ��
	class CDogTimer : public CGessTimer
	{
	public:
		CDogTimer():m_pParent(0){}
		virtual ~CDogTimer(){}
		void Bind(CSamplerCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnDogTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CSamplerCpMgr* m_pParent;
	};

	//��ʱ������ʱ��
	class CResetTimer: public CGessAbsTimer
	{
	public:
		CResetTimer():m_pParent(0){}
		~CResetTimer(void){}
		void Bind(CSamplerCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& sKey)
		{
			if (0 != m_pParent)
				return m_pParent->OnResetTimeout(sKey);
			return -1;
		}
		void TimerCanceled(const string& ulKey){}
	private:
		CSamplerCpMgr* m_pParent;
	};

	//ͳ�������ܵ��߳�
	class CStaticsPipeThread :public CWorkThread
	{
	public:
		CStaticsPipeThread():m_pParent(0),m_hPipeSvr(INVALID_HANDLE_VALUE){}
		virtual ~CStaticsPipeThread(){}
		void Bind(CSamplerCpMgr* p){m_pParent = p;}
	private:
		int ThreadEntry()
		{
			try
			{
				m_hPipeSvr = CreateNamedPipe("\\\\.\\pipe\\statics_pipe\\",PIPE_ACCESS_DUPLEX,PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,1,0,0,10000,0);
				if( INVALID_HANDLE_VALUE == m_hPipeSvr)
				{
					CRLog(E_ERROR,"Pipe create fail!");
					return 0;
				}

				char szBuf[2048] = {0};
				while(!m_bEndThread)
				{
					try
					{
						BOOL blConnect = 0;
						blConnect = ConnectNamedPipe(m_hPipeSvr,0);
						if (blConnect)// || (!blConnect && GetLastError() == ERROR_PIPE_CONNECTED))
						{
							BOOL blRead = false;
							unsigned long uiRead = 0;
							do
							{
								blRead = ReadFile(m_hPipeSvr,szBuf,sizeof(szBuf),&uiRead,0);
								if (0 != uiRead)
								{
									m_pParent->HandleStatics(szBuf,uiRead);
									string sRsp = "ok";
									unsigned long ulWrited = 0;
									WriteFile(m_hPipeSvr,sRsp.c_str(),sRsp.length(),&ulWrited,0);
								}
							} while (blRead && uiRead != 0);
							DisconnectNamedPipe(m_hPipeSvr);
						}
						else
						{
							if( INVALID_HANDLE_VALUE != m_hPipeSvr)
							{
								CloseHandle(m_hPipeSvr);
							}

							m_hPipeSvr = CreateNamedPipe("\\\\.\\pipe\\statics_pipe\\",PIPE_ACCESS_DUPLEX,PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,1,0,0,10000,0);
							if( INVALID_HANDLE_VALUE == m_hPipeSvr)
							{
								CRLog(E_ERROR,"Pipe create fail!");
								break;
							}
						}
					}
					catch(...)
					{
						CRLog(E_ERROR,"Unknown exception!");
						msleep(1);
					}
				}

				if( INVALID_HANDLE_VALUE != m_hPipeSvr)
				{
					CloseHandle(m_hPipeSvr);
				}

				CRLog(E_SYSINFO,"Sampler_Pipe_Thread exit!");
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
			char *sWrite = "1";
			char szRead[128] = {0};
			unsigned long uiRead = 0;
			CallNamedPipe("\\\\.\\pipe\\statics_pipe\\",sWrite,strlen(sWrite),szRead,sizeof(szRead),&uiRead,NMPWAIT_USE_DEFAULT_WAIT);
			return 0;
		}
	private:
		CSamplerCpMgr* m_pParent;
		HANDLE m_hPipeSvr;
	};
public:
	CSamplerCpMgr();
	virtual ~CSamplerCpMgr();
	
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
	void StopMe();
	int StartMe();

	//ת��Ŀ�Ķ���writer
	int ToXQueue(QUOTATION& stQuotation);

	int HandleConsolMsg(unsigned int uiMsg);

    // added by Jerry Lee, 2010-12-24
    int ToHisDataQueue(CSamplerPacket& oPktSrc); 

    // added by Jerry Lee, 2011-1-18
    int ToTickDataQueue(CSamplerPacket& oPktSrc); 

    // added by Jerry Lee, 2011-2-21
    int ToInfoDataQueue(CSamplerPacket& oPktSrc);

private:

	//�����Ա����ָ��
	typedef string (CSamplerCpMgr::*MFP_CmdHandleApi)(const string& sCmd, const vector<string>& vecPara);
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
private:
	int HandleNetLogMsg(const string & sNetLogMsg);
	int HandleCmdLine(string& sIn);
	int HandleStatics(const char* pBuf,unsigned int uiLen);

	//��ʱ��������
	int OnResetTimeout(const string& sTmKey);	
	int OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan);

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
	CConnectPointAsyn*		m_pCpInterfaceZC2;		//�ɼ�����������ӿ�
	CConnectPointAsyn*		m_pServiceHandler1;		//
	CConnectPointAsyn*		m_pServiceHandler2;		//

	CTranslator*			m_pTranslator;			//ת����
	CIfSvAgent*				m_pSvAgent;				//���ж˼�ش���
	CNetMgrModule*			m_pNetMagModule;		//���ܴ���
	CConnectPointCmd		m_oCpCmdHandler;		//Telnet�����д���

	//�������������������    kenny[20170824]
	//XQueue����
	//CXQueueIo< QUOTATION >* 			m_pQueueIo;
	vector< CXQueueIo<QUOTATION>* >			m_vecQueueIo;


    // added by Jerry Lee, 2010-12-24
    CXQueueIo< HistoryDataBuf >*      m_pQueHisData;
	//���Ź������ڴ�
	CMemShareAlive			m_oMemShareAlive;

	//�����д����߳�
	CCommandLineThread		m_oCmdLineThread;
	//������־�����߳�
	CNetLogThread			m_oNetLogThread;
	//
	//ͳ�ƹܵ��߳�
	CStaticsPipeThread		m_oStaticsPipeThread;

	//��־����
	CNetLogHost				m_oNetLogHost;
	
	//ϵͳ���ע�����
	CSamplerMgrModule		m_oNmoModule;

	//Զ��Telnet�������ն˶����б�
	map<string,string>  m_deqTelnets;
	CGessMutex			m_csTelnets;

	//���߳̿�����������
	CCondMutex	m_deqCondMutex;

	//���Ź���ʱ��
	CDogTimer m_oIfkTimer;

	//��ʱ������ʱ��
	CResetTimer m_oResetTimer;

	unsigned int		m_uiNodeID;
	unsigned int		m_uiNodeType;
	CConfigImpl*		m_pConfig;
	volatile bool m_bStop;

	//�Ƿ���˵���ʱ����
	volatile bool m_bTimeDelayFilter;
private:
	//�ӳ�ͳ����Ϣ
	unsigned int  m_uiFwdTotal;
	unsigned int  m_uiDelayLess0s;
	unsigned int  m_uiDelayLess1s;
	unsigned int  m_uiDelayLess2s;
	unsigned int  m_uiDelayLess3s;
	unsigned int  m_uiDelayLess5s;
	unsigned int  m_uiDelayLess10s;
	unsigned int  m_uiDelayLess30s;
	unsigned int  m_uiDelayLess60s;
	unsigned int  m_uiDelayLess120s;
	unsigned int  m_uiDelayMore120s;
	unsigned int  m_uiDelayMin;
	unsigned int  m_uiDelayMax;
private:
	//����������ַ�
	string OnCmd(const string& sCmdLine, const vector<string>& vecPara);

	string OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineMem(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineHelp(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineEvtTest(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineSysInfo(const string& sCmdLine, const vector<string>& vecPara);
	string OnCmdLineQue(const string& sCmd, const vector<string>& vecPara);
private:
	int CancelOrder();
	int HandleCmdSpecial(const string& sCmdID, CPacket &pkt);

	int Query(CNMO& oNmo);
};

#endif