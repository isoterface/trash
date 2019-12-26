/**
* @file		ring_buffer.h
* @brief	リングバッファ処理
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


RING_BUFFER*	init_queue(int nSize);
int				delete_queue(RING_BUFFER* pstRing);
int				clear_queue(RING_BUFFER *pstRing);
int				queue_push(RING_BUFFER* pstRing, const unsigned char* puchData, int nLen);
int				queue_pop(RING_BUFFER* pstRing, unsigned char* puchBuff, int nLen);
int				queue_peek(RING_BUFFER* pstRing, unsigned char* puchBuff, int nLen);
int				queue_data_count(RING_BUFFER* pstRing);
static int		_calc_buffsize(int nSize);
static void		_buff_lock_init(RING_BUFFER* pstRing);
static void		_buff_lock_delete(RING_BUFFER* pstRing);
static void		_lock_buff(RING_BUFFER* pstRing);
static void		_unlock_buff(RING_BUFFER* pstRing);
static void		debug_print(RING_BUFFER* pstRing);


/**
 * @fn		init_queue
 * @brief	リングバッファ構造体を生成、初期化する
 * @param	[IN]	int nSize			: リングバッファのサイズ
 * @return	リングバッファ構造体へのポインタ
 * @remarks
 *		実際に確保されるリングバッファのサイズは指定サイズよりも大きな2のべき乗の値となります。
 *		本関数内で malloc でデータ領域を確保しています。
 *		リングバッファ使用終了時に必ず delete_queue を呼んでください。
 */
RING_BUFFER* init_queue(int nSize)
{
	RING_BUFFER* pstRing;

	pstRing = (RING_BUFFER*)malloc(sizeof(RING_BUFFER));
	if (pstRing == NULL) {
		return NULL;
	}
	memset(pstRing, 0, sizeof(RING_BUFFER));
	pstRing->nBuffSize = _calc_buffsize(nSize);
	pstRing->nModMask = pstRing->nBuffSize - 1;
	pstRing->puchBuff = (unsigned char*)malloc(sizeof(unsigned char) * pstRing->nBuffSize);
	if (pstRing->puchBuff == NULL) {
		free(pstRing);
		return NULL;
	}
	_buff_lock_init(pstRing);
	clear_queue(pstRing);

	return pstRing;
}

/**
 * @fn		delete_queue
 * @brief	リングバッファ構造体を使用終了する
 * @param	[IN]	RING_BUFFER* pstRing		: リングバッファ構造体へのポインタ
 * @return	0:成功, -1:失敗
 * @remarks 
 *		本関数内で init_queue にて確保されたデータ領域を開放しています。
 *		リングバッファ使用終了時に必ず本関数を呼んでください。
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
	_buff_lock_delete(pstRing);
	free(pstRing);

	return 0;
}

/**
 * @fn		clear_queue
 * @brief	リングバッファ構造体をクリアする
 * @param	[IN]	RING_BUFFER* pstRing		: リングバッファ構造体へのポインタ
 * @return	0:成功, -1:失敗
 */
int clear_queue(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	_lock_buff(pstRing);
	memset(pstRing->puchBuff, 0, sizeof(unsigned char) * pstRing->nBuffSize);
	pstRing->nDataHead = 0;
	pstRing->nDataLength = 0;
	_unlock_buff(pstRing);

	return 0;
}

/**
 * @fn		queue_push
 * @brief	リングバッファにデータを追加する
 * @param	[IN]	RING_BUFFER* pstRing			: リングバッファ構造体へのポインタ
 * @param	[IN]	const unsigned char* puchData	: 追加するデータへのポインタ
 * @param	[IN]	int nLen						: 追加するデータの大きさ
 * @return	0:成功, -1:失敗
 */
int queue_push(RING_BUFFER* pstRing, const unsigned char* puchData, int nLen)
{
	if (pstRing == NULL || puchData == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	_lock_buff(pstRing);
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
	_unlock_buff(pstRing);

	debug_print(pstRing);

	return count;
}

/**
 * @fn		queue_pop
 * @brief	リングバッファよりデータを取得し、バッファのデータを削除する
 * @param	[IN]	RING_BUFFER* pstRing		: リングバッファ構造体へのポインタ
 * @param	[OUT]	unsigned char* puchBuff		: 取得したデータを格納する領域へのポインタ
 * @param	[IN]	int nLen					; 取得するデータの大きさ
 * @return	0:成功, -1:失敗
 */
int queue_pop(RING_BUFFER* pstRing, unsigned char* puchBuff, int nLen)
{
	if (pstRing == NULL || puchBuff == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	_lock_buff(pstRing);
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
	_unlock_buff(pstRing);

	//debug_print(pstRing);

	return count;
}

/**
 * @fn		queue_peek
 * @brief	リングバッファよりデータを取得する(バッファのデータは削除されない)
 * @param	[IN]	RING_BUFFER* pstRing		: リングバッファ構造体へのポインタ
 * @param	[OUT]	unsigned char* puchBuff		: 取得したデータを格納する領域へのポインタ
 * @param	[IN]	int nLen					: 取得するデータの大きさ
 * @return	0:成功, -1:失敗
 */
int queue_peek(RING_BUFFER* pstRing, unsigned char* puchBuff, int nLen)
{
	if (pstRing == NULL || puchBuff == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	_lock_buff(pstRing);
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
	_unlock_buff(pstRing);

	//debug_print(pstRing);

	return count;
}

/**
 * @fn		queue_data_count
 * @brief	リングバッファ内のデータ数を取得する
 * @param	[IN]	RING_BUFFER* pstRing		: リングバッファ構造体へのポインタ
 * @return	0～:データの数, -1:失敗
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
 * @brief	指定サイズよりも大きな2のべき乗のサイズを返す
 * @param	[IN]	int nSize		: バッファサイズ
 * @return	バッファサイズ(2のべき乗)
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
 * @fn		_buff_lock_init
 * @brief	リングバッファの排他処理を初期化する
 * @param	[IN]	RING_BUFFER* pstRing		: リングバッファ構造体へのポインタ
 */
static void _buff_lock_init(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) { assert(FALSE); }
	::InitializeCriticalSection(&(pstRing->stCS));
}

/**
 * @fn		_buff_lock_delete
 * @brief	リングバッファの排他処理を終了する
 * @param	[IN]	RING_BUFFER* pstRing		: リングバッファ構造体へのポインタ
 */
static void _buff_lock_delete(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) { assert(FALSE); }
	::DeleteCriticalSection(&(pstRing->stCS));
}

/**
 * @fn		_lock_buff
 * @brief	リングバッファを排他ロックする
 * @param	[IN]	RING_BUFFER* pstRing		: リングバッファ構造体へのポインタ
 */
static void _lock_buff(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) { assert(FALSE); }
	::EnterCriticalSection(&(pstRing->stCS));
}

/**
 * @fn		_unlock_buff
 * @brief	リングバッファの排他ロックを解除する
 * @param	[IN]	RING_BUFFER* pstRing		: リングバッファ構造体へのポインタ
 */
static void _unlock_buff(RING_BUFFER* pstRing)
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
