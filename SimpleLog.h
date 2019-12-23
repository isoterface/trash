/**
 * @file	SimpleLog.h
 * @brief	ログ出力
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
#define MAX_FILE_SIZE						(1024)		// 1kByte単位
#define MAX_LOG_BACKUP						(3)

#define LOG_START(id, path)					CLog::Start(id, path)
#define LOG_END(id)							CLog::End(id)		// LOG_END()でも可
#define LOG_WRITE(id, level, fmt, ...)		CLog::Write(id, level, fmt, __VA_ARGS__)
#define LOG_DEBUG(id, level, fmt, ...)		CLog::Debug(id, level, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)


 /**
  * @enum	_LOG_LEVEL
  * @brief	ログレベル定義
  */
typedef enum _LOG_LEVEL {
	ERR = 1,
	WAR = 2,
	INF = 3,
	DBG = 4
} LOG_LEVEL;


/**
 * @class	CLog
 * @brief	ログ出力クラス
 */
class CLog
{
private:
	// スタティックメンバ変数（宣言のみ、実体の確保はcpp側で行う）
	static CRITICAL_SECTION	m_stCS[MAX_ID];						//! ログIDごとの排他オブジェクト
	static char				m_szLogPath[MAX_ID][MAX_PATH + 1];	//! ログファイルパス
	static BOOL				m_bUsed[MAX_ID];					//! ログID使用状態
	// ログファイル情報
	static char				m_szDir[MAX_ID][MAX_PATH + 1];		//! ドライブ、ディレクトリ名
	static char				m_szFname[MAX_ID][MAX_PATH + 1];	//! ファイル名（拡張子除去）
	static char				m_szFext[MAX_ID][MAX_PATH + 1];		//! 拡張子名
	static int				m_nFileSize[MAX_ID];				//! ログファイルサイズ
	static int				m_nLogBackup[MAX_ID];				//! ファイルバックアップ数

public:
	CLog();
	//~CLog();

	static int				Start(int nID, const char* szPath);
	static int				End(int nID);
	static int				End();
	static int				Write(int nID, int nLevel, const char* szFmt, ...);
	static int				Debug(int nID, int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...);

private:
	// 以下のオーバーロードはマクロから呼び出した際に識別ができない
	static int				Start(const char* szPath);
	static int				Write(int nLevel, const char* szFmt, ...);
	static int				Debug(int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...);

private:
	static int				write(int nID, int nLevel, const char* szFmt, va_list arg);
	static int				debug(int nID, int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, va_list arg);
	static const char*		logLevel(int nLevel);
	static int				getId(int nID);
	static const char*		getFnameFromPath(const char* szPath, char* szBuff, int nSize);

	static void				lockInit(int nID);
	static void				lockDelete(int nID);
	static void				lock(int nID);
	static void				unlock(int nID);

	static int				copyFilePath(int nID, const char* szPath);
	static int				getBackupName(int nID, int nBkNo, char* szBuff, int nSize);
	static int				backupFile(int nID);
	static long				getFileSize(const char* szPath);
};


//! スタティックメンバ変数の実体の確保、初期化
//! ログIDごとの排他オブジェクト
CRITICAL_SECTION CLog::m_stCS[MAX_ID];
//! ログファイルパス
char CLog::m_szLogPath[MAX_ID][MAX_PATH + 1];
//! ログID使用状態
int CLog::m_bUsed[MAX_ID] = {
	FALSE, FALSE, FALSE, FALSE, FALSE
};

//! ドライブ、ディレクトリ名
char CLog::m_szDir[MAX_ID][MAX_PATH + 1];
//! ファイル名（拡張子除去）
char CLog::m_szFname[MAX_ID][MAX_PATH + 1];
//! 拡張子名
char CLog::m_szFext[MAX_ID][MAX_PATH + 1];
//! ログファイルサイズ
int CLog::m_nFileSize[MAX_ID] = {
	MAX_FILE_SIZE, MAX_FILE_SIZE, MAX_FILE_SIZE, MAX_FILE_SIZE, MAX_FILE_SIZE
};
//! ファイルバックアップ数
int CLog::m_nLogBackup[MAX_ID] = {
	MAX_LOG_BACKUP, MAX_LOG_BACKUP, MAX_LOG_BACKUP, MAX_LOG_BACKUP, MAX_LOG_BACKUP
};


/**
 * コンストラクタ
 */
CLog::CLog()
{
	// インスタンスは生成しない
	assert(FALSE);
}

//CLog::~CLog()
//{
//}

/**
 * @fn		Start
 * @brief	指定ログファイルへのログ出力を開始する
 * @param	[in]	int nID				: ログ出力時の指定ログID((0)～(MAX_ID-1), -1でIDを自動指定)
 * @param	[in]	const char* szPath	: ログ出力先ファイルパス
 * @return	(0)～(MAX_ID-1):成功(以後のログ出力は取得したIDを使用する), -1:失敗
 * @remarks	※排他処理はログIDごとのため、異なるログIDで同じログファイルを指定した場合の動作は保証されない
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

	// ログファイルにバックアップ番号を付与
	copyFilePath(nID, szPath);

	//memset(m_szLogPath[id], 0, sizeof(m_szLogPath[id]));
	//strncpy(m_szLogPath[id], szPath, sizeof(m_szLogPath[id]));

	//::InitializeCriticalSection(&(m_stCS[id]));
	lockInit(id);

	m_bUsed[id] = TRUE;

	return id;
}

/**
 * @fn		Start
 * @brief	指定ログファイルへのログ出力を開始する(IDは自動指定)
 * @param	[in]	const char* szPath	: ログ出力先ファイルパス
 * @return	(0)～(MAX_ID-1):成功(以後のログ出力は取得したIDを使用する), -1:失敗
 */
int CLog::Start(const char* szPath)
{
	if (szPath == NULL) {
		return -1;
	}
	return CLog::Start(-1, szPath);		// 初回呼び出しであれば自動的に ID=0 が割り当てられる
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

	// 指定IDのみ開放
	if (m_bUsed[nID] == TRUE) {
		//::DeleteCriticalSection(&(m_stCS[nID]));
		lockDelete(nID);
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
	// 全ID開放
	for (int i = 0; i < MAX_ID; i++) {
		if (m_bUsed[i] == TRUE) {
			//::DeleteCriticalSection(&(m_stCS[i]));
			lockDelete(i);
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

	// TODO: 呼び元でロックする？
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

	// TODO: 呼び元でロックする？
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = write(0, nLevel, szFmt, arg);
	va_end(arg);
	return ret;
}

/**
 * @fn		Debug
 * @brief
 * @param	[in]
 * @return
 */
int CLog::Debug(int nID, int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...)
{
	if (szFmt == NULL || szFile == NULL || szFunc == NULL) {
		return -1;
	}

	// TODO: 呼び元でロックする？
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = debug(nID, nLevel, szFile, nLine, szFunc, szFmt, arg);
	va_end(arg);
	return ret;
}

/**
 * @fn		Debug
 * @brief
 * @param	[in]
 * @return
 */
int CLog::Debug(int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...)
{
	if (szFmt == NULL || szFile == NULL || szFunc == NULL) {
		return -1;
	}

	// TODO: 呼び元でロックする？
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = debug(0, nLevel, szFile, nLine, szFunc, szFmt, arg);
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
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	//::EnterCriticalSection(&(m_stCS[nID]));
	lock(nID);

	backupFile(nID);

	char szBuff0[MAX_LOG_TEXT];
	char szBuff1[MAX_LOG_TEXT * 2];
	vsnprintf(szBuff0, sizeof(szBuff0), szFmt, arg);

	errno = 0;
	FILE *fp = fopen(m_szLogPath[nID], "ab+");
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
		, logLevel(nLevel)
		, szBuff0);
	fputs(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		//::LeaveCriticalSection(&(m_stCS[nID]));
		unlock(nID);
		return -1;
	}
	//::LeaveCriticalSection(&(m_stCS[nID]));
	unlock(nID);
	return 0;
}

/**
 * @fn		debug
 * @brief
 * @param	[in]
 * @return
 */
int CLog::debug(int nID, int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, va_list arg)
{
	if (szFmt == NULL || szFile == NULL || szFunc == NULL) {
		return -1;
	}

	if (nID < 0 || MAX_ID <= nID || m_bUsed[nID] == FALSE) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	//::EnterCriticalSection(&(m_stCS[nID]));
	lock(nID);

	backupFile(nID);

	char szFilename[MAX_PATH];
	memset(szFilename, 0, sizeof(szFilename));

	char szBuff0[MAX_LOG_TEXT];
	char szBuff1[MAX_LOG_TEXT * 2];
	vsnprintf(szBuff0, sizeof(szBuff0), szFmt, arg);

	errno = 0;
	FILE *fp = fopen(m_szLogPath[nID], "ab+");
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
		, logLevel(nLevel)
		, getFnameFromPath(szFile, szFilename, sizeof(szFilename))
		, nLine
		, szFunc
		, szBuff0);
	fputs(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		//::LeaveCriticalSection(&(m_stCS[nID]));
		unlock(nID);
		return -1;
	}
	//::LeaveCriticalSection(&(m_stCS[nID]));
	unlock(nID);
	return 0;
}

/**
 * @fn		logLevel
 * @brief	ログレベルに対応する名称を取得
 * @param	[in]	int nLevel	: ログレベル(LOG_LEVEL)
 * @return	ログレベル名称
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
 * @fn		getId
 * @brief	使用可能なログIDを取得する
 * @param	[in]	int nID		: 使用するログID (-1:で空いているIDを取得)
 * @return	-1:ログID取得失敗(空き無し), 0～:使用可能なログID
 */
int CLog::getId(int nID)
{
	if (nID < -1 || MAX_ID <= nID) {
		return -1;
	}

	int id = -1;

	if (0 <= nID) {
		// 指定IDは使用可能か
		if (m_bUsed[nID] == FALSE) {
			id = nID;
		}
	}
	else {
		// -1:空いているIDから検索
		for (int i = 0; i < MAX_ID; i++) {
			if (m_bUsed[i] == FALSE) {
				id = i;
				break;
			}
		}
	}
	return id;
}

/**
 * @fn		getFilename
 * @brief	フルパスよりファイル名のみを取得
 * @param	[IN]	char* szPath	: ファイルパス文字列
 * @param	[OUT]	char* szBuff	: ファイル名を格納するバッファ領域
 * @param	[IN]	int nSize		: バッファ領域のサイズ
 * @return	バッファ領域へのポインタ(処理失敗時は"(NULL)"の文字が返る
 */
const char* CLog::getFnameFromPath(const char* szPath, char* szBuff, int nSize)
{
	if (szPath == NULL || szBuff == NULL) {
		return "(NULL)";
	}

	//char szDrv[MAX_PATH + 1];
	//char szDir[MAX_PATH + 1];
	char szFname[MAX_PATH + 1];
	char szExt[MAX_PATH + 1];

	//_splitpath(szPath, szDrv, szDir, szFname, szExt);
	_splitpath(szPath, NULL, NULL, szFname, szExt);
	snprintf(szBuff, nSize, "%s%s", szFname, szExt);

	return szBuff;
}

/**
 * @fn		_LockInit
 * @brief	排他処理初期化
 * @param	[in]	int nID		 : ログID
 */
inline void CLog::lockInit(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::InitializeCriticalSection(&(m_stCS[nID]));
	}
}

/**
 * @fn		_LockDelete
 * @brief	排他処理初期化
 * @param	[in]	int nID		 : ログID
 */
inline void CLog::lockDelete(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::DeleteCriticalSection(&(m_stCS[nID]));
	}
}

/**
 * @fn		_Lock
 * @brief	排他処理初期化
 * @param	[in]	int nID		 : ログID
 */
inline void CLog::lock(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::EnterCriticalSection(&(m_stCS[nID]));
	}
}

/**
 * @fn		_Unlock
 * @brief	排他処理初期化
 * @param	[in]	int nID		 : ログID
 */
inline void CLog::unlock(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::LeaveCriticalSection(&(m_stCS[nID]));
	}
}

/**
 * @fn		copyFilePath
 * @brief	指定ログファイル名にバックアップ番号を付与する
 * @param	[in]	int nID				: ログID
 * @param	[in]	const char* szPath	: ログ出力先ファイルパス
 */
int CLog::copyFilePath(int nID, const char* szPath)
{
	if (szPath == NULL || nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	char tmpDrive[MAX_PATH + 1];
	char tmpDir[MAX_PATH + 1];
	char tmpFname[MAX_PATH + 1];
	char tmpFext[MAX_PATH + 1];

	_splitpath(szPath, tmpDrive, tmpDir, tmpFname, tmpFext);

	memset(m_szDir[nID], 0, sizeof(m_szDir[nID]));
	memset(m_szFname[nID], 0, sizeof(m_szFname[nID]));
	memset(m_szFext[nID], 0, sizeof(m_szFext[nID]));
	memset(m_szLogPath[nID], 0, sizeof(m_szLogPath[nID]));

	snprintf(m_szDir[nID], sizeof(m_szDir[nID]), "%s%s", tmpDrive, tmpDir);
	snprintf(m_szFname[nID], sizeof(m_szFname[nID]), "%s", tmpFname);
	snprintf(m_szFext[nID], sizeof(m_szFext[nID]), "%s", tmpFext);

	// ファイル名_0.拡張子
	//snprintf(m_szLogPath[nID], sizeof(m_szLogPath[nID])
	//	, "%s%s_%d%s"
	//	, m_szDir[nID], m_szFname[nID], 0, m_szFext[nID]);
	int ret = getBackupName(nID, 0, m_szLogPath[nID], sizeof(m_szLogPath[nID]));

	return ret;
}

/**
 * @fn		createBackupName
 * @brief	指定ログファイル名にバックアップ番号を付与する
 * @param	[in]	int nID			: ログID
 * @param	[in]	int nBkNo		: バックアップ番号
 * @param	[in]	char* szBuff	: 出力先バッファ領域
 * @param	[in]	int nSize		: バッファ領域のサイズ
 * @return	0:成功, -1:失敗
 */
int CLog::getBackupName(int nID, int nBkNo, char* szBuff, int nSize)
{
	if (szBuff == NULL || nID < 0 || MAX_ID <= nID || nBkNo < 0 || m_nLogBackup[nID] < nBkNo)
	{
		return -1;
	}
	snprintf(szBuff, nSize, "%s%s_%d%s", m_szDir[nID], m_szFname[nID], nBkNo, m_szFext[nID]);
	return 0;
}

/**
 * @fn		backupFile
 * @brief	ログファイルバックアップ処理
 * @param	[in]	int nID		:ログID
 * @return	0:成功, -1:失敗
 */
int CLog::backupFile(int nID)
{
	if (nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	//lock(nID);

	long fsize = 0;
	if ((fsize = getFileSize(m_szLogPath[nID])) < 0) {
		//unlock(nID);
		return 0;
	}

	char szBefore[MAX_PATH + 1];
	char szAfter[MAX_PATH + 1];

	if (m_nFileSize[nID] <= fsize) {
		// ファイルバックアップ処理
		// 最も古いファイルは削除
		getBackupName(nID, m_nLogBackup[nID], szBefore, sizeof(szBefore));
		if (::PathFileExists(szBefore) && !::PathIsDirectory(szBefore)) {
			::DeleteFile(szBefore);
		}
		// ファイルをリネーム、1個ずづずらす
		for (int nBkNo = m_nLogBackup[nID] - 1; 0 <= nBkNo; nBkNo--) {
			getBackupName(nID, nBkNo, szBefore, sizeof(szBefore));
			if (::PathFileExists(szBefore) && !::PathIsDirectory(szBefore)) {
				getBackupName(nID, nBkNo + 1, szAfter, sizeof(szBefore));
				::MoveFile(szBefore, szAfter);
			}
		}
	}

	//unlock(nID);
	return 0;
}

/**
 * @fn		getFileSize
 * @brief	ファイルサイズ取得(2GBまで)
 * @param	[in]	char* szPath		: ファイルパス
 * @return	ファイルサイズ(-1:失敗)
 */
long CLog::getFileSize(const char* szPath)
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


//LOG_START(0, "log_test_0.log");
//LOG_START(1, "log_test_1.log");
//
//LOG_WRITE(0, INF, "start %d.", 0);
//LOG_WRITE(1, INF, "start %d.", 1);
//LOG_DEBUG(0, INF, "end %d.", 0);
//LOG_DEBUG(1, INF, "end 1.");
//
//LOG_END(1);
//LOG_END();
