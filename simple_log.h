/**
 * @file	log.h
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
#include <io.h>
#include <windows.h>
#include <Shlwapi.h>


#define MAX_ID								(5)
#define MAX_LOG_TEXT						(256)
#define MAX_FILE_SIZE						(1024)		// 1kByte�P��
#define MAX_LOG_BACKUP						(3)

#define LOG_START(log, path)				log_start(log, path)
#define LOG_END(log)						log_end(log)
#define LOG_WRITE(log, level, fmt, ...)		log_write(log, level, fmt, __VA_ARGS__)
#define LOG_DEBUG(log, level, fmt, ...)		log_debug(log, level, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)


 /**
  * @enum	_LOG_LEVEL
  * @brief	���O���x����`
  */
typedef enum _LOG_LEVEL {
	ERR = 1,
	WAR = 2,
	INF = 3,
	DBG = 4
} LOG_LEVEL;


/*
 * @struct	_LOG_INFO
 * @brief		���O���
 */
typedef struct _LOG_INFO {
	CRITICAL_SECTION	stCS;						//! ���OID���Ƃ̔r���I�u�W�F�N�g
	char				szLogPath[MAX_PATH + 1];	//! ���O�t�@�C���p�X
	BOOL				bUsed;					//! ���OID�g�p���

	char				szDir[MAX_PATH + 1];		//! �h���C�u�A�f�B���N�g����
	char				szFname[MAX_PATH + 1];	//! �t�@�C�����i�g���q�����j
	char				szFext[MAX_PATH + 1];		//! �g���q��
	int					nFileSize;				//! ���O�t�@�C���T�C�Y
	int					nLogBackup;				//! �t�@�C���o�b�N�A�b�v��
} LOG_INFO;


int					log_start(LOG_INFO* pstLog, const char* szPath);
int					log_end(LOG_INFO* pstLog);
int					log_write(LOG_INFO* pstLog, int nLevel, const char* szFmt, ...);
int					log_debug(LOG_INFO* pstLog, int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...);
static const char*	_log_level(int nLevel);
static const char*	_get_fname_from_path(const char* szPath, char* szBuff, int nSize);
static void			_lock_init(LOG_INFO* pstLog);
static void			_lock_delete(LOG_INFO* pstLog);
static void			_lock(LOG_INFO* pstLog);
static void			_unlock(LOG_INFO* pstLog);
static int			_copy_filepath(LOG_INFO* pstLog, const char* szPath);
static int			_get_backupname(LOG_INFO* pstLog, int nBkNo, char* szBuff, int nSize);
static int			_backup_file(LOG_INFO* pstLog);
static long			_get_filesize(const char* szPath);


/**
* @fn		End
* @brief
* @param	[in]
* @return
*/
int log_start(LOG_INFO* pstLog, const char* szPath)
{
	if (pstLog == NULL || szPath == NULL) {
		assert(FALSE);
	}

	memset(pstLog->szLogPath, 0, sizeof(pstLog->szLogPath));
	memset(pstLog->szDir, 0, sizeof(pstLog->szDir));
	memset(pstLog->szFname, 0, sizeof(pstLog->szFname));
	memset(pstLog->szFext, 0, sizeof(pstLog->szFext));
	pstLog->bUsed = TRUE;
	pstLog->nFileSize = MAX_FILE_SIZE;
	pstLog->nLogBackup = MAX_LOG_BACKUP;

	_lock_init(pstLog);

	// ���O�t�@�C���Ƀo�b�N�A�b�v�ԍ���t�^
	_copy_filepath(pstLog, szPath);

	return 0;
}

/**
* @fn		End
* @brief
* @param	[in]
* @return
*/
int log_end(LOG_INFO* pstLog)
{
	if (pstLog == NULL) {
		assert(FALSE);
	}
	_lock_delete(pstLog);
	pstLog->bUsed = FALSE;
	return 0;
}

/**
* @fn		write
* @brief
* @param	[in]
* @return
*/
int log_write(LOG_INFO* pstLog, int nLevel, const char* szFmt, ...)
{
	if (pstLog == NULL || szFmt == NULL || pstLog->bUsed == FALSE) {
		assert(FALSE);
	}

	char szBuff0[MAX_LOG_TEXT];
	char szBuff1[MAX_LOG_TEXT * 2];

	_lock(pstLog);
	_backup_file(pstLog);

	va_list arg;
	va_start(arg, szFmt);
	vsnprintf(szBuff0, sizeof(szBuff0), szFmt, arg);
	va_end(arg);

	errno = 0;
	FILE *fp = fopen(pstLog->szLogPath, "ab+");
	if (fp == NULL) {
		if (errno != 0) perror(NULL);
		return -1;
	}

	SYSTEMTIME stTime;
	::GetLocalTime((LPSYSTEMTIME)&stTime);
	snprintf(szBuff1, sizeof(szBuff1),
		"%04d/%02d/%02d, %02d:%02d:%02d.%03d, %s, " \
		"%s\r\n"
		, stTime.wYear
		, stTime.wMonth
		, stTime.wDay
		, stTime.wHour
		, stTime.wMinute
		, stTime.wSecond
		, stTime.wMilliseconds
		, _log_level(nLevel)
		, szBuff0);
	fputs(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		_unlock(pstLog);
		return -1;
	}
	_unlock(pstLog);
	return 0;
}

/**
* @fn		debug
* @brief
* @param	[in]
* @return
*/
int log_debug(LOG_INFO* pstLog, int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...)
{
	if (pstLog == NULL || szFmt == NULL || szFile == NULL || szFunc == NULL || pstLog->bUsed == FALSE) {
		assert(FALSE);
	}

	_lock(pstLog);
	_backup_file(pstLog);

	char szBuff0[MAX_LOG_TEXT];
	char szBuff1[MAX_LOG_TEXT * 2];
	char szFilename[MAX_PATH];
	memset(szFilename, 0, sizeof(szFilename));

	va_list arg;
	va_start(arg, szFmt);
	vsnprintf(szBuff0, sizeof(szBuff0), szFmt, arg);
	va_end(arg);

	errno = 0;
	FILE *fp = fopen(pstLog->szLogPath, "ab+");
	if (fp == NULL) {
		if (errno != 0) perror(NULL);
		return -1;
	}

	SYSTEMTIME stTime;
	::GetLocalTime((LPSYSTEMTIME)&stTime);
	snprintf(szBuff1, sizeof(szBuff1),
		"%04d/%02d/%02d, %02d:%02d:%02d.%03d, %s, " \
		"%s(%d), %s, " \
		"%s\r\n"
		, stTime.wYear
		, stTime.wMonth
		, stTime.wDay
		, stTime.wHour
		, stTime.wMinute
		, stTime.wSecond
		, stTime.wMilliseconds
		, _log_level(nLevel)
		, _get_fname_from_path(szFile, szFilename, sizeof(szFilename))
		, nLine
		, szFunc
		, szBuff0);
	fputs(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		_unlock(pstLog);
		return -1;
	}
	_unlock(pstLog);
	return 0;
}

/**
* @fn		logLevel
* @brief	���O���x���ɑΉ����閼�̂��擾
* @param	[in]	int nLevel	: ���O���x��(LOG_LEVEL)
* @return	���O���x������
*/
static const char* _log_level(int nLevel)
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

///**
//* @fn		getId
//* @brief	�g�p�\�ȃ��OID���擾����
//* @param	[in]	int nID		: �g�p���郍�OID (-1:�ŋ󂢂Ă���ID���擾)
//* @return	-1:���OID�擾���s(�󂫖���), 0�`:�g�p�\�ȃ��OID
//*/
//int CLog::getId(int nID)
//{
//	if (nID < -1 || MAX_ID <= nID) {
//		return -1;
//	}
//
//	int id = -1;
//
//	if (0 <= nID) {
//		// �w��ID�͎g�p�\��
//		if (m_bUsed[nID] == FALSE) {
//			id = nID;
//		}
//	}
//	else {
//		// -1:�󂢂Ă���ID���猟��
//		for (int i = 0; i < MAX_ID; i++) {
//			if (m_bUsed[i] == FALSE) {
//				id = i;
//				break;
//			}
//		}
//	}
//	return id;
//}

/**
* @fn		getFilename
* @brief	�t���p�X���t�@�C�����݂̂��擾
* @param	[IN]	char* szPath	: �t�@�C���p�X������
* @param	[OUT]	char* szBuff	: �t�@�C�������i�[����o�b�t�@�̈�
* @param	[IN]	int nSize		: �o�b�t�@�̈�̃T�C�Y
* @return	�o�b�t�@�̈�ւ̃|�C���^(�������s����"(NULL)"�̕������Ԃ�
*/
static const char* _get_fname_from_path(const char* szPath, char* szBuff, int nSize)
{
	if (szPath == NULL || szBuff == NULL) {
		return "(NULL)";
	}

	char szFname[MAX_PATH + 1];
	char szExt[MAX_PATH + 1];

	_splitpath(szPath, NULL, NULL, szFname, szExt);
	snprintf(szBuff, nSize, "%s%s", szFname, szExt);

	return szBuff;
}

/**
* @fn		_LockInit
* @brief	�r������������
* @param	[in]	int nID		 : ���OID
*/
static void _lock_init(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	::InitializeCriticalSection(&(pstLog->stCS));
}

/**
* @fn		_LockDelete
* @brief	�r������������
* @param	[in]	int nID		 : ���OID
*/
static void _lock_delete(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	::DeleteCriticalSection(&(pstLog->stCS));
}

/**
* @fn		_Lock
* @brief	�r������������
* @param	[in]	int nID		 : ���OID
*/
static void _lock(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	::EnterCriticalSection(&(pstLog->stCS));
}

/**
* @fn		_Unlock
* @brief	�r������������
* @param	[in]	int nID		 : ���OID
*/
static void _unlock(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	::LeaveCriticalSection(&(pstLog->stCS));
}

/**
* @fn		copyFilePath
* @brief	�w�胍�O�t�@�C�����Ƀo�b�N�A�b�v�ԍ���t�^����
* @param	[in]	int nID				: ���OID
* @param	[in]	const char* szPath	: ���O�o�͐�t�@�C���p�X
*/
static int _copy_filepath(LOG_INFO* pstLog, const char* szPath)
{
	if (pstLog == NULL || szPath == NULL) {
		return -1;
	}

	char tmpDrive[MAX_PATH + 1];
	char tmpDir[MAX_PATH + 1];
	char tmpFname[MAX_PATH + 1];
	char tmpFext[MAX_PATH + 1];

	_splitpath(szPath, tmpDrive, tmpDir, tmpFname, tmpFext);

	memset(pstLog->szDir, 0, sizeof(pstLog->szDir));
	memset(pstLog->szFname, 0, sizeof(pstLog->szFname));
	memset(pstLog->szFext, 0, sizeof(pstLog->szFext));
	memset(pstLog->szLogPath, 0, sizeof(pstLog->szLogPath));

	snprintf(pstLog->szDir, sizeof(pstLog->szDir), "%s%s", tmpDrive, tmpDir);
	snprintf(pstLog->szFname, sizeof(pstLog->szFname), "%s", tmpFname);
	snprintf(pstLog->szFext, sizeof(pstLog->szFext), "%s", tmpFext);

	// �t�@�C����_0.�g���q
	int ret = _get_backupname(pstLog, 0, pstLog->szLogPath, sizeof(pstLog->szLogPath));

	return ret;
}

/**
* @fn		createBackupName
* @brief	�w�胍�O�t�@�C�����Ƀo�b�N�A�b�v�ԍ���t�^����
* @param	[in]	int nID			: ���OID
* @param	[in]	int nBkNo		: �o�b�N�A�b�v�ԍ�
* @param	[in]	char* szBuff	: �o�͐�o�b�t�@�̈�
* @param	[in]	int nSize		: �o�b�t�@�̈�̃T�C�Y
* @return	0:����, -1:���s
*/
static int _get_backupname(LOG_INFO* pstLog, int nBkNo, char* szBuff, int nSize)
{
	if (pstLog == NULL || szBuff == NULL || nBkNo < 0 || pstLog->nLogBackup < nBkNo) {
		assert(FALSE);
	}
	snprintf(szBuff, nSize, "%s%s_%d%s", pstLog->szDir, pstLog->szFname, nBkNo, pstLog->szFext);
	return 0;
}

/**
* @fn		backupFile
* @brief	���O�t�@�C���o�b�N�A�b�v����
* @param	[in]	int nID		:���OID
* @return	0:����, -1:���s
*/
static int _backup_file(LOG_INFO* pstLog)
{
	if (pstLog == NULL) {
		return -1;
	}

	long fsize = 0;
	if ((fsize = _get_filesize(pstLog->szLogPath)) < 0) {
		return 0;
	}

	char szBefore[MAX_PATH + 1];
	char szAfter[MAX_PATH + 1];

	if (pstLog->nFileSize <= fsize) {
		// �t�@�C���o�b�N�A�b�v����
		// �ł��Â��t�@�C���͍폜
		_get_backupname(pstLog, pstLog->nLogBackup, szBefore, sizeof(szBefore));
		if (::PathFileExists(szBefore) && !::PathIsDirectory(szBefore)) {
			::DeleteFile(szBefore);
		}
		// �t�@�C�������l�[���A1���Â��炷
		for (int nBkNo = pstLog->nLogBackup - 1; 0 <= nBkNo; nBkNo--) {
			_get_backupname(pstLog, nBkNo, szBefore, sizeof(szBefore));
			if (::PathFileExists(szBefore) && !::PathIsDirectory(szBefore)) {
				_get_backupname(pstLog, nBkNo + 1, szAfter, sizeof(szBefore));
				::MoveFile(szBefore, szAfter);
			}
		}
	}

	return 0;
}

/**
* @fn		getFileSize
* @brief	�t�@�C���T�C�Y�擾(2GB�܂�)
* @param	[in]	char* szPath		: �t�@�C���p�X
* @return	�t�@�C���T�C�Y(-1:���s)
*/
static long _get_filesize(const char* szPath)
{
	if (szPath == NULL) {
		return -1;
	}

	//errno = 0;
	FILE *fp = fopen(szPath, "rb");
	if (fp == NULL) {
		//if (errno != 0) perror(NULL);
		return -1;
	}

	long fsize = _filelength(_fileno(fp));
	fsize >>= 10;		// fsize/1024

	if (fclose(fp) != 0) {
		//if (errno != 0) perror(NULL);
		return -1;
	}
	return fsize;
}
