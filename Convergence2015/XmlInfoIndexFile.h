/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      cvg 
* File Name:	XmlInfoIndexFile.h 
* Programer(s):	Jerry Lee 
* Created:      20110224 
* Description:	interface for xml information index file
* History:
*******************************************************************************/


#pragma once


#include <vector>

using namespace std;

#ifndef MAX_LENGTH
#define MAX_LENGTH 256
#endif

// ��Ѷ�ļ���ʽ
// ��Ѷ�����ļ���һ����¼
typedef struct tagInfoFile
{
    char marketType[12];      //kenny ��������  2014-1-9
    char productCode[6];
    char title[128];
    int dateTime;
    char path[MAX_LENGTH];
    int fileSize;
} InfoFile;


// ��Ѷ�����ļ�
typedef struct tagInfoIndex
{
    int version;
    int count;
    InfoFile infoFiles[1];
} InfoIndex;

//
class CXmlInfoIndexFile
{
public:
    CXmlInfoIndexFile();
    ~CXmlInfoIndexFile();
public:
    // ���ļ�
    bool Open(const char *filename);

    // �ر��ļ�
    void Close();

    // ���������õ��ڴ��ַ
    InfoFile *GetItem(int index);

    // �õ�Item������
    int GetCount()
    {
        return static_cast<int>(m_infofiles.size());
    }

private:
    InfoIndex *m_pInfoIndex;

    vector<InfoFile> m_infofiles;

    double m_version;

    int m_count;
};
