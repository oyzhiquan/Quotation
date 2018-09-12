#include "ServiceHandlerCln.h"
#include "ProviderCpMgr.h"

//���Ķ�Ӧ�Ĵ����Ա�������ñ�
CServiceHandlerCln::PktCmd2Api CServiceHandlerCln::m_PktCmd2Api[] =
{
	//����ApiName					��������ָ��
	{"80000001",							&CServiceHandlerCln::OnRecvLoginRsp},
	{"80000002",							&CServiceHandlerCln::OnRecvLogoutRsp},
	{"80000003",							&CServiceHandlerCln::OnRecvSubscripRsp},
	{"80000004",							&CServiceHandlerCln::OnRecvUnSubscripRsp},
	{"80000005",							0},//&CServiceHandlerCln::OnRecvHelloRsp}
};

CServiceHandlerCln::CServiceHandlerCln(void)
:m_pProviderCpMgr(0)
,m_pCfg(0)
{
}

CServiceHandlerCln::~CServiceHandlerCln(void)
{
}

//��ʼ�����ã������̳߳�
int CServiceHandlerCln::Init(CConfig* pCfg)
{
	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;
	//CConfig* pConfig = m_pCfg->GetCfgGlobal();

	return 0;
}

//���������̼߳������̳߳�
int CServiceHandlerCln::Start()
{
	//���������߳�
	BeginThread();
	return 0;
}

//ֹͣ�����̳߳ؼ������߳�
void CServiceHandlerCln::Stop()
{
	//ֹͣ�����߳�
	CRLog(E_APPINFO,"%s","Stop ServiceHandler Thread");
	EndThread();
}

//������Դ
void CServiceHandlerCln::Finish()
{
	m_deqService.clear();
}

void CServiceHandlerCln::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_ulKey = ulKey; 
	m_pProviderCpMgr = dynamic_cast<CProviderCpMgr*>(pCpMgr);
}


int CServiceHandlerCln::SendPacket(CPacket &pkt)
{
	try
	{
		CSamplerPacket & pktService = dynamic_cast<CSamplerPacket&>(pkt);
				
		std::string sCmdID = pkt.GetCmdID();
		if (sCmdID == "00000006")
		{
			CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pktService.GetMsg());
			string sQuotationVal;
			msg.GetBinaryField(MSG_QUOTATION_RECS,sQuotationVal);
			
			if (0 != m_pProviderCpMgr)
			{
				m_pProviderCpMgr->ToHisDataFx(sQuotationVal);
			}
			return 0;
		}

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

int CServiceHandlerCln::ThreadEntry()
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

int CServiceHandlerCln::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","ServiceHanlder thread wait end");
	Wait();
	return 0;
}

//ƥ�䱨�Ĵ����Ա���������е��ô��� 
int CServiceHandlerCln::RunPacketHandleApi(CSamplerPacket& pkt)
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

		CRLog(E_ERROR,"Unknown packet! sCmdID= %s" ,sCmdID.c_str());
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

int CServiceHandlerCln::OnRecvLoginRsp(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);
	m_sNodeID = ToHexString<unsigned int>(uiNodeID);

	unsigned int uiRst = 0;
	msg.GetField(MSG_LOGIN_RESULT,uiRst);
	CRLog(E_PROINFO,"OnRecvLoginRsp SeqNo:%u, NodeID:%u, Result:%u",uiSeqNo, uiNodeID, uiRst);

	return 0;
}

int CServiceHandlerCln::OnRecvLogoutRsp(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);
	m_sNodeID = ToHexString<unsigned int>(uiNodeID);

	unsigned int uiRst = 0;
	msg.GetField(MSG_LOGIN_RESULT,uiRst);
	CRLog(E_PROINFO,"OnRecvLogoutRsp SeqNo:%u, NodeID:%u, Result:%u",uiSeqNo, uiNodeID, uiRst);

	return 0;
}

int CServiceHandlerCln::OnRecvSubscripRsp(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);

	unsigned int uiRst = 0;
	msg.GetField(MSG_SUBSCRIP_RESULT,uiRst);
	CRLog(E_PROINFO,"OnRecvSubscripRsp SeqNo:%u, NodeID:%u, Result:%u",uiSeqNo, uiNodeID, uiRst);


	return 0;
}

int CServiceHandlerCln::OnRecvUnSubscripRsp(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);

	unsigned int uiRst = 0;
	msg.GetField(MSG_SUBSCRIP_RESULT,uiRst);
	CRLog(E_PROINFO,"OnRecvUnSubscripRsp SeqNo:%u, NodeID:%u, Result:%u",uiSeqNo, uiNodeID, uiRst);


	return 0;
}


//int CServiceHandlerCln::OnRecvHelloRsp(CSamplerPacket& pkt)
//{
//	
//	CRLog(E_APPINFO,"OnRecvHelloRsp");
//	return 0;
//}
