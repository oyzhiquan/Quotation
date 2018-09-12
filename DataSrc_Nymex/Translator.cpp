#include <cassert>
#include "Translator.h"
#include "DataSrcCpMgr.h"

//2014-8-14 ���á�ŦԼԭ�͡����
#define DZH_NY_MARKET	0x5400
//2014-8-14 ���á�������ҵƷ�����
#define PB_NY_MARKET	0x5600

CTranslator::CTranslator(void)
:m_pDataSrcCpMgr(0)
,m_pCfg(0)
,m_ucInstState(I_INIT)
,m_uiInCount(0)
,m_nZipOut(0)
{
}

CTranslator::~CTranslator(void)
{
}

//��ʼ������ 
int CTranslator::Init(CConfig* pCfg)
{
	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;

    // ���Ӷ�ȡ��������, Jerry Lee, 2012-3-26
    // begin
    char buf[16] = {0};
    string sTmp;
    for (int i = 0; i < 7; i++)
    {   
        sprintf(buf, "FILTER.week%d", i+1);
        if (0 == m_pCfg->GetProperty(buf,sTmp))
        {
            m_dataFilter.timeFilters[i].set(sTmp);
        }
    }

    sprintf(buf, "FILTER.holiday");
    if (0 == m_pCfg->GetProperty(buf,sTmp))
    {
        m_dataFilter.setHoliday(sTmp);
    }

    // end

	std::string sNameConvertFile;
	sNameConvertFile = DEFUALT_CONF_PATH PATH_SLASH;
	sNameConvertFile = sNameConvertFile + "NameConvert";
	sNameConvertFile = sNameConvertFile + ".cfg";
	
	m_oNameConvertFile.Load(sNameConvertFile);
	vector<string> vKeys = m_oNameConvertFile.GetKeys();
	for (vector<string>::iterator it = vKeys.begin(); it != vKeys.end(); ++it)
	{
		string sTmp;
		if (0 == m_oNameConvertFile.GetProperty(*it, sTmp))
		{
			m_mapNamePair[*it] = sTmp;
		}
	}
	return 0;
}

//�����߳�
int CTranslator::Start()
{
	//���������߳�
	BeginThread();
	return 0;
}

//ֹͣ�߳�
void CTranslator::Stop()
{
	//ֹͣ�����߳�
	CRLog(E_APPINFO,"%s","Stop Translator Thread");
	EndThread();
}


//������Դ
void CTranslator::Finish()
{
	m_deqQuotation.clear();

	m_mapQuotation.clear();
}


void CTranslator::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_ulKey = ulKey; 
	m_pDataSrcCpMgr = dynamic_cast<CDataSrcCpMgr*>(pCpMgr);
}

int CTranslator::SendPacket(CPacket &pkt)
{
	try
	{
		CBroadcastPacket & pktQuotation = dynamic_cast<CBroadcastPacket&>(pkt);
		string sCmdID = pktQuotation.GetCmdID();
		if (sCmdID == "onRecvRtnDeferInstStateUpdate"
			|| sCmdID == "onRecvRtnForwardInstStateUpdate"
			|| sCmdID == "onRecvRtnSpotInstStateUpdate")
		{
			HandleInstState(pktQuotation);
			return 0;
		}
		else if (sCmdID == "onSysStatChange")
		{
			HandleSysStat(pktQuotation);
			return 0;
		}
	
		m_deqCondMutex.Lock();
		m_deqQuotation.push_back(pktQuotation);
		m_deqCondMutex.Signal();
		m_deqCondMutex.Unlock();
		m_uiInCount++;
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


int CTranslator::ThreadEntry()
{
	try
	{
        while(!m_bEndThread)
		{
			m_deqCondMutex.Lock();
			while(m_deqQuotation.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}

			CBroadcastPacket& pktSrc = m_deqQuotation.front();
			m_deqCondMutex.Unlock();

			try
			{
				Translate(pktSrc);				
			}
			catch(...)
			{
				CRLog(E_CRITICAL,"%s","Unknown exception");
			}

			m_deqCondMutex.Lock();
			m_deqQuotation.pop_front();
			m_deqCondMutex.Unlock();
		}
		CRLog(E_APPINFO,"%s","RiskHandler Thread exit!");
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

int CTranslator::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","ServiceHanlder thread wait end");
	Wait();
	return 0;
}

int CTranslator::Translate(CBroadcastPacket& oPktSrc)
{
	try
	{
        // ��ʱ�汾�������������Ͼŵ�������һ�����֮������ݲ�ת��, Jerry Lee, 2012-3-2
        /*
        �������ļ�������������Ϣ
        FILTER.week1=12:00-15:59;21:00-23:59;00:00-5:59
        FILTER.week2=12:00-15:59;21:00-23:59;00:00-5:59
        FILTER.week3=12:00-15:59;21:00-23:59;00:00-5:59
        FILTER.week4=21:00-23:59;00:00-5:59;12:00-15:11
        FILTER.week5=12:00-15:59;21:00-23:59;00:00-5:59
        FILTER.week6=12:00-15:59;21:00-23:59;00:00-5:59
        FILTER.week7=12:00-15:59;21:00-23:59;00:00-5:59
        FILTER.holiday=20120307,20120328,20120329
        */
        SYSTEMTIME stNow;
        GetLocalTime(&stNow);

		//kenny  ����ʱ�䣬����MAUTD
		
        if (m_dataFilter.isFilter(stNow.wYear, stNow.wMonth, stNow.wDay, 
            stNow.wDayOfWeek, stNow.wHour, stNow.wMinute))
        {
            CRLog(E_APPINFO,"�����ǽ���ʱ�����������!");

            return -1;
        }
		
        /*
        if ((stNow.wDayOfWeek == 0 && stNow.wHour >= 21 && stNow.wHour <= 23)
            || (stNow.wDayOfWeek == 1 && stNow.wHour >= 0 && stNow.wHour < 6))
        {
            CRLog(E_APPINFO,"�����ǽ���ʱ�����������, �г�����Ϊ: %d!", uiMarketType);

            return -1;
        }
        */
        //

        string sZipVal;
		string sInstID;
		oPktSrc.GetParameterVal("StockId",sInstID);

		if (sInstID == "40")
		{
			return -1;
		}

		map<std::string, QUOTATION>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			QUOTATION& stQuotation = (*it).second;
			if (0 != oPktSrc.GetParameterVal("sZipBuff",sZipVal))
			{
				vector<string> vKey = oPktSrc.GetKeys();
				if (vKey.size() > 2)
				{
					if (0 != TranslateUnzipPacket(oPktSrc, stQuotation))
					{
						return -1;
					}
				}
				else
				{
					//CRLog
					return -1;
				}
			}
			else
			{
				if (0 != TranslateZipPacket(oPktSrc, stQuotation))
				{
					return -1;
				}
			}

			//����ÿ�����̼��������Ż���ͣ����Ҵ���������̺�����ʲôʱ�ζ�ת��
			//һ��ϵͳԭ����ʱ�������״̬�������յ�
			if (0 != m_pDataSrcCpMgr )
			{
				m_pDataSrcCpMgr->ToXQueue(stQuotation);
			}
		}
		else
		{
			QUOTATION stQuotation = {0};
			SYSTEMTIME st;
			::GetLocalTime(&st);
			stQuotation.m_uiDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
			stQuotation.m_uiTime = st.wHour * 10000000 + st.wMinute * 100000 + st.wSecond * 1000 + st.wMilliseconds;
			
			if (0 != oPktSrc.GetParameterVal("sZipBuff",sZipVal))
			{
				vector<string> vKey = oPktSrc.GetKeys();
				if (vKey.size() > 2)
				{
					if (0 != TranslateUnzipPacket(oPktSrc, stQuotation))
					{
						return -1;
					}
				}
				else
				{
					//CRLog
					return -1;
				}
			}
			else
			{
				if (0 != TranslateZipPacket(oPktSrc, stQuotation))
				{
					return -1;
				}
			}


			string sNewInstID = sInstID;
			ConvertInstID(sNewInstID);
			if(m_MaketSource == "M002")//���ǻ�
			{
				stQuotation.m_CodeInfo.m_usMarketType = DZH_NY_MARKET;//���ǻ��г�
			}
			else if (m_MaketSource == "M001")//��
			{
				stQuotation.m_CodeInfo.m_usMarketType = PB_NY_MARKET;//���ǻ��г�
			}

			memcpy(stQuotation.m_CodeInfo.m_acCode, sNewInstID.c_str(), min(sizeof(stQuotation.m_CodeInfo.m_acCode), sNewInstID.length()));
			//stQuotation.m_CodeInfo.m_usMarketType = NY_MARKET;//���ǻ��г�
			m_mapQuotation[sInstID] = stQuotation;

			//���̸�����ʱ��ͨѶ�ӿڻ���ȡ��ȫ��������ݽ���ʱ�ξ����Ƿ�ת��
			/*if (I_INITING == m_ucInstState || I_INIT == m_ucInstState 
				|| I_END == m_ucInstState || I_PAUSE == m_ucInstState)
			{
				CRLog(E_DEBUG, "����(TradeState=%c),[%s]%u-%u", m_ucInstState,stQuotation.m_CodeInfo.m_acCode,stQuotation.m_uiDate,stQuotation.m_uiTime);
				return 0;
			}
			else */if (0 != m_pDataSrcCpMgr)
			{
				m_pDataSrcCpMgr->ToXQueue(stQuotation);
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

int CTranslator::TranslateUnzipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation)
{
	//
	int nRtn = 0;
	string strTmp = "";
	if ( 0 == oPktSrc.GetParameterVal("StockName",strTmp) )
	{
		//strncpy(stQuotation.m_CodeInfo.m_acName, strTmp.c_str(), min(strTmp.length(), sizeof(stQuotation.m_CodeInfo.m_acName)));
	}

	//�����г���Դ 
	if ( 0 == oPktSrc.GetParameterVal("market_code",strTmp) )
	{
		m_MaketSource = strTmp;
	}

	if (0 == oPktSrc.GetParameterVal("seq",strTmp))
	{
		unsigned int uiSeq = FromString<unsigned int>(strTmp);
		if (stQuotation.m_uiSeqNo < uiSeq)
		{
			stQuotation.m_uiSeqNo = uiSeq;
		}
		else
		{			
			CRLog(E_DEBUG,"�ɵ����кţ�%u, �����кţ�%u,", stQuotation.m_uiSeqNo, uiSeq);
			stQuotation.m_uiSeqNo = uiSeq;

			//nRtn = -1;
		}
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("Day",strTmp))
	{
		unsigned int uiDate = FromString<unsigned int>(strTmp);
		if (stQuotation.m_uiDate < uiDate)
		{
			stQuotation.m_uiDate = uiDate;
		}
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("Time",strTmp))
	{
		if(strTmp.length() >= 10)
		{
			char sTime[24+1] = {0};			
			time_t rawtime = FromString<unsigned int>(strTmp);
			///*time( &rawtime );*/
			struct tm *pTM = localtime(&rawtime);
			//strftime(sTime, sizeof(sTime), "%H:%M:%S",pTM);
			int nHour = pTM->tm_hour;
			int nMin  = pTM->tm_min;
			int nSec  = pTM->tm_sec;
			stQuotation.m_uiTime = nHour * 10000000 + nMin * 100000 + nSec*1000 + 0;
		}
		else
		{
			//ֱ��ȡ����Դʱ��   2013-12-5
			SYSTEMTIME st;
			::GetLocalTime(&st);
			stQuotation.m_uiTime = st.wHour * 10000000 + st.wMinute * 100000 + st.wSecond * 1000 + st.wMilliseconds;
		}
		

		//vector<string> v = strutils::explode(":",strTmp);
		//if (v.size() == 3)
		//{
		//	int nHour = FromString<int>(v[0]);
		//	int nMin = FromString<int>(v[1]);
		//	int nSec = FromString<int>(v[2]);
		//	//stQuotation.m_uiTime = nHour * 10000 + nMin * 100 + nSec;
		//	stQuotation.m_uiTime = nHour * 10000000 + nMin * 100000 + nSec*1000 + 0;
		//}

		//ȡ����ʱ��  2013-12-5
		/*SYSTEMTIME st;
		::GetLocalTime(&st);
		stQuotation.m_uiTime = st.wHour * 10000000 + st.wMinute * 100000 + st.wSecond * 1000 + st.wMilliseconds;*/

	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("OpenPrice",strTmp))
	{
		stQuotation.m_uiOpenPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("HightPrice",strTmp))	
	{
		stQuotation.m_uiHigh = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("LowPrice",strTmp))	
	{
		stQuotation.m_uiLow = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("LastClose",strTmp))
	{
		stQuotation.m_uilastClose = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("NewPrice",strTmp))
	{		
		stQuotation.m_uiLast = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("Volume",strTmp))	
	{
		stQuotation.m_uiVolume = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("close",strTmp))
	{// ������
		stQuotation.m_uiClose = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";	
	if (0 == oPktSrc.GetParameterVal("BuyPrice1",strTmp))	
	{
		stQuotation.m_Ask[0].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("BuyPrice2",strTmp))	
	{
		stQuotation.m_Ask[1].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("BuyPrice3",strTmp))	
	{
		stQuotation.m_Ask[2].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("BuyPrice4",strTmp))	
	{
		stQuotation.m_Ask[3].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("BuyPrice5",strTmp))	
	{
		stQuotation.m_Ask[4].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("BuyVol1",strTmp))	
	{
		stQuotation.m_Ask[0].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("BuyVol2",strTmp))	
	{
		stQuotation.m_Ask[1].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("BuyVol3",strTmp))	
	{
		stQuotation.m_Ask[2].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("BuyVol4",strTmp))	
	{
		stQuotation.m_Ask[3].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("BuyVol5",strTmp))	
	{
		stQuotation.m_Ask[4].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("SellPrice1",strTmp))	
	{
		stQuotation.m_Bid[0].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("SellPrice2",strTmp))	
	{
		stQuotation.m_Bid[1].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("SellPrice3",strTmp))	
	{
		stQuotation.m_Bid[2].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("SellPrice4",strTmp))	
	{
		stQuotation.m_Bid[3].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("SellPrice5",strTmp))	
	{
		stQuotation.m_Bid[4].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("SellVol1",strTmp))	
	{
		stQuotation.m_Bid[0].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("SellVol2",strTmp))	
	{
		stQuotation.m_Bid[1].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("SellVol3",strTmp))	
	{
		stQuotation.m_Bid[2].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("SellVol4",strTmp))	
	{
		stQuotation.m_Bid[3].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("SellVol5",strTmp))	
	{
		stQuotation.m_Bid[4].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("turnOver",strTmp))
	{//�� /����������
		//stQuotation.m_dbTurnOver = (FromString<double>(strTmp)/10000);
		stQuotation.m_uiTurnOver = RoundToInt<double>(FromString<double>(strTmp),-2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("AdvStop",strTmp))
	{// ��ͣ��
		stQuotation.m_uiHighLimit = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("DesStop",strTmp))
	{// ��ͣ��
		stQuotation.m_uiLowLimit = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("average",strTmp))
	{
		stQuotation.m_uiAverage = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("Amount",strTmp))
	{
		stQuotation.m_uiWeight = RoundToInt<double>(FromString<double>(strTmp),3);
	}
	
	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("OpenInterest",strTmp))
	{
		stQuotation.m_uiChiCangLiang = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("SettlePrice",strTmp))
	{
		stQuotation.m_uiSettle = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("lastSettle",strTmp))
	{
		stQuotation.m_uiLastSettle = RoundToInt<double>(FromString<double>(strTmp),2);
	}

	//KENNY  2013-3  
	stQuotation.m_uiLastSettle = stQuotation.m_uilastClose;
	return nRtn;
}

int CTranslator::TranslateZipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation)
{
	int nRtn = 0;
	string sZipVal;
	string sInstID;
	oPktSrc.GetParameterVal("instID",sInstID);
	oPktSrc.GetParameterVal("sZipBuff",sZipVal);

	string sTmp;
	if (0 == oPktSrc.GetParameterVal("quoteDate", sTmp))
	{
		stQuotation.m_uiDate = FromString<unsigned int>(sTmp);
	}

	//SYSTEMTIME sysTime;
	//unsigned int uiTime = 0;

	//ȥ��\r\n
	sZipVal = strutils::stripNewLines(sZipVal);
	//base64����
	sZipVal = strutils::base64Decode(sZipVal);

	//int nLen = CEncode::unbase64(const_cast<char*>(sZipVal.c_str()),sZipVal.length(),aZipData);
	//aZipData ���ն��FLV���Ӷ��� ����һ�ֽڵ�6bit��ʾFieldID,���2bit��ֵ+3=����ʵ��ֵռ���ֽ���,ʵ��ֵ���������ֽ���

	int nIdx = 0;
	char cField;
	char cLen;
	char acValues[6];
	unsigned int uiValue;
	unsigned char* lpPointer = (unsigned char*)sZipVal.data();
	int nLen = sZipVal.length();
	char cByte;

	string sFldQuo;
	while (nIdx < nLen && nRtn != -1)
	{
		cByte = lpPointer[nIdx];
		// BIT:0 - 5
		cField = (cByte & 0xFC) >> 2 ;//11111100;
		// BIT:6 - 7   x + 3
		cLen   = (cByte & 0x03) + 3;
		if(nIdx + cLen >= nLen)
		{
			CRLog(E_ERROR,"���鱨�ĳ����쳣!");
			nRtn = -1;
			return nRtn;
		}
		
		nIdx ++;
		memset(acValues, 0, 6);
		memcpy(acValues, &lpPointer[nIdx], cLen);        
		nIdx += cLen;
		double dbValue = 0.0;
		for (int i = 0; i < cLen; i++)
		{
			unsigned char chTmp = acValues[i];

			for(int j = 7; j >= 0; j --)
			{
				unsigned int nIndex = ((cLen - i - 1 ) * 8 + j);
				int nFlag = ((chTmp >> j) & 0x01);
				if(nFlag > 0)
				{
					dbValue += pow(2.0, (double)nIndex);
				}
			}
		}
		uiValue = (unsigned int)dbValue;
		switch (cField)
		{
		case FIELDKEY_LASTSETTLE:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "LastSettle(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiLastSettle);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiLastSettle = uiValue/10;
			break;
		case FIELDKEY_LASTCLOSE:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "lastClose(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uilastClose);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uilastClose = uiValue/10;
			break;
		case FIELDKEY_OPEN:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "OpenPrice(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiOpenPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiOpenPrice = uiValue/10;
			break;
		case FIELDKEY_HIGH:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "High(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiHigh);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiHigh = uiValue/10;
			break;   
		case FIELDKEY_LOW:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Low(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiLow);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiLow = uiValue/10;
			break;  
		case FIELDKEY_LAST:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Last(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiLast);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiLast = uiValue/10;
			break;
		case FIELDKEY_CLOSE:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Close(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiClose);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiClose = uiValue/10;
			break;			
		case FIELDKEY_SETTLE:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Settle(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiSettle);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiSettle = uiValue/10;
			break;
		case FIELDKEY_BID1:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid1p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[0].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[0].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_BIDLOT1:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid1v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[0].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[0].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_BID2:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid2p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[1].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[1].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_BIDLOT2:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid2v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[1].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[1].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_BID3:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid3p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[2].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[2].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_BIDLOT3:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid3v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[2].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[2].m_uiVol = uiValue/1000;
			break;		
		case FIELDKEY_BID4:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid4p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[3].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[3].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_BIDLOT4:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid4v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[3].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[3].m_uiVol = uiValue/1000;
			break;		
		case FIELDKEY_BID5:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid5p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[4].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[4].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_BIDLOT5:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid5v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[4].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[4].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_ASK1:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask1p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[0].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[0].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_ASKLOT1:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask1v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[0].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[0].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_ASK2:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask2p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[1].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[1].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_ASKLOT2:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask2v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[1].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[1].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_ASK3:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask3p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[2].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[2].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_ASKLOT3:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask3v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[2].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[2].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_ASK4:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask4p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[3].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[3].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_ASKLOT4:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask4v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[3].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[3].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_ASK5:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask5p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[4].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[4].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_ASKLOT5:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask5v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[40].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[4].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_VOLUME:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Volume(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiVolume);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_uiVolume = uiValue /1000;
			break;
		case FIELDKEY_WEIGHT:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Weight(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiWeight);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue);
				sFldQuo += ")|";
			}

			stQuotation.m_uiWeight = uiValue;
			break;
		case FIELDKEY_HIGHLIMIT:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "HighLimit(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiHighLimit);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiHighLimit = uiValue/10;
			break;
		case FIELDKEY_LOWLIMIT:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "LowLimit(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiLowLimit);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiLowLimit = uiValue/10;
			break;
		case FIELDKEY_POSI:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "ChiCangLiang(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiChiCangLiang);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_uiChiCangLiang = uiValue /1000;
			break;
		case FIELDKEY_UPDOWN: // ����Ҫ����
			break;
		case FIELDKEY_TURNOVER:
			if (1 == m_nZipOut)
			{
				sFldQuo += "TurnOver(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiTurnOver);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(dbValue / 1000.00 / 100.00);
				sFldQuo += ")|";
			}
		
			//��WΪ��λ
			//stQuotation.m_dbTurnOver = dbValue / 1000 / 10000;
			stQuotation.m_uiTurnOver = static_cast<unsigned int>(dbValue / 1000.00 / 100.00);
			break;
		case FIELDKEY_AVERAGE:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Average(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiAverage);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiAverage = uiValue/10;
			break;
		case FIELDKEY_SEQUENCENO:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "SeqNo(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiSeqNo);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			if (stQuotation.m_uiSeqNo < uiValue/1000)
			{
				stQuotation.m_uiSeqNo = uiValue/1000;
			}
			else
			{
				CRLog(E_DEBUG,"(%s)�ɵ����кţ�%u, �����кţ�%u", sInstID.c_str(),stQuotation.m_uiSeqNo, uiValue/1000);
				stQuotation.m_uiSeqNo = uiValue/1000;
				//nRtn = -1;
			}
			break;
		case FIELDKEY_QUOTETIME: //���ֶδ�����Ҫ��ȷ��
			if (1 == m_nZipOut)
			{
				sFldQuo += "Time(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiTime);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue*1000);
				sFldQuo += ")|";
			}

			stQuotation.m_uiTime = uiValue*1000;
			//GetLocalTime(&sysTime);
			//uiTime = sysTime.wHour*10000000 + sysTime.wMinute*100000 + sysTime.wSecond*1000 + sysTime.wMilliseconds ;
			//stQuotation.m_uiTime = uiTime;
			break;
		case FIELDKEY_QUOTEDATE: //���ֶδ�����Ҫ��ȷ��
			if (1 == m_nZipOut)
			{
				sFldQuo += "Date(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiDate);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue);
				sFldQuo += ")|";
			}

			stQuotation.m_uiDate = uiValue;
			break;
		case FIELDKEY_UPDOWNRATE: // ����Ҫ����
			break;
		default:
			CRLog(E_ERROR,"���鱨���ֶ����[%d]������!",cField);
			break;
		}
		
	}

	if (1 == m_nZipOut)
	{
		CRLog(E_DEBUG, "%s %s", sInstID.c_str(), sFldQuo.c_str());
	}
	return nRtn;
}

//ϵͳ״̬
//#ApiName=onSysStatChange#b_sys_stat=1#exch_date=20110616#m_sys_stat=1#sys_date=20110615#
void CTranslator::HandleSysStat(CBroadcastPacket& pkt)
{
	try
	{
		//���ڱ�����δ����У����ڽ��̳�������ʱ��ͬ������֤��ʼȫ�������ȴ���
		msleep(2);

		CCondMutexGuard oGuard(m_deqCondMutex);
		string sTmp;
		if (0 == pkt.GetParameterVal("exch_date", sTmp))
		{
			CGessDate oDate;
			oDate.FromString(sTmp);

			int nDiff = oDate.IntervalToToday();
			if (nDiff >= 0)
			{			
				CRLog(E_DEBUG, "onSysStatChange:%d", nDiff);

				unsigned int uiDate = FromString<unsigned int>(oDate.ToString());
				for (map<std::string, QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
				{
					(*it).second.m_uiDate = uiDate;
				}
			}
		}
	}
	catch(...)
	{
		CRLog(E_CRITICAL, "%s", "unknown exception");
	}
}

//��Լ״̬ת��
void CTranslator::HandleInstState(CBroadcastPacket& pkt)
{
	try
	{
		CCondMutexGuard oGuard(m_deqCondMutex);

		string sTmp;
		if (0 == pkt.GetParameterVal("tradeState", sTmp))
		{
			if (sTmp.length() > 0)
			{
				unsigned char ucInstStateOld = m_ucInstState;
				m_ucInstState = sTmp.at(0);

				if (I_END != ucInstStateOld && I_END == m_ucInstState
					|| I_END == ucInstStateOld && I_END != m_ucInstState)
				{
					for (map<std::string, QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
					{
						(*it).second.m_uiSeqNo = 0;
					}
				}

				if (I_NORMAL != ucInstStateOld && I_NORMAL == m_ucInstState)
				{					
					SYSTEMTIME sysTime;
					::GetLocalTime(&sysTime);
					unsigned int uiTime = sysTime.wHour*10000000 + sysTime.wMinute*100000 + sysTime.wSecond*1000 + sysTime.wMilliseconds ;					
					for (map<std::string, QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
					{
						(*it).second.m_uiTime = uiTime;
					}
				}
			}
		}
		CRLog(E_DEBUG, "TradeState=%c", m_ucInstState);
	}
	catch(...)
	{
		CRLog(E_CRITICAL, "%s", "unknown exception");
	}
}

//��ԼIDת��
void CTranslator::ConvertInstID(string& sInstID)
{
	map<string, string>::iterator it = m_mapNamePair.find(sInstID);
	if (it != m_mapNamePair.end())
	{
		sInstID = (*it).second;
	}
	else
	{
		if (sInstID.length() > 8)
		{
			sInstID = sInstID.substr(0,8);
		}
	}
}

//ѹ�����ĵ����������
void CTranslator::ZipPktOut(int nOut)
{
	m_nZipOut = nOut;
	return;
}