/**
 * @file	log.h
 * @brief	ログ出力
 * @author	?
 * @date	?
 */
#pragma once

// ワーニングC4996抑止開始
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


#define MAX_LOG_TEXT		(256)		// ログ出力当たりの最大テキスト長
#define MAX_FILE_SIZE		(1024)		// ログファイル最大サイズ[kByte]
#define MAX_LOG_BACKUP		(3)			// ログバックアップ保存数

#define LOG_START(pInf, path)				log_start(pInf, path)
#define LOG_END(pInf)						log_end(pInf)
#define LOG_WRITE(pInf, level, fmt, ...)	log_write(pInf, level, fmt, __VA_ARGS__)
#define LOG_DEBUG(pInf, level, fmt, ...)	log_debug(pInf, level, _T(__FILE__), __LINE__, _T(__FUNCTION__), fmt, __VA_ARGS__)


 /**
  * @enum	_LOG_LEVEL
  * @brief	ログレベル定義
  */
typedef enum {
	ERR = 1,
	WAR = 2,
	INF = 3,
	DBG = 4
} LOG_LEVEL;


/*
 * @struct	_LOG_INFO
 * @brief	ログ情報
 */
typedef struct _LOG_INFO {
	CRITICAL_SECTION	stCS;						//! ログIDごとの排他オブジェクト
	TCHAR				szLogPath[MAX_PATH + 1];	//! ログファイルパス
	BOOL				bUsed;						//! ログID使用状態
	// ログバックアップ情報
	int					nFileSize;					//! ログファイルサイズ
	int					nLogBackup;					//! ファイルバックアップ数
	TCHAR				szDir[MAX_PATH + 1];		//! ドライブ、ディレクトリ名
	TCHAR				szFname[MAX_PATH + 1];		//! ファイル名（拡張子除去）
	TCHAR				szFext[MAX_PATH + 1];		//! 拡張子名
} LOG_INFO;


int					log_start(LOG_INFO* pstLog, LPCTSTR lpszPath);
int					log_end(LOG_INFO* pstLog);
int					log_write(LOG_INFO* pstLog, LOG_LEVEL enLevel, LPCTSTR lpszFmt, ...);
int					log_debug(LOG_INFO* pstLog, LOG_LEVEL enLevel, LPCTSTR lpszFile, int nLine, LPCTSTR lpszFunc, LPCTSTR lpszFmt, ...);
static LPCTSTR		_log_level(LOG_LEVEL enLevel);
static LPCTSTR		_get_fname_from_path(LPCTSTR lpszPath, LPTSTR lpszBuff, int nSize);
static void			_log_lock_init(LOG_INFO* pstLog);
static void			_log_lock_delete(LOG_INFO* pstLog);
static void			_log_lock(LOG_INFO* pstLog);
static void			_log_unlock(LOG_INFO* pstLog);
static int			_copy_filepath(LOG_INFO* pstLog, LPCTSTR lpszPath);
static int			_get_backupname(LOG_INFO* pstLog, int nBkNo, LPTSTR lpszBuff, int nSize);
static int			_backup_file(LOG_INFO* pstLog);
static long			_get_filesize(LPCTSTR lpszPath);


/**
 * @fn		log_start
 * @brief	ログ出力を開始する
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 * @param	[in]	LPCTSTR lpszPath		: ログファイルパス
 * @return	0:成功, -1:失敗
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

	// ログファイルにバックアップ番号を付与
	_copy_filepath(pstLog, lpszPath);

	return 0;
}

/**
 * @fn		log_end
 * @brief	ログ出力を終了する
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 * @return	0:成功, -1:失敗
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
 * @brief	ログ出力
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 * @param	[in]	LOG_LEVEL enLevel		: ログレベル
 * @param	[in]	LPCTSTR lpszFmt			: ログ出力書式
 * @param	[in]	...						: 出力書式パラメータ
 * @return	0:成功, -1:失敗
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
 * @fn		log_debug
 * @brief	ログ出力
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 * @param	[in]	LOG_LEVEL enLevel		: ログレベル
 * @param	[in]	LPCTSTR lpszFile		: 出力時のファイル名
 * @param	[in]	int nLine				: 出力時の行数
 * @param	[in]	LPCTSTR lpszFunc		: 出力時の関数名
 * @param	[in]	LPCTSTR lpszFmt			: ログ出力書式
 * @param	[in]	...						: 出力書式パラメータ
 * @return	0:成功, -1:失敗
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
 * @fn		_log_level
 * @brief	ログレベルに対応する名称を取得
 * @param	[in]	LOG_LEVEL enLevel	: ログレベル
 * @return	ログレベル名称
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
 * @fn		_get_fname_from_path
 * @brief	フルパスよりファイル名のみを取得
 * @param	[in]	LPCTSTR lpszPath	: ファイルパス文字列
 * @param	[out]	LPTSTR lpszBuff		: ファイル名を格納するバッファ領域
 * @param	[in]	int nSize			: バッファ領域のサイズ
 * @return	バッファ領域へのポインタ(処理失敗時は"(NULL)"の文字が返る
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
 * @fn		_log_lock_init
 * @brief	排他処理初期化
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 */
static void _log_lock_init(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	InitializeCriticalSection(&(pstLog->stCS));
}

/**
 * @fn		_log_lock_delete
 * @brief	排他処理初期化
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 */
static void _log_lock_delete(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	DeleteCriticalSection(&(pstLog->stCS));
}

/**
 * @fn		_log_lock
 * @brief	排他処理初期化
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 */
static void _log_lock(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	EnterCriticalSection(&(pstLog->stCS));
}

/**
 * @fn		_log_unlock
 * @brief	排他処理初期化
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 */
static void _log_unlock(LOG_INFO* pstLog)
{
	if (pstLog == NULL) { assert(FALSE); }
	LeaveCriticalSection(&(pstLog->stCS));
}

/**
 * @fn		_copy_filepath
 * @brief	ログファイルパスを作業用変数にコピー
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 * @param	[in]	LPCTSTR lpszPath		: ログ出力先ファイルパス
 * @return	0:成功, -1:失敗
 */
static int _copy_filepath(LOG_INFO* pstLog, LPCTSTR lpszPath)
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

	// ファイル名_0.拡張子
	int ret = _get_backupname(pstLog, 0, pstLog->szLogPath, sizeof(pstLog->szLogPath));

	return ret;
}

/**
 * @fn		_get_backupname
 * @brief	指定ログファイル名にバックアップ番号を付与する
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 * @param	[in]	int nBkNo				: バックアップ番号
 * @param	[out]	LPTSTR lpszBuff			: 出力先バッファ領域
 * @param	[in]	int nSize				: バッファ領域のサイズ
 * @return	0:成功, -1:失敗
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
 * @brief	ログファイルバックアップ処理
 * @param	[in]	LOG_INFO* pstLog		: ログ情報
 * @return	0:成功, -1:失敗
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
		// ファイルバックアップ処理
		// 最も古いファイルは削除
		_get_backupname(pstLog, pstLog->nLogBackup, szBefore, sizeof(szBefore));
		if (PathFileExists(szBefore) && !PathIsDirectory(szBefore)) {
			DeleteFile(szBefore);
		}
		// ファイルをリネーム、1個ずづずらす
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
 * @fn		_get_filesize
 * @brief	ファイルサイズ取得(2GBまで)
 * @param	[in]	LPCTSTR lpszPath		: ファイルパス
 * @return	ファイルサイズ(-1:失敗)
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

// ワーニングC4996抑止終了
#pragma warning(pop)
