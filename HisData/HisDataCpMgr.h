#ifndef _DATA_PROVIDER_CP_MGR_H
#define _DATA_PROVIDER_CP_MGR_H
#include "netlogdev.h"
#include "IpcPacket.h"
#include "Comm.h"
#include "WorkThreadNm.h"
#include "SamplerPacket.h"
#include <vector>
#include <string>
#include <iostream>
#include "XQueueIo.h"
#include "MemShareAlive.h"

using namespace std;

//���ӵ�key����
typedef enum tagEnumKey
{
	EnumKeyHisDataHandler,
	EnumKeyIfH1,
	EnumKeyIfH2,
	EnumKeyIfCmd,
	EnumKeyCmdHandler,
	EnumNetMagModule,
	EnumKeyUnknown
} EnumKeyIf;

//�������ӵ���������
const string gc_sCfgReader = "XQUEUE";
const string gc_sCfgHisData = "IFHISDATA";
const string gc_sCfgIfH1 = "IFH1";
const string gc_sCfgIfH2 = "IFH2";
const string gc_sCfgIfCmd = "IFCMD";
const string gc_sCfgNetMagModule = "net_mgr";

//ȱʡ������ƥ��
const string gc_sDefaultCmdID = "#";

class CGessTimerMgr;
class CConfigImpl;
class CNetMgrModule;
class CHisDataHandler;
class CHisDataCpMgr:public CProtocolCpMgr
{
private:	
	class CXQueMgrImpl:public CXQueueCallback<QUOTATION>
	{
	public:
		CXQueMgrImpl(CHisDataCpMgr* p = 0):m_pParent(p){}
		~CXQueMgrImpl(){}
		
		int Init(CConfig* pCfg)
		{
			m_oReader.Bind(this);
			return m_oReader.Init(pCfg);
		}
		void Finish(){return m_oReader.Finish();}
		int Start(){return m_oReader.Start();}
		void Stop(){return m_oReader.Stop();}
		int Bind(CHisDataCpMgr* p)
		{
			m_pParent = p; 
			return 0;
		}
	private:
		CHisDataCpMgr*		m_pParent;
		CXQueueIo<QUOTATION>	m_oReader;

		int OnXQueuePkt(QUOTATION& msg)
		{
			if (0 != m_pParent)
				return m_pParent->OnXQueuePkt(msg);
			return 0;
		}
	};

	//�����д����߳�
	class CCommandLineThread :public CWorkThreadNm
	{
	public:
		CCommandLineThread():m_pParent(0){}
		virtual ~CCommandLineThread(){}
		void Bind(CHisDataCpMgr* p){m_pParent = p;}
	private:
		int ThreadEntry()
		{
			string sIn("");	
			cout << "HisData->";
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
		CHisDataCpMgr* m_pParent;
	};

	//������־����Ϣ�����߳�
	class CNetLogThread :public CWorkThreadNm
	{
	public:
		CNetLogThread():m_pParent(0){}
		virtual ~CNetLogThread(){}
		void Bind(CHisDataCpMgr* p){m_pParent = p;}
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
		CHisDataCpMgr* m_pParent;
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
			m_pMgr=dynamic_cast<CHisDataCpMgr*>(pCpMgr); 
			m_ulKey=ulKey;
		}
		int SendPacket(CPacket &GessPacket)
		{
			if (0==m_pMgr)
				return -1;
			return m_pMgr->OnPacketCmd(GessPacket);
		}
	private:
		CHisDataCpMgr* m_pMgr;
		unsigned long m_ulKey;
	};

	//��־����
	class CNetLogHost:public CSubscriber
	{
	public:
		CNetLogHost():m_pMgr(0){}
		virtual ~CNetLogHost(){}
		void Bind(CHisDataCpMgr* p){m_pMgr = p;}
		void OnNotify(const string& sMsg)
		{
			if (0 != m_pMgr)
				m_pMgr->OnNetLogMsg(sMsg);
		}
	private:
		CHisDataCpMgr* m_pMgr;
	};

	//K�ӿ�������ʱ��
	class CDogTimer : public CGessTimer
	{
	public:
		CDogTimer():m_pParent(0){}
		virtual ~CDogTimer(){}
		void Bind(CHisDataCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnDogTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CHisDataCpMgr* m_pParent;
	};

	//��ʱ������ʱ��
	class CResetTimer: public CGessAbsTimer
	{
	public:
		CResetTimer():m_pParent(0){}
		~CResetTimer(void){}
		void Bind(CHisDataCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& sKey)
		{
			if (0 != m_pParent)
				return m_pParent->OnResetTimeout(sKey);
			return -1;
		}
		void TimerCanceled(const string& ulKey){}
	private:
		CHisDataCpMgr* m_pParent;
	};
public:
	CHisDataCpMgr();
	virtual ~CHisDataCpMgr();
	
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

	int OnXQueuePkt(QUOTATION& stQuotation);
private:
	//�����Ա����ָ��
	typedef string (CHisDataCpMgr::*MFP_CmdHandleApi)(const string& sCmd, const vector<string>& vecPara);
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
	CHisDataHandler*		m_pHisDataHandler;		//
	CXQueMgrImpl*			m_pReader;

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


	CCondMutex	m_deqCondMutex;

	//��ʱ��������
	CGessTimerMgr* m_pGessTimerMgr;
	//K�ӿڶ�ʱ��
	CDogTimer m_oIfkTimer;

	//��ʱ������ʱ��
	CResetTimer m_oResetTimer;
	//��ʱ����ʱ�������
	vector<CGessTime> m_vResetTime;	

	//�ӿ����ӵ���������
	int m_nConNumIf[EnumKeyUnknown];
	CGessMutex	m_csConNum;

	unsigned int		  m_uiNodeID;
	unsigned int		  m_uiNodeType;
	CConfigImpl*		m_pConfig;
	volatile bool m_bStop;

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
	string OnCmdLineRecord(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineTestCheck(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineXQueInf(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineStatics(const string& sCmd, const vector<string>& vecPara);
private:
	//Quit����
	int OnRecvQuit(CIpcPacket& pkt);
};
#endif