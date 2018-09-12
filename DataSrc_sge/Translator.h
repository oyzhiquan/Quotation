#ifndef _TRANSLATOR_CP_H
#define _TRANSLATOR_CP_H
#include "BroadcastPacket.h"
#include "SamplerPacket.h"
#include "WorkThread.h"
#include "ConfigImpl.h"

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

// ���ڶ���
typedef struct tagDatePoint
{
    int year;
    int month;
    int day;
}DatePoint;

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

    // strTimeFilter��ʽ: 12:00-15:59;21:00-23:59;00:00-5:59, �����������,
    // 12:00-15:59����һ��ʱ���, ��;�Ÿ���ÿ��ʱ���
    void set(string strTimeFilter)
    {
        slices.clear();

        vector<string> vTimeSlice = explodeQuoted(";", strTimeFilter);
        for (int i = 0; i < vTimeSlice.size(); i++)
        {
            vector<string> vTimePoint = explodeQuoted("-",vTimeSlice[i]);

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
                int nX = hour*100 + minute;
                int nBegin = ts.begin.hour*100 + ts.begin.minute;
                int nEnd = ts.end.hour*100 + ts.end.minute;
                if (nX >= nBegin && nX <= nEnd)
                {
                    bRet = true;
                    break;
                }
            }
        }

        return bRet;
    }
}TimeFilter;


// ���ݹ�����
class DataFilter
{

public:
    DataFilter()
    {
        // �������21:00��23:59��
        TimeSlice ts;
        ts.begin.hour = 21;
        ts.begin.minute = 0;
        ts.end.hour = 23;
        ts.end.minute = 59;
        timeFilters[6].slices.push_back(ts);

        // ����һ��00:00��05:59��
        ts.begin.hour = 0;
        ts.begin.minute = 0;
        ts.end.hour = 5;
        ts.end.minute = 59;
        timeFilters[0].slices.push_back(ts);
    }

public:
    vector<DatePoint> holidays;

    // ���ݹ���ʱ��Ƭ����
    TimeFilter timeFilters[7];

    // strHoliday��ʽ����: 20120307,20120328,20120329
    void setHoliday(string strHoliday)
    {
        holidays.clear();

        vector<string> vHoliday = explodeQuoted(",", strHoliday);
        for (int i = 0; i < vHoliday.size(); i++)
        {
            if (vHoliday[i].length() == 8)
            {
                DatePoint dp;
                string strTmp;
                strTmp = vHoliday[i].substr(0, 4); 
                dp.year = atoi(strTmp.c_str());
                strTmp = vHoliday[i].substr(4, 2); 
                dp.month = atoi(strTmp.c_str());
                strTmp = vHoliday[i].substr(6, 2); 
                dp.day = atoi(strTmp.c_str());
                holidays.push_back(dp);
            }
        }
    }

    // dayOfWeek: 0Ϊ�����죬�������¼���
    bool isFilter(int year, int month, int day, int dayOfWeek, int hour, int minute)
    {
        int i = (dayOfWeek+6)%7;

        if (isFilter(year, month, day))
        {
            return true;
        }

        return timeFilters[i].isFilter(hour, minute);
    }

    bool isFilter(int year, int month, int day)
    {
        bool bRet = false;

        if (!holidays.empty())
        {
            for (vector<DatePoint>::iterator it = holidays.begin(); 
                it != holidays.end(); it++)
            {
                DatePoint& dp = *it;
                if ((dp.day == day) && (dp.month == month) && (dp.year == year))
                {
                    bRet = true;
                    break;
                }
            }
        }

        return bRet;
    }
};
//

class CDataSrcCpMgr;
class CTranslator : public CConnectPointAsyn, public CWorkThread
{
public:
	CTranslator(void);
	~CTranslator(void);

	//����CConnectPointAsyn�Ľӿ�
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
	//����CWorkThread�߳̽ӿ�
	int ThreadEntry();
	int End();

	int Translate(CBroadcastPacket& oPktSrc);
	int TranslateUnzipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation);
	int TranslateZipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation);
	void HandleInstState(CBroadcastPacket& pkt);
	void HandleSysStat(CBroadcastPacket& pkt);

	//��ԼIDת��
	void ConvertInstID(string& sInstID);

private:
	CDataSrcCpMgr*		m_pDataSrcCpMgr;
	unsigned long		m_ulKey;
	CConfig*			m_pCfg;

	CConfigImpl			m_oNameConvertFile;
	map<string, string>	m_mapNamePair;

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
    DataFilter m_dataFilter;
	
};
#endif