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
#include <windows.h>


#define MAX_ID		(10)
#define LOG_MAX		(256)


 /**
  * @enum	_LOG_LEVEL
  * @brief	ログレベル定義
  */
typedef enum LOG_LEVEL {
	ERR = 1,
	WAR = 2,
	INF = 3,
	DBG = 4
}_LOG_LEVEL;


/**
 * @class	CLog
 * @brief	ログ出力クラス
 */
class CLog
{
private:
	//! スタティックメンバ変数（宣言のみ、実体の確保はcpp側で行う）
	static CRITICAL_SECTION	m_stCS[MAX_ID];						//! ログIDごとの排他オブジェクト
	static char				m_szLogPath[MAX_ID][MAX_PATH];		//! ログファイルパス
	static BOOL				m_bUsed[MAX_ID];					//! ログID使用状態

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


//! スタティックメンバ変数の実体の確保、初期化
//! ログIDごとの排他オブジェクト
CRITICAL_SECTION CLog::m_stCS[MAX_ID];
//! ログファイルパス
char CLog::m_szLogPath[MAX_ID][MAX_PATH];
//! ログID使用状態
int CLog::m_bUsed[MAX_ID] = {
	FALSE, FALSE, FALSE, FALSE, FALSE,
	FALSE, FALSE, FALSE, FALSE, FALSE
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

	// TODO:パスチェック

	memset(m_szLogPath[id], 0, sizeof(m_szLogPath[id]));
	strncpy(m_szLogPath[id], szPath, sizeof(m_szLogPath[id]));
	InitializeCriticalSection(&(m_stCS[id]));
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
	// 全ID開放
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
