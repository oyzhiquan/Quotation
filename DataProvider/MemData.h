#ifndef _MEM_DATE_H
#define _MEM_DATE_H
#include "Gess.h"
#include "Singleton.h"
#include "SubscriberTbl.h"
#include "QuotationTbl.h"

class CMemData : public CSingleton< CMemData >
{
	friend class CSingleton< CMemData >;
protected:
	CMemData();
	virtual ~CMemData();

public:
	//CQuotationTbl& GetQuotationTbl();

	///��ȡ���Ĺ���
	CSubscriberTbl& GetSubscriberTbl();

	///��ȡ��Լ����
	//CContractMgr& GetContractMgr();

private:
	//CQuotationTbl  m_QuotationTbl;
	CSubscriberTbl m_SubscriberTbl;  //��㶩�Ĺ�����
};
#endif