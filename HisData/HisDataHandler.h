#ifndef _HIS_DATA_HANDLER_H
#define _HIS_DATA_HANDLER_H

#include <fstream>
#include <iostream>
#include "GessDate.h"
#include "Workthread.h"
#include "BroadcastPacket.h"
#include "SamplerPacket.h"

/*
�г�����壺
��λ�����ʾ���£�
15		   12		8					0
|			|	  	  |					|
| ���ڷ���	|�г����� |	����Ʒ�ַ���	|
*/
typedef short HSMarketDataType;			  // �г�������������
/*���ڴ���*/
#define STOCK_MARKET			 0X1000   // ��Ʊ
#	define SH_BOURSE			 0x0100   // �Ϻ�
#	define SZ_BOURSE			 0x0200   // ����
#	define SYSBK_BOURSE			 0x0400   // ϵͳ���
#	define USERDEF_BOURSE		 0x0800   // �Զ��壨��ѡ�ɻ����Զ����飩
#			define KIND_INDEX		 0x0000   // ָ�� 
#			define KIND_STOCKA		 0x0001   // A�� 
#			define KIND_STOCKB		 0x0002   // B�� 
#			define KIND_BOND		 0x0003   // ծȯ
#			define KIND_FUND		 0x0004   // ����
#			define KIND_THREEBOAD	 0x0005   // ����
#			define KIND_SMALLSTOCK	 0x0006   // ��С�̹�
#			define KIND_PLACE		 0x0007	  // ����
#			define KIND_LOF			 0x0008	  // LOF
#			define KIND_ETF			 0x0009   // ETF
#			define KIND_QuanZhen	 0x000A   // Ȩ֤

#			define KIND_OtherIndex	 0x000E   // ������������࣬��:����ָ��

#			define SC_Others		 0x000F   // ���� 0x09
#			define KIND_USERDEFINE	 0x0010   // �Զ���ָ��

// �۹��г�
#define HK_MARKET				 0x2000 // �۹ɷ���
#	define HK_BOURSE			 0x0100 // �����г�
#	define	GE_BOURSE			 0x0200 // ��ҵ���г�(Growth Enterprise Market)
#	define	INDEX_BOURSE		 0x0300	// ָ���г�
#	define	NX_BOURSE		     0x0400	// ţ��֤
#		define HK_KIND_INDEX			 0x0000   // ��ָ
#		define HK_KIND_FUTURES_INDEX	 0x0001   // ��ָ
//#		define	KIND_Option				 0x0002	  // �۹���Ȩ

#	define SYSBK_BOURSE			 0x0400 // �۹ɰ��(H��ָ���ɷݹɣ�Ѷ��ָ���ɷݹɣ���
#	define USERDEF_BOURSE		 0x0800 // �Զ��壨��ѡ�ɻ����Զ����飩
#			define HK_KIND_BOND		 0x0000   // ծȯ
#			define HK_KIND_MulFund	 0x0001   // һ�����Ϲ�֤
#			define HK_KIND_FUND		 0x0002   // ����
#			define KIND_WARRANTS	 0x0003   // �Ϲ�֤
#			define KIND_JR			 0x0004   // ����
#			define KIND_ZH			 0x0005   // �ۺ�
#			define KIND_DC			 0x0006   // �ز�
#			define KIND_LY			 0x0007   // ����
#			define KIND_GY			 0x0008   // ��ҵ
#			define KIND_GG			 0x0009   // ����
#			define KIND_QT			 0x000A   // ����

/*�ڻ�����*/
#define FUTURES_MARKET			 0x4000 // �ڻ�
#		define DALIAN_BOURSE		 0x0100	// ����
#				define KIND_BEAN		 0x0001	// ����
#				define KIND_YUMI		 0x0002	// ��������
#				define KIND_SHIT		 0x0003	// ����ʳ��
#				define KIND_DZGY		 0x0004	// ���ڹ�ҵ1
#				define KIND_DZGY2		 0x0005	// ���ڹ�ҵ2
#				define KIND_DOUYOU		 0x0006	// ������
#				define KIND_JYX			 0x0007	// ����ϩ
#				define KIND_ZTY			 0x0008	// �����

#		define SHANGHAI_BOURSE	  0x0200	// �Ϻ�
#				define KIND_METAL		 0x0001	// �Ϻ�����
#				define KIND_RUBBER		 0x0002	// �Ϻ���
#				define KIND_FUEL		 0x0003	// �Ϻ�ȼ��
//#				define KIND_GUZHI		 0x0004	// ��ָ�ڻ�
#				define KIND_QHGOLD		 0x0005	// �Ϻ��ƽ�

#		define ZHENGZHOU_BOURSE	  0x0300	// ֣��
#				define KIND_XIAOM		 0x0001	// ֣��С��
#				define KIND_MIANH		 0x0002	// ֣���޻�
#				define KIND_BAITANG		 0x0003	// ֣�ݰ���
#				define KIND_PTA			 0x0004	// ֣��PTA
#				define KIND_CZY			 0x0005	// ������

#		define HUANGJIN_BOURSE	  0x0400		// �ƽ�����
#				define KIND_GOLD		 0x0001	// �Ϻ��ƽ�

#		define GUZHI_BOURSE		  0x0500		// ��ָ�ڻ�
#				define KIND_GUZHI		 0x0001	// ��ָ�ڻ�

#		define SELF_BOURSE		  0x0600	// �Զ�������

#		define DZGT_BOURSE		  0x0610	// ���ڸ�������

/*���̴���*/
#define WP_MARKET				 ((HSMarketDataType)0x5000) // ����
#		define WP_INDEX				0x0100	// ����ָ�� // ������
#		define WP_LME				0x0200	// LME		// ������
#			define WP_LME_CLT			0x0210 //"����ͭ";
#			define WP_LME_CLL			0x0220 //"������";
#			define WP_LME_CLM			0x0230 //"������";
#			define WP_LME_CLQ			0x0240 //"����Ǧ";
#			define WP_LME_CLX			0x0250 //"����п";
#			define WP_LME_CWT			0x0260 //"������";
#			define WP_LME_CW			0x0270 //"����";
#			define WP_LME_SUB			0x0000

#			define WP_CBOT				0x0300	// CBOT			
#			define WP_NYMEX	 			0x0400	// NYMEX	 	
#			define WP_NYMEX_YY			0x0000	//"ԭ��";
#			define WP_NYMEX_RY			0x0001	//"ȼ��";
#			define WP_NYMEX_QY			0x0002	//"����";

#			define WP_COMEX	 			0x0500	// COMEX	 	
#			define WP_TOCOM	 			0x0600	// TOCOM	 	
#			define WP_IPE				0x0700	// IPE			
#			define WP_NYBOT				0x0800	// NYBOT		
#			define WP_NOBLE_METAL		0x0900	// �����	
#			  define WP_NOBLE_METAL_XH	0x0000  //"�ֻ�";
#			  define WP_NOBLE_METAL_HJ	0x0001  //"�ƽ�";
#			  define WP_NOBLE_METAL_BY	0x0002  //"����";

#			define WP_FUTURES_INDEX		0x0a00	// ��ָ
#			define WP_SICOM				0x0b00	// SICOM		
#			define WP_LIBOR				0x0c00	// LIBOR		
#			define WP_NYSE				0x0d00	// NYSE
#			define WP_CEC				0x0e00	// CEC


#			define WP_Other_TZTHuanjin	0x0F10	// �ƽ��ڻ���������,����������
#			define WP_Other_JinKaiXun	0x0F20	// ��Ѷ������
#			define WP_JKX               WP_Other_JinKaiXun
#			define WP_XJP               0x0F30	// �¼�������


#			define WP_INDEX_AZ	 		0x0110 //"����";
#			define WP_INDEX_OZ	 		0x0120 //"ŷ��";
#			define WP_INDEX_MZ	 		0x0130 //"����";
#			define WP_INDEX_TG	 		0x0140 //"̩��";
#			define WP_INDEX_YL	 		0x0150 //"ӡ��";
#			define WP_INDEX_RH	 		0x0160 //"�պ�";
#			define WP_INDEX_XHP 		0x0170 //"�¼���";
#			define WP_INDEX_FLB 		0x0180 //"���ɱ�";
#			define WP_INDEX_CCN 		0x0190 //"�й���½";
#			define WP_INDEX_TW  		0x01a0 //"�й�̨��";
#			define WP_INDEX_MLX 		0x01b0 //"��������";
#			define WP_INDEX_SUB 		0x0000 


/*������*/
#define FOREIGN_MARKET			 ((HSMarketDataType)0x8000) // ���
#	define WH_BASE_RATE			0x0100	// ��������
#	define WH_ACROSS_RATE		0x0200	// �������
#		define FX_TYPE_AU 			0x0000 // AU	��Ԫ
#		define FX_TYPE_CA 			0x0001 // CA	��Ԫ
#		define FX_TYPE_CN 			0x0002 // CN	�����
#		define FX_TYPE_DM 			0x0003 // DM	���
#		define FX_TYPE_ER 			0x0004 // ER	ŷԪ	 
#		define FX_TYPE_HK 			0x0005 // HK	�۱�
#		define FX_TYPE_SF 			0x0006 // SF	��ʿ 
#		define FX_TYPE_UK 			0x0007 // UK	Ӣ��
#		define FX_TYPE_YN 			0x0008 // YN	��Ԫ

#	define WH_FUTURES_RATE			0x0300  // �ڻ�

/*������*/
#define HJ_MARKET			 ((HSMarketDataType)0x6000) // �ƽ�
#	define HJ_SH_CURR			0x0100	// �Ϻ��ֻ�
#	define HJ_SH_QH		        0x0200	// �Ϻ��ڻ�
#	define HJ_WORLD	        0x0300	// �����г�
#	define HJ_OTHER	        0x0400	// �����г�

class CHisDataCpMgr;
class CHisDataHandler : public CWorkThread
{
public:
	CHisDataHandler(CHisDataCpMgr* p);
	~CHisDataHandler(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int Enque(QUOTATION& stQuotation);

	void OpenHisdataFile();
	string HandleCmdLine(const string& sCmd, const vector<string>& vecPara);
private:
	void WriteTick(const QUOTATION& stQuotation);
private:
	CHisDataCpMgr*     m_pProviderCpMgr;
	CConfig*		m_pCfg;
	
	vector<string>			m_vTickInst;	
	map<string, ofstream*>	m_mapOfsTick;
	//ofstream		m_ofsTick;

	//Test
	ofstream		m_ofsQuotation;
	string			m_sFilePathAbs;
	int				m_nTest;
	unsigned int	m_uiPkts;

	std::deque<QUOTATION> m_deqQuotation;
	CCondMutex	m_deqCondMutex;

	CGessDate		m_oDateLast;
	//
	map<std::string, QUOTATION> m_mapQuotation;

	int ThreadEntry();
	int End();

};

#endif