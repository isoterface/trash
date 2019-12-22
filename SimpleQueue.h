/**
 * @file	SimpleQueue.h
 * @brief	キューデータ
 * @author	?
 * @date	?
 */
#pragma once

#include <windows.h>


/**
 * @class	CQueue
 * @brief	キューデータクラス
 */
class CQueue
{
private:
	CRITICAL_SECTION	m_stCS;
	unsigned char*		m_pbyQueue;
	int					m_nQueueSize;
	int					m_nModMask;
	int					m_nHead;
	int					m_nLength;

public:
	CQueue();
	CQueue(int nSize);
	~CQueue();

	int					Clear();
	int					Enqueue(const unsigned char* pbyData, int nLen);
	int					Dequeue(unsigned char* pbyBuff, int nLen);
	int					Peek(unsigned char* pbyBuff, int nLen);
	int					IsEmpty();
	int					GetLength();

private:
	int					getPow2Size(int nSize);

	void				lockInit();
	void				lockDelete();
	void				lock();
	void				unlock();

	void				debug_print();
};


/**
 * コンストラクタ
 */
CQueue::CQueue() : CQueue(1024)
{
}
CQueue::CQueue(int nSize)
{
	m_nQueueSize = getPow2Size(nSize);
	m_pbyQueue = new unsigned char[m_nQueueSize];
	memset(m_pbyQueue, 0, m_nQueueSize);

	m_nModMask = m_nQueueSize - 1;
	m_nHead = 0;
	m_nLength = 0;

	lockInit();
}

/**
 * デストラクタ
 */
CQueue::~CQueue()
{
	delete[] m_pbyQueue;
	lockDelete();
}

/**
 * @fn		Clear
 * @brief	
 * @return	
 */
int CQueue::Clear()
{
	lock();
	memset(m_pbyQueue, 0, m_nQueueSize);
	m_nHead = 0;
	m_nLength = 0;
	unlock();

	return 0;
}

/**
 * @fn		Enqueue
 * @brief	
 * @param	[IN]
 * @param	[IN]
 * @return
 */
int CQueue::Enqueue(const unsigned char* pbyData, int nLen)
{
	if (pbyData == NULL) {
		return -1;
	}

	lock();
	int count = 0;
	int ptr = 0;
	for (int i = 0; i < nLen; i++) {
		if (m_nLength < m_nQueueSize) {
			ptr = ((m_nHead + m_nLength) & m_nModMask);
			m_pbyQueue[ptr] = pbyData[i];
			m_nLength++;
			count++;
		}
		else {
			break;
		}
	}
	unlock();

	debug_print();

	return count;
}

/**
 * @fn		Dequeue
 * @brief	
 * @param	[IN]
 * @param	[IN]
 * @return
 */
int CQueue::Dequeue(unsigned char* pbyBuff, int nLen)
{
	if (pbyBuff == NULL) {
		return -1;
	}

	lock();
	int count = 0;
	for (int i = 0; i < nLen; i++) {
		if (0 < m_nLength) {
			pbyBuff[i] = m_pbyQueue[m_nHead];
			m_pbyQueue[m_nHead] = 0;
			m_nHead = ((m_nHead + 1) & m_nModMask);
			m_nLength--;
			count++;
		}
		else {
			break;
		}
	}
	unlock();

	debug_print();

	return count;
}

/**
 * @fn		Peek
 * @brief
 * @param	[IN]
 * @param	[IN]
 * @return
 */
int CQueue::Peek(unsigned char* pbyBuff, int nLen)
{
	if (pbyBuff == NULL) {
		return -1;
	}

	lock();
	int ptr = m_nHead;
	int count = 0;
	for (int i = 0; i < nLen; i++) {
		if (0 < m_nLength) {
			pbyBuff[i] = m_pbyQueue[ptr];
			ptr = ((ptr + 1) & m_nModMask);
			count++;
			if (ptr == m_nHead) break;
		}
		else {
			break;
		}
	}
	unlock();

	debug_print();

	return count;
}

/**
 * @fn		IsEmpty
 * @brief	
 * @return	
 */
int CQueue::IsEmpty()
{
	return ((m_nLength == 0) ? (0) : (-1));
}

/**
 * @fn		GetLength
 * @brief
 * @return
 */
int CQueue::GetLength()
{
	return m_nLength;
}

/**
 * @fn		getPow2Size
 * @brief	
 * @param	[IN]	
 * @return	
 */
int CQueue::getPow2Size(int nSize)
{
	int exp_size = 1;
	int org_size = nSize;

	do {
		exp_size <<= 1;
		org_size >>= 1;
	} while (0 < org_size);

	return exp_size;
}

/**
 * @fn		lockInit
 * @brief	排他処理初期化
 */
inline void CQueue::lockInit()
{
	::InitializeCriticalSection(&m_stCS);
}
/**
 * @fn		lockDelete
 * @brief	排他処理初期化
 */
inline void CQueue::lockDelete()
{
	::DeleteCriticalSection(&m_stCS);
}
/**
 * @fn		lock
 * @brief	排他処理初期化
 */
inline void CQueue::lock()
{
	::EnterCriticalSection(&m_stCS);
}
/**
 * @fn		unlock
 * @brief	排他処理初期化
 */
inline void CQueue::unlock()
{
	::LeaveCriticalSection(&m_stCS);
}


void CQueue::debug_print()
{
	char szBuff[256];
	mem_dump(m_pbyQueue, m_nQueueSize, szBuff, sizeof(szBuff));
	printf("H:%02d,L:%02d,[%s]\r\n", m_nHead, m_nLength, szBuff);
}
