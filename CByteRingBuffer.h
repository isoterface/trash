#pragma once

#include <windows.h>
#include <assert.h>
#include "misc.h"


/**
 * @class		CByteRingBuffer
 * @brief		リングバッファ処理
 */
class CByteRingBuffer
{
	/**
	 * @struct		RING_BUFFER
	 * @brief		リングバッファデータ構造
	 */
	struct RING_BUFFER {
		CRITICAL_SECTION	stCS;				//!< 排他処理ロックオブジェクト
		unsigned char*		pbyBuff;			//!< バッファメモリ
		int					nBuffSize;			//!< バッファサイズ
		int					nDataHead;			//!< データの先頭アドレス
		int					nDataLength;		//!< データの長さ
		int					nModMask;			//!< &演算で剰余を求めるためのビットマスク
	};

private:
	RING_BUFFER			m_stRing;				//!< リングバッファデータ

public:
	CByteRingBuffer(int nSize = 1024);
	~CByteRingBuffer();

	//! リングバッファ構造体をクリアする
	int					Clear();
	//! リングバッファにデータを追加する
	int					Push(const unsigned char* pbySrc, int nLen);
	//! リングバッファよりデータを取得し、バッファのデータを削除する
	int					Pop(unsigned char* pbyDest, int nLen);
	//! リングバッファよりデータを取得する(バッファのデータは削除されない)
	int					Peek(unsigned char* pbyDest, int nLen);
	//! リングバッファ内のデータ数を取得する
	int					Count();

private:
	//! 指定サイズよりも大きな2のべき乗のサイズを返す
	int					calcBuffsize(int nSize);
	//! リングバッファの排他処理を初期化する
	inline void			initLock();
	//! リングバッファの排他処理を終了する
	inline void			deleteLock();
	//! リングバッファを排他ロックする
	inline void			lock();
	//! リングバッファの排他ロックを解除する
	inline void			unlock();
	//! debug
	void				debugPrint(const char* szProc);

//public:
//	//! debug
//	static void			test();
};


/**
 * @fn			コンストラクタ
 * @brief		リングバッファ構造体を生成、初期化する
 * @param[in]	int nSize			: リングバッファのサイズ
 * @remarks
 *		実際に確保されるリングバッファのサイズは指定サイズよりも大きな2のべき乗の値となります。
 *      (&演算で剰余を求めるため)
 */
CByteRingBuffer::CByteRingBuffer(int nSize/*=1024*/)
{
	memset(&m_stRing, 0, sizeof(RING_BUFFER));
	m_stRing.nBuffSize = calcBuffsize(nSize);
	m_stRing.nModMask = m_stRing.nBuffSize - 1;
	m_stRing.pbyBuff = new unsigned char[m_stRing.nBuffSize];
	if (m_stRing.pbyBuff == NULL) {
		return;
	}
	initLock();
	Clear();
}


/**
 * @fn			デコンストラクタ
 * @brief		リングバッファ構造体を使用終了する
 */
CByteRingBuffer::~CByteRingBuffer()
{
	delete(m_stRing.pbyBuff);
	deleteLock();
}


/**
 * @fn			Clear
 * @brief		リングバッファ構造体をクリアする
 * @return		0:成功, -1:失敗
 */
int CByteRingBuffer::Clear()
{
	lock();
	memset(m_stRing.pbyBuff, 0, sizeof(unsigned char) * m_stRing.nBuffSize);
	m_stRing.nDataHead = 0;
	m_stRing.nDataLength = 0;
	unlock();

	return 0;
}


/**
 * @fn			Push
 * @brief		リングバッファにデータを追加する
 * @param[in]	const unsigned char* pbySrc	: 追加するデータへのポインタ
 * @param[in]	int nLen						: 追加するデータの大きさ
 * @return		0:成功, -1:失敗
 * @remarks		バッファがフルの時は追加できません
 */
int CByteRingBuffer::Push(const unsigned char* pbySrc, int nLen)
{
	if (m_stRing.pbyBuff == NULL || pbySrc == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	int count = 0;
	int ptr = 0;

	lock();
	for (int i = 0; i < nLen; i++) {
		if (m_stRing.nDataLength < m_stRing.nBuffSize) {
			ptr = ((m_stRing.nDataHead + m_stRing.nDataLength) & m_stRing.nModMask);
			m_stRing.pbyBuff[ptr] = pbySrc[i];
			m_stRing.nDataLength++;
			count++;
		}
	}
	unlock();

	debugPrint("Push");

	return count;
}


/**
 * @fn				Pop
 * @brief			リングバッファよりデータを取得し、バッファのデータを削除する
 * @param[in,out]	unsigned char* pbyDest		: 取得したデータを格納する領域へのポインタ
 * @param[in]		int nLen					; 取得するデータの大きさ
 * @return			0:成功, -1:失敗
 */
int CByteRingBuffer::Pop(unsigned char* pbyDest, int nLen)
{
	if (m_stRing.pbyBuff == NULL || pbyDest == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	int count = 0;

	lock();
	for (int i = 0; i < nLen; i++) {
		if (0 < m_stRing.nDataLength) {
			pbyDest[i] = m_stRing.pbyBuff[m_stRing.nDataHead];
			m_stRing.pbyBuff[m_stRing.nDataHead] = 0;
			m_stRing.nDataHead = ((m_stRing.nDataHead + 1) & m_stRing.nModMask);
			m_stRing.nDataLength--;
			count++;
		}
		else {
			break;
		}
	}
	unlock();

	debugPrint("Pop");

	return count;
}


/**
 * @fn				Peek
 * @brief			リングバッファよりデータを取得する(バッファのデータは削除されない)
 * @param[in,out]	unsigned char* pbyDest		: 取得したデータを格納する領域へのポインタ
 * @param[in]		int nLen					: 取得するデータの大きさ
 * @return			0:成功, -1:失敗
 */
int CByteRingBuffer::Peek(unsigned char* pbyDest, int nLen)
{
	if (m_stRing.pbyBuff == NULL || pbyDest == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	int ptr = m_stRing.nDataHead;
	int count = 0;

	lock();
	for (int i = 0; i < nLen; i++) {
		if (0 < m_stRing.nDataLength) {
			pbyDest[i] = m_stRing.pbyBuff[ptr];
			ptr = ((ptr + 1) & m_stRing.nModMask);
			count++;
		}
		else {
			break;
		}
	}
	unlock();

	debugPrint("Peek");

	return count;
}


/**
 * @fn			Count
 * @brief		リングバッファ内のデータ数を取得する
 * @return		0～:データの数, -1:失敗
 */
int CByteRingBuffer::Count()
{
	return m_stRing.nDataLength;
}


/**
 * @fn			calcBuffsize
 * @brief		指定サイズよりも大きな2のべき乗のサイズを返す
 * @param[in]	int nSize		: バッファサイズ
 * @return		バッファサイズ(2のべき乗)
 */
int CByteRingBuffer::calcBuffsize(int nSize)
{
	int exp_size = 1;
	int org_size = nSize;
	int bits = 0;

	do {
		if (org_size & 0x1 != 0) bits++;
		exp_size <<= 1;
		org_size >>= 1;
	} while (0 < org_size);

	// 指定サイズは2のべき乗丁度
	if (bits == 1) exp_size >>= 1;

	return exp_size;
}


/**
 * @fn			initLock
 * @brief		リングバッファの排他処理を初期化する
 */
void CByteRingBuffer::initLock()
{
	::InitializeCriticalSection(&(m_stRing.stCS));
}


/**
 * @fn			deleteLock
 * @brief		リングバッファの排他処理を終了する
 */
void CByteRingBuffer::deleteLock()
{
	::DeleteCriticalSection(&(m_stRing.stCS));
}


/**
 * @fn			lock
 * @brief		リングバッファを排他ロックする
 */
void CByteRingBuffer::lock()
{
	::EnterCriticalSection(&(m_stRing.stCS));
}


/**
 * @fn			unlock
 * @brief		リングバッファの排他ロックを解除する
 */
void CByteRingBuffer::unlock()
{
	::LeaveCriticalSection(&(m_stRing.stCS));
}


/**
 * @fn			debugPrint
 * @brief
 * @return
 */
void CByteRingBuffer::debugPrint(const char* szProc)
{
	char szBuff[256];
	int size = (16 < m_stRing.nBuffSize) ? (m_stRing.nBuffSize) : (16);
	mem_dump(m_stRing.pbyBuff, size, szBuff, sizeof(szBuff));
	printf("%s: H:%02d,L:%02d,[%s]\r\n", szProc, m_stRing.nDataHead, m_stRing.nDataLength, szBuff);
}


/*
void CByteRingBuffer::test()
{
	CByteRingBuffer cBuff(16);

	unsigned char pbySrc[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
	unsigned char dest[128];
	char dump[256];

	cBuff.Push(pbySrc, 1);
	cBuff.Push(pbySrc, 2);
	cBuff.Push(pbySrc, 3);
	cBuff.Push(pbySrc, 4);
	cBuff.Push(pbySrc, 5);
	cBuff.Push(pbySrc, 6);

	memset(dest, 0, COUNT_OF_ARRAY(dest));
	cBuff.Pop(dest, 1);
	printf(">> %s\r\n", mem_dump(dest, 1, dump, sizeof(dump)));
	memset(dest, 0, COUNT_OF_ARRAY(dest));
	cBuff.Pop(dest, 2);
	printf(">> %s\r\n", mem_dump(dest, 2, dump, sizeof(dump)));
	memset(dest, 0, COUNT_OF_ARRAY(dest));
	cBuff.Pop(dest, 3);
	printf(">> %s\r\n", mem_dump(dest, 3, dump, sizeof(dump)));

	cBuff.Push(pbySrc, 6);
	cBuff.Push(pbySrc, 5);

	getchar();
}
*/
