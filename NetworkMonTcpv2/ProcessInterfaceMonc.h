//Э������ ������ 
#ifndef _PROCESS_INTERFACE_MONC_H
#define _PROCESS_INTERFACE_MONC_H

#include "PktBinBlockMon.h"
#include "ProtocolProcess.h"

#define DEFAULT_IDLE_TIMEOUT						16
#define DEFAULT_HELLO_RESEND_INTERVAL				6
#define DEFAULT_HELLO_RESEND_COUNT					8

class CProcessInterfaceMonc : public CTcpAppProcessClient
{
public:
	CProcessInterfaceMonc(void);
	~CProcessInterfaceMonc(void);

	typedef struct tagGessPktInfo
	{
		unsigned long ulIdleInterval;		//����ʱ��������Hello���ʱ�䣬Ҳ����·�ϵ������ʱ��
		unsigned long ulIntervalReSend;		//��Ӧ����ؼ����
		unsigned long ulHelloReSend;		//��Ӧ����ط������Ĵ���
							
		int	nNeedCheckServicePkt;			//�Ƿ���Ҫ���ҵ���ĵ�����
		std::string	node_type;
		std::string	node_id;
		std::string	node_name;
		std::string	host_id;
		std::string	sUserName;				//���ͻ�ʱ�����˵�¼�û���
		std::string	sPassword;				//���ͻ�ʱ�����˵�¼����

		bool blNeedLogin;					//�Ƿ���Ҫ��¼	
	} GessPktInfo,*PGessPktInfo;

	virtual int OnConnect();		// ���ͻ������ӳɹ���ص�
	virtual void OnClose();	        // �Ͽ�����ʱ����
	
	int HandleTimeout(unsigned long& ulTmSpan) {return 0;}
protected:
	//���ඨ��Ļص�����ʵ��
	virtual int OnPacket(char * pData, size_t nSize);
	virtual void GetPacketInfo(PacketInfo & stInfo);//���ĸ�ʽ��Ϣ

private:
	GessPktInfo m_GessPktInfo;
};
#endif