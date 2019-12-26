/**
* @file		ring_buffer.h
* @brief	�����O�o�b�t�@����
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
 * @brief	�����O�o�b�t�@
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
 * @brief	�����O�o�b�t�@�\���̂𐶐��A����������
 * @param	[IN]	int nSize			: �����O�o�b�t�@�̃T�C�Y
 * @return	�����O�o�b�t�@�\���̂ւ̃|�C���^
 * @remarks
 *		���ۂɊm�ۂ���郊���O�o�b�t�@�̃T�C�Y�͎w��T�C�Y�����傫��2�ׂ̂���̒l�ƂȂ�܂��B
 *		�{�֐����� malloc �Ńf�[�^�̈���m�ۂ��Ă��܂��B
 *		�����O�o�b�t�@�g�p�I�����ɕK�� delete_queue ���Ă�ł��������B
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
 * @brief	�����O�o�b�t�@�\���̂��g�p�I������
 * @param	[IN]	RING_BUFFER* pstRing		: �����O�o�b�t�@�\���̂ւ̃|�C���^
 * @return	0:����, -1:���s
 * @remarks 
 *		�{�֐����� init_queue �ɂĊm�ۂ��ꂽ�f�[�^�̈���J�����Ă��܂��B
 *		�����O�o�b�t�@�g�p�I�����ɕK���{�֐����Ă�ł��������B
 */
int delete_queue(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}
	// �o�b�t�@�A�r�������N���A������Ƀ����O�o�b�t�@�����J��
	free(pstRing->puchBuff);
	_buff_lock_delete(pstRing);
	free(pstRing);

	return 0;
}

/**
 * @fn		clear_queue
 * @brief	�����O�o�b�t�@�\���̂��N���A����
 * @param	[IN]	RING_BUFFER* pstRing		: �����O�o�b�t�@�\���̂ւ̃|�C���^
 * @return	0:����, -1:���s
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
 * @brief	�����O�o�b�t�@�Ƀf�[�^��ǉ�����
 * @param	[IN]	RING_BUFFER* pstRing			: �����O�o�b�t�@�\���̂ւ̃|�C���^
 * @param	[IN]	const unsigned char* puchData	: �ǉ�����f�[�^�ւ̃|�C���^
 * @param	[IN]	int nLen						: �ǉ�����f�[�^�̑傫��
 * @return	0:����, -1:���s
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
 * @brief	�����O�o�b�t�@���f�[�^���擾���A�o�b�t�@�̃f�[�^���폜����
 * @param	[IN]	RING_BUFFER* pstRing		: �����O�o�b�t�@�\���̂ւ̃|�C���^
 * @param	[OUT]	unsigned char* puchBuff		: �擾�����f�[�^���i�[����̈�ւ̃|�C���^
 * @param	[IN]	int nLen					; �擾����f�[�^�̑傫��
 * @return	0:����, -1:���s
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
 * @brief	�����O�o�b�t�@���f�[�^���擾����(�o�b�t�@�̃f�[�^�͍폜����Ȃ�)
 * @param	[IN]	RING_BUFFER* pstRing		: �����O�o�b�t�@�\���̂ւ̃|�C���^
 * @param	[OUT]	unsigned char* puchBuff		: �擾�����f�[�^���i�[����̈�ւ̃|�C���^
 * @param	[IN]	int nLen					: �擾����f�[�^�̑傫��
 * @return	0:����, -1:���s
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
 * @brief	�����O�o�b�t�@���̃f�[�^�����擾����
 * @param	[IN]	RING_BUFFER* pstRing		: �����O�o�b�t�@�\���̂ւ̃|�C���^
 * @return	0�`:�f�[�^�̐�, -1:���s
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
 * @brief	�w��T�C�Y�����傫��2�ׂ̂���̃T�C�Y��Ԃ�
 * @param	[IN]	int nSize		: �o�b�t�@�T�C�Y
 * @return	�o�b�t�@�T�C�Y(2�ׂ̂���)
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
 * @brief	�����O�o�b�t�@�̔r������������������
 * @param	[IN]	RING_BUFFER* pstRing		: �����O�o�b�t�@�\���̂ւ̃|�C���^
 */
static void _buff_lock_init(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) { assert(FALSE); }
	::InitializeCriticalSection(&(pstRing->stCS));
}

/**
 * @fn		_buff_lock_delete
 * @brief	�����O�o�b�t�@�̔r���������I������
 * @param	[IN]	RING_BUFFER* pstRing		: �����O�o�b�t�@�\���̂ւ̃|�C���^
 */
static void _buff_lock_delete(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) { assert(FALSE); }
	::DeleteCriticalSection(&(pstRing->stCS));
}

/**
 * @fn		_lock_buff
 * @brief	�����O�o�b�t�@��r�����b�N����
 * @param	[IN]	RING_BUFFER* pstRing		: �����O�o�b�t�@�\���̂ւ̃|�C���^
 */
static void _lock_buff(RING_BUFFER* pstRing)
{
	if (pstRing == NULL) { assert(FALSE); }
	::EnterCriticalSection(&(pstRing->stCS));
}

/**
 * @fn		_unlock_buff
 * @brief	�����O�o�b�t�@�̔r�����b�N����������
 * @param	[IN]	RING_BUFFER* pstRing		: �����O�o�b�t�@�\���̂ւ̃|�C���^
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
