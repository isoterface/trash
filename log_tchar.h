/**
 * @file		log_tchar.h
 * @brief		���O�o��(Unicode�Ή�)
 * @author		?
 * @date		?
 */
#pragma once

// ���[�j���OC4996�}�~�J�n
#pragma warning(push)
#pragma warning(disable:4996)

#pragma comment(lib, "Shlwapi.lib")

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <io.h>
#include <tchar.h>
#include <windows.h>
#include <Shlwapi.h>


#define MAX_LOG_TEXT		(256)		//!< ���O�o�͓�����̍ő�e�L�X�g��
#define MAX_FILE_SIZE		(1024)		//!< ���O�t�@�C���ő�T�C�Y[kByte]
#define MAX_LOG_BACKUP		(3)			//!< ���O�o�b�N�A�b�v�ۑ���

//! ���O�o�͊J�n
#define LOG_START(pInf, path)				log_start(pInf, path)
//! ���O�o�͏I��
#define LOG_END(pInf)						log_end(pInf)
//! ���O�o��
#define LOG_WRITE(pInf, level, fmt, ...)	log_write(pInf, level, fmt, __VA_ARGS__)
//! ���O�o��(�f�o�b�O)
#define LOG_DEBUG(pInf, level, fmt, ...)	log_debug(pInf, level, _T(__FILE__), __LINE__, _T(__FUNCTION__), fmt, __VA_ARGS__)


 /**
  * @enum		LOG_LEVEL
  * @brief		���O���x����`
  */
typedef enum {
	ERR = 1,		//!< �G���[
	WAR,			//!< �x��
	INF,			//!< ���
	DBG				//!< �f�o�b�O���
} LOG_LEVEL;


/*
 * @struct		LOG_INFO
 * @brief		���O���
 */
typedef struct {
	CRITICAL_SECTION	stCS;						//!< ���OID���Ƃ̔r���I�u�W�F�N�g
	TCHAR				szLogPath[MAX_PATH + 1];	//!< ���O�t�@�C���p�X
	BOOL				bUsed;						//!< ���OID�g�p���
	// ���O�o�b�N�A�b�v���
	int					nFileSize;					//!< ���O�t�@�C���T�C�Y
	int					nLogBackup;					//!< �t�@�C���o�b�N�A�b�v��
	TCHAR				szDir[MAX_PATH + 1];		//!< �h���C�u�A�f�B���N�g����
	TCHAR				szFname[MAX_PATH + 1];		//!< �t�@�C�����i�g���q�����j
	TCHAR				szFext[MAX_PATH + 1];		//!< �g���q��
} LOG_INFO;


//! ���O�o�͂��J�n����
int					log_start(LOG_INFO* pstLog, LPCTSTR lpszPath);
//! ���O�o�͂��I������
int					log_end(LOG_INFO* pstLog);
//! ���O�o��
int					log_write(LOG_INFO* pstLog, LOG_LEVEL enLevel, LPCTSTR lpszFmt, ...);
//! ���O�o��(�f�o�b�O)
int					log_debug(LOG_INFO* pstLog, LOG_LEVEL enLevel, LPCTSTR lpszFile, int nLine, LPCTSTR lpszFunc, LPCTSTR lpszFmt, ...);
//! ���O���x���ɑΉ����閼�̂��擾
static LPCTSTR		_log_level(LOG_LEVEL enLevel);
//! �t���p�X���t�@�C�����݂̂��擾
static LPCTSTR		_get_fname_from_path(LPCTSTR lpszPath, LPTSTR lpszBuff, int nSize);
//! �r������������
static void			_log_lock_init(LOG_INFO* pstLog);
//! �r�������I��
static void			_log_lock_delete(LOG_INFO* pstLog);
//! �r�����b�N�J�n
static void			_log_lock(LOG_INFO* pstLog);
//! �r�����b�N����
static void			_log_unlock(LOG_INFO* pstLog);
//! ���O�t�@�C���p�X����Ɨp�ϐ��ɃR�s�[
static int			_save_filepath(LOG_INFO* pstLog, LPCTSTR lpszPath);
//! �w�胍�O�t�@�C�����Ƀo�b�N�A�b�v�ԍ���t�^����
static int			_get_backupname(LOG_INFO* pstLog, int nBkNo, LPTSTR lpszBuff, int nSize);
//! ���O�t�@�C���o�b�N�A�b�v����
static int			_backup_file(LOG_INFO* pstLog);
//! �t�@�C���T�C�Y�擾(2GB�܂�)
static long			_get_filesize(LPCTSTR lpszPath);


/**
 * @fn			log_start
 * @brief		���O�o�͂��J�n����
 * @param[in]	LOG_INFO* pstLog		: ���O���
 * @param[in]	LPCTSTR lpszPath		: ���O�t�@�C���p�X
 * @return		0:����, -1:���s
 */
int log_start(LOG_INFO* pstLog, LPCTSTR lpszPath)
{
	if (pstLog == NULL || lpszPath == NULL) {
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
	_save_filepath(pstLog, lpszPath);

	return 0;
}

/**
 * @fn			log_end
 * @brief		���O�o�͂��I������
 * @param[in]	LOG_INFO* pstLog		: ���O���
 * @return		0:����, -1:���s
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
 * @fn			log_write
 * @brief		���O�o��
 * @param[in]	LOG_INFO* pstLog		: ���O���
 * @param[in]	LOG_LEVEL enLevel		: ���O���x��
 * @param[in]	LPCTSTR lpszFmt			: ���O�o�͏���
 * @param[in]	...						: �o�͏����p�����[�^
 * @return		0:����, -1:���s
 */
int log_write(LOG_INFO* pstLog, LOG_LEVEL enLevel, LPCTSTR lpszFmt, ...)
{
	if (pstLog == NULL || lpszFmt == NULL || pstLog->bUsed == FALSE) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	TCHAR szBuff0[MAX_LOG_TEXT];
	TCHAR szBuff1[MAX_LOG_TEXT * 2];

	_log_lock(pstLog);
	_backup_file(pstLog);

	va_list arg;
	va_start(arg, lpszFmt);
	_vsntprintf(szBuff0, sizeof(szBuff0), lpszFmt, arg);
	va_end(arg);

	errno = 0;
	FILE *fp = _tfopen(pstLog->szLogPath, _T("ab+"));
	if (fp == NULL) {
		if (errno != 0) perror(NULL);
		return -1;
	}

	SYSTEMTIME stTime;
	GetLocalTime((LPSYSTEMTIME)&stTime);
	_sntprintf(szBuff1, sizeof(szBuff1),
		_T("%04d/%02d/%02d, %02d:%02d:%02d.%03d, %s, " \
			"%s\r\n")
		, stTime.wYear
		, stTime.wMonth
		, stTime.wDay
		, stTime.wHour
		, stTime.wMinute
		, stTime.wSecond
		, stTime.wMilliseconds
		, _log_level(enLevel)
		, szBuff0);
	_fputts(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		_log_unlock(pstLog);
		return -1;
	}
	_log_unlock(pstLog);
	return 0;
}

/**
 * @fn			log_debug
 * @brief		���O�o��
 * @param[in]	LOG_INFO* pstLog		: ���O���
 * @param[in]	LOG_LEVEL enLevel		: ���O���x��
 * @param[in]	LPCTSTR lpszFile		: �o�͎��̃t�@�C����
 * @param[in]	int nLine				: �o�͎��̍s��
 * @param[in]	LPCTSTR lpszFunc		: �o�͎��̊֐���
 * @param[in]	LPCTSTR lpszFmt			: ���O�o�͏���
 * @param[in]	...						: �o�͏����p�����[�^
 * @return		0:����, -1:���s
 */
int log_debug(LOG_INFO* pstLog, LOG_LEVEL enLevel, LPCTSTR lpszFile, int nLine, LPCTSTR lpszFunc, LPCTSTR lpszFmt, ...)
{
	if (pstLog == NULL || lpszFmt == NULL || lpszFile == NULL || lpszFunc == NULL || pstLog->bUsed == FALSE) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	_log_lock(pstLog);
	_backup_file(pstLog);

	TCHAR szBuff0[MAX_LOG_TEXT];
	TCHAR szBuff1[MAX_LOG_TEXT * 2];
	TCHAR szFilename[MAX_PATH];
	memset(szFilename, 0, sizeof(szFilename));

	va_list arg;
	va_start(arg, lpszFmt);
	_vsntprintf(szBuff0, sizeof(szBuff0), lpszFmt, arg);
	va_end(arg);

	errno = 0;
	FILE *fp = _tfopen(pstLog->szLogPath, _T("ab+"));
	if (fp == NULL) {
		if (errno != 0) perror(NULL);
		return -1;
	}

	SYSTEMTIME stTime;
	GetLocalTime((LPSYSTEMTIME)&stTime);
	_sntprintf(szBuff1, sizeof(szBuff1),
		_T("%04d/%02d/%02d, %02d:%02d:%02d.%03d, %s, " \
			"%s(%d), %s, " \
			"%s\r\n")
		, stTime.wYear
		, stTime.wMonth
		, stTime.wDay
		, stTime.wHour
		, stTime.wMinute
		, stTime.wSecond
		, stTime.wMilliseconds
		, _log_level(enLevel)
		, _get_fname_from_path(lpszFile, szFilename, sizeof(szFilename))
		, nLine
		, lpszFunc
		, szBuff0);
	_fputts(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		_log_unlock(pstLog);
		return -1;
	}
	_log_unlock(pstLog);
	return 0;
}

/**
 * @fn			_log_level
 * @brief		���O���x���ɑΉ����閼�̂��擾
 * @param[in]	LOG_LEVEL enLevel	: ���O���x��
 * @return		���O���x������
 */
static LPCTSTR _log_level(LOG_LEVEL enLevel)
{
	switch (enLevel) {
	case ERR:
		return _T("ERR");
	case WAR:
		return _T("WAR");
	case INF:
		return _T("INF");
	case DBG:
		return _T("DBG");
	default:
		return _T("???");
	}
}

/**
 * @fn				_get_fname_from_path
 * @brief			�t���p�X���t�@�C�����݂̂��擾
 * @param[in]		LPCTSTR lpszPath	: �t�@�C���p�X������
 * @param[in,out]	LPTSTR lpszBuff		: �t�@�C�������i�[����o�b�t�@�̈�
 * @param[in]		int nSize			: �o�b�t�@�̈�̃T�C�Y
 * @return			�o�b�t�@�̈�ւ̃|�C���^(�������s����"(NULL)"�̕������Ԃ�
 */
static LPCTSTR _get_fname_from_path(LPCTSTR lpszPath, LPTSTR lpszBuff, int nSize)
{
	if (lpszPath == NULL || lpszBuff == NULL) {
		return _T("(NULL)");
	}

	TCHAR szFname[MAX_PATH + 1];
	TCHAR szExt[MAX_PATH + 1];

	_tsplitpath(lpszPath, NULL, NULL, szFname, szExt);
	_sntprintf(lpszBuff, nSize, _T("%s%s"), szFname, szExt);

	return lpszBuff;
}

/**
 * @fn			_log_lock_init
 * @brief		�r������������
 * @param[in]	LOG_INFO* pstLog		: ���O���
 */
static void _log_lock_init(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	InitializeCriticalSection(&(pstLog->stCS));
}

/**
 * @fn			_log_lock_delete
 * @brief		�r�������I��
 * @param[in]	LOG_INFO* pstLog		: ���O���
 */
static void _log_lock_delete(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	DeleteCriticalSection(&(pstLog->stCS));
}

/**
 * @fn			_log_lock
 * @brief		�r�����b�N�J�n
 * @param[in]	LOG_INFO* pstLog		: ���O���
 */
static void _log_lock(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	EnterCriticalSection(&(pstLog->stCS));
}

/**
 * @fn			_log_unlock
 * @brief		�r�����b�N����
 * @param[in]	LOG_INFO* pstLog		: ���O���
 */
static void _log_unlock(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	LeaveCriticalSection(&(pstLog->stCS));
}

/**
 * @fn			_save_filepath
 * @brief		���O�t�@�C���p�X����Ɨp�ϐ��ɃR�s�[
 * @param[in]	LOG_INFO* pstLog		: ���O���
 * @param[in]	LPCTSTR lpszPath		: ���O�o�͐�t�@�C���p�X
 * @return		0:����, -1:���s
 */
static int _save_filepath(LOG_INFO* pstLog, LPCTSTR lpszPath)
{
	if (pstLog == NULL || lpszPath == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	TCHAR tmpDrive[MAX_PATH + 1];
	TCHAR tmpDir[MAX_PATH + 1];
	TCHAR tmpFname[MAX_PATH + 1];
	TCHAR tmpFext[MAX_PATH + 1];

	_tsplitpath(lpszPath, tmpDrive, tmpDir, tmpFname, tmpFext);

	memset(pstLog->szDir, 0, sizeof(pstLog->szDir));
	memset(pstLog->szFname, 0, sizeof(pstLog->szFname));
	memset(pstLog->szFext, 0, sizeof(pstLog->szFext));
	memset(pstLog->szLogPath, 0, sizeof(pstLog->szLogPath));

	_sntprintf(pstLog->szDir, sizeof(pstLog->szDir), _T("%s%s"), tmpDrive, tmpDir);
	_sntprintf(pstLog->szFname, sizeof(pstLog->szFname), _T("%s"), tmpFname);
	_sntprintf(pstLog->szFext, sizeof(pstLog->szFext), _T("%s"), tmpFext);

	// �t�@�C����_0.�g���q
	int ret = _get_backupname(pstLog, 0, pstLog->szLogPath, sizeof(pstLog->szLogPath));

	return ret;
}

/**
 * @fn			_get_backupname
 * @brief		�w�胍�O�t�@�C�����Ƀo�b�N�A�b�v�ԍ���t�^����
 * @param[in]	LOG_INFO* pstLog		: ���O���
 * @param[in]	int nBkNo				: �o�b�N�A�b�v�ԍ�
 * @param[in,out]	LPTSTR lpszBuff			: �o�͐�o�b�t�@�̈�
 * @param[in]	int nSize				: �o�b�t�@�̈�̃T�C�Y
 * @return		0:����, -1:���s
 */
static int _get_backupname(LOG_INFO* pstLog, int nBkNo, LPTSTR lpszBuff, int nSize)
{
	if (pstLog == NULL || lpszBuff == NULL || nBkNo < 0 || pstLog->nLogBackup < nBkNo) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}
	_sntprintf(lpszBuff, nSize, _T("%s%s_%d%s"), pstLog->szDir, pstLog->szFname, nBkNo, pstLog->szFext);
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

	long fsize = _get_filesize(pstLog->szLogPath);
	if (fsize < 0) {
		return 0;
	}

	TCHAR szBefore[MAX_PATH + 1];
	TCHAR szAfter[MAX_PATH + 1];

	if (pstLog->nFileSize <= fsize) {
		// �t�@�C���o�b�N�A�b�v����
		// �ł��Â��t�@�C���͍폜
		_get_backupname(pstLog, pstLog->nLogBackup, szBefore, sizeof(szBefore));
		if (PathFileExists(szBefore) && !PathIsDirectory(szBefore)) {
			DeleteFile(szBefore);
		}
		// �t�@�C�������l�[���A1���Â��炷
		for (int nBkNo = pstLog->nLogBackup - 1; 0 <= nBkNo; nBkNo--) {
			_get_backupname(pstLog, nBkNo, szBefore, sizeof(szBefore));
			if (PathFileExists(szBefore) && !PathIsDirectory(szBefore)) {
				_get_backupname(pstLog, nBkNo + 1, szAfter, sizeof(szBefore));
				MoveFile(szBefore, szAfter);
			}
		}
	}

	return 0;
}

/**
 * @fn			_get_filesize
 * @brief		�t�@�C���T�C�Y�擾(2GB�܂�)
 * @param[in]	LPCTSTR lpszPath		: �t�@�C���p�X
 * @return		�t�@�C���T�C�Y(-1:���s)
 */
static long _get_filesize(LPCTSTR lpszPath)
{
	if (lpszPath == NULL) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	//errno = 0;
	FILE *fp = _tfopen(lpszPath, _T("rb"));
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

// ���[�j���OC4996�}�~�I��
#pragma warning(pop)
