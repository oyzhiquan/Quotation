#ifndef _TRANSLATOR_CP_H
#define _TRANSLATOR_CP_H
#include "BroadcastPacket.h"
#include "SamplerPacket.h"
#include "WorkThread.h"

//��Լ״̬ ������:TInstStateFlag
#define I_INITING		'0'     //��ʼ����
#define I_INIT		'1'     //��ʼ�����
#define I_BEGIN		'2'     //����
#define I_GRP_ORDER		'3'     //���۱���
#define I_GRP_MATCH		'4'     //���۴��
#define I_NORMAL		'5'     //��������
#define I_PAUSE		'6'     //��ͣ
#define I_DERY_APP		'7'     //�����걨
#define I_DERY_MATCH		'8'     //�����걨����
#define I_MID_APP		'9'     //�������걨
#define I_MID_MATCH		'A'     //�����걨���
#define I_END		'B'     //����


// ����ʱ�䶨��, Jerry Lee, 2012-3-26

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

class CDataSrcCpMgr;
class CTranslator : public CConnectPointAsyn, public CWorkThread
{
public:
	CTranslator(void);
	~CTranslator(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &pkt);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);
	unsigned int QueueLen() {return m_deqQuotation.size();}	
	unsigned int InPktStatic() {return m_uiInCount;}
	void ZipPktOut(int nOut);
private:
	CDataSrcCpMgr*     m_pDataSrcCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;

	//�г���Լ״̬
	unsigned char	m_ucInstState;
	//���հ�����
	unsigned int	m_uiInCount;	
	//ѹ�����ĵ������
	int				m_nZipOut;

	//����㲥����(ȫ��������)
	std::deque<CBroadcastPacket> m_deqQuotation;
	CCondMutex	m_deqCondMutex;
	
	//�������������ƽ�����,���ڷ���ȫ������
	map<std::string, QUOTATION> m_mapQuotation;

    // ���ݹ���ʱ��Ƭ����
    TimeFilter m_timeFilters[7];

	int ThreadEntry();
	int End();

	int Translate(CBroadcastPacket& oPktSrc);
	int TranslateUnzipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation);
	int TranslateZipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation);
	void HandleInstState(CBroadcastPacket& pkt);
	void HandleSysStat(CBroadcastPacket& pkt);

	//��ԼIDת��
	void ConvertInstID(string& sInstID);
};
#endif