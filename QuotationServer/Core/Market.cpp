#include "Market.h"
#include <iostream>
#include "strutils.h"

CMarketMgr::CMarketMgr(void)
{
	m_pConfig = new CConfigImpl();

}


CMarketMgr::~CMarketMgr(void)
{

}
int CMarketMgr::Init()
{
	string sCfgFile = MARKET_MGR_CFG;

	cout << "���������ļ�..." << endl;

	std::string sCfgFilename;
	sCfgFilename = DEFUALT_CONF_PATH PATH_SLASH;
	sCfgFilename = sCfgFilename + sCfgFile;

	if (m_pConfig->Load(sCfgFilename) != 0)
	{
		cout << "���������ļ�[" << sCfgFilename << "]ʧ��!" << endl;
		msleep(3);
		return -1;
	}

	// ȡ���г���������
	string sTmp = "";
	if (0 == m_pConfig->GetProperty(CFG_MARKET_CLASS, sTmp))
	{
		cout << "���������ļ��" << CFG_MARKET_CLASS << "��ʧ��!" << endl;
		msleep(3);
		return -1;
	}

	// ���ݷ������������ʼ���г�����
	vector<string> vMarketTypes = explodeQuoted(",",sTmp);
	for (int i = 0; i < vMarketTypes.size(); i ++)
	{
		CConfig *pCfgMarket;
		pCfgMarket = m_pConfig->GetProperties(vMarketTypes[i]);
		if (!pCfgMarket)
		{
			cout << "���������ļ��" << vMarketTypes[i] << "��ʧ��!" << endl;
			msleep(3);
			continue;
		}

		HSMarketDataType marketType = vMarketTypes[i];
		CMarket* pMarket = new CMarket();
		if (!pMarket)
		{
			cout << "new CMarket ʧ��!" << endl;
			msleep(3);
			return -1;
		}
		//pMarket->Init(pCfgMarket);
		pMarket->Init(pCfgMarket);//marketType
		m_mapMarkets[marketType] = pMarket;
	}
}


#define CFG_MARKET_CLASS                  "�г�����"
#define CFG_MARKET_SECTION_TYPE           "�г�����"
#define CFG_MARKET_SECTION_NAME           "�г�����"
#define CFG_MARKET_SECTION_TRADEPHASE     "����ʱ��"
#define CFG_MARKET_SECTION_PERHAND        "ÿ�ֵ�λ"
#define CFG_MARKET_SECTION_EXTENDMUTILE   "�Ŵ���"

int CMarket::Init(CConfig* pCfg)
{
	if (m_bInit)
		return E_ALREADDONE;

	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;
	m_pCfg = pCfg;

	if (0 == m_pCfg->GetProperty(CFG_MARKET_SECTION_TYPE, sTmp))
	{
		cout << "���������ļ��" << CFG_MARKET_SECTION_TYPE << "��ʧ��!" << endl;
		msleep(3);
		return -1;
	}

	HSMarketDataType marketType;
	stringstream sStream(sTmp);
	sStream >> hex >> marketType;
	cout << marketType << endl;

	// ����Ӳ���ļ�
	if (LoadFromDisk(marketType))
	{
		return true;
	}

	QMarketInfo stMarkeInfo;
	QTradePhase* pMarketTradePhases = NULL;
	memset(&stMarkeInfo, 0, sizeof(QMarketInfo));
	stMarkeInfo.m_MarketType    = marketType;

	if (m_pCfg->GetProperty(CFG_MARKET_SECTION_NAME, sTmp))
	{
		if (sTmp.size() <= MARKETNAME_SIZE)
			strcpy(stMarkeInfo.m_acMarketName, sTmp.c_str()); 
		else
			strncpy(stMarkeInfo.m_acMarketName, sTmp.c_str(), MARKETNAME_SIZE); 
	}

	// ȡ���г��Ľ���ʱ���
	if (m_pCfg->GetProperty(CFG_MARKET_SECTION_TRADEPHASE, sTmp))
	{
		vector<string> vTradePhases = explodeQuoted(",",sTmp);
		stMarkeInfo.m_sPhasesCount = vTradePhases.size();
		pMarketTradePhases = new QTradePhase[vTradePhases.size()];
		if (!pMarketTradePhases)
		{
			char acErr[256];
			sprintf(acErr, "�����ڴ� %s! ��new QTradePhase[%d]��ʧ��!", stMarkeInfo.m_acMarketName, vTradePhases.size());
			CRLog(E_CRITICAL, acErr);
			exit(1);
		}

		for (int i = 0; i < vTradePhases.size(); i ++)
		{
			vector<string> vPhase = explodeQuoted("-", vTradePhases[i]);
			if (vPhase.size() < 2)
			{
				char acErr[256];
				sprintf(acErr, "����ʱ�����ô���! %s", stMarkeInfo.m_acMarketName);
				CRLog(E_CRITICAL, acErr);
				exit(1);
			}
			pMarketTradePhases[i].m_nBeginTime = atoi(vPhase[0]);
			pMarketTradePhases[i].m_nEndTime = atoi(vPhase[1]);
		}
		stMarkeInfo.m_pTradePhases = pMarketTradePhases;
	}

	// ȡ���г��������
	if (m_pCfg->GetProperty(CFG_MARKET_SECTION_SORT, sTmp))
	{
		vector<string> vSorts = explodeQuoted(",",sTmp);
		stMarkeInfo.m_sSortCount = vSorts.size();
		QStockSort* pSort = new QStockSort[vSorts.size()];
		if (!pSort)
		{
			char acErr[256];
			sprintf(acErr, "�����ڴ� %s! ��new QStockSort[%d]��ʧ��!", stMarkeInfo.m_acMarketName, vSorts.size());
			CRLog(E_CRITICAL, acErr);
			exit(1);
		}

		for (int i = 0; i < vTradePhases.size(); i ++)
		{
			vector<string> vPhase = explodeQuoted("-", vTradePhases[i]);
			if (vPhase.size() < 2)
			{
				char acErr[256];
				sprintf(acErr, "����ʱ�����ô���! %s", stMarkeInfo.m_acMarketName);
				CRLog(E_CRITICAL, acErr);
				exit(1);
			}
			pMarketTradePhases[i].m_nBeginTime = atoi(vPhase[0]);
			pMarketTradePhases[i].m_nEndTime = atoi(vPhase[1]);
		}
		stMarkeInfo.m_pTradePhases = pMarketTradePhases;
	}


	m_Buffer_Market.Alloc();

	// ͨ����������ʼ��

	// ȡ�ý���ʱ���

	// ȡ��Ʒ������

	// ȡ�ð������

	string szSection;
	sprintf(szSection, "%s.CFG_MARKET_SECTION_NAME", vMarketTypes[i]);
	if (0 == m_pConfig->GetProperty(szSection, sTmp))
	{
		cout << "���������ļ��" << szSection << "��ʧ��!" << endl;
		msleep(3);
		return -1;
	}

	stringstream sStream(vMarketTypes[i]);
	sStream >> hex >> marketType;
	cout << marketType << endl;


	vector< pair<string,string> > vpaOid;
	pair<string,string> pa;
	string sNmKey = "0";

	pa.first = gc_sFwdCount;
	pa.second = gc_sFwdCount + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sQuoPktMBytes;
	pa.second = gc_sQuoPktMBytes + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sNowBandWidth;
	pa.second = gc_sNowBandWidth + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sMaxBandWidth;
	pa.second = gc_sMaxBandWidth + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sMinBandWidth;
	pa.second = gc_sMinBandWidth + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sAvgBandWidth;
	pa.second = gc_sAvgBandWidth + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sQuoPerPkt;
	pa.second = gc_sQuoPerPkt + "." + sNmKey;
	vpaOid.push_back(pa);


	pa.first = gc_sBytesPerPkt;
	pa.second = gc_sBytesPerPkt + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sSubscribers;
	pa.second = gc_sSubscribers + "." + sNmKey;
	vpaOid.push_back(pa);

	// ���Ӷ�ȡ��������, Jerry Lee, 2012-3-22
	// begin
	// ����һ��21:00��23:59��
	TimeSlice ts;
	ts.begin.hour = 21;
	ts.begin.minute = 0;
	ts.end.hour = 23;
	ts.end.minute = 59;
	m_timeFilters[6].slices.push_back(ts);

	// �������00:00��05:59��
	ts.begin.hour = 0;
	ts.begin.minute = 0;
	ts.end.hour = 5;
	ts.end.minute = 59;
	m_timeFilters[0].slices.push_back(ts);


	char buf[13] = {0};
	string sTmp;
	for (int i = 0; i < 7; i++)
	{   
		sprintf(buf, "FILTER.week%d", i+1);
		if (0 == m_pCfg->GetProperty(buf,sTmp))
		{
			m_timeFilters[i].set(sTmp);
		}
	}
	// end

	m_oMgrModule.Bind(this);
	CNetMgr::Instance()->Register(&m_oMgrModule,vpaOid);
	return 0;
}
