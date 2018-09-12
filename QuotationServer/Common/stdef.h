#ifndef _STDEF_H
#define _STDEF_H

// ҵ��㹫�����ݽṹ
#define HSMarketDataType unsigned short
#define ULONG unsigned long
#define MARKETNAME_SIZE    32
#define CODE_SIZE          8
#define NAME_SIZE          32



/*
�г�����壺
��λ�����ʾ���£�
15		   12		8					0
|			|	  	  |					|
| ���ڷ���	|�г����� |	����Ʒ�ַ���	|
*/


/*���ڹ�Ʊ�г�*/
#define STOCK_MARKET			 0X1000   // ��Ʊ
#	define SH_BOURSE			 0x0100   // �Ϻ�
#	define SZ_BOURSE			 0x0200   // ����
#	define SYSBK_BOURSE			 0x0400   // ϵͳ���
#	define USERDEF_BOURSE		 0x0800   // �Զ��壨��ѡ�ɻ����Զ����飩
#		define KIND_INDEX		 0x0000   // ָ�� 
#		define KIND_STOCKA		 0x0001   // A�� 
#		define KIND_STOCKB		 0x0002   // B�� 
#		define KIND_BOND		 0x0003   // ծȯ
#		define KIND_FUND		 0x0004   // ����
#		define KIND_THREEBOAD	 0x0005   // ����
#		define KIND_SMALLSTOCK	 0x0006   // ��С�̹�
#		define KIND_PLACE		 0x0007	  // ����
#		define KIND_LOF			 0x0008	  // LOF
#		define KIND_ETF			 0x0009   // ETF
#		define KIND_QuanZhen	 0x000A   // Ȩ֤

#		define KIND_OtherIndex	 0x000E   // ������������࣬��:����ָ��
#		define SC_Others		 0x000F   // ���� 0x09
#		define KIND_USERDEFINE	 0x0010   // �Զ���ָ��


// �۹��г�
#define HK_MARKET				 0x2000 // �۹ɷ���
#	define HK_BOURSE			 0x0100 // �����г�
#	define	GE_BOURSE			 0x0200 // ��ҵ���г�(Growth Enterprise Market)
#	define	INDEX_BOURSE		 0x0300	// ָ���г�	
#		define HK_KIND_INDEX			 0x0000   // ��ָ
#		define HK_KIND_FUTURES_INDEX	 0x0001   // ��ָ

#	define SYSBK_BOURSE			 0x0400 // �۹ɰ��(H��ָ���ɷݹɣ�Ѷ��ָ���ɷݹɣ���
#	define USERDEF_BOURSE		 0x0800 // �Զ��壨��ѡ�ɻ����Զ����飩

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
#				define KIND_QHGOLD		 0x0005	// �Ϻ��ƽ�

#		define ZHENGZHOU_BOURSE	  0x0300	// ֣��
#				define KIND_XIAOM		 0x0001	// ֣��С��
#				define KIND_MIANH		 0x0002	// ֣���޻�
#				define KIND_BAITANG		 0x0003	// ֣�ݰ���
#				define KIND_PTA			 0x0004	// ֣��PTA
#				define KIND_CZY			 0x0005	// ������

#		define GUZHI_BOURSE		  0x0500		// ��ָ�ڻ�
#				define KIND_GUZHI		 0x0001	// ��ָ�ڻ�

/*���̴���*/
#define WP_MARKET				 ((HSMarketDataType)0x5000) // ����
#		define WP_INDEX				0x0100	// ����ָ��
#		define WP_INDEX_RJ	 		0x0001 //"�վ�"; 7120
#		define WP_INDEX_HZ	 		0x0002 //"��ָ"; 7121
#		define WP_INDEX_NH	 		0x0003 //"�Ϻ��ۺ�";7122
#		define WP_INDEX_TG	 		0x0004 //"̨�ɼ�Ȩ";7123
#		define WP_INDEX_XG	 		0x0005 //"�ǹɺ�Ͽ";7124
#		define WP_INDEX_MG	 		0x0006 //"����ۺ�";7125
#		define WP_INDEX_TGZH 		0x0007 //"̩���ۺ�";7126
#		define WP_INDEX_YN 		    0x0008 //"ӡ���ۺ�";7127
#		define WP_INDEX_AZ 		    0x0009 //"�����ۺ�";7128
#		define WP_INDEX_NXL  		0x000a //"Ŧ����";  7129
#		define WP_INDEX_SGX         0x000b //"SGXĦ̨"; 7130
#		define WP_INDEX_SENSEX      0x000c //"ӡSENSEX";7164
#		define WP_INDEX_KOSPI       0x000d //"KOSPI200";7185
#		define WP_INDEX_DQGY        0x000e //"����ҵ";7301
#		define WP_INDEX_DQYS        0x000f //"��������";7302
#		define WP_INDEX_DQGG        0x0010 //"������";7303
#		define WP_INDEX_NSDK        0x0011 //"��˹���" 7304
#		define WP_INDEX_BZPE        0x0012 //"��׼�ն�" 7305
#		define WP_INDEX_CRBYX       0x0013 //"CRB����"  7306
#		define WP_INDEX_CRBZS       0x0014 //"CRBָ��"  7307
#		define WP_INDEX_JND         0x0015 //"���ô�"   7308
#		define WP_INDEX_FS100       0x0016 //"��ʱ100"  7309
#		define WP_INDEX_FACAC       0x0017 //"��CAC40" 7310
#		define WP_INDEX_DEDAX       0x0018 //"��DAX"   7312
#		define WP_INDEX_HEAEX       0x0019 //"����AEX" 7313
#		define WP_INDEX_DMKFX       0x001a //"����KFX" 7314
#		define WP_INDEX_BLS         0x001b //"����ʱ"  7315
#		define WP_INDEX_RSSSMI      0x001c //"��ʿSSMI" 7316
#		define WP_INDEX_BXBVSP      0x001d //"����BVSP" 7317
#		define WP_INDEX_BDI         0x001e //"BDIָ��"  7321
#		define WP_INDEX_BP100       0x001f //"����100"  7322
#		define WP_INDEX_ERTS        0x0020 //"��RTS"    7323
#		define WP_INDEX_YFTMIB      0x0021 //"��FTMIB"  7324


#		define WP_LME				0x0200	// LME		// ������
#		define WP_CBOT				0x0300	// CBOT			
#		define WP_NYMEX	 			0x0400	// NYMEX	 	
#		    define WP_NYMEX_YY			0x0000	//"ԭ��";
#		    define WP_NYMEX_RY			0x0001	//"ȼ��";
#		    define WP_NYMEX_QY			0x0002	//"����";

#		define WP_COMEX	 			0x0500	// COMEX	 	
#		define WP_TOCOM	 			0x0600	// TOCOM	 	
#		define WP_IPE				0x0700	// IPE			
#		define WP_NYBOT				0x0800	// NYBOT		
#		define WP_NOBLE_METAL		0x0900	// �����	
#		   define WP_NOBLE_METAL_XH	    0x0000  //"�ֻ�";
#		   define WP_NOBLE_METAL_HJ   	0x0001  //"�ƽ�";
#		   define WP_NOBLE_METAL_BY	    0x0002  //"����";

#		define WP_FUTURES_INDEX		0x0a00	// ��ָ
#		define WP_SICOM				0x0b00	// SICOM		
#		define WP_LIBOR				0x0c00	// LIBOR		
#		define WP_NYSE				0x0d00	// NYSE
#		define WP_CEC				0x0e00	// CEC


/*������*/
#define FOREIGN_MARKET			 ((HSMarketDataType)0x8000) // ���
#	define WH_BASE_RATE			0x0100	// ��������
#	define WH_ACROSS_RATE		0x0200	// �������
#	define WH_FUTURES_RATE			0x0300  // �ڻ�

/*�ƽ����*/
#define HJ_MARKET			 ((HSMarketDataType)0x6000) // �ƽ�
#		define SGE_BOURSE	     0x0100		// �ƽ�����
#				define KIND_TD		     0x0001	// ����
#				define KIND_TN		     0x0002	// Զ��
#				define KIND_SPOT		 0x0003	// �ֻ�
#	define HJ_SH_QH		        0x0200	// �Ϻ��ڻ�
#	define HJ_WORLD	        0x0300	// �����г�
#	define HJ_OTHER	        0x0400	// �����г�

static int	MakeMarket(HSMarketDataType x)
{
	return ((HSMarketDataType)((x) & 0xF000));
}
static int  MakeMainMarket(HSMarketDataType x)
{
	return ((HSMarketDataType)((x) & 0xFF00));
}

static int MakeSubMarket(HSMarketDataType x)
{
	if (MakeMainMarket(x) == (WP_MARKET | WP_INDEX))
		return ((HSMarketDataType)((x) & 0x00FF));
	else
		return ((HSMarketDataType)((x) & 0x000F));
}

static int MakeHexSubMarket(HSMarketDataType x)
{
	return ( (HSMarketDataType)((x) & 0x000F) );
}

static int MakeSubMarketPos(HSMarketDataType x)
{
	if (MakeMainMarket(x) == (WP_MARKET | WP_INDEX))
		return MakeSubMarket(x);
	return ( ((MakeHexSubMarket(x) / 16) * 10) + (MakeHexSubMarket(x) % 16) );
}

// Ʒ������
typedef enum tag_StockType
{
	Kind_Index,
	Kind_StockA,
	Kind_StockB,
	Kind_Bond,
	Kind_Fund,
	Kind_ETF,
	Kind_Warrents,
	Kind_Spot,
	Kind_Futures,
	Kind_Foreign
}STOCKTYPE;


// �г�ʱ��
typedef struct tag_DateTime
{
	unsigned short m_Year;
	unsigned char  m_Month;
	unsigned char  m_Day;
	unsigned char  m_Hour;
	unsigned char  m_Minute;
	unsigned char  m_Second;
	unsigned char  m_Reserved;
}QDateTime;

// ����ʱ��
typedef struct tag_TradePhase
{
	unsigned int m_nBeginTime;
	unsigned int m_nEndTime;
}QTradePhase;


typedef struct tag_StockInfo
{
	ULONG           m_ulMarketType;     // �г�����
	STOCKTYPE       m_StockType;        // Ʒ������
	char            m_szCode[CODE_SIZE];// ����
	char            m_szName[NAME_SIZE];// Ʒ������
	short           m_sDecimal;         // С����
	short           m_sUnit;            // ÿ�ֵ�λ
	ULONG           m_Values[10];       // ������ֵ 
	/*
	ULONG           m_lPrePrice;    ����/���
	ULONG           m_lPreHolding;  ���
	ULONG           m_l5DayVol;     5�ճɽ���
	ULONG           m_lYearHigh;    �����
	ULONG           m_lYearLow;     �����
	ULONG           m_lMonthHigh;   �����
	ULONG           m_lMonthLow;    �����*/
}QStockInfo;


typedef struct tag_Market_Base
{
    HSMarketDataType m_MarketType;         // �г�����
	char  m_acMarketName[MARKETNAME_SIZE]; // �г�����
	ULONG m_ulVersion;                     // �汾
	short m_sStatus;                       // �г�״̬
}QMarket_Base;

typedef struct tag_StockSort
{
	HSMarketDataType m_MarketType;
	char  m_szMarketName[MARKETNAME_SIZE]; // �������
	short            m_sPhasesCount;
	QTradePhase*     m_pTradePhases;       // ʱ������ָ��
	char             m_acCodeRange;        // ���뷶Χ
}QStockSort;

typedef struct tag_MarketInfo : public tag_Market_Base
{
    QDateTime        m_DateTime;               // ����ʱ��
	short            m_sPhasesCount;       // ʱ������
	QTradePhase*     m_pTradePhases;       // ʱ������ָ��
	short            m_sSortCount;         // ������
	QStockSort*      m_pStockSort;         // �����
	unsigned short   m_sStockCount;        // Ʒ������
	QStockInfo*      m_pStockInfo;         // Ʒ������ָ��
}QMarketInfo;











#endif