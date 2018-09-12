#ifndef _DATA_SRC_CP_MGR_H
#define _DATA_SRC_CP_MGR_H
#include "netlogdev.h"
#include "IpcPacket.h"
#include "Comm.h"
#include <iostream>
#include "WorkThreadNm.h"
#include "WorkThread.h"
#include "Logger.h"
#include "SamplerPacket.h"
#include "XQueueIo.h"
#include "MemShareAlive.h"


using namespace std;


//���ӵ�key����
typedef enum tagEnumKey
{
	EnumKeyIfH1,
	EnumKeyIfH2,
	EnumKeyIfMC,
	EnumKeyIfMS,
	EnumKeyIfCmd,
	EnumKeyCmdHandler,
	EnumNetMagModule,
	EnumKeyTraderLoginMgr,
	EnumKeyUnknown
} EnumKeyIf;

//�������ӵ���������
const string gc_sCfgIfH1 = "IFH1";
const string gc_sCfgIfH2 = "IFH2";
const string gc_sCfgIfCmd = "IFCMD";
const string gc_sCfgIfMC = "IFMC";
const string gc_sCfgIfMS = "IFMS";
const string gc_sCfgNetMagModule = "net_mgr";
const string gc_sCfgTraderLoginMgr = "login_mgr";

//ȱʡ������ƥ��
const string gc_sDefaultCmdID = "#";


class CConfigImpl;
class CXQueueWriter;

class CGessTimerMgr;
class CConfigImpl;
class COfferConnPoint;
class CLoginMgr;
class CNetMgrModule;

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

class CDataSrcCpMgr:public CProtocolCpMgr
{
	//������ʱ��
	class CDogTimer : public CGessTimer
	{
	public:
		CDogTimer():m_pParent(0){}
		virtual ~CDogTimer(){}
		void Bind(CDataSrcCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnDogTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CDataSrcCpMgr* m_pParent;
	};

	//��ʱ������ʱ��
	class CResetTimer: public CGessAbsTimer
	{
	public:
		CResetTimer():m_pParent(0){}
		~CResetTimer(void){}
		void Bind(CDataSrcCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& sKey)
		{
			if (0 != m_pParent)
				return m_pParent->OnResetTimeout(sKey);
			return -1;
		}
		void TimerCanceled(const string& ulKey){}
	private:
		CDataSrcCpMgr* m_pParent;
	};
private:
	int Query(CNMO& oNmo) ;
	class CDataSrcCpMgrNm: public CNmoModule
	{
	public:
		CDataSrcCpMgrNm():m_pParent(0){}
		virtual ~CDataSrcCpMgrNm(){}
		void Bind(CDataSrcCpMgr* pParent){m_pParent = pParent;}
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
		CDataSrcCpMgr * m_pParent;
	};
public:
	CDataSrcCpMgr();
	virtual ~CDataSrcCpMgr();

	int Init(const string& sProcName);
	void Finish();
	int Start();
	void Stop();

	int OnRecvQuotation(QUOTATION& stQuotation, int nDelay);

	bool Stock_Dll_Init();
	bool Stock_Dll_Release();


public:
	int OnConnect(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	int OnAccept(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);	
	int OnClose(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);
	int OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	//int OnLogout
	int Forward(CPacket &GessPacket,const unsigned long& ulKey);

	int Run();
	void StopMe();
	int StartMe();

	int Bind(long ulHwnd);

	bool Logout();

	int InitRouterTbl();

private:
	int OnResetTimeout(const string& sTmKey);
	int OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan);
	
	//�����¼�֪ͨ
	void NotifyEvent(const string& sEvt,int nGrade = 1);	

private:

	//�����Ա����ָ��
	typedef string (CDataSrcCpMgr::*MFP_CmdHandleApi)(const string& sCmd, const vector<string>& vecPara);
	//�����������뱨�Ĵ����Ա����ӳ��ṹ
	typedef struct tagCmdLine2Api
	{
		string sCmdName;					//CmdName
		string sCmdAbbr;					//������д
		MFP_CmdHandleApi pMemberFunc;		//���Ĵ�����ָ��
		string sHelp;						//����˵��
	} CmdLine2Api;
	//��������������������Ա����ӳ���
	static CmdLine2Api m_CmdLine2Api[];


	//Դ�ӿ�+�������������Ӧ·�ɽӿ�ӳ��ṹ
	typedef struct tagIfRouterCfg
	{
		CConnectPointAsyn* pCp;
		unsigned long ulIfTo;
		unsigned long ulIfFrom;
		string sCmdID;
	} IfRouterCfg;
	//Դ�ӿ�+�������������Ӧ·�ɽӿ�ӳ���ϵ���ñ�
	static IfRouterCfg m_tblIfRouterCfg[];

	//·�ɵ�
	typedef multimap<string,CConnectPointAsyn*> MMAP_CP;
	typedef MMAP_CP::iterator MMAP_IT;
	typedef pair<MMAP_IT,MMAP_IT> RANGE_CP;
	typedef struct tagIfRouterPoint
	{
		unsigned long ulIfFrom;
		MMAP_CP  mmapCmds;
	} IfRouterPoint;
	//�ڴ�·�ɱ�
	IfRouterPoint m_tblIfRouter[EnumKeyUnknown];

private:
	CConnectPointAsyn*		m_pCpInterfaceMC;		//���̻�����M�ͻ��˽ӿ�
	CConnectPointAsyn*		m_pCpInterfaceMS;		//���̻�����M����˽ӿ�

	CLoginMgr*				m_pOfferLoginMgr;		//������¼����


	CConnectPointAsyn*		m_pCpInterfaceH1;		//ϵͳ���H1�ӿ�
	CConnectPointAsyn*		m_pCpInterfaceH2;		//ϵͳ���H2�ӿ�
	CNetMgrModule*			m_pNetMagModule;		//���ܴ���


	//���Ź������ڴ�
	CMemShareAlive			m_oMemShareAlive;

	//
	CDataSrcCpMgrNm			m_oNmoModule;

	//dog��ʱ��
	CDogTimer m_oIfkTimer;
	//��ʱ������ʱ��
	CResetTimer m_oResetTimer;
	//��ʱ����ʱ�������
	vector<CGessTime> m_vResetTime;	


	unsigned int		  m_uiNodeID;
	unsigned int		  m_uiNodeType;
	CConfigImpl*		m_pConfig;
	volatile bool m_bFwdStop;

	long m_hwndOwner;
	string m_sWjfName;

    // ���ݹ���ʱ��Ƭ����
    DataFilter m_dataFilter;

public:
	string& GetWjfName(){return m_sWjfName;}

private:
	vector< CXQueueIo<QUOTATION>* >			m_vecQueueIo;          //        //
	
	//ת������
	unsigned int  m_uiFwdTotal;

	//�ӳ�ͳ����Ϣ
	unsigned int  m_uiDelayLess0s;
	unsigned int  m_uiDelayLess1s;
	unsigned int  m_uiDelayLess2s;
	unsigned int  m_uiDelayLess3s;
	unsigned int  m_uiDelayLess5s;
	unsigned int  m_uiDelayLess10s;
	unsigned int  m_uiDelayLess30s;
	unsigned int  m_uiDelayLess60s;
	unsigned int  m_uiDelayLess120s;
	unsigned int  m_uiDelayMore120s;
	unsigned int  m_uiDelayMin;
	unsigned int  m_uiDelayMax;
};
#endif