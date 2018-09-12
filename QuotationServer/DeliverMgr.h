#ifndef _DELIVER_CP_H
#define _DELIVER_CP_H
#include "Comm.h"
#include "workthread.h"
#include "GessDate.h"
#include "GessTime.h"
#include "SamplerPacket.h"
#include "MemData.h"
#include "NetMgr.h"

#define MAX_UINT (0xFFFFFFFF)

// ����ʱ�䶨��, Jerry Lee, 2012-3-22

// ʱ��㶨��
typedef struct tagTimePoint
{
    int hour;
    int minute;
}TimePoint;

// ʱ��Ƭ����
typedef struct tagTimeSlice
{
    TimePoint begin; // ��ʼʱ��
    TimePoint end;   // ����ʱ��
}TimeSlice;

// ����ʱ��ζ���
typedef struct tagTimeFilter
{
    vector<TimeSlice> slices;

    void set(string strTimeFilter)
    {
        slices.clear();

        vector<string> vTimeSlice = explodeQuoted(";", strTimeFilter);
        for (int i = 0; i < vTimeSlice.size(); i++)
        {
            vector<string> vTimePoint = explodeQuoted(",",vTimeSlice[i]);

            TimeSlice ts;
            if (vTimePoint.size() == 2)
            {
                vector<string> vBegin = explodeQuoted(":", vTimePoint[0]);
                vector<string> vEnd = explodeQuoted(":", vTimePoint[1]);

                if ((vBegin.size() == 2) && (vEnd.size() == 2))
                {
                    ts.begin.hour = strutils::FromString<int>(vBegin[0]);
                    ts.begin.minute = strutils::FromString<int>(vBegin[1]);

                    ts.end.hour = strutils::FromString<int>(vEnd[0]);
                    ts.end.minute = strutils::FromString<int>(vEnd[1]);

                    slices.push_back(ts);
                }
            }
        }
    }

    bool isFilter(int hour, int minute)
    {
        bool bRet = false;

        if (!slices.empty())
        {
            for (vector<TimeSlice>::iterator it = slices.begin(); 
                it != slices.end(); it++)
            {
                TimeSlice& ts = *it;
                if (((hour >= ts.begin.hour) && (minute >= ts.begin.minute))
                    && ((hour <= ts.end.hour) && (minute <= ts.end.minute)))
                {
                    bRet = true;
                    break;
                }
            }
        }

        return bRet;
    }
}TimeFilter;

class CQuoSvrCpMgr;
class CMemData;
class CDeliverMgr :public CConnectPointAsyn, public CWorkThread
{
	class CQuoSvrMgrModule: public CNmoModule
	{
	public:
		CQuoSvrMgrModule():m_pParent(0){}
		virtual ~CQuoSvrMgrModule(){}
		void Bind(CDeliverMgr* p){m_pParent = p;}
		//���ܵ�����ѯ�ӿ�
		int Query(CNMO& oNmo) const
		{
			if (0 == m_pParent)
				return -1;

			return m_pParent->Query(oNmo);
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
		CDeliverMgr* m_pParent;
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
	
	// added by Ben, 2011-04-01, ת����ʷ����Ѷ��ʵʱ��
	int HandleHistoryData(CSamplerPacket& oPkt);

	// added by Jerry Lee, 2010-12-23, �������ݰ����ͻ���
    void HandleHistoryData(CPacket &pkt, unsigned int uiNodeId);

    // added by Jerry Lee, 2011-2-25, ������Ѷ���ͻ���
    void HandleInfoData(CPacket &pkt, unsigned int uiNodeId);

private:
	CQuoSvrCpMgr*     m_pQuoSvrCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;
	CQuoSvrMgrModule	m_oMgrModule;

	//ͳ�ƴ���	
	unsigned int m_uiFwdCount;				//�ۼƽ��հ���
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

	std::deque<CSamplerPacket> m_deqQuotation;

	typedef multimap<string,unsigned int> MMAP_ORDER;
	typedef MMAP_ORDER::iterator MMAP_IT;
	typedef pair<MMAP_IT,MMAP_IT> RANGE_ORDER;
	MMAP_ORDER m_mmapQuotationOrder;		//��һ�ζ�����Ҫ������������ĺ�Լ

	CCondMutex	m_deqCondMutex;

    TimeFilter m_timeFilters[7];

	int ThreadEntry();
	int End();

	int HandlePacket(CSamplerPacket& oPkt);
	int HandleOrder(CSamplerPacket& oPkt);
	int HandleCancelOrder(CSamplerPacket& oPkt);
	int HandleQuotationDlv(CSamplerPacket& oPkt);
	int HandleFirstQuotation(string sKey,unsigned int uiNodeID);
	int AssemblePkt(QUOTATION& stQuotation, string& sQuotationVal, bitset<FIELDKEY_UNKNOWN>& bsQuotation);
	int GetInstID(CSamplerPacket& oPkt, map<string,string>& mapInstID);
	bool Statics(string& sQuotationVal);
	int Query(CNMO& oNmo);

	///���ݽ�㶩����Ϣ�жϵ�ǰ��Լ�Ƿ���Ҫ֪ͨ��0--��Ҫ֪ͨ����0-����֪ͨ
	int CheckSendMsgRole(SUB_CONTEXT& SubscriberContext,const unsigned int & musMarketType);

};
#endif