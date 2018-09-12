#include "ProcessInterfaceMonc.h"
#include "Logger.h"
#include "strutils.h"

using namespace strutils;

CProcessInterfaceMonc::CProcessInterfaceMonc()
{
	m_GessPktInfo.ulIdleInterval = 5;//DEFAULT_IDLE_TIMEOUT;
	m_GessPktInfo.ulIntervalReSend = DEFAULT_HELLO_RESEND_INTERVAL;
	m_GessPktInfo.ulHelloReSend = DEFAULT_HELLO_RESEND_COUNT;
	m_GessPktInfo.blNeedLogin = false;
	m_GessPktInfo.nNeedCheckServicePkt = 0;
}

CProcessInterfaceMonc::~CProcessInterfaceMonc(void)
{
}


/******************************************************************************
��������:��ȡЭ�鶨��ı��ĸ�ʽ��Ϣ
���ú���:�������ص�
�������:PacketInfo & stInfo ���ĸ�ʽ��Ϣ
        eLengthType �����ֶ����� ����/10�����ַ���/16�����ַ���
		nLengthBytes ��Ϣ�������ֶεĳ���,Ϊ����ʱֻ֧�� 1 2 4�ֽ�
		nLengthPos �����ֶ���ʼλ��
		blLenIncludeHeader �����Ƿ񺬱���ͷ
		nFixHeadLen  �������ֶ�ָʾ�ĳ�����,���Ĺ̶����ֳ���
������  :��ΰ
��������:2008.07.22
�޸ļ�¼:
******************************************************************************/
void CProcessInterfaceMonc::GetPacketInfo(PacketInfo & stInfo)
{
	stInfo.eLengthType = ltInterger;
	stInfo.nLengthPos = 0;
	stInfo.nLengthBytes = 4;
	stInfo.blLenIncludeHeader = true;
	stInfo.nFixHeadLen = 12;
}

/******************************************************************************
��������:Э�鱨�Ĵ�������Ҫ����,���ݱ��������ֽ���ֱ�Ӵ����ת��
���ú���:�������ص�
��������:OnRecvPacket,�ϴ�����
�������:char * pData���ջ���, size_t nSize�����С
����ֵ  :int �������⴦��
������  :��ΰ
��������:2008.07.22
�޸ļ�¼:
******************************************************************************/
int CProcessInterfaceMonc::OnPacket(char * pData, size_t nSize)
{
	CBinBlockPkt oPacket;
	oPacket.Decode(pData, nSize);

	OnRecvPacket(oPacket);
	return 0;
}

/******************************************************************************
��������:socket���ӳɹ�����Ϊ�ͻ��˱�֪ͨ,һ�����ڴ�����֤���ķ���
���ú���:ͨѶ����������ص�
����ֵ  :int �������⴦��
������  :��ΰ
��������:2008.07.22
�޸ļ�¼:
******************************************************************************/
int CProcessInterfaceMonc::OnConnect()
{
	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"MONC OnConnect socket (%d) Peer(%s:%d),Me(%s:%d)",SocketID(),sPeerIp.c_str(),nPeerPort,sLocalIp.c_str(),nLocalPort);
	return 0;
}


/******************************************************************************
��������:socket�����жϺ���֪ͨ,�ɸ���Э��Ҫ����д���
���ú���:�������ص�
����ֵ  :int �������⴦��
������  :��ΰ
��������:2008.07.22
�޸ļ�¼:
******************************************************************************/
void CProcessInterfaceMonc::OnClose()
{
	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"MONC OnClose socket (%d) Peer(%s:%d),Me(%s:%d)",SocketID(),sPeerIp.c_str(),nPeerPort,sLocalIp.c_str(),nLocalPort);

	return;
}