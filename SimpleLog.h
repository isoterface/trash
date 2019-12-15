/**
 * @file	SimpleLog.h
 * @brief	���O�o��
 * @author	?
 * @date	?
 */
#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <windows.h>


#define MAX_ID		(10)
#define LOG_MAX		(256)


 /**
  * @enum	_LOG_LEVEL
  * @brief	���O���x����`
  */
typedef enum LOG_LEVEL {
	ERR = 1,
	WAR = 2,
	INF = 3,
	DBG = 4
}_LOG_LEVEL;


/**
 * @class	CLog
 * @brief	���O�o�̓N���X
 */
class CLog
{
private:
	//! �X�^�e�B�b�N�����o�ϐ��i�錾�̂݁A���̂̊m�ۂ�cpp���ōs���j
	static CRITICAL_SECTION	m_stCS[MAX_ID];						//! ���OID���Ƃ̔r���I�u�W�F�N�g
	static char				m_szLogPath[MAX_ID][MAX_PATH];		//! ���O�t�@�C���p�X
	static BOOL				m_bUsed[MAX_ID];					//! ���OID�g�p���

public:
	CLog();
	//~CLog();

	static int				Start(int nID, const char* szPath);
	static int				Start(const char* szPath);
	static int				End(int nID);
	static int				End();
	static int				Write(int nID, int nLevel, const char* szFmt, ...);
	static int				Write(int nLevel, const char* szFmt, ...);

private:
	static int				write(int nID, int nLevel, const char* szFmt, va_list arg);
	static const char*		logLevel(int nLevel);
	static int				getId(int nID);
};


//! �X�^�e�B�b�N�����o�ϐ��̎��̂̊m�ہA������
//! ���OID���Ƃ̔r���I�u�W�F�N�g
CRITICAL_SECTION CLog::m_stCS[MAX_ID];
//! ���O�t�@�C���p�X
char CLog::m_szLogPath[MAX_ID][MAX_PATH];
//! ���OID�g�p���
int CLog::m_bUsed[MAX_ID] = {
	FALSE, FALSE, FALSE, FALSE, FALSE,
	FALSE, FALSE, FALSE, FALSE, FALSE
};


/**
 * �R���X�g���N�^
 */
CLog::CLog()
{
	// �C���X�^���X�͐������Ȃ�
	assert(FALSE);
}

//CLog::~CLog()
//{
//}

/**
 * @fn		Start
 * @brief	�w�胍�O�t�@�C���ւ̃��O�o�͂��J�n����
 * @param	[in]	int nID				: ���O�o�͎��̎w�胍�OID((0)�`(MAX_ID-1), -1��ID�������w��)
 * @param	[in]	const char* szPath	: ���O�o�͐�t�@�C���p�X
 * @return	(0)�`(MAX_ID-1):����(�Ȍ�̃��O�o�͎͂擾����ID���g�p����), -1:���s
 */
int CLog::Start(int nID, const char* szPath)
{
	if (szPath == NULL) {
		return -1;
	}

	int id = getId(nID);
	if (id < 0) {
		return -1;
	}

	// TODO:�p�X�`�F�b�N

	memset(m_szLogPath[id], 0, sizeof(m_szLogPath[id]));
	strncpy(m_szLogPath[id], szPath, sizeof(m_szLogPath[id]));
	InitializeCriticalSection(&(m_stCS[id]));
	m_bUsed[id] = TRUE;

	return id;
}

/**
 * @fn		Start
 * @brief	�w�胍�O�t�@�C���ւ̃��O�o�͂��J�n����(ID�͎����w��)
 * @param	[in]	const char* szPath	: ���O�o�͐�t�@�C���p�X
 * @return	(0)�`(MAX_ID-1):����(�Ȍ�̃��O�o�͎͂擾����ID���g�p����), -1:���s
 */
int CLog::Start(const char* szPath)
{
	if (szPath == NULL) {
		return -1;
	}
	return CLog::Start(-1, szPath);		// ����Ăяo���ł���Ύ����I�� ID=0 �����蓖�Ă���
}

/**
 * @fn		End
 * @brief
 * @param	[in]
 * @return
 */
int CLog::End(int nID)
{
	if (nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	// �w��ID�̂݊J��
	if (m_bUsed[nID] == TRUE) {
		DeleteCriticalSection(&(m_stCS[nID]));
		m_bUsed[nID] = FALSE;
	}
	return 0;
}

/**
 * @fn		End
 * @brief
 * @param	[in]
 * @return
 */
int CLog::End()
{
	// �SID�J��
	for (int i = 0; i < MAX_ID; i++) {
		if (m_bUsed[i] == TRUE) {
			DeleteCriticalSection(&(m_stCS[i]));
			m_bUsed[i] = FALSE;
		}
	}
	return 0;
}

/**
 * @fn		Write
 * @brief
 * @param	[in]
 * @return
 */
int CLog::Write(int nID, int nLevel, const char* szFmt, ...)
{
	if (szFmt == NULL) {
		return -1;
	}

	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = write(nID, nLevel, szFmt, arg);
	va_end(arg);
	return ret;
}

/**
 * @fn		Write
 * @brief
 * @param	[in]
 * @return
 */
int CLog::Write(int nLevel, const char* szFmt, ...)
{
	if (szFmt == NULL) {
		return -1;
	}

	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = write(0, nLevel, szFmt, arg);
	va_end(arg);
	return ret;
}

/**
 * @fn		write
 * @brief
 * @param	[in]
 * @return
 */
int CLog::write(int nID, int nLevel, const char* szFmt, va_list arg)
{
	if (szFmt == NULL) {
		return -1;
	}

	if (nID < 0 || MAX_ID <= nID || m_bUsed[nID] == FALSE) {
		return -1;
	}

	char szBuff0[LOG_MAX];
	char szBuff1[LOG_MAX * 2];

	vsnprintf(szBuff0, sizeof(szBuff0), szFmt, arg);

	EnterCriticalSection(&(m_stCS[nID]));
	errno = 0;
	FILE *fp = fopen(m_szLogPath[nID], "a+");
	if (fp == NULL) {
		if (errno != 0) perror(NULL);
		return -1;
	}

	SYSTEMTIME stTime;
	GetLocalTime((LPSYSTEMTIME)&stTime);
	snprintf(szBuff1, sizeof(szBuff1),
		"%04d/%02d/%02d, %02d:%02d:%02d.%03d, %s, %s\n"
		, stTime.wYear
		, stTime.wMonth
		, stTime.wDay
		, stTime.wHour
		, stTime.wMinute
		, stTime.wSecond
		, stTime.wMilliseconds
		, logLevel(nLevel)
		, szBuff0);
	fputs(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		LeaveCriticalSection(&(m_stCS[nID]));
		return -1;
	}
	LeaveCriticalSection(&(m_stCS[nID]));
	return 0;
}

/**
 * @fn		log_level
 * @brief
 * @param	[in]	int nLevel	:
 * @return
 */
const char* CLog::logLevel(int nLevel)
{
	switch (nLevel) {
	case ERR:
		return "ERR";
	case WAR:
		return "WAR";
	case INF:
		return "INF";
	case DBG:
		return "DBG";
	default:
		return "???";
	}
}

/**
 * @fn		get_id
 * @brief
 * @param	[in]	int nID		:
 * @return
 */
int CLog::getId(int nID)
{
	if (nID < -1 || MAX_ID <= nID) {
		return -1;
	}

	int id = -1;

	if (0 <= nID) {
		// �w��ID�͎g�p�\��
		if (m_bUsed[nID] == FALSE) {
			id = nID;
		}
	}
	else {
		// -1:�󂢂Ă���ID���猟��
		for (int i = 0; i < MAX_ID; i++) {
			if (m_bUsed[i] == FALSE) {
				id = i;
				break;
			}
		}
	}
	return id;
}
