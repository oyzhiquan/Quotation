/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      HisData 
* File Name:	FileMappingList.h 
* Programer(s):	Jerry Lee 
* Created:      20101227 
* Description:	interface for file mapping list
*               ��ģ�������ڶ�ȡ�̶��ṹ�����ļ�, ֧��������ʺ�˳����ʣ�˳����ʵ�Ч�ʸ���
* History:
*******************************************************************************/

#ifndef _FILEMAPPINGLIST_H
#define _FILEMAPPINGLIST_H 

#pragma once

#include <string>
#include <vector>
#include "windows.h"
#include "YLFileMapping.h"

using namespace std;


template<typename T, int _SIZE=sizeof(T)>
class CFileMappingList
{
public:
    CFileMappingList() : m_strFileName("")
    {

    }

    ~CFileMappingList()
    {

    }

    // methods
public:
    // ���ļ�
    void Open(const char *filename)
    {
        m_fileMapping.Open(filename);

        if (m_fileMapping.IsOpen())
        {
            m_strFileName = filename;
        }
    }

    // �ر��ļ�
    void Close()
    {
        m_fileMapping.Close();

        m_strFileName.clear();
    }

    // �ж��ļ��Ƿ��Ѿ���
    bool IsOpen()
    {
        return !m_strFileName.empty() && m_fileMapping.IsOpen();
    }

    // �Ƿ��Ѿ����ڿ���
    bool IsBof()
    {
        return m_fileMapping.IsBof();
    }

    // �Ƿ��Ѿ�����ĩ��
    bool IsEof()
    {
        return m_fileMapping.IsEof();
    }

    // �õ���ǰItemָ�룬�����ļ�ָ������ƶ�һ��Item���÷�������˳�����
    T *Next()
    {
        char *p = m_fileMapping.GetCurrent();
        
        m_fileMapping.Forward(_SIZE);  

        return (T *)p;
    }

    // �õ���ǰItemָ�룬�����ļ�ָ����ǰ�ƶ�һ��Item���÷�������˳�����
    T *Prior()
    {
        byte *p = m_fileMapping.GetCurrent();

        m_fileMapping.Backward(_SIZE);  

        return (T *)p;
    }


    // properties
public:        
    // ���������õ��ڴ��ַ
    T *GetItem(int index)
    {
        byte *p = m_fileMapping.GetMemory() + index*_SIZE;

        return (T *)p;
    }
        
    // �õ�Item������
    int GetCount()
    {
        return m_fileMapping.GetSize()/_SIZE;
    }

private:
    string  m_strFileName;

    CYLFileMapping m_fileMapping;
};

#endif
