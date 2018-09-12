#pragma once
#include <map>
#include <string>
#include "osdepend.h"
#include "xmemmap.h"

using namespace std;


//�г����� ������:TMarketType
#define MARKET_SPOT			's'     //�ֻ�
#define MARKET_DEFER		'd'     //����
#define MARKET_FUTURES		'f'     //�ڻ�
#define MARKET_FORWARD		'w'     //Զ��


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


//�г�״̬ ������:TMarketStateFlag
#define M_INITING		'0'     //��ʼ����
#define M_INIT		'1'     //��ʼ�����
#define M_OPEN		'2'     //����
#define M_TRADE		'3'     //����
#define M_PAUSE		'4'     //��ͣ
#define M_CLOSE		'5'     //����


typedef struct
{
	char  instID[16];				//��Լ����
	char  marketID;					//�г�
	char  tradeState;				//��Լ����״̬
	unsigned int	uiSeqNo;
	unsigned int	uiClose;
	QUOTATION	stQuotation;
} INST_INFO,*PINST_INFO;


#define MAX_INST_NUMBER	32
typedef struct stBasicInf
{
	int nDate;
	int nCount;
	INST_INFO aInstState[MAX_INST_NUMBER];
} BASIC_INF;

//
class CBasicInfMemFile : public CXMemMapFile
{
public:
	CBasicInfMemFile();
	virtual ~CBasicInfMemFile(void);
	
public:
	BOOL Create(LPCTSTR lpszFileName);
	void Close();
	string ToString();

	BOOL GetInstState(const string& sInst, char& cState);
	BOOL SetInstState(const string& sInst, char cState, char cMarketID);
	int IsSeqNo(const string& sInst, unsigned int uiSeqNo,unsigned int uiClose);
	int SwitchTradeDate(unsigned int uiDate);
	unsigned int TradeDate();
private:
	BASIC_INF*					m_pBasicInf;
	map<string, INST_INFO*>		m_mapInstState;
private:

};
