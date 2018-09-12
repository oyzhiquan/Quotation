#ifndef _COMM_H
#define _COMM_H

// ��д��
class CRWLock  
{  
protected:  
	CRWLock();  
	~CRWLock();  
	void ReadLock();  
	bool TryReadLock();  
	void WriteLock();  
	bool TryWriteLock();  
	void Unlock();  

private: 
#ifndef WIN32
	pthread_rwlock_t m_rwl;
#endif
};  

// ���ݻ�����
class CDataBuffer
{
private:
	char *  m_pszBuffer;   // ��ǰ���ͺͽ��յ����ݰ�
	ULONG   m_lBufferSize; // ���ܳ�����
	ULONG   m_lMaxSize;    // ��󳤶�
	ULONG   m_lIndex;      // ����

	CRWLock m_rwLock; // ��д��

public:
	CDataBuffer();

	~CDataBuffer();

	bool Realloc(ULONG lSize); // �����еĻ��������·���
	bool Alloc(ULONG lSize); //�����ڴ�
	void Free();   //�ͷ��ڴ�
	int IsValid() { return (m_lBufferSize > 0 && m_pszBuffer != NULL); }

	int Copy(CDataBuffer* pData);
	int Copy(const char * pszBuf, ULONG lSize);
	int Add(const char * pszBuf, ULONG lSize);

	int GetBufferSize(){return m_lBufferSize;}
	const char* GetBuffer(){return m_pszBuffer;}

	CDataBuffer& operator=(const char* lpsz)
	{
		Copy(lpsz, strlen(lpsz));
		return *this;
	}

	CDataBuffer& operator=(char* lpsz)
	{
		Copy(lpsz, strlen(lpsz));
		return *this;
	}

	CDataBuffer& operator=(CDataBuffer& pData)
	{
		Copy(pData);
		return *this;
	}

};

// ���߳����ݻ�����
class CMTDataBuffer : public CDataBuffer 
{
private:
	CRWLock m_rwLock; // ��д��

public:
	CMTDataBuffer();

	~CMTDataBuffer();

	bool Alloc(ULONG lSize); //�����ڴ�

	void Free();   //�ͷ��ڴ�

	int IsValid();

	int Copy(CDataBuffer* pData);
	int Copy(const char * pszBuf, ULONG lSize);

	int GetBufferSize();
	const char* GetBuffer();
};

// ���л�������
class CArchiveBuffer: public CMTDataBuffer
{
public:
	CArchiveBuffer();
	~CArchiveBuffer();

	WriteToFile();
	LoadFromFile();

	// create file mapping
	void Create(const char* filename, const unsigned int size);

	// destroy file mapping
	void Destroy();

	// determines whether the file mapping is opened
	bool isOpen()
	{
		return (m_pMemory != NULL) && (m_handle != NULL);
	}

	// �Ƿ��Ѿ������ڴ�ӳ���ĩ��?
	bool isEof()
	{
		return (m_pCurrent - m_pMemory) >= m_fileSize;
	}

	// �Ƿ��Ѿ������ڴ�ӳ��Ŀ���?
	bool isBof()
	{
		return m_pCurrent <= m_pMemory;
	}

	// �ļ�ָ����ǰ�ƶ�һ��size
	void forward(const unsigned int size)
	{
		m_pCurrent += size;
	}

	// �ļ�ָ������ƶ�һ��size
	void backward(const unsigned int size)
	{
		m_pCurrent -= size;
	}

	// �����ļ�ָ��
	void reset()
	{
		m_pCurrent = m_pMemory;
	}

	// write data to current position of share memory
	void write(const char* data, int len);

	// read data from current position of share memory
	void read(char* data, int len);

	// properties
public:
	// �õ��ڴ�ָ��
	char *getMemory()
	{
		return m_pMemory;
	}

	// �õ���ǰָ��
	char *getCurrent()
	{
		return m_pCurrent;
	}

	// �õ�����ڴ�ӳ��Ĵ�С
	int getSize()
	{
		return m_fileSize;
	}

private:
#ifdef WIN32
	HANDLE m_handle; // �ļ�ӳ����
#else
	int m_handle;
#endif
	string m_strFileName; // �ļ���
	char *m_pMemory; // �ļ�ӳ���׵�ַ
	char *m_pCurrent; // ��ǰָ��
	unsigned int m_fileSize; // �ļ���С

};

#endif