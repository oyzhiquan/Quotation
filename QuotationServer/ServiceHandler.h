#ifndef _SERVICE_HANDLER_H
#define _SERVICE_HANDLER_H
#include "workthread.h"
#include "SamplerPacket.h"

class CQuoSvrCpMgr;
class CServiceHandler :public CConnectPointAsyn, public CWorkThread
{
public:
	CServiceHandler(void);
	~CServiceHandler(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &pkt);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);

private:
	//�����Ա����ָ��
	typedef int (CServiceHandler::*MFP_PktHandleApi)(CSamplerPacket& pkt);
	//�����������뱨�Ĵ����Ա����ӳ��ṹ
	typedef struct tagPktCmd2Api
	{
		string sApiName;						//����ApiName���״���
		MFP_PktHandleApi pMemberFunc;			//���Ĵ�����ָ��
	} PktCmd2Api;
	//�����������뱨�Ĵ����Ա����ӳ���
	static PktCmd2Api m_PktCmd2Api[];
	int RunPacketHandleApi(CSamplerPacket& pkt);
	CQuoSvrCpMgr*      m_pQuoSvrCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;
	string			m_sNodeID;

	std::deque<CSamplerPacket> m_deqService;
	CCondMutex	m_deqCondMutex;

	int ThreadEntry();
	int End();

	int OnRecvLoginRsp(CSamplerPacket& pkt);
	int OnRecvSubscripRsp(CSamplerPacket& pkt);
	int OnRecvLogoutRsp(CSamplerPacket& pkt);
	int OnRecvUnSubscripRsp(CSamplerPacket& pkt);
	//int OnRecvHelloRsp(CSamplerPacket& pkt);

	int OnRecvLogin(CSamplerPacket& pkt);
	int OnRecvSubscrip(CSamplerPacket& pkt);
	int OnRecvLogout(CSamplerPacket& pkt);
	int OnRecvUnSubscrip(CSamplerPacket& pkt);
	int OnRecvHello(CSamplerPacket& pkt);
};
#endif