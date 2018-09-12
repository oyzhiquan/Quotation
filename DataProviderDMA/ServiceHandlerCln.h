#ifndef _SERVICE_HANDLER_CLN_H
#define _SERVICE_HANDLER_CLN_H
#include "workthread.h"
#include "SamplerPacket.h"

class CProviderCpMgr;
class CServiceHandlerCln :public CConnectPointAsyn, public CWorkThread
{
public:
	CServiceHandlerCln(void);
	~CServiceHandlerCln(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &pkt);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);

private:
	//�����Ա����ָ��
	typedef int (CServiceHandlerCln::*MFP_PktHandleApi)(CSamplerPacket& pkt);
	//�����������뱨�Ĵ����Ա����ӳ��ṹ
	typedef struct tagPktCmd2Api
	{
		string sApiName;						//����ApiName���״���
		MFP_PktHandleApi pMemberFunc;			//���Ĵ�����ָ��
	} PktCmd2Api;
	//�����������뱨�Ĵ����Ա����ӳ���
	static PktCmd2Api m_PktCmd2Api[];
	int RunPacketHandleApi(CSamplerPacket& pkt);
	CProviderCpMgr*     m_pProviderCpMgr;
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
};
#endif