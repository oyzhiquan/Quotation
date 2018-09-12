#include "ServiceHandler.h"
#include "ProviderCpMgr.h"
#include "MemData.h"

//���Ķ�Ӧ�Ĵ����Ա�������ñ�
CServiceHandler::PktCmd2Api CServiceHandler::m_PktCmd2Api[] =
{
	//����ApiName					��������ָ��
	{"00000001",							&CServiceHandler::OnRecvLogin},
	{"00000002",							&CServiceHandler::OnRecvLogout},
	{"00000003",							&CServiceHandler::OnRecvSubscrip},
	{"00000004",							&CServiceHandler::OnRecvUnSubscrip},
	{"00000005",							&CServiceHandler::OnRecvHello}
};

CServiceHandler::CServiceHandler(void)
:m_pProviderCpMgr(0)
,m_pCfg(0)
{
}

CServiceHandler::~CServiceHandler(void)
{
}

//��ʼ�����ã������̳߳�
int CServiceHandler::Init(CConfig* pCfg)
{
	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;
	//CConfig* pConfig = m_pCfg->GetCfgGlobal();

	return 0;
}

//���������̼߳������̳߳�
int CServiceHandler::Start()
{
	//���������߳�
	BeginThread();
	return 0;
}

//ֹͣ�����̳߳ؼ������߳�
void CServiceHandler::Stop()
{
	//ֹͣ�����߳�
	CRLog(E_APPINFO,"%s","Stop ServiceHandler Thread");
	EndThread();
}

//������Դ
void CServiceHandler::Finish()
{
	m_deqService.clear();
}

void CServiceHandler::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_ulKey = ulKey; 
	m_pProviderCpMgr = dynamic_cast<CProviderCpMgr*>(pCpMgr);
}


int CServiceHandler::SendPacket(CPacket &pkt)
{
	try
	{
		CSamplerPacket & pktService = dynamic_cast<CSamplerPacket&>(pkt);
				
		m_deqCondMutex.Lock();
		m_deqService.push_back(pktService);
		m_deqCondMutex.Signal();
		m_deqCondMutex.Unlock();
		return 0;
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"%s","packet error!");
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception!");
		return -1;
	}
}

int CServiceHandler::ThreadEntry()
{
	try
	{
		while(!m_bEndThread)
		{
			m_deqCondMutex.Lock();
			while(m_deqService.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}

			CSamplerPacket pkt = m_deqService.front();
			m_deqService.pop_front();
			m_deqCondMutex.Unlock();

			try
			{
				RunPacketHandleApi(pkt);
			}
			catch(...)
			{
				CRLog(E_CRITICAL,"%s","Unknown exception");
			}
		}
		CRLog(E_APPINFO,"%s","RiskHandler Thread exit!");
		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception!");
		return -1;
	}
}

int CServiceHandler::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","ServiceHanlder thread wait end");
	Wait();
	return 0;
}

//ƥ�䱨�Ĵ����Ա���������е��ô��� 
int CServiceHandler::RunPacketHandleApi(CSamplerPacket& pkt)
{
	std::string sCmdID;
	try
	{
		sCmdID = pkt.GetCmdID();
	
		int nSize = sizeof(m_PktCmd2Api)/sizeof(PktCmd2Api);
		for ( int i = 0 ; i < nSize ; i++ )
		{
			if ( m_PktCmd2Api[i].sApiName == sCmdID )
			{
				if (m_PktCmd2Api[i].pMemberFunc == 0)
					break;

				return (this->*(m_PktCmd2Api[i].pMemberFunc))(pkt);				
			}
		}

		//CRLog(E_ERROR,"Unknown packet!");
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

int CServiceHandler::OnRecvLogin(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	string sUserID;
	msg.GetField(MSG_LOGIN_ID,sUserID);

	string sPwd;
	msg.GetField(MSG_LOGIN_PWD,sPwd);

	unsigned short usEnc = 0;
	msg.GetField(MSG_LOGIN_PWD_ENC,usEnc);

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);

	CRLog(E_PROINFO,"OnRecvLogin SeqNo:%u, NodeID:%u, UserID:%s, Pwd:%s, Enc:%d",uiSeqNo, uiNodeID, sUserID.c_str(),sPwd.c_str(),usEnc);


	CMessageImpl oMsgRsp;
	oMsgRsp.SetField(MSG_SEQ_ID,uiSeqNo);
	oMsgRsp.SetField(MSG_NODE_ID,uiNodeID);

	unsigned int nRst = 0;
	oMsgRsp.SetField(MSG_LOGIN_RESULT,nRst);

	CSamplerPacket oPktRsp(oMsgRsp,YL_LOGIN_RSP);
	if (0 != m_pProviderCpMgr)
		return m_pProviderCpMgr->Forward(oPktRsp,m_ulKey);

	return 0;
}


int CServiceHandler::OnRecvLogout(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	string sUserID;
	msg.GetField(MSG_LOGIN_ID,sUserID);

	string sPwd;
	msg.GetField(MSG_LOGIN_PWD,sPwd);

	unsigned short usEnc = 0;
	msg.GetField(MSG_LOGIN_PWD_ENC,usEnc);

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);

	CRLog(E_PROINFO,"OnRecvLogout SeqNo:%u, NodeID:%u, UserID:%s, Pwd:%s, Enc:%d",uiSeqNo, uiNodeID, sUserID.c_str(),sPwd.c_str(),usEnc);


	CMessageImpl oMsgRsp;
	oMsgRsp.SetField(MSG_SEQ_ID,uiSeqNo);
	oMsgRsp.SetField(MSG_NODE_ID,uiNodeID);

	unsigned int nRst = 0;
	oMsgRsp.SetField(MSG_LOGIN_RESULT,nRst);

	CSamplerPacket oPktRsp(oMsgRsp,YL_LOGOUT_RSP);
	if (0 != m_pProviderCpMgr)
		return m_pProviderCpMgr->Forward(oPktRsp,m_ulKey);

	return 0;
}

int CServiceHandler::OnRecvSubscrip(CSamplerPacket& pkt)
{
	CRLog(E_APPINFO,"%s","App Packet Route error!");
	return 0;
}


int CServiceHandler::OnRecvUnSubscrip(CSamplerPacket& pkt)
{
	CRLog(E_APPINFO,"%s","App Packet Route error!");
	return 0;
}

int CServiceHandler::OnRecvHello(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);

	string sHelloCtn;
	msg.GetBinaryField(MSG_HELLO_CONTENT,sHelloCtn);

	CRLog(E_STATICS,"OnHello SeqNo:%u, NodeID:%u, %s",uiSeqNo, uiNodeID, sHelloCtn.c_str());

	//CMessageImpl oMsgRsp;
	//oMsgRsp.SetField(MSG_SEQ_ID,uiSeqNo);
	//oMsgRsp.SetField(MSG_NODE_ID,uiNodeID);

	//CSamplerPacket oPktRsp(oMsgRsp,YL_HELLO_RSP);
	//if (0 != m_pProviderCpMgr)
	//	return m_pProviderCpMgr->Forward(oPktRsp,m_ulKey);

	return 0;
}