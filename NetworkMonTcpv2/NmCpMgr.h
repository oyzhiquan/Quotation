#ifndef _NETWORK_MONITOR_CP_MGR_H
#define _NETWORK_MONITOR_CP_MGR_H
#include "Comm.h"
#include <vector>
#include <string>
#include <iostream>
#include "NmService.h"
//#include "MemShareAlive.h"

using namespace std;



//���ӵ�key����
typedef enum tagEnumKey
{
	EnumKeyNmSvr,
	EnumKeyIfMons,
	EnumKeyIfMonc1,
	EnumKeyIfMonc2,
	EnumKeyIfMonc3,
	//
	EnumKeyReserve,
	EnumKeyUnknown
} EnumKeyIf;

//�������ӵ���������
const string gc_sCfgNmService = "NMSERVICE";
const string gc_sCfgIfMons = "IFMONS";

//ȱʡ������ƥ��
const string gc_sDefaultCmdID = "#";

class CGessTimerMgr;
class CConfigImpl;
class CNmCpMgr:public CProtocolCpMgr
{
private:
	//�����д����߳�
	class CCommandLineThread :public CWorkThread
	{
	public:
		CCommandLineThread():m_pParent(0){}
		virtual ~CCommandLineThread(){}
		void Bind(CNmCpMgr* p){m_pParent = p;}
	private:
		int ThreadEntry()
		{
			string sIn("");	
			cout << "NmMon->";
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

				CRLog(E_SYSINFO,"NmMon_CmdLine_Thread exit!");
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
	private:
		CNmCpMgr* m_pParent;
	};

	//������ʱ��
	class CDogTimer : public CGessTimer
	{
	public:
		CDogTimer():m_pParent(0){}
		virtual ~CDogTimer(){}
		void Bind(CNmCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnDogTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CNmCpMgr* m_pParent;
	};

public:
	CNmCpMgr();
	virtual ~CNmCpMgr();
	
	int OnConnect(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	int OnAccept(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);	
	int OnClose(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);
	int OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	//int OnLogout
	int Forward(CPacket &GessPacket,const unsigned long& ulKey);

	int Init(const string& sProcName = "NmMon");
	void Finish();
	int Start();
	void Stop();
	int Run();

	int ToMoncx(CPacket &oPacket,const unsigned long& ulKey);
	int ToMons(CPacket &oPacket);
private:	
	//�����Ա����ָ��
	typedef string (CNmCpMgr::*MFP_CmdHandleApi)(const string& sCmd, const vector<string>& vecPara);
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

	
	int HandleCmdLine(string& sIn);
	int OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan);

	//����������ַ�
	string OnCmd(const string& sCmdLine, const vector<string>& vecPara);
	string OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineHelp(const string& sCmd, const vector<string>& vecPara);
	
private:
	std::string				m_sProcName;			//��������
	
	CConnectPointAsyn*		m_pCpInterfaceMons;		//
	
	CConfigImpl*			m_pCfgMonc1;
	CConnectPointAsyn*		m_pCpInterfaceMonc1;		//
	
	CConfigImpl*			m_pCfgMonc2;
	CConnectPointAsyn*		m_pCpInterfaceMonc2;		//

	CConfigImpl*			m_pCfgMonc3;
	CConnectPointAsyn*		m_pCpInterfaceMonc3;		//
	CNmService*				m_pNmService;

	//���Ź������ڴ�
	//CMemShareAlive			m_oMemShareAlive;

	//�����д����߳�
	CCommandLineThread		m_oCmdLineThread;

	
	//��ʱ��������
	//CGessTimerMgr* m_pGessTimerMgr;
	//���Ź���ʱ��
	//CDogTimer m_oIfkTimer;


	//���߳̿�����������
	CCondMutex				m_deqCondMutex;

	CConfigImpl*			m_pConfig;
	volatile bool			m_bStop;
};
#endif