#ifndef _DELIVER_CP_H
#define _DELIVER_CP_H
#include "Comm.h"
#include "workthread.h"
#include "GessDate.h"
#include "GessTime.h"
#include "ConfigImpl.h"
#include "SamplerPacket.h"
#include "MemData.h"
#include "NetMgr.h"

class CProviderCpMgr;
class CMemData;
class CDeliverMgr :public CConnectPointAsyn, public CWorkThread
{
private:
	int Query(CNMO& oNmo) ;
	class CDeliverMgrNm: public CNmoModule
	{
	public:
		CDeliverMgrNm():m_pParent(0){}
		virtual ~CDeliverMgrNm(){}
		void Bind(CDeliverMgr* pParent){m_pParent = pParent;}
		//������ѯ�ӿ�
		int Query(CNMO& oNmo) const
		{
			if (0 != m_pParent)
				return m_pParent->Query(oNmo);
			return -1;
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
		CDeliverMgr * m_pParent;
	};
public:
	CDeliverMgr(void);
	~CDeliverMgr(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &pkt);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);
	
	///������Ϣ�����
	int Enque(QUOTATION& stQuotation);
	string HandleCmdLine(const string& sCmd, const vector<string>& vecPara);
private:
	CProviderCpMgr*     m_pProviderCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;
	CConfigImpl		m_oUnitFile;
	map<string, unsigned int>	m_mapUnit;

	//
	CDeliverMgrNm	m_oNmoModule;

	//�������������ƽ�����,���������Ƚ�
	map<std::string, QUOTATION> m_mapQuotation;
	string			m_sFileLast;

	//ͳ�ƴ���
	unsigned int m_uiFwdCount;				//�ۼƷ��Ͱ���
	unsigned int m_uiFwdGBytes;				//�ۼƷ���M�ֽ���
	unsigned int m_uiFwdBytes;				//�ۼƷ����ֽ���
	unsigned int m_uiFwdGBytesLast;			//�ϴμ���ʱM�ֽ���
	unsigned int m_uiFwdBytesLast;			//�ϴμ���ʱ�ֽ���
	CGessDate		m_oRecDate;				//�ϴμ���ʱ����
	CGessTime		m_oRecTime;				//�ϴμ���ʱʱ��
	int				m_nInterval;			//�ۼ�����
	unsigned int	m_uiLastBandWidth;		//���ͳ�ƴ���
	unsigned int	m_uiMaxBandWidth;		//���ͳ�ƴ���
	unsigned int	m_uiMinBandWidth;		//��Сͳ�ƴ���
	unsigned int	m_uiAvgBandWidth;		//ƽ��ͳ�ƴ���
	
	unsigned int m_uiQuoPktCount;			//�ۼ��������
	unsigned int m_uiQuoPktBytes;			//�ۼ�������ֽ���
	unsigned int m_uiQuoPktGBytes;			//�ۼƷ���M�ֽ���
	unsigned int m_uiOrderFwdCount;			//���Ĵ������Ͱ���
	unsigned int m_uiOrderFwdBytes;			//���Ĵ��������ֽ���

	unsigned char   m_ucMaxPerPkt;			//ÿ���������������
	//std::deque<CSamplerPacket> m_deqQuotation;
	std::deque<QUOTATION> m_deqQuotation;
	//CSamplerPacket��ʽ���ı���
	std::deque<CSamplerPacket> m_deqOrder; 

	typedef multimap<string,unsigned int> MMAP_ORDER;
	typedef MMAP_ORDER::iterator MMAP_IT;
	typedef pair<MMAP_IT,MMAP_IT> RANGE_ORDER;
	MMAP_ORDER m_mmapQuotationOrder;		//��һ�ζ�����Ҫ������������ĺ�Լ

	CCondMutex	m_deqCondMutex;

	int ThreadEntry();
	int End();

	int HandleQuotationOrder(CSamplerPacket& oOrder);
	int HandleOrder(CSamplerPacket& oOrder);
	int HandleCancelOrder(CSamplerPacket& pkt);
	int HandleFirstQuotation(const string& sKey,unsigned int uiNodeID);
	int AssemblePkt(QUOTATION& stQuotation, string& sQuotationVal, bitset<FIELDKEY_UNKNOWN>& bsQuotation);
	int FindDifference(QUOTATION& stQuotation, bitset<FIELDKEY_UNKNOWN>& bsQuotation);
	int HandleQuotationDlv(QUOTATION& stQuotation, unsigned char& cPkt, string& sQuotationVal,unsigned int& uiLastMarketType);
	int DelieveQuotation(unsigned int uiMarketType, string& sQuotationVal);
	bool Statics(string& sQuotationVal);

	///���ݽ�㶩����Ϣ�жϵ�ǰ��Լ�Ƿ���Ҫ֪ͨ��0--��Ҫ֪ͨ����0-����֪ͨ
	int CheckSendMsgRole(SUB_CONTEXT& SubscriberContext,const unsigned int & musMarketType);

	//
	string HandleCmdLineReplay(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineBuffer(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineLoad(const string& sCmd, const vector<string>& vecPara);
};
#endif