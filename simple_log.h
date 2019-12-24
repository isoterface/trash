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
 * @brief	���O���
 */
typedef struct _LOG_INFO {
	CRITICAL_SECTION	stCS;						//! ���OID���Ƃ̔r���I�u�W�F�N�g
	char				szLogPath[MAX_PATH + 1];	//! ���O�t�@�C���p�X
	BOOL				bUsed;						//! ���OID�g�p���
	// ���O�o�b�N�A�b�v���
	int					nFileSize;					//! ���O�t�@�C���T�C�Y
	int					nLogBackup;					//! �t�@�C���o�b�N�A�b�v��
	char				szDir[MAX_PATH + 1];		//! �h���C�u�A�f�B���N�g����
	char				szFname[MAX_PATH + 1];		//! �t�@�C�����i�g���q�����j
	char				szFext[MAX_PATH + 1];		//! �g���q��
} LOG_INFO;


int					log_start(LOG_INFO* pstLog, const char* szPath);
int					log_end(LOG_INFO* pstLog);
int					log_write(LOG_INFO* pstLog, LOG_LEVEL enLevel, const char* szFmt, ...);
int					log_debug(LOG_INFO* pstLog, LOG_LEVEL enLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...);
static const char*	_log_level(LOG_LEVEL enLevel);
static const char*	_get_fname_from_path(const char* szPath, char* szBuff, int nSize);
static void			_log_lock_init(LOG_INFO* pstLog);
static void			_log_lock_delete(LOG_INFO* pstLog);
static void			_log_lock(LOG_INFO* pstLog);
static void			_log_unlock(LOG_INFO* pstLog);
static int			_copy_filepath(LOG_INFO* pstLog, const char* szPath);
static int			_get_backupname(LOG_INFO* pstLog, int nBkNo, char* szBuff, int nSize);
static int			_backup_file(LOG_INFO* pstLog);
static long			_get_filesize(const char* szPath);


/**
 * @fn		log_start
 * @brief	���O�o�͂��J�n����
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 * @param	[in]	const char* szPath		: ���O�t�@�C���p�X
 * @return	0:����, -1:���s
 */
int log_start(LOG_INFO* pstLog, const char* szPath)
{
	if (pstLog == NULL || szPath == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	memset(pstLog->szLogPath, 0, sizeof(pstLog->szLogPath));
	memset(pstLog->szDir, 0, sizeof(pstLog->szDir));
	memset(pstLog->szFname, 0, sizeof(pstLog->szFname));
	memset(pstLog->szFext, 0, sizeof(pstLog->szFext));
	pstLog->bUsed = TRUE;
	pstLog->nFileSize = MAX_FILE_SIZE;
	pstLog->nLogBackup = MAX_LOG_BACKUP;

	_log_lock_init(pstLog);

	// ���O�t�@�C���Ƀo�b�N�A�b�v�ԍ���t�^
	_copy_filepath(pstLog, szPath);

	return 0;
}

/**
 * @fn		log_end
 * @brief	���O�o�͂��I������
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 * @return	0:����, -1:���s
 */
int log_end(LOG_INFO* pstLog)
{
	if (pstLog == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}
	_log_lock_delete(pstLog);
	pstLog->bUsed = FALSE;
	return 0;
}

/**
 * @fn		log_write
 * @brief	���O�o��
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 * @param	[in]	LOG_LEVEL enLevel		: ���O���x��
 * @param	[in]	const char* szFmt		: ���O�o�͏���
 * @param	[in]	...						: �o�͏����p�����[�^
 * @return	0:����, -1:���s
 */
int log_write(LOG_INFO* pstLog, LOG_LEVEL enLevel, const char* szFmt, ...)
{
	if (pstLog == NULL || szFmt == NULL || pstLog->bUsed == FALSE) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	char szBuff0[MAX_LOG_TEXT];
	char szBuff1[MAX_LOG_TEXT * 2];

	_log_lock(pstLog);
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
		, _log_level(enLevel)
		, szBuff0);
	fputs(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		_log_unlock(pstLog);
		return -1;
	}
	_log_unlock(pstLog);
	return 0;
}

/**
 * @fn		log_debug
 * @brief	���O�o��
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 * @param	[in]	LOG_LEVEL enLevel		: ���O���x��
 * @param	[in]	const char* szFile		: �o�͎��̃t�@�C����
 * @param	[in]	int nLine				: �o�͎��̍s��
 * @param	[in]	const char* szFunc		: �o�͎��̊֐���
 * @param	[in]	const char* szFmt		: ���O�o�͏���
 * @param	[in]	...						: �o�͏����p�����[�^
 * @return	0:����, -1:���s
 */
int log_debug(LOG_INFO* pstLog, LOG_LEVEL enLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...)
{
	if (pstLog == NULL || szFmt == NULL || szFile == NULL || szFunc == NULL || pstLog->bUsed == FALSE) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	_log_lock(pstLog);
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
		, _log_level(enLevel)
		, _get_fname_from_path(szFile, szFilename, sizeof(szFilename))
		, nLine
		, szFunc
		, szBuff0);
	fputs(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		_log_unlock(pstLog);
		return -1;
	}
	_log_unlock(pstLog);
	return 0;
}

/**
 * @fn		_log_level
 * @brief	���O���x���ɑΉ����閼�̂��擾
 * @param	[in]	LOG_LEVEL enLevel	: ���O���x��
 * @return	���O���x������
 */
static const char* _log_level(LOG_LEVEL enLevel)
{
	switch (enLevel) {
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
 * @fn		_get_fname_from_path
 * @brief	�t���p�X���t�@�C�����݂̂��擾
 * @param	[in]	const char* szPath	: �t�@�C���p�X������
 * @param	[out]	char* szBuff		: �t�@�C�������i�[����o�b�t�@�̈�
 * @param	[in]	int nSize			: �o�b�t�@�̈�̃T�C�Y
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
 * @fn		_log_lock_init
 * @brief	�r������������
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 */
static void _log_lock_init(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	::InitializeCriticalSection(&(pstLog->stCS));
}

/**
 * @fn		_log_lock_delete
 * @brief	�r������������
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 */
static void _log_lock_delete(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	::DeleteCriticalSection(&(pstLog->stCS));
}

/**
 * @fn		_log_lock
 * @brief	�r������������
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 */
static void _log_lock(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	::EnterCriticalSection(&(pstLog->stCS));
}

/**
 * @fn		_log_unlock
 * @brief	�r������������
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 */
static void _log_unlock(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	::LeaveCriticalSection(&(pstLog->stCS));
}

/**
 * @fn		_copy_filepath
 * @brief	���O�t�@�C���p�X����Ɨp�ϐ��ɃR�s�[
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 * @param	[in]	const char* szPath		: ���O�o�͐�t�@�C���p�X
 * @return	0:����, -1:���s
 */
static int _copy_filepath(LOG_INFO* pstLog, const char* szPath)
{
	if (pstLog == NULL || szPath == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
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
 * @fn		_get_backupname
 * @brief	�w�胍�O�t�@�C�����Ƀo�b�N�A�b�v�ԍ���t�^����
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 * @param	[in]	int nBkNo				: �o�b�N�A�b�v�ԍ�
 * @param	[out]	char* szBuff			: �o�͐�o�b�t�@�̈�
 * @param	[in]	int nSize				: �o�b�t�@�̈�̃T�C�Y
 * @return	0:����, -1:���s
 */
static int _get_backupname(LOG_INFO* pstLog, int nBkNo, char* szBuff, int nSize)
{
	if (pstLog == NULL || szBuff == NULL || nBkNo < 0 || pstLog->nLogBackup < nBkNo) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}
	snprintf(szBuff, nSize, "%s%s_%d%s", pstLog->szDir, pstLog->szFname, nBkNo, pstLog->szFext);
	return 0;
}

/**
 * @fn		_backup_file
 * @brief	���O�t�@�C���o�b�N�A�b�v����
 * @param	[in]	LOG_INFO* pstLog		: ���O���
 * @return	0:����, -1:���s
 */
static int _backup_file(LOG_INFO* pstLog)
{
	if (pstLog == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
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
 * @fn		_get_filesize
 * @brief	�t�@�C���T�C�Y�擾(2GB�܂�)
 * @param	[in]	const char* szPath		: �t�@�C���p�X
 * @return	�t�@�C���T�C�Y(-1:���s)
 */
static long _get_filesize(const char* szPath)
{
	if (szPath == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
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
