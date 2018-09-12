#ifndef _OFFER_CONSTANT_H_
#define _OFFER_CONSTANT_H_

#include <string>
using namespace std;

namespace OfferConst
{
	//��¼����ص�ֵ
	const int gc_nLoginCallbackOk				= 0;				//��¼�ɹ�
	const int gc_nLoginCallbackFail				= 1;				//��¼ʧ��

	//��¼���ý��ֵ
	const int gc_nLoginCallOk					= 0;				//��¼���óɹ�
	const int gc_nLoginCallFail					= -1;				//��¼����ʧ��
	const int gc_nLoginCallRepeat				= -2;				//��¼����ʧ�� ԭ���ظ���¼
	const int gc_nLoginCallPwd					= -3;				//��¼����ʧ�� ԭ���������

	//�ǳ����ý��ֵ
	const int gc_nLogoutCallOk					= 0;				//�ǳ����óɹ�
	const int gc_nLogoutCallFail				= 1;				//��¼����ʧ��

	//�ǳ�����ص�ֵ
	const int gc_nLogoutCallbackOk				= 0;				//�ǳ��ɹ�
	const int gc_nLogoutCallbackFail			= -1;				//�ǳ�ʧ��

	//���ӱ�־
	const int gc_nMaster						= 1;				//��
	const int gc_nSlave							= 0;				//��

	//��¼״̬
	const int gc_nStateLoginUnknown				= 0;				//δ֪ 	
	const int gc_nStateLoginInit				= 1;				//��ʼ�� 	
	const int gc_nStateLoginPrepare				= 2;				//�ȴ���¼
	const int gc_nStateLogined					= 3;				//�Ѿ���¼ 
	const int gc_nStateLogouted					= 4;				//�Ѿ��ǳ� 
	const int gc_nStateLoginning				= 5;				//��¼�� 
	const int gc_nStateLogoutting				= 6;				//�ǳ��� 
	const int gc_nStateLoginErrRep				= 7;				//�ظ���¼ 
	const int gc_nStateLoginErrPwd				= 8;				//��¼�������
	const int gc_nStateLoginErrOth				= 9;				//��¼ʧ�� 
	const int gc_nStateLogoutErrOth				= 10;				//�ǳ�ʧ��
	const int gc_nStateLoginClosed				= 11;				//�������ر�

	//��¼ָʾ 
	const int gc_nIndUnknown					= 0;				//��ָʾ
	const int gc_nIndLogin						= 1;				//ָʾ��¼
	const int gc_nIndLogout						= 2;				//ָʾ�ǳ�

	//��Զ�����״̬
	const int gc_nStateInit						= 0;				//��ʼδ֪
	const int gc_nStateConnected				= 1;				//������
	const int gc_nStateDisConnected				= 2;				//���ж�

	//����״̬
	const int gc_nLoginTokenHold				= 1;				//����
	const int gc_nLoginTokenWait				= 0;				//��������
	const int gc_nLoginTokenGiveup				= 2;				//��������

	//����Э�̽��
	const int gc_nTokenNegotiate				= 0;				//��ҪЭ��
	const int gc_nTokenWait2Hold				= 1;				//δ����Э��Ϊ����
	const int gc_nTokenHold2Wait				= 2;				//����Э��Ϊδ����
	const int gc_nTokenWait2Wait				= 3;				//����δ����
	const int gc_nTokenHold2Hold				= 4;				//���ֳ���
	
	//��¼�澯
	const int gc_nLoginNormal					= 0;				//��¼״̬����
	const int gc_nLoginAlarm					= 1;				//��¼״̬�澯

	const int gc_nPktRequest					= 0;				//����
	const int gc_nPktResponse					= 1;				//Ӧ��


}

#endif