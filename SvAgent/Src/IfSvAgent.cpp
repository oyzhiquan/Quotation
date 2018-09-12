#include "IfSvAgent.h"
#include "Logger.h"
#include "strutils.h"
#include "NetMgrModule.h"

using namespace strutils;

CIfSvAgent::CIfSvAgent(void)
:m_pNetMgrModule(0)
,m_pIfH1(0)
,m_pIfH2(0)
,m_pCpMgr(0)
,m_ulKey(0)
,m_pCfg(0)
,m_uiNodeID(0)
{

}

CIfSvAgent::~CIfSvAgent(void)
{

}

//��ʼ������ 
int CIfSvAgent::Init(CConfig* pCfg)
{
	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;
	string sTmp = "";
	CConfig* pCfgGloabal = pCfg->GetCfgGlobal();
	if (0 == pCfgGloabal)
		return 0;

	if (0 == pCfgGloabal->GetProperty("node_id",sTmp))
	{
		m_uiNodeID = FromString<unsigned int>(sTmp);
	}
	return 0;
}

//�����߳� 
int CIfSvAgent::Start()
{
	//���������߳�
	CRLog(E_APPINFO,"%s","Start CIfSvAgent Thread");
	BeginThread();
	return 0;
}

//ֹͣ�߳�
void CIfSvAgent::Stop()
{
	//ֹͣ�����߳�
	CRLog(E_APPINFO,"%s","Stop CIfSvAgent Thread");
	EndThread();
}

//������Դ
void CIfSvAgent::Finish()
{
	m_deqSv.clear();
}

void CIfSvAgent::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_ulKey = ulKey; 
	m_pCpMgr = pCpMgr;
}

//
void CIfSvAgent::SetObj(CNetMgrModule* pNetMgrModule, CConnectPointAsyn* pIfH1,  CConnectPointAsyn* pIfH2)
{
	m_pNetMgrModule = pNetMgrModule;
	m_pIfH1 = pIfH1;
	m_pIfH2 = pIfH2;
}

//�����ͱ����벻ͬ����
int CIfSvAgent::SendPacket(CPacket &pkt)
{
	try
	{
		CSamplerPacket & pktSrc = dynamic_cast<CSamplerPacket&>(pkt);
	
		m_deqCondMutex.Lock();
		m_deqSv.push_back(pktSrc);
		m_deqCondMutex.Signal();
		m_deqCondMutex.Unlock();
		return 0;
	}
	catch(std::bad_cast)
	{
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

	
	try
	{
		CTradePacket & pktSrc = dynamic_cast<CTradePacket&>(pkt);
	
		m_deqCondMutex.Lock();
		m_deqH1.push_back(pktSrc);
		m_deqCondMutex.Signal();
		m_deqCondMutex.Unlock();
		return 0;
	}
	catch(std::bad_cast)
	{
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


	try
	{
		CBroadcastPacket & pktSrc = dynamic_cast<CBroadcastPacket&>(pkt);
	
		m_deqCondMutex.Lock();
		m_deqH2.push_back(pktSrc);
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

int CIfSvAgent::ThreadEntry()
{
	try
	{
		while(!m_bEndThread)
		{
			m_deqCondMutex.Lock();
			while(m_deqSv.empty() && m_deqH2.empty() && m_deqH1.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}
			
			if (!m_deqSv.empty())
			{
				CSamplerPacket& oPkt = m_deqSv.front();
				m_deqCondMutex.Unlock();

				//
				HandleSamplerPacket(oPkt);

				m_deqCondMutex.Lock();
				m_deqSv.pop_front();
				m_deqCondMutex.Unlock();
			}
			else if (!m_deqH1.empty())
			{
				CTradePacket& oPkt = m_deqH1.front();
				m_deqCondMutex.Unlock();

				//
				HandleH1Packet(oPkt);

				m_deqCondMutex.Lock();
				m_deqH1.pop_front();
				m_deqCondMutex.Unlock();
			}
			else if (!m_deqH2.empty())
			{
				CBroadcastPacket& oPkt = m_deqH2.front();
				m_deqCondMutex.Unlock();

				//
				HandleH2Packet(oPkt);

				m_deqCondMutex.Lock();
				m_deqH2.pop_front();
				m_deqCondMutex.Unlock();
			}
			else
			{
			}
		}
		CRLog(E_APPINFO,"%s","CIfSvAgent Thread exit!");
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

int CIfSvAgent::HandleSamplerPacket(CSamplerPacket& oPkt)
{
	try
	{
		string sCmdID = oPkt.GetCmdID();
		string sBody;
		CMessageImpl& msg = dynamic_cast<CMessageImpl&>(oPkt.GetMsg());
		
		if (strutils::ToHexString<unsigned int>(YL_SV_REQ) == sCmdID)
		{//���ϼ��ڵ�ת���ļ�����ѯ����		
			if (0 != msg.GetBinaryField(MSG_IFH1_DATA,sBody))
			{
				//CRLog(E_APPINFO,"��ȡ���ݴ���!");
				return -1;
			}

			//H1�������������MSG_IFH1_DATA��
			CTradePacket oPktH1;
			oPktH1.Decode(sBody.data(), sBody.length());

			//Ԥ��ѯĿ��ڵ��
			unsigned int uiNodeIDDest = 0;
			string sTmp;
			if (0 == oPktH1.GetParameterVal("node_id" , sTmp))
			{
				uiNodeIDDest = FromString<unsigned int>(sTmp);
			}

			if (uiNodeIDDest == m_uiNodeID)
			{//�����ļ�����ѯ
				if (0 != m_pNetMgrModule)
				{
					m_pNetMgrModule->SendPacket(oPktH1);					
				}
				return 0;
			}
			else
			{//�����ڵ�ļ�����ѯ
				//����·�ɱ�
				map<unsigned int, NODE_LIST>::iterator it = m_mapNodeRoute.find(uiNodeIDDest);
				if (it == m_mapNodeRoute.end())
					return -1;
				
				//���ı���ͷ�ڵ��,���ڿ����ȷ·�ɵ���һ��
				CMessage &  msg = oPkt.GetMsg();
				msg.SetField(MSG_NODE_ID, (*it).second.uiNextNode);
				if (0 != m_pCpMgr)
					m_pCpMgr->Forward(oPkt, m_ulKey);
				return 0;
			}
		}
		else if (strutils::ToHexString<unsigned int>(YL_SV_RSP) == sCmdID)
		{//���¼��ڵ�ת���ļ�����ѯӦ��
			if (0 != msg.GetBinaryField(MSG_IFH1_DATA,sBody))
			{
				//CRLog(E_APPINFO,"��ȡ���ݴ���!");
				return -1;
			}

			if (0 != m_pIfH1)
			{//ת����H1�ӿ�
				CTradePacket oPktH1;
				oPktH1.Decode(sBody.data(), sBody.length());
				m_pIfH1->SendPacket(oPkt);
				return 0;
			}
			else
			{//��������ת��
				if (0 != m_pCpMgr)
					m_pCpMgr->Forward(oPkt, m_ulKey);
				return 0;
			}
		}
		else if (strutils::ToHexString<unsigned int>(YL_SV_NTF) == sCmdID)
		{//���¼��ڵ�ת���ļ�����¼�֪ͨ
			if (0 != msg.GetBinaryField(MSG_IFH2_DATA,sBody))
			{
				//CRLog(E_APPINFO,"��ȡ���ݴ���!");
				return -1;
			}

			if (0 != m_pIfH2)
			{//ת����H2�ӿ�
				CBroadcastPacket oPktH2;
				oPktH2.Decode(sBody.data(), sBody.length());
				m_pIfH2->SendPacket(oPktH2);
				CRLog(E_DEBUG, "%s", oPktH2.Print().c_str());
				return 0;
			}
			else
			{//��������ת��
				if (0 != m_pCpMgr)
					m_pCpMgr->Forward(oPkt, m_ulKey);
				return 0;
			}
		}
		else if (strutils::ToHexString<unsigned int>(YL_SUBSCRIP) == sCmdID)
		{//�¼��ڵ㶩��
			unsigned int uiNodeID = 0;
			msg.GetField(MSG_NODE_ID,uiNodeID);

			string sSubData;
			string sTmp = ToString<unsigned int>(uiNodeID);			
			sSubData.append(sTmp.data(), sTmp.length());
			sSubData.append(1,'|');	
			sTmp = ToString<unsigned int>(m_uiNodeID);
			sSubData.append(sTmp.data(), sTmp.length());

			//����·�ɱ�
			NODE_LIST nodeLst;
			nodeLst.uiNextNode = uiNodeID;
			nodeLst.sNodes = sSubData;
			m_mapNodeRoute[uiNodeID] = nodeLst;

			if (0 != m_pIfH2)
			{//���ط�����֪ͨ�ڵ��ϵ
				CBroadcastPacket oPktH2("onNodeRelationNtf");
				oPktH2.AddParameter("mode","sub");
				oPktH2.AddParameter("node_relation",sSubData);
				m_pIfH2->SendPacket(oPktH2);
				return 0;
			}
			else
			{//���ϼ��ڵ�ת���ڵ��ϵ
				CSamplerPacket oPktSub(YL_SV_SUB_NTF);
				CMessageImpl& oMsgSub = dynamic_cast<CMessageImpl&>(oPktSub.GetMsg());
				oMsgSub.SetField(MSG_SEQ_ID,static_cast<unsigned int>(0));
				oMsgSub.SetField(MSG_NODE_ID,m_uiNodeID);
				oMsgSub.SetBinaryField(MSG_SV_SUB_DATA,sSubData);				
				if (0 != m_pCpMgr)
					return m_pCpMgr->Forward(oPktSub,m_ulKey);
				return 0;
			}
			return 0;
		}
		else if (strutils::ToHexString<unsigned int>(YL_SUBSCRIP_RSP) == sCmdID)
		{//���ϼ��ڵ㶩�ĳɹ�
			if (0 == m_pIfH2)
			{//���ж� ��Ҫ������·����Ϣ��ת
				for (map<unsigned int, NODE_LIST>::iterator it =	m_mapNodeRoute.begin(); it != m_mapNodeRoute.end(); ++it)
				{
					CSamplerPacket oPktSub(YL_SV_SUB_NTF);
					CMessageImpl& oMsgSub = dynamic_cast<CMessageImpl&>(oPktSub.GetMsg());
					oMsgSub.SetField(MSG_SEQ_ID,static_cast<unsigned int>(0));
					oMsgSub.SetField(MSG_NODE_ID,m_uiNodeID);
					oMsgSub.SetBinaryField(MSG_SV_SUB_DATA,(*it).second.sNodes);
					if (0 != m_pCpMgr)
						return m_pCpMgr->Forward(oPktSub,m_ulKey);
				}
			}
			return 0;
		}
		else if (strutils::ToHexString<unsigned int>(YL_UNSUBSCRIP) == sCmdID)
		{//�¼��ڵ��˶�
			unsigned int uiNodeID = 0;
			msg.GetField(MSG_NODE_ID,uiNodeID);
			
			string sSubData;
			string sTmp = ToString<unsigned int>(uiNodeID);
			sSubData.append(sTmp.data(), sTmp.length());
			sSubData.append(1,'|');
			sTmp = ToString<unsigned int>(m_uiNodeID);
			sSubData.append(sTmp.data(), sTmp.length());
			
			map<unsigned int, NODE_LIST>::iterator it =	m_mapNodeRoute.find(uiNodeID);
			if (it != m_mapNodeRoute.end())
				m_mapNodeRoute.erase(it);

			if (0 != m_pIfH2)
			{
				CBroadcastPacket oPktH2("onNodeRelationNtf");
				oPktH2.AddParameter("mode","unsub");
				oPktH2.AddParameter("node_relation",sSubData);
				m_pIfH2->SendPacket(oPktH2);
				return 0;
			}
			else
			{
				CSamplerPacket oPktUnSub(YL_SV_UNSUB_NTF);
				CMessageImpl& oMsgUnSub = dynamic_cast<CMessageImpl&>(oPktUnSub.GetMsg());
				oMsgUnSub.SetField(MSG_SEQ_ID,static_cast<unsigned int>(0));
				oMsgUnSub.SetField(MSG_NODE_ID,m_uiNodeID);				
				oMsgUnSub.SetBinaryField(MSG_SV_SUB_DATA,sSubData);
				if (0 != m_pCpMgr)
					return m_pCpMgr->Forward(oPktUnSub,m_ulKey);
				return 0;
			}
		}
		else if (strutils::ToHexString<unsigned int>(YL_SV_SUB_NTF) == sCmdID)
		{//�¼��ڵ㶩��֪ͨ
			unsigned int uiNodeID = 0;
			msg.GetField(MSG_NODE_ID,uiNodeID);
			string sSubData;
			if (0 == msg.GetBinaryField(MSG_SV_SUB_DATA, sSubData))
			{
				sSubData.append(1,'|');
				string sTmp = ToString<unsigned int>(m_uiNodeID);
				sSubData.append(sTmp.data(), sTmp.length());

				vector<string> vNodeID = strutils::explodeQuoted("|",sSubData);
				if (vNodeID.size() > 0)
				{
					NODE_LIST nodeLst;
					nodeLst.uiNextNode = uiNodeID;
					nodeLst.sNodes = sSubData;
					unsigned int uiNodeDest = FromString<unsigned int>(vNodeID[0]);
					m_mapNodeRoute[uiNodeDest] = nodeLst;
				}				

				if (0 != m_pIfH2)
				{
					CBroadcastPacket oPktH2("onNodeRelationNtf");
					oPktH2.AddParameter("mode","sub");
					oPktH2.AddParameter("node_relation",sSubData);
					m_pIfH2->SendPacket(oPktH2);
					return 0;
				}
				else
				{
					CSamplerPacket oPktSub(YL_SV_SUB_NTF);
					CMessageImpl& oMsgSub = dynamic_cast<CMessageImpl&>(oPktSub.GetMsg());
					oMsgSub.SetField(MSG_SEQ_ID,static_cast<unsigned int>(0));
					oMsgSub.SetField(MSG_NODE_ID,m_uiNodeID);
					oMsgSub.SetBinaryField(MSG_SV_SUB_DATA,sSubData);

					if (0 != m_pCpMgr)
						return m_pCpMgr->Forward(oPktSub,m_ulKey);
					return 0;
				}
			}
			return 0;
		}
		else if (strutils::ToHexString<unsigned int>(YL_SV_UNSUB_NTF) == sCmdID)
		{//�¼��ڵ��˶�֪ͨ
			unsigned int uiNodeID = 0;
			msg.GetField(MSG_NODE_ID,uiNodeID);
			string sSubData;
			if (0 == msg.GetBinaryField(MSG_SV_SUB_DATA, sSubData))
			{
				vector<string> vNodeID = strutils::explodeQuoted("|", sSubData);
				if (vNodeID.size() > 0)
				{
					unsigned int uiNodeDest = FromString<unsigned int>(vNodeID[0]);
					map<unsigned int, NODE_LIST>::iterator it =	m_mapNodeRoute.find(uiNodeDest);
					if (it != m_mapNodeRoute.end())
						m_mapNodeRoute.erase(it);
				}

				sSubData.append(1,'|');
				string sTmp = ToString<unsigned int>(m_uiNodeID);
				sSubData.append(sTmp.data(), sTmp.length());

				if (0 != m_pIfH2)
				{
					CBroadcastPacket oPktH2("onNodeRelationNtf");
					oPktH2.AddParameter("mode","unsub");
					oPktH2.AddParameter("node_relation",sSubData);
					m_pIfH2->SendPacket(oPktH2);
					return 0;
				}
				else
				{
					CSamplerPacket oPktSub(YL_SV_UNSUB_NTF);
					CMessageImpl& oMsgSub = dynamic_cast<CMessageImpl&>(oPktSub.GetMsg());
					oMsgSub.SetField(MSG_SEQ_ID,static_cast<unsigned int>(0));
					oMsgSub.SetField(MSG_NODE_ID,m_uiNodeID);
					oMsgSub.SetBinaryField(MSG_SV_SUB_DATA,sSubData);
					if (0 != m_pCpMgr)
						return m_pCpMgr->Forward(oPktSub,m_ulKey);
					return 0;
				}
			}
			return 0;
		}
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

//
int CIfSvAgent::HandleH1Packet(CTradePacket& oPkt)
{
	try
	{
		unsigned int uiNodeID = 0;
		string sTmp;
		if (0 == oPkt.GetParameterVal("node_id" , sTmp))
		{
			uiNodeID = FromString<unsigned int>(sTmp);
		}

		int nPktType = oPkt.PktType();
		if (HEAD_REQ == nPktType)
		{//������ ֱ��������H1�ӿ�
			if (uiNodeID == m_uiNodeID)
			{//�����ļ�����ѯ
				if (0 != m_pNetMgrModule)
				{
					m_pNetMgrModule->SendPacket(oPkt);					
				}
				return 0;
			}
			else
			{//�����ڵ�ļ�����ѯ
				//
				map<unsigned int, NODE_LIST>::iterator it = m_mapNodeRoute.find(uiNodeID);
				if (it == m_mapNodeRoute.end())
					return -1;
				
				unsigned int  nLen = 0;
				const char* pcBuf = oPkt.Encode(nLen);
				if (0 == pcBuf || 0 == nLen)
					return -1;

				CSamplerPacket oPacketReq(YL_SV_REQ);
				CMessage &  msg = oPacketReq.GetMsg();

				string sBody;
				sBody.append(pcBuf, nLen);
				msg.SetField(MSG_SEQ_ID, static_cast<unsigned int>(0));
				msg.SetField(MSG_NODE_ID, (*it).second.uiNextNode);
				msg.SetBinaryField(MSG_IFH1_DATA, sBody);

				if (0 != m_pCpMgr)
					m_pCpMgr->Forward(oPacketReq, m_ulKey);
				return 0;
			}
		}
		else
		{//Ӧ���� ֱ��������m_pNetMgrAgent
			if (0 != m_pIfH1)
			{
				m_pIfH1->SendPacket(oPkt);
				return 0;
			}
			else
			{
				unsigned int  nLen = 0;
				const char* pcBuf = oPkt.Encode(nLen);
				if (0 == pcBuf || 0 == nLen)
					return -1;

				CSamplerPacket oPacketRsp(YL_SV_RSP);
				CMessage &  msg = oPacketRsp.GetMsg();

				string sBody;
				sBody.append(pcBuf, nLen);
				msg.SetField(MSG_SEQ_ID, static_cast<unsigned int>(0));
				msg.SetField(MSG_NODE_ID, m_uiNodeID);
				msg.SetBinaryField(MSG_IFH1_DATA, sBody);

				if (0 != m_pCpMgr)
					m_pCpMgr->Forward(oPacketRsp, m_ulKey);
				return 0;
			}
		}
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

int CIfSvAgent::HandleH2Packet(CBroadcastPacket& oPkt)
{
	try
	{
		//����m_pNetMgrAgent �¼��澯�౨��
		if (0 != m_pIfH2)
		{//������H2�ӿ� ���ڵ�ֱ��ת����H2�ӿ�
			m_pIfH2->SendPacket(oPkt);
			return 0;
		}
		else
		{//��������H2�ӿ� ����ת��
			unsigned int  nLen = 0;
			const char* pcBuf = oPkt.Encode(nLen);
			if (0 == pcBuf || 0 == nLen)
				return -1;

			CSamplerPacket oPacketNtf(YL_SV_NTF);
			CMessage &  msg = oPacketNtf.GetMsg();

			string sBody;
			sBody.append(pcBuf, nLen);
			msg.SetField(MSG_SEQ_ID, static_cast<unsigned int>(0));
			msg.SetField(MSG_NODE_ID, m_uiNodeID);
			msg.SetBinaryField(MSG_IFH2_DATA, sBody);

			if (0 != m_pCpMgr)
				m_pCpMgr->Forward(oPacketNtf, m_ulKey);
			return 0;
		}
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

int CIfSvAgent::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","CIfSvAgent thread wait end");
	Wait();
	return 0;
}

