#pragma once

#include <windows.h>
#include <assert.h>
#include "misc.h"


/**
 * @class		CByteRingBuffer
 * @brief		�����O�o�b�t�@����
 */
class CByteRingBuffer
{
	/**
	 * @struct		RING_BUFFER
	 * @brief		�����O�o�b�t�@�f�[�^�\��
	 */
	struct RING_BUFFER {
		CRITICAL_SECTION	stCS;				//!< �r���������b�N�I�u�W�F�N�g
		unsigned char*		pbyBuff;			//!< �o�b�t�@������
		int					nBuffSize;			//!< �o�b�t�@�T�C�Y
		int					nDataHead;			//!< �f�[�^�̐擪�A�h���X
		int					nDataLength;		//!< �f�[�^�̒���
		int					nModMask;			//!< &���Z�ŏ�]�����߂邽�߂̃r�b�g�}�X�N
	};

private:
	RING_BUFFER			m_stRing;				//!< �����O�o�b�t�@�f�[�^

public:
	CByteRingBuffer(int nSize = 1024);
	~CByteRingBuffer();

	//! �����O�o�b�t�@�\���̂��N���A����
	int					Clear();
	//! �����O�o�b�t�@�Ƀf�[�^��ǉ�����
	int					Push(const unsigned char* pbySrc, int nLen);
	//! �����O�o�b�t�@���f�[�^���擾���A�o�b�t�@�̃f�[�^���폜����
	int					Pop(unsigned char* pbyDest, int nLen);
	//! �����O�o�b�t�@���f�[�^���擾����(�o�b�t�@�̃f�[�^�͍폜����Ȃ�)
	int					Peek(unsigned char* pbyDest, int nLen);
	//! �����O�o�b�t�@���̃f�[�^�����擾����
	int					Count();

private:
	//! �w��T�C�Y�����傫��2�ׂ̂���̃T�C�Y��Ԃ�
	int					calcBuffsize(int nSize);
	//! �����O�o�b�t�@�̔r������������������
	inline void			initLock();
	//! �����O�o�b�t�@�̔r���������I������
	inline void			deleteLock();
	//! �����O�o�b�t�@��r�����b�N����
	inline void			lock();
	//! �����O�o�b�t�@�̔r�����b�N����������
	inline void			unlock();
	//! debug
	void				debugPrint(const char* szProc);

//public:
//	//! debug
//	static void			test();
};


/**
 * @fn			�R���X�g���N�^
 * @brief		�����O�o�b�t�@�\���̂𐶐��A����������
 * @param[in]	int nSize			: �����O�o�b�t�@�̃T�C�Y
 * @remarks
 *		���ۂɊm�ۂ���郊���O�o�b�t�@�̃T�C�Y�͎w��T�C�Y�����傫��2�ׂ̂���̒l�ƂȂ�܂��B
 *      (&���Z�ŏ�]�����߂邽��)
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
 * @fn			�f�R���X�g���N�^
 * @brief		�����O�o�b�t�@�\���̂��g�p�I������
 */
CByteRingBuffer::~CByteRingBuffer()
{
	delete(m_stRing.pbyBuff);
	deleteLock();
}


/**
 * @fn			Clear
 * @brief		�����O�o�b�t�@�\���̂��N���A����
 * @return		0:����, -1:���s
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
 * @brief		�����O�o�b�t�@�Ƀf�[�^��ǉ�����
 * @param[in]	const unsigned char* pbySrc	: �ǉ�����f�[�^�ւ̃|�C���^
 * @param[in]	int nLen						: �ǉ�����f�[�^�̑傫��
 * @return		0:����, -1:���s
 * @remarks		�o�b�t�@���t���̎��͒ǉ��ł��܂���
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
 * @brief			�����O�o�b�t�@���f�[�^���擾���A�o�b�t�@�̃f�[�^���폜����
 * @param[in,out]	unsigned char* pbyDest		: �擾�����f�[�^���i�[����̈�ւ̃|�C���^
 * @param[in]		int nLen					; �擾����f�[�^�̑傫��
 * @return			0:����, -1:���s
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
 * @brief			�����O�o�b�t�@���f�[�^���擾����(�o�b�t�@�̃f�[�^�͍폜����Ȃ�)
 * @param[in,out]	unsigned char* pbyDest		: �擾�����f�[�^���i�[����̈�ւ̃|�C���^
 * @param[in]		int nLen					: �擾����f�[�^�̑傫��
 * @return			0:����, -1:���s
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
 * @brief		�����O�o�b�t�@���̃f�[�^�����擾����
 * @return		0�`:�f�[�^�̐�, -1:���s
 */
int CByteRingBuffer::Count()
{
	return m_stRing.nDataLength;
}


/**
 * @fn			calcBuffsize
 * @brief		�w��T�C�Y�����傫��2�ׂ̂���̃T�C�Y��Ԃ�
 * @param[in]	int nSize		: �o�b�t�@�T�C�Y
 * @return		�o�b�t�@�T�C�Y(2�ׂ̂���)
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

	// �w��T�C�Y��2�ׂ̂��撚�x
	if (bits == 1) exp_size >>= 1;

	return exp_size;
}


/**
 * @fn			initLock
 * @brief		�����O�o�b�t�@�̔r������������������
 */
void CByteRingBuffer::initLock()
{
	::InitializeCriticalSection(&(m_stRing.stCS));
}


/**
 * @fn			deleteLock
 * @brief		�����O�o�b�t�@�̔r���������I������
 */
void CByteRingBuffer::deleteLock()
{
	::DeleteCriticalSection(&(m_stRing.stCS));
}


/**
 * @fn			lock
 * @brief		�����O�o�b�t�@��r�����b�N����
 */
void CByteRingBuffer::lock()
{
	::EnterCriticalSection(&(m_stRing.stCS));
}


/**
 * @fn			unlock
 * @brief		�����O�o�b�t�@�̔r�����b�N����������
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
