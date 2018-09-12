#ifndef _MARKET_H
#define _MARKET_H

#include "stdef.h"
#include "Comm.h"

#include "configImpl.h"

#define E_ALREADDONE      0
#define E_SUCCEED         1
#define E_FAIL           -1

#define MARKET_MGR_CFG   "markets.cfg"

#define CFG_MARKET_CLASS                  "�г�����"
#define CFG_MARKET_SECTION_NAME           "�г�����"
#define CFG_MARKET_SECTION_SORT           "������"
#define CFG_MARKET_SECTION_TRADEPHASE     "����ʱ��"
#define CFG_MARKET_SECTION_PERHAND        "ÿ�ֵ�λ"
#define CFG_MARKET_SECTION_EXTENDMUTILE   "�Ŵ���"


class CDataBuffer;
class CMTDataBuffer;
class CMarket
{
public:
	CMarket();
	~CMarket();
	int Init(HSMarketDataType marketType, string szMarketName);  //��ʼ�����������ͺ����ƴӱ����ļ������г���Ϣ
	int Init(CConfig* pCfg);
	int Open();
	int Close();

	void OnAsk(const char* pszBuf, const int nSize, CDataBuffer& outBuffer);
	void OnRecv(const char* pszBuf, const int nSize);

	QMarketInfo* GetMarketInfo();

	CMTDataBuffer* GetMarketBuffer(){return m_Buffer_Market;}

private:
	CMTDataBuffer   *m_Buffer_Market;
	string          m_szFile;
	CConfig*	    m_pCfg;	
	bool            m_bInit;

private:
	void LoadFromDisk(HSMarketDataType marketType);
	void WriteToFile();
};

class CMarketMgr
{
public:
	// �¼�����ʱ��
	class CEventTimer : public CGessTimer
	{
	public:
		CEventTimer():m_pParent(0){}
		virtual ~CEventTimer(){}
		void Bind(CMarketMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey, unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnEventTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CMarketMgr* m_pParent;
	};

public:
	 CMarketMgr(void);
	 ~CMarketMgr(void);

	int Init();   // ��ʼ��������m_mapMarkets

	void OnAsk(const char* pszBuf, const int nSize, CDataBuffer& outBuffer);
	void OnRecv(const char* pszBuf, const int nSize);

	CMarket* GetMarket(ULONG lMarketType);           // �����г�����ȡ���г�ָ��
	CMarket* GetMarket(const char * pszCode);        // ����Ʒ�ִ���ȡ���г�ָ��

	CEventTimer m_oEventTimer;

	int OnEventTimeout(const string& ulKey, unsigned long& ulTmSpan);
private:
	map<ULONG, CMarket*> m_mapMarkets;
	CConfigImpl*    m_pConfig;

};



#endif