#pragma once
#include "PacketBodyStructIpcOffer.h"
#include "OfferConstant.h"
#include "MibConstant.h"
#include "WorkThreadNm.h"
#include "Comm.h"
#include "Logger.h"
#include "IpcPacket.h"

using namespace MibConst;
using namespace OfferConst;
using namespace ipcoffer;

class CLoginMgr:public CConnectPointAsyn, public CWorkThreadNm
{
private:
	//��¼��ʱ��,���ڵ�¼ʧ�ܺ��Ъ�Ե����³���
	/*class COperationTimer : public CGessTimer
	{
	public:
		COperationTimer():m_pParent(0){}
		virtual ~COperationTimer(){}
		void Bind(CLoginMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			//if (0 != m_pParent)
			//	return m_pParent->LoginTimeout(ulKey,ulTmSpan);

			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CLoginMgr* m_pParent;
	};
*/
	//���ܽӿ�
	class CLoginNm: public CNmoModule
	{
	public:
		CLoginNm():m_pParent(0){}
		virtual ~CLoginNm(){}
		void Bind(CLoginMgr* pParent){m_pParent = pParent;}
		//������ѯ�ӿ�
		int Query(CNMO& oNmo) const
		{
			if (0 == m_pParent)
				return -1;

			return m_pParent->LoginQuery(oNmo);
		}
		//������ѯ�ӿ�
		int Query(vector< CNMO > & vNmo) const
		{
			if (0 == m_pParent)
				return -1;

			for (vector< CNMO >::iterator it = vNmo.begin(); it != vNmo.end(); ++it)
			{
				m_pParent->LoginQuery(*it);
			}
			return 0;
		}
		//���ƽӿ�
		int Operate(const string &sOid, int nflag, const string &sDstValue, const string &sParamer) 
		{
			if (0 == m_pParent)
				return -1;
			return m_pParent->LoginOperate(sOid, nflag, sDstValue, sParamer);
		}
	private:
		CLoginMgr* m_pParent;
	};
public:	
	CLoginMgr();
	virtual ~CLoginMgr();
public:
	//�����Ǽ̳���CConnectPointAsyn
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);	
	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int SendPacket(CPacket &GessPacket);
	int OnRecvPacket(CPacket &GessPacket);

	int LogoutInd();


	
	//��¼�ǳ����֪ͨ,����ģ�����
	void OnLogin(int nResult);
	void OnLogout(int nResult);
 
	//���ӶԶ˽��֪ͨ
	void ConnectNtf(int nFlag);
	//���ӶԶ��ж�֪ͨ
	void DisconnectNtf();
	//���յ��Զ�����֪ͨ
	void AcceptNtf();

private:
	int ThreadEntry();
	int End();
	
	//�������߳�״̬�Ƿ���Ҫ������
	bool IsNetManaged(string& sKeyName);
	//���ܲ�ѯ
	int LoginQuery(CNMO& oNmo) const;
	//���ܿ��ƽӿ�
	int LoginOperate(const string &sOid, int nflag, const string &sDstValue, const string &sParamer) {return 0;}
	//�Ƿ�澯״̬
	int GetAlarmStat() const;

	//�Ƿ���Ҫ��¼
	bool IsNeedLogin();
	//�Ƿ���Ҫ�ǳ�
	bool IsNeedLogout();
	//ִ�е�¼
	int PrepareLogin();
	//�Ƿ��Ѿ���¼
	bool IsLogined();

	//����״̬��Ϣ
	int SendStateInfo(bool blMagicGen = false);
	//״̬��Ϣ����
	bool HandleStateInfo(StateInfo& stBody,int& nResult);
	
	//��ʱ����ʱ������
	int LoginTimeout(const string& ulKey,unsigned long& ulTmSpan);

	CConnectPointManager*	m_pCpMgr;		//���ӵ������
	unsigned int	m_ulKey;				//���ӵ�key
	CConfig *		m_pCfg;					//���ýӿ�
	std::deque<CIpcPacket> m_deqMIf;		//˫��״̬��Ϣ���Ķ���
	CCondMutex	m_deqCondMutex;

	typedef struct tagLoginStateInfo
	{
		int	nMasterSlave;					//���ӱ�־����1�� 0��
		int nLoginIndication;				//��¼ָʾ 0ָʾ��¼ 1ָʾ�ǳ�
		int nLoginToken;					//��¼���� 0���� 1δ����
		time_t			tmToken;			//��ȡ���Ƶ�ʱ���		
		int nLoginState;                    //��¼״̬
		int nLoginResult;                   //��¼���
		int nMagicNum;						//�����
		bool blInfoSended;					//�ѷ��ͱ�־

		tagLoginStateInfo()
		{
			nMasterSlave = gc_nMaster;
			nLoginIndication = gc_nIndUnknown;
			nLoginToken = gc_nLoginTokenWait;
			tmToken = 0;
			nLoginState = gc_nStateLoginInit;
			nLoginResult = gc_nStateLoginInit;
			nMagicNum = 0;
			blInfoSended = false;
		}
	} LOGIN_STATE_INFO, *PLOGIN_STATE_INFO;

	LOGIN_STATE_INFO m_stLoginInfoLocal;	//���˵�¼��Ϣ
	LOGIN_STATE_INFO m_stLoginInfoRemote;	//�Զ˵�¼��Ϣ
	bool m_blTokenConflict;					//���Ƴ�ͻ
	int	m_nConnectState;					//��Զ�����״̬
	bool m_blFirstConnectState;				//�Ƿ��һ��������ĵ�һ������״̬��Ϣ
	int	m_nNodeID;							//�ڵ�ID
	mutable CGessMutex m_csState;

	CLoginNm			 m_oNm;				//���ܽӿ�ģ��
private:
	//�����Ա����ָ��
	typedef int (CLoginMgr::*MFP_PacketHandleApi)(CIpcPacket& pkt);

	//�����������뱨�Ĵ����Ա����ӳ��ṹ
	typedef struct tagCmd2Api
	{
		string sApiName;						//����ApiName���״���
		MFP_PacketHandleApi pMemberFunc;		//���Ĵ�����ָ��
	} Cmd2Api;

	//�����������뱨�Ĵ����Ա����ӳ���
	static Cmd2Api m_Cmd2Api[];

	int RunPacketHandleApi(CIpcPacket& pkt);
private:
	int OnStateInfo(CIpcPacket& pkt);
	int OnStateInfoRsp(CIpcPacket& pkt);
};
