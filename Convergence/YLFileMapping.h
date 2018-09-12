/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      HisData 
* File Name:	YLFileMapping.h 
* Programer(s):	Jerry Lee 
* Created:      20101227 
* Description:	interface for file mapping list
* History:
*******************************************************************************/

#ifndef _YLFILEMAPPING_H
#define _YLFILEMAPPING_H 

#pragma once

#include <string>
#include <vector>
#include "windows.h"
#include "datatypes.h"


using namespace std;

/*
* �ļ�ӳ����-CYLFileMapping
* ��;��������Ҫ���ڿ��ٶ�ȡ�ļ�
*/
class CYLFileMapping
{
public:
    CYLFileMapping();

    CYLFileMapping(const char *filename);

    ~CYLFileMapping();

    // methods
public:
    // ���ļ�
    void Open(const char *filename);

    // �ر��ļ�
    void Close();

    // �жϵ�ǰ�Ƿ����ļ�����
    bool IsOpen()
    {
        return (m_pMemory != YLNULL) && (m_handle != YLNULL);
    }

    // �Ƿ��Ѿ������ڴ�ӳ���ĩ��
    bool IsEof()
    {
        return (m_pCurrent-m_pMemory) >= m_fileSize;
    }

    // �Ƿ��Ѿ������ڴ�ӳ��Ŀ���
    bool IsBof()
    {
        return m_pCurrent <= m_pMemory;
    }

    // �ļ�ָ����ǰ�ƶ�һ��size
    void Forward(const unsigned int size)
    {
        m_pCurrent += size;
    }

    // �ļ�ָ������ƶ�һ��size
    void Backward(const unsigned int size)
    {
        m_pCurrent -= size;
    }

    // �����ļ�ָ��
    void Reset()
    {
        m_pCurrent = m_pMemory;
    }

    // properties
public:
    // �õ��ڴ�ָ��
    char *GetMemory()
    {
        return m_pMemory;
    }

    // �õ���ǰָ��
    char *GetCurrent()
    {
        return m_pCurrent;
    }

    // �õ������ڴ�ӳ��Ĵ�С
    int GetSize()
    {
        return m_fileSize;
    }

private:
    HANDLE  m_handle;       // �ļ�ӳ����
    string  m_strFileName;  // �ļ���
    char    *m_pMemory;     // �ļ�ӳ���׵�ַ
    char    *m_pCurrent;    // ��ǰָ��
    int     m_fileSize;     // �ļ���С
};


#endif