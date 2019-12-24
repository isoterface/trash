/**
* @file		queue.h
* @brief	キューデータ
* @author	?
* @date		?
*/
#pragma once

#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include "misc.h"


/**
 * @struct	RING_BUFFER
 * @brief	リングバッファ
 */
typedef struct _RING_BUFFER {
	CRITICAL_SECTION	stCS;
	unsigned char*		puchBuff;
	int					nBuffSize;
	int					nDataHead;
	int					nDataLength;
	int					nModMask;
} RING_BUFFER;


RING_BUFFER*	create_queue(int nSize);
int				delete_queue(RING_BUFFER* pstRing);
int				queue_clear(RING_BUFFER *pstRing);
int				queue_push(RING_BUFFER* pstRing, const unsigned char* puchData, int nLen);
int				queue_pop(RING_BUFFER* pstRing, unsigned char* puchBuff, int nLen);
int				queue_peek(RING_BUFFER* pstRing, unsigned char* puchBuff, int nLen);
int				queue_data_count(RING_BUFFER* pstRing);
static int		_calc_buffsize(int nSize);
static void		_queue_lock_init(RING_BUFFER* pstRing);
static void		_queue_lock_delete(RING_BUFFER* pstRing);
static void		_lock_queue(RING_BUFFER* pstRing);
static void		_unlock_queue(RING_BUFFER* pstRing);
static void		debug_print(RING_BUFFER* pstRing);


/**
 * @fn		create_queue
 * @brief	
 * @param	[IN]
 * @return	
 */
RING_BUFFER* create_queue(int nSize)
{
	RING_BUFFER* pstRing;

	pstRing = (RING_BUFFER*)malloc(sizeof(RING_BUFFER));
	if (pstRing == NULL) {
		return NULL;
	}
	pstRing->nBuffSize = _calc_buffsize(nSize);
	pstRing->nModMask = pstRing->nBuffSize - 1;
	pstRing->puchBuff = (unsigned char*)malloc(sizeof(unsigned char) * pstRing->nBuffSize);
	if (pstRing->puchBuff == NULL) {
		free(pstRing);
		return NULL;
	}
	_queue_lock_init(pstRing);
	queue_clear(pstRing);

	return pstRing;
}

/**
 * @fn		delete_queue
 * @brief	
 * @param	[IN]
 * @return	
 */
int delete_queue(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}
	// バッファ、排他情報をクリアした後にリングバッファ情報を開放
	free(pstRing->puchBuff);
	_queue_lock_delete(pstRing);
	free(pstRing);

	return 0;
}

/**
 * @fn		queue_clear
 * @brief	
 * @param	[IN]
 * @return	
 */
int queue_clear(RING_BUFFER *pstRing)
{
	if (pstRing == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	_lock_queue(pstRing);
	memset(pstRing->puchBuff, 0, sizeof(unsigned char) * pstRing->nBuffSize);
	pstRing->nDataHead = 0;
	pstRing->nDataLength = 0;
	_unlock_queue(pstRing);

	return 0;
}

/**
 * @fn		queue_push
 * @brief	
 * @param	[IN]
 * @return	
 */
int queue_push(RING_BUFFER* pstRing, const unsigned char* puchData, int nLen)
{
	if (pstRing == NULL || puchData == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	_lock_queue(pstRing);
	int count = 0;
	int ptr = 0;
	for (int i = 0; i < nLen; i++) {
		if (pstRing->nDataLength < pstRing->nBuffSize) {
			ptr = ((pstRing->nDataHead + pstRing->nDataLength) & pstRing->nModMask);
			pstRing->puchBuff[ptr] = puchData[i];
			pstRing->nDataLength++;
			count++;
		}
		else {
			break;
		}
	}
	_unlock_queue(pstRing);

	debug_print(pstRing);

	return count;
}

/**
 * @fn		queue_pop
 * @brief	
 * @param	[IN]
 * @return	
 */
int queue_pop(RING_BUFFER* pstRing, unsigned char* puchBuff, int nLen)
{
	if (pstRing == NULL || puchBuff == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	_lock_queue(pstRing);
	int count = 0;
	for (int i = 0; i < nLen; i++) {
		if (0 < pstRing->nDataLength) {
			puchBuff[i] = pstRing->puchBuff[pstRing->nDataHead];
			pstRing->puchBuff[pstRing->nDataHead] = 0;
			pstRing->nDataHead = ((pstRing->nDataHead + 1) & pstRing->nModMask);
			pstRing->nDataLength--;
			count++;
		}
		else {
			break;
		}
	}
	_unlock_queue(pstRing);

	debug_print(pstRing);

	return count;
}

/**
 * @fn		queue_peek
 * @brief	
 * @param	[IN]
 * @return	
 */
int queue_peek(RING_BUFFER* pstRing, unsigned char* puchBuff, int nLen)
{
	if (pstRing == NULL || puchBuff == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	_lock_queue(pstRing);
	int ptr = pstRing->nDataHead;
	int count = 0;
	for (int i = 0; i < nLen; i++) {
		if (0 < pstRing->nDataLength) {
			puchBuff[i] = pstRing->puchBuff[ptr];
			ptr = ((ptr + 1) & pstRing->nModMask);
			count++;
		}
		else {
			break;
		}
	}
	_unlock_queue(pstRing);

	debug_print(pstRing);

	return count;
}

/**
 * @fn		queue_data_count
 * @brief	
 * @param	[IN]
 * @return	
 */
int queue_data_count(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}
	return pstRing->nDataLength;
}

/**
 * @fn		_calc_buffsize
 * @brief	
 * @param	[IN]
 * @return	
 */
static int _calc_buffsize(int nSize)
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
 * @fn		_queue_lock_init
 * @brief	
 * @param	[IN]
 * @return	
 */
static void _queue_lock_init(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) { assert(FALSE); }
	::InitializeCriticalSection(&(pstRing->stCS));
}

/**
 * @fn		_queue_lock_delete
 * @brief	
 * @param	[IN]
 * @return	
 */
static void _queue_lock_delete(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) { assert(FALSE); }
	::DeleteCriticalSection(&(pstRing->stCS));
}

/**
 * @fn		_lock_queue
 * @brief	
 * @param	[IN]
 * @return	
 */
static void _lock_queue(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) { assert(FALSE); }
	::EnterCriticalSection(&(pstRing->stCS));
}

/**
 * @fn		_unlock_queue
 * @brief	
 * @param	[IN]
 * @return	
 */
static void _unlock_queue(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) { assert(FALSE); }
	::LeaveCriticalSection(&(pstRing->stCS));
}

/**
 * @fn		debug_print
 * @brief	
 * @param	[IN]
 * @return	
 */
static void debug_print(RING_BUFFER* pstRing)
{
	char szBuff[256];
	int size = (16 < pstRing->nBuffSize) ? (pstRing->nBuffSize) : (16);
	mem_dump(pstRing->puchBuff, size, szBuff, sizeof(szBuff));
	printf("H:%02d,L:%02d,[%s]\r\n", pstRing->nDataHead, pstRing->nDataLength, szBuff);
}
