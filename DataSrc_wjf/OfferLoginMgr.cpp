#include "stdafx.h"
#include "Logger.h"
#include "OfferLoginMgr.h"
#include "PacketStructTransferIpcOffer.h"
#include "GessTimerMgrPosix.h"

#include "DataSrcCpMgr.h"

using namespace ipcoffer;

CLoginMgr::Cmd2Api CLoginMgr::m_Cmd2Api[] =
{
	{"StateInfo",	&CLoginMgr::OnStateInfo},
	{"StateRsp",	&CLoginMgr::OnStateInfoRsp}
};

CLoginMgr::CLoginMgr()
:m_pCpMgr(0)
,m_ulKey(0xFFFFFFFF)
,m_pCfg(0)
,m_blTokenConflict(false)
,m_nConnectState(gc_nStateInit)
,m_blFirstConnectState(true)
,m_nNodeID(0)
{
	m_stLoginInfoRemote.nLoginState = gc_nStateLoginUnknown;
}

CLoginMgr::~CLoginMgr()
{}

//ƥ�䱨�Ĵ����Ա���������е��ô��� �����Ż�Ϊ��ϣ�����
int CLoginMgr::RunPacketHandleApi(CIpcPacket& pkt)
{
	std::string sCmdID;
	try
	{
		sCmdID = pkt.GetCmdID();
	
		int nSize = sizeof(m_Cmd2Api)/sizeof(Cmd2Api);
		for ( int i = 0 ; i < nSize ; i++ )
		{
			if ( m_Cmd2Api[i].sApiName == sCmdID )
			{
				if (m_Cmd2Api[i].pMemberFunc == 0)
					break;

				return (this->*(m_Cmd2Api[i].pMemberFunc))(pkt);
			}
		}
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s,Handle Packet:%s", e.what(),sCmdID.c_str());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception,Handle Packet:%s",sCmdID.c_str());
		return -1;
	}
}

void CLoginMgr::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	//���ӵ������
	m_pCpMgr = pCpMgr; 
	if (m_pCpMgr == 0)
	{
		CRLog(E_ERROR,"����ָ������.");
	}

	//Ψһ��ʶ���ӵ��key
	m_ulKey=ulKey;
}

int CLoginMgr::Init(CConfig* pConfig) 
{
	assert(0 != pConfig);
	if (0 == pConfig)
		return -1;

	m_pCfg = pConfig;	
	
	//���ܽӿ�
	m_oNm.Bind(this);

	//��ȡ����...
	string sCfgTmp = "1000";
	CConfig* pCfgGlobal = pConfig->GetCfgGlobal();
	if (0 == pCfgGlobal->GetProperty("node_id",sCfgTmp))
	{
		m_nNodeID = FromString<int>(sCfgTmp);
	}

	//��������
	sCfgTmp = ToString<int>(gc_nSlave);
	if (0 == pCfgGlobal->GetProperty("master",sCfgTmp))
	{
		int nFlag = FromString<int>(sCfgTmp);
		if (gc_nMaster == nFlag)
		{
			m_stLoginInfoLocal.nMasterSlave = gc_nMaster;
		}
		else
		{
			m_stLoginInfoLocal.nMasterSlave = gc_nSlave;
		}
	}

	//�Ƿ��ʼ������Ϊ����,����ģʽ��Ҫ����
	sCfgTmp = ToString<int>(gc_nLoginTokenWait);
	if (0 == pConfig->GetProperty("init_token",sCfgTmp))
	{
		int nToken = FromString<int>(sCfgTmp);
		if (gc_nLoginTokenHold == nToken)
		{
			m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
			m_stLoginInfoLocal.tmToken = time(0);
		}
	}

	//�Ƿ���Ҫ��ʼ�������
	sCfgTmp = "1";
	int nNeedInit = 1;
	if (0 == pConfig->GetProperty("need_init_magic",sCfgTmp))
	{
		nNeedInit = FromString<int>(sCfgTmp);		
	}
	if (0 != nNeedInit)
	{
		srand(static_cast<unsigned int>(time(0)));
		int RANGE_MIN = 0;
		int RANGE_MAX = 10000;
		m_stLoginInfoLocal.nMagicNum = rand() * (RANGE_MAX - RANGE_MIN) / RAND_MAX + RANGE_MIN;
	}


	CRLog(E_APPINFO,"������Ϣ:���ڵ�ID:%d ���ӱ�־:%d �����:%d",m_nNodeID,m_stLoginInfoLocal.nMasterSlave,m_stLoginInfoLocal.nMagicNum);
	return 0;
}

int CLoginMgr::Start()
{
	BeginThread();

	//ע���¼״̬��
	vector< pair<string,string> > vNmo;
	pair<string,string> pa;
	pa.first = gc_sSgeLoginState;
	pa.second = gc_sSgeLoginState + ".0";
	vNmo.push_back(pa);

	pa.first = gc_sSgeLoginToken;
	pa.second = gc_sSgeLoginToken + ".0";
	vNmo.push_back(pa);

	pa.first = gc_sSgeLoginInd;
	pa.second = gc_sSgeLoginInd + ".0";
	vNmo.push_back(pa);

	pa.first = gc_sSgeLoginAlarm;
	pa.second = gc_sSgeLoginAlarm + ".0";
	vNmo.push_back(pa);
	CNetMgr::Instance()->Register(&m_oNm, vNmo);

	CNMO oNmo;
	oNmo.m_sOid = gc_sSgeLoginAlarm;
	oNmo.m_sOidIns = gc_sSgeLoginAlarm + ".0";
	oNmo.m_nQuality = gc_nQuolityGood;
	oNmo.m_sTimeStamp = CGessDate::NowToString("-") + " " + CGessTime::NowToString(":");
	oNmo.m_sValue = ToString<int>(gc_nLoginNormal);
	CNetMgr::Instance()->Report(oNmo);
	return 0;
}

void CLoginMgr::Stop()
{
	m_csState.Lock();
	m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
	m_csState.Unlock();

	EndThread();

	//����ע��
	CNetMgr::Instance()->UnRegisterModule(&m_oNm);
}

void CLoginMgr::Finish()
{
	m_deqMIf.clear();
	return;
}

int CLoginMgr::SendPacket(CPacket &GessPacket)
{
	try
	{
		CIpcPacket & pkt = dynamic_cast<CIpcPacket &>(GessPacket);
		
		m_deqCondMutex.Lock();
		m_deqMIf.push_back(pkt);
		m_deqCondMutex.Unlock();
		m_deqCondMutex.Signal();
		return 0;
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

int CLoginMgr::OnRecvPacket(CPacket &GessPacket)
{
	if (0 != m_pCpMgr)
		return m_pCpMgr->Forward(GessPacket,m_ulKey);

	return -1;
}


//�������ӶԶ˱��̻�������д���
void CLoginMgr::ConnectNtf(int nFlag)
{
	try
	{
		m_csState.Lock();
		if (0 != nFlag /*&& m_blFirstConnectState*/)
		{//�״����ӶԶ˱��̻���ʧ��
			CRLog(E_APPINFO,"������Ϣ���Զ�����ʧ��!�Զ���õ�¼����");
			m_blFirstConnectState = false;

			//m_stLoginInfoLocal.nLoginIndication = gc_nIndLogin; // added by Ben 20110503
			m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
			m_stLoginInfoLocal.tmToken = time(0);
			bool blLogin = IsNeedLogin();
			m_csState.Unlock();	

			if (blLogin)
			{
				PrepareLogin();
			}
			return;
		}

		if (0 == nFlag)
		{
			m_nConnectState = gc_nStateConnected;
		}
		else
		{
			m_nConnectState = gc_nStateDisConnected;
		}

		if (m_blFirstConnectState)
			m_blFirstConnectState = false;
		
		if (0 == nFlag && !m_stLoginInfoLocal.blInfoSended)
		{
			CRLog(E_APPINFO,"������Ϣ:���ӶԶ˳ɹ������ͱ��ص�¼��Ϣ!");
			m_stLoginInfoLocal.blInfoSended = true;
			SendStateInfo();
		}
		m_csState.Unlock();
	}
	catch(...)
	{
		m_csState.Unlock();
		CRLog(E_ERROR,"Unknown exception");
	}
}

//�Զ˱��̻������ж�
void CLoginMgr::DisconnectNtf()
{
	CRLog(E_APPINFO,"������Ϣ���Զ������ж�");

	bool blLogin = false;
	m_csState.Lock();
	m_nConnectState = gc_nStateDisConnected;
	if (gc_nLoginTokenWait == m_stLoginInfoLocal.nLoginToken)
	{
		CRLog(E_APPINFO,"������Ϣ���Զ������ж�!�Զ���õ�¼����");
		m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
		m_stLoginInfoLocal.tmToken = time(0);
		blLogin = IsNeedLogin();
	}
	m_stLoginInfoLocal.blInfoSended = false;
	m_csState.Unlock();

	if (blLogin)
	{
		PrepareLogin();
	}
}

void CLoginMgr::AcceptNtf()
{
}

//׼����¼
int CLoginMgr::PrepareLogin()
{
	unsigned int uiLeft = 2;
	CDataSrcCpMgr* pCpMgr =  dynamic_cast<CDataSrcCpMgr*>(m_pCpMgr);

	if (pCpMgr->Stock_Dll_Init())
	{
		OnLogin(gc_nStateLogined);
	}
	else
		OnLogin(0);

	return 0;
}

bool CLoginMgr::IsNeedLogout()
{
	bool blFlag = false;
	m_csState.Lock();
	if (gc_nStateLogined == m_stLoginInfoLocal.nLoginState)
	{
		blFlag = true;
	}
	m_csState.Unlock();

	return blFlag;
}

//�ǳ�ָʾ,���н��׷������ж�ָʾ�ǳ�
int CLoginMgr::LogoutInd()
{
	m_csState.Lock();
	m_stLoginInfoLocal.nLoginIndication = gc_nIndLogout;

	//����Ѿ���¼��ǳ�
	if (!IsNeedLogout())
	{
		CRLog(E_APPINFO,"������Ϣ:���߱��ǳ�����!");
		m_csState.Unlock();
		return -1;
	}

	CDataSrcCpMgr* pCpMgr =  dynamic_cast<CDataSrcCpMgr*>(m_pCpMgr);

	if (pCpMgr->Stock_Dll_Release())
	{
		OnLogout(4);
	}
	m_csState.Unlock();
	return 0;
}

//��¼�ǳ����֪ͨ ����ģ�����
void CLoginMgr::OnLogin(int nResult)
{
	CRLog(E_APPINFO,"������Ϣ:��¼���ָʾ(%d)!",nResult);

	bool blNegotiate = false;
	bool blConnFlag = true;
	m_csState.Lock();
	m_stLoginInfoLocal.nLoginResult = nResult;
	if (gc_nStateLogined == nResult)
	{
		m_stLoginInfoLocal.nLoginState = gc_nStateLogined;
		m_stLoginInfoLocal.nLoginIndication = gc_nIndLogin;
	}
	else if (gc_nStateLoginning != nResult && m_blTokenConflict)
	{
		CRLog(E_APPINFO,"������Ϣ:���Ƴ�ͻ,��������,ֹͣ��¼!");
		m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
		m_stLoginInfoLocal.nLoginState = gc_nStateLoginInit;
		m_stLoginInfoLocal.nLoginResult = gc_nStateLoginInit;
		blNegotiate = true;
	}
	blConnFlag = (gc_nStateConnected == m_nConnectState);
	m_csState.Unlock();

	//�ϱ�����
	CNMO oNmo;
	oNmo.m_sOid = gc_sSgeLoginAlarm;
	oNmo.m_sOidIns = gc_sSgeLoginAlarm + ".0";
	oNmo.m_nQuality = gc_nQuolityGood;
	oNmo.m_sTimeStamp = CGessDate::NowToString("-") + " " + CGessTime::NowToString(":");

	if (blNegotiate)
	{
		 CRLog(E_APPINFO,"������Ϣ:���Ƴ�ͻ,��������,����Э��!"); 
		 SendStateInfo();

		 oNmo.m_sValue = ToString<int>(gc_nLoginNormal);
	}
	else
	{
		if (gc_nStateLogined == nResult && blConnFlag)
		{
			CRLog(E_APPINFO,"������Ϣ:��¼�ɹ�,���͵�¼��Ϣ!");	
			SendStateInfo();
		}

		oNmo.m_sValue = ToString<int>(GetAlarmStat());
	}

	CNetMgr::Instance()->Report(oNmo);
}

//��¼�ǳ����֪ͨ ����ģ�����
void CLoginMgr::OnLogout(int nResult)
{
	CRLog(E_APPINFO,"������Ϣ:�ǳ����:%d!",nResult);

	bool blConnFlag = true;
	m_csState.Lock();
	m_stLoginInfoLocal.nLoginResult = nResult;
	if (gc_nStateLogouted == nResult)
	{
		m_stLoginInfoLocal.nLoginState = gc_nStateLogouted;
	}
	
	blConnFlag = (gc_nStateConnected == m_nConnectState);
	//m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
	m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenGiveup;
	m_csState.Unlock();

	if (blConnFlag && gc_nStateLogoutting != nResult)
	{
		CRLog(E_APPINFO,"������Ϣ:�ǳ�,���͵�¼��Ϣ!");
		SendStateInfo();
	}
}


//����M�ӿڱ����߳�
int CLoginMgr::ThreadEntry()
{
	try
	{
		while(!m_bEndThread)
		{
			m_deqCondMutex.Lock();
			while(m_deqMIf.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}

			if ( !m_deqMIf.empty())
			{
				CIpcPacket pkt = m_deqMIf.front();
				m_deqMIf.pop_front();
				m_deqCondMutex.Unlock();

				try
				{
					RunPacketHandleApi(pkt);
				}
				catch(...)
				{
					CRLog(E_ERROR,"Unknown exception! ");	
				}
				continue;
			}
			m_deqCondMutex.Unlock();
		}

		CRLog(E_SYSINFO,"Trader_Backup Thread exit!");
		return 0;	
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s! m_bEndThread=%s",e.what(),m_bEndThread?"true":"false");
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"Unknown exception! m_bEndThread=%s",m_bEndThread?"true":"false");		
		return -1;
	}
}

int CLoginMgr::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();
	Wait();
	return 0;
}

//�������߳�״̬�Ƿ���Ҫ������
bool CLoginMgr::IsNetManaged(string& sKeyName)
{
	sKeyName = "���̻����������߳�";
	return true;
}

//����Э��
bool CLoginMgr::HandleStateInfo(StateInfo& stBody,int& nResult)
{
	bool blNegotiate = false;

	m_csState.Lock();
	m_stLoginInfoRemote.nMasterSlave = FromString<int>(stBody.ms_flag);
	m_stLoginInfoRemote.nLoginIndication = FromString<int>(stBody.ind_login);
	m_stLoginInfoRemote.nLoginState = FromString<int>(stBody.login_state);
	m_stLoginInfoRemote.nLoginToken = FromString<int>(stBody.token);
	m_stLoginInfoRemote.tmToken = FromString<time_t>(stBody.tm_token);
	int nPeerNodeID = 0;
	int nPktFlag = FromString<int>(stBody.pkt_flag);
	if (gc_nPktRequest == nPktFlag)
	{
		nPeerNodeID = FromString<int>(stBody.node_id);
	}
	else
	{
		nPeerNodeID = FromString<int>(stBody.node_peer_id);
	}
	int nMagicNum = FromString<int>(stBody.magic_number);
	
	//��ʼ��Ϊ�޳�ͻ���Ƿ��ͻÿ��Э�����½����ж�
	m_blTokenConflict = false;

	if (gc_nLoginTokenHold != m_stLoginInfoRemote.nLoginToken)
	{
		if (gc_nLoginTokenHold != m_stLoginInfoLocal.nLoginToken)
		{//��δ��������
			if (gc_nLoginTokenGiveup == m_stLoginInfoRemote.nLoginToken)
			{
				CRLog(E_APPINFO,"������Ϣ:���˶�δ��������,�Զ���������,�����Զ��������!");
				m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
				m_stLoginInfoLocal.tmToken = time(0);
				nResult = gc_nTokenWait2Hold;
			}
			else if (gc_nMaster == m_stLoginInfoLocal.nMasterSlave && gc_nMaster != m_stLoginInfoRemote.nMasterSlave)
			{//�Զ�δ�������� ������ΪMaster���������
				CRLog(E_APPINFO,"������Ϣ:���˶�δ��������,��������Ϊ��,�Զ�����Ϊ��,�����Զ��������!");
				m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
				m_stLoginInfoLocal.tmToken = time(0);
				nResult = gc_nTokenWait2Hold;
			}
			else if (gc_nMaster != m_stLoginInfoLocal.nMasterSlave && gc_nMaster == m_stLoginInfoRemote.nMasterSlave)
			{
				CRLog(E_APPINFO,"������Ϣ:���˶�δ��������,��������Ϊ��,�Զ�����Ϊ��,����δ������!");
				nResult = gc_nTokenWait2Wait;
			}
			else if (gc_nMaster == m_stLoginInfoLocal.nMasterSlave && gc_nMaster == m_stLoginInfoRemote.nMasterSlave)
			{
				CRLog(E_APPINFO,"������Ϣ:���ó�ͻ,���˶�δ��������,���˶�����Ϊ��!");
				if (m_nNodeID > nPeerNodeID)
				{
					CRLog(E_APPINFO,"������Ϣ:���˽ڵ�Ŵ��ڶԶ˽ڵ��,������������Ϊ��,δ������!");
					m_stLoginInfoLocal.nMasterSlave = gc_nSlave;
					nResult = gc_nTokenWait2Wait;
				}
				else if (m_nNodeID < nPeerNodeID)
				{
					CRLog(E_APPINFO,"������Ϣ:���˽ڵ��С�ڶԶ˽ڵ��,����Ϊ��,������!");
					m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
					m_stLoginInfoLocal.tmToken = time(0);
					nResult = gc_nTokenWait2Hold;
				}
				else
				{
					CRLog(E_APPINFO,"������Ϣ:���˽ڵ�ŵ��ڶԶ˽ڵ��,�����漴���������Ӿ���!");
					if (nMagicNum < m_stLoginInfoLocal.nMagicNum)
					{
						CRLog(E_APPINFO,"������Ϣ:������������ڶԶ������,������������Ϊ��,δ������!");
						m_stLoginInfoLocal.nMasterSlave = gc_nSlave;
						nResult = gc_nTokenWait2Wait;
					}
					else if (nMagicNum > m_stLoginInfoLocal.nMagicNum)
					{
						CRLog(E_APPINFO,"������Ϣ:���������С�ڶԶ������,����Ϊ��,������!");
						m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
						m_stLoginInfoLocal.tmToken = time(0);
						nResult = gc_nTokenWait2Hold;
					}
					else
					{
						CRLog(E_APPINFO,"������Ϣ:������������ڶԶ������,��������������������Ӿ���!");
						nResult = gc_nTokenWait2Wait;
						blNegotiate = true;
					}
				}
			}
			else
			{
				CRLog(E_APPINFO,"������Ϣ:���ó�ͻ,���˶�δ��������,���˶�����Ϊ��!");	
				if (m_nNodeID > nPeerNodeID)
				{
					CRLog(E_APPINFO,"������Ϣ:���˽ڵ�Ŵ��ڶԶ˽ڵ��,���˱���Ϊ��,δ������!");
					m_stLoginInfoLocal.nMasterSlave = gc_nSlave;
					nResult = gc_nTokenWait2Wait;
				}
				else if (m_nNodeID < nPeerNodeID)
				{
					CRLog(E_APPINFO,"������Ϣ:���˽ڵ��С�ڶԶ˽ڵ��,��������Ϊ��,������!");
					m_stLoginInfoLocal.nMasterSlave = gc_nMaster;
					m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
					m_stLoginInfoLocal.tmToken = time(0);
					nResult = gc_nTokenWait2Hold;
				}
				else
				{
					CRLog(E_APPINFO,"������Ϣ:���˽ڵ�ŵ��ڶԶ˽ڵ��,�����漴���������Ӿ���!");
					if (nMagicNum < m_stLoginInfoLocal.nMagicNum)
					{
						CRLog(E_APPINFO,"������Ϣ:������������ڶԶ������,���˱���Ϊ��,δ������!");
						m_stLoginInfoLocal.nMasterSlave = gc_nSlave;
						nResult = gc_nTokenWait2Wait;
					}
					else if (nMagicNum > m_stLoginInfoLocal.nMagicNum)
					{
						CRLog(E_APPINFO,"������Ϣ:���������С�ڶԶ������,��������Ϊ��,������!");
						m_stLoginInfoLocal.nMasterSlave = gc_nMaster;
						m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
						m_stLoginInfoLocal.tmToken = time(0);
						nResult = gc_nTokenWait2Hold;
					}
					else
					{
						CRLog(E_APPINFO,"������Ϣ:������������ڶԶ������,��������������������Ӿ���!");
						nResult = gc_nTokenWait2Wait;
						blNegotiate = true;
					}
				}
			}
		}
		else
		{
			CRLog(E_APPINFO,"������Ϣ:�Զ�δ������,���˳�����!");
			nResult = gc_nTokenHold2Hold;
		}
	}
	else
	{
		if (gc_nLoginTokenHold == m_stLoginInfoLocal.nLoginToken)
		{//��ͻ
			CRLog(E_APPINFO,"������Ϣ:��ͻ,����ͬʱ��������!");
			if (gc_nStateLogined != m_stLoginInfoLocal.nLoginState)
			{//����δ��¼�ɹ�
				if (gc_nStateLogined != m_stLoginInfoRemote.nLoginState)
				{//�Է�δ��¼�ɹ� 
					if (gc_nIndLogin != m_stLoginInfoLocal.nLoginIndication)
					{//���������޵�¼ָʾ,���������
						CRLog(E_APPINFO,"������Ϣ:�����޵�¼ָʾ,���˷�������,����Э��!");
						m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
						nResult = gc_nTokenHold2Wait;
						blNegotiate = true;
					}
					else
					{//�������е�¼ָʾ,���־��ͻ,����Э��
						CRLog(E_APPINFO,"������Ϣ:�����Ƴ�ͻ��־,���ݵ�¼�������Э��!");
						nResult = gc_nTokenHold2Hold;
						m_blTokenConflict = true;
					}
					//��Ƚϻ�ȡ���Ƶ�ʱ��ͽڵ�Ŵ�С
					//if (m_stLoginInfoLocal.tmToken == m_stLoginInfoRemote.tmToken)
					//{
					//	CRLog(E_APPINFO,"������Ϣ:���˻������ʱ���һ��!");
					//	if (m_nNodeID > nPeerNodeID)
					//	{
					//		CRLog(E_APPINFO,"������Ϣ:���˽ڵ�Ŵ��ڶԶ˽ڵ��,���˷�������!");
					//		m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
					//	}
					//	else if (m_nNodeID < nPeerNodeID)
					//	{
					//		CRLog(E_APPINFO,"������Ϣ:���˽ڵ��С�ڶԶ˽ڵ��,���˼�����������!");
					//	}
					//	else
					//	{
					//		CRLog(E_APPINFO,"������Ϣ:���˽ڵ�ŵ��ڶԶ˽ڵ��,�����漴���������Ӿ���!");
					//		if (nMagicNum < m_stLoginInfoLocal.nMagicNum)
					//		{
					//			CRLog(E_APPINFO,"������Ϣ:������������ڶԶ������,���˷�������!");
					//			m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
					//		}
					//		else if (nMagicNum > m_stLoginInfoLocal.nMagicNum)
					//		{
					//			CRLog(E_APPINFO,"������Ϣ:���������С�ڶԶ������,���˼�����������!");
					//		}
					//		else
					//		{
					//			CRLog(E_APPINFO,"������Ϣ:������������ڶԶ������,��������������������Ӿ���!");
					//			blMagicNum = true;
					//		}
					//	}
					//}
					//else if (m_stLoginInfoLocal.tmToken < m_stLoginInfoRemote.tmToken)
					//{
					//	CRLog(E_APPINFO,"������Ϣ:���˻������ʱ��ȶԶ���,���˼�����������");
					//}
					//else
					//{
					//	CRLog(E_APPINFO,"������Ϣ:�Զ˻������ʱ��ȱ�����,���˷�������");
					//	m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
					//}
				}
				else
				{//�Է��ѵ�¼�ɹ�
					if (gc_nIndLogin != m_stLoginInfoLocal.nLoginIndication)
					{//���������޵�¼ָʾ,���������
						CRLog(E_APPINFO,"������Ϣ:�����޵�¼ָʾ,���˷�������,����Э��!");
						m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
						nResult = gc_nTokenHold2Wait;
						blNegotiate = true;
					}
					else
					{//�������е�¼ָʾ,���־��ͻ,����Э��
						CRLog(E_APPINFO,"������Ϣ:�����Ƴ�ͻ��־,���ݵ�¼�������Э��!");
						nResult = gc_nTokenHold2Hold;
						m_blTokenConflict = true;
					}
				}
			}
			else
			{
				if (gc_nStateLogined != m_stLoginInfoRemote.nLoginState)
				{//Ŀǰ��Զ�������߼�
					CRLog(E_APPINFO,"������Ϣ:�����ѵ�¼�ɹ�,������������!");
					nResult = gc_nTokenHold2Hold;
				}
				else
				{
					CRLog(E_APPINFO,"������Ϣ:�����ѵ�¼�ɹ�,����Э��!");
					//????
					m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
					nResult = gc_nTokenHold2Wait;
					m_blTokenConflict = true;
					blNegotiate = true;
					//�˳�
				}
			}
		}
		else
		{
			CRLog(E_APPINFO,"������Ϣ:�Զ˳�������,����δ������!");
			nResult = gc_nTokenWait2Wait;
		}
	}
	
	m_stLoginInfoLocal.blInfoSended = true;
	m_csState.Unlock();
	return blNegotiate;
}

//����Զ�״̬��Ϣ����
int CLoginMgr::OnStateInfo(CIpcPacket& pkt)
{
	StateInfo stBody;
	CPacketStructIpcOffer::Packet2Struct(stBody, pkt);
	CRLog(E_APPINFO,"����״̬��Ϣ::�Զ˽ڵ�ID(%s),���ӱ�־(%s),��¼ָʾ(%s),��¼״̬(%s),����(%s),����ʱ��(%s),�����(%s)",stBody.node_id.c_str(),stBody.ms_flag.c_str(),stBody.ind_login.c_str(),stBody.login_state.c_str(),stBody.token.c_str(),stBody.tm_token.c_str(),stBody.magic_number.c_str());

	//�Ƿ���Ҫ��¼
	bool blLogout = false;
	bool blLogin = false;
	int nResult = gc_nTokenWait2Wait;	
	m_csState.Lock();
	bool blNegotiate = HandleStateInfo(stBody,nResult);
	if (gc_nTokenWait2Hold == nResult)
	{
		blLogin = true;
	}
	else if (gc_nTokenHold2Wait == nResult)
	{
		//�˳�
		blLogout = true;
	}

	StateInfo stBodyRsp;
	stBodyRsp.node_id = stBody.node_id;
	stBodyRsp.ms_flag = ToString<int>(m_stLoginInfoLocal.nMasterSlave);
	stBodyRsp.ind_login = ToString<int>(m_stLoginInfoLocal.nLoginIndication);
	stBodyRsp.login_state = ToString<int>(m_stLoginInfoLocal.nLoginState);
	stBodyRsp.token = ToString<int>(m_stLoginInfoLocal.nLoginToken);
	stBodyRsp.tm_token = ToString<time_t>(m_stLoginInfoLocal.tmToken);
	stBodyRsp.magic_number = ToString<int>(m_stLoginInfoLocal.nMagicNum);
	stBodyRsp.pkt_flag = ToString<int>(gc_nPktResponse);
	stBodyRsp.node_peer_id = ToString<int>(m_nNodeID);
	m_csState.Unlock();

	//������Ӧ
	CIpcPacket pktRsp("StateRsp");
	CPacketStructIpcOffer::Struct2Packet(stBodyRsp, pktRsp);
	if (0 != m_pCpMgr)
		m_pCpMgr->Forward(pktRsp,m_ulKey);

	CRLog(E_APPINFO,"����״̬��Ӧ:���˽ڵ�ID(%d),���ӱ�־(%s),��¼ָʾ(%s),��¼״̬(%s),����(%s),����ʱ��(%s),�����(%s)",m_nNodeID,stBodyRsp.ms_flag.c_str(),stBodyRsp.ind_login.c_str(),stBodyRsp.login_state.c_str(),stBodyRsp.token.c_str(),stBodyRsp.tm_token.c_str(),stBodyRsp.magic_number.c_str());
	
	//�����Ҫ���е�¼
	if (blLogin)
	{
		PrepareLogin();
	}

	if (blLogout)
	{
		LogoutInd();
	}
	return 0;
}

//���յ���״̬Ӧ��
int CLoginMgr::OnStateInfoRsp(CIpcPacket& pktRsp)
{
	StateInfo stBody;
	CPacketStructIpcOffer::Packet2Struct(stBody, pktRsp);
	CRLog(E_APPINFO,"����״̬��Ӧ::�Զ˽ڵ�ID(%s),���ӱ�־(%s),��¼ָʾ(%s),��¼״̬(%s),����(%s),����ʱ��(%s),�����(%s)",stBody.node_peer_id.c_str(),stBody.ms_flag.c_str(),stBody.ind_login.c_str(),stBody.login_state.c_str(),stBody.token.c_str(),stBody.tm_token.c_str(),stBody.magic_number.c_str());

	bool blNegotiate = false;
	bool blLogin = false;
	int nResult = gc_nTokenWait2Wait;
	m_csState.Lock();
	blNegotiate = HandleStateInfo(stBody,nResult);
	if (gc_nTokenWait2Hold == nResult)
	{
		blLogin = true;
	}
	m_csState.Unlock();

	//�����Ҫ���е�¼
	if (blLogin)
	{
		PrepareLogin();
	}
	else if (blNegotiate)
	{//�������������,����Э��
		SendStateInfo(blNegotiate);
	}
	return 0;
}

//���͵�¼״̬��Ϣ
int CLoginMgr::SendStateInfo(bool blMagicGen)
{
	CIpcPacket pkt("StateInfo");
	StateInfo stBody;
	stBody.node_id = ToString<int>(m_nNodeID);
	
	//���������
	int nMagicNum = 0;
	if (blMagicGen)
	{
		srand(static_cast<unsigned int>(time(0)));
		int RANGE_MIN = 0;
		int RANGE_MAX = 10000;
		nMagicNum = rand() * (RANGE_MAX - RANGE_MIN) / RAND_MAX + RANGE_MIN;
	}

	m_csState.Lock();
	if (blMagicGen)
	{
		m_stLoginInfoLocal.nMagicNum = nMagicNum;
	}
	stBody.ms_flag = ToString<int>(m_stLoginInfoLocal.nMasterSlave);
	stBody.ind_login = ToString<int>(m_stLoginInfoLocal.nLoginIndication);
	stBody.login_state = ToString<int>(m_stLoginInfoLocal.nLoginState);
	stBody.token = ToString<int>(m_stLoginInfoLocal.nLoginToken);
	stBody.tm_token = ToString<time_t>(m_stLoginInfoLocal.tmToken);
	stBody.magic_number = ToString<int>(m_stLoginInfoLocal.nMagicNum);
	stBody.pkt_flag = ToString<int>(gc_nPktRequest);
	m_csState.Unlock();

	CPacketStructIpcOffer::Struct2Packet(stBody, pkt);
	if (0 != m_pCpMgr)
		m_pCpMgr->Forward(pkt,m_ulKey);

	CRLog(E_APPINFO,"����״̬��Ϣ:���˽ڵ�ID(%s),���ӱ�־(%s),��¼ָʾ(%s),��¼״̬(%s),����(%s),����ʱ��(%s),�����(%s)",stBody.node_id.c_str(),stBody.ms_flag.c_str(),stBody.ind_login.c_str(),stBody.login_state.c_str(),stBody.token.c_str(),stBody.tm_token.c_str(),stBody.magic_number.c_str());
	return 0;
}

//�Ƿ��Ѿ���¼
bool CLoginMgr::IsLogined()
{
	bool blFlag = false;
	m_csState.Lock();
	if (gc_nStateLogined == m_stLoginInfoLocal.nLoginResult)
	{
		blFlag = true;
	}
	m_csState.Unlock();
	return blFlag;
}
bool CLoginMgr::IsNeedLogin()
{
	bool blFlag = false;
	m_csState.Lock();
	if (gc_nLoginTokenHold == m_stLoginInfoLocal.nLoginToken && 
		/*gc_nIndLogin == m_stLoginInfoLocal.nLoginIndication && */
		gc_nStateLogined != m_stLoginInfoLocal.nLoginState)
	{
		blFlag = true;
	}
	m_csState.Unlock();

	return blFlag;
}

//���ܲ�ѯ
int CLoginMgr::LoginQuery(CNMO& oNmo) const
{
	oNmo.m_nQuality = gc_nQuolityUncertain;
	oNmo.m_sTimeStamp = CGessDate::NowToString("-") + " " + CGessTime::NowToString(":");
	if (oNmo.m_sOid == gc_sSgeLoginState)
	{
		oNmo.m_nQuality = gc_nQuolityGood;
		//oNmo.m_sValue = ToString<int>(m_stLoginInfoLocal.nLoginState);
		oNmo.m_sValue = ToString<int>(m_stLoginInfoLocal.nLoginResult);
	}
	else if(oNmo.m_sOid == gc_sSgeLoginToken)
	{
		oNmo.m_nQuality = gc_nQuolityGood;
		oNmo.m_sValue = ToString<int>(m_stLoginInfoLocal.nLoginToken);
	}
	else if(oNmo.m_sOid == gc_sSgeLoginInd)
	{
		oNmo.m_nQuality = gc_nQuolityGood;
		oNmo.m_sValue = ToString<int>(m_stLoginInfoLocal.nLoginIndication);
	}
	else if(oNmo.m_sOid == gc_sSgeLoginAlarm)
	{
		oNmo.m_nQuality = gc_nQuolityGood;
		int nAlmStat = GetAlarmStat();
		oNmo.m_sValue = ToString<int>(nAlmStat);
	}
	return 0;
}

//�Ƿ�澯״̬
int CLoginMgr::GetAlarmStat() const
{
	int nAlm = gc_nLoginNormal;
	m_csState.Lock();
	if (gc_nLoginTokenHold == m_stLoginInfoLocal.nLoginToken 
		&& gc_nIndLogin == m_stLoginInfoLocal.nLoginIndication 
		&& gc_nStateLogined != m_stLoginInfoLocal.nLoginState)
	{
		nAlm = gc_nLoginAlarm;
	}
	m_csState.Unlock();
	return nAlm;
}