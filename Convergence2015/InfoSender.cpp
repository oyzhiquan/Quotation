/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      cvg 
* File Name:	InfoSender.cpp 
* Programer(s):	Jerry Lee 
* Created:      20110228 
* Description:	implementation of sending information
* History:
*******************************************************************************/

#include "InfoSender.h"
#include "YLFileMapping.h"

CInfoSender::CInfoSender(const char *contentPath, CDeliverMgr *pDeliverMgr,
                         unsigned int nodeId)
: m_contentPath(contentPath),
m_nodeId(nodeId),
m_pDeliverMgr(pDeliverMgr)
{
}

CInfoSender::CInfoSender(CDeliverMgr *pDeliverMgr, unsigned int nodeId)
:m_pDeliverMgr(pDeliverMgr),
 m_nodeId(nodeId)
{
	
}

CInfoSender::~CInfoSender()
{
}


void CInfoSender::Send()
{
    
}

void CInfoSender::Send(const InfoFile *pIF)
{
    // һ����Ѷ
    InfoData info;
    memset(&info, 0, sizeof(info));

    // �����Ѷͷ
    memcpy(info.header.marketType, pIF->marketType, sizeof(pIF->marketType));
    memcpy(info.header.productCode, pIF->productCode, sizeof(pIF->productCode));
    memcpy(info.header.dataType, "INFO", 4);
    memcpy(info.title, pIF->title, sizeof(pIF->title));
    info.dateTime = pIF->dateTime;

    // �����Ѷ����
    string strFileName = m_contentPath + "\\" + pIF->path;
    CYLFileMapping fileMapping;
    fileMapping.Open(strFileName.c_str());
    if (fileMapping.IsOpen())
    {
        info.header.length = min(fileMapping.GetSize(), sizeof(m_buf));
        memcpy(m_buf, fileMapping.GetMemory(), info.header.length);
    }
    else
    {
        // added by Jerry Lee, 2012-2-23
        CRLog(E_APPINFO, "Could not create file mapping: %s", strFileName.c_str()); 

        return;
    }

    fileMapping.Close();

    // �趨����
    info.content = m_buf;

    // �������ݰ�
    SendPacket(info);
}

void CInfoSender::SendPacket(const InfoData &infoData)
{
    // ����ʷ���ݿ�������������
    string strInfoData;
    strInfoData.append((char *)&infoData.header, sizeof(infoData.header));
    strInfoData.append((char *)&infoData.dateTime, sizeof(infoData.dateTime));
    strInfoData.append(infoData.title, sizeof(infoData.title));
    strInfoData.append(infoData.content, infoData.header.length);

    // ������ʷ���ݷ��ͱ���
    CSamplerPacket oPkt(YL_INFODATA);
    CMessageImpl &msg = dynamic_cast<CMessageImpl &>(oPkt.GetMsg());	
    msg.SetBinaryField(MSG_INFO_DATA, strInfoData);
    msg.SetField(MSG_NODE_ID, m_nodeId);
    unsigned int uiSeqNo = 0;
    msg.SetField(MSG_SEQ_ID, uiSeqNo);

    // ��������
    m_pDeliverMgr->HandleInfoData(oPkt, m_nodeId);
}