/**
 * @file	log.h
 * @brief	ログ出力
 * @author	?
 * @date	?
 */
#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <windows.h>


#define MAX_ID		(10)
#define LOG_MAX		(256)
#define NOT_USED	(0)
#define USED		(1)

//#define LOG_START(path)						log_start(path)
//#define LOG_END(id)							log_end(id)
//#define LOG_WRITE(id, level, fmt, ...)		log_write(id, level, fmt, __VA_ARGS__)


static CRITICAL_SECTION	g_cs[MAX_ID];		//! ログIDごとの排他オブジェクト
static char				g_szLogPath[MAX_ID][MAX_PATH];		//! ログファイルパス
static int				g_nUseStatus[MAX_ID] = {			//! ログID使用状態
	NOT_USED, NOT_USED, NOT_USED, NOT_USED, NOT_USED,
	NOT_USED, NOT_USED, NOT_USED, NOT_USED, NOT_USED };


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
 * @fn		log_level
 * @brief	
 * @param	[in]	int nLevel	:
 * @return	
 */
static const char* log_level(int nLevel)
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
static int get_id(int nID)
{
	if (nID < -1 || MAX_ID <= nID) {
		return -1;
	}

	int id = -1;

	if (0 <= nID) {
		// 指定IDは使用可能か
		if (g_nUseStatus[nID] == NOT_USED) {
			id = nID;
		}
	}
	else {
		// -1:空いているIDから検索
		for (int i = 0; i < MAX_ID; i++) {
			if (g_nUseStatus[i] == NOT_USED) {
				id = i;
				break;
			}
		}
	}
	return id;
}

/**
 * @fn		log_start
 * @brief	
 * @param	[in]	int nID			:
 * @param	[in]	char* szPath	:
 * @return　
 */
int log_start(int nID, const char* szPath)
{
	int id = get_id(nID);
	if (id < 0) {
		return -1;
	}

	// TODO:パスチェック

	memset(g_szLogPath[id], 0, sizeof(g_szLogPath[id]));
	strncpy(g_szLogPath[id], szPath, sizeof(g_szLogPath[id]));
	InitializeCriticalSection(&(g_cs[id]));
	g_nUseStatus[id] = USED;

	return id;
}

/**
 * @fn		_log_start
 * @brief
 * @param	[in]	char* szPath	:
 * @return　
 */
int _log_start(const char* szPath)
{
	return log_start(0, szPath);
}

/**
 * @fn		log_end
 * @brief	
 * @param	[in]	int nID		:
 * @return　
 */
int log_end(int nID)
{
	if (nID < -1 || MAX_ID <= nID) {
		return -1;
	}

	if (0 <= nID) {
		// 指定IDのみ開放
		if (g_nUseStatus[nID] == USED) {
			DeleteCriticalSection(&(g_cs[nID]));
			g_nUseStatus[nID] = NOT_USED;
		}
	}
	else {
		// 全ID開放
		for (int i = 0; i < MAX_ID; i++) {
			if (g_nUseStatus[i] == USED) {
				DeleteCriticalSection(&(g_cs[i]));
				g_nUseStatus[i] = NOT_USED;
			}
		}
	}
	return 0;
}

/**
 * @fn		_log_end
 * @brief
 * @return　
 */
int _log_end()
{
	return log_end(0);
}

/**
 * @fn		_write
 * @brief	
 * @param	[in]	int nID			:
 * @param	[in]	int nLevel		:
 * @param	[in]	char* szFmt		:
 * @param	[in]	va_list arg		:
 * @return　
 */
//int log_write(int nID, int nLevel, char* szFmt, ...)
static int _write(int nID, int nLevel, const char* szFmt, va_list arg)
{
	if (nID < 0 || MAX_ID <= nID || g_nUseStatus[nID] != USED) {
		return -1;
	}

	char szBuff0[LOG_MAX];
	char szBuff1[LOG_MAX * 2];

	//va_list arg;
	//va_start(arg, szFmt);
	vsnprintf(szBuff0, sizeof(szBuff0), szFmt, arg);
	//va_end(arg);

	EnterCriticalSection(&(g_cs[nID]));
	errno = 0;
	FILE *fp = fopen(g_szLogPath[nID], "a+");
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
		, log_level(nLevel)
		, szBuff0);
	fputs(szBuff1, fp);

	if (fclose(fp) == 0) {
		if (errno != 0) perror(NULL);
		LeaveCriticalSection(&(g_cs[nID]));
		return -1;
	}
	LeaveCriticalSection(&(g_cs[nID]));
	return 0;
}

/**
 * @fn		log_write
 * @brief	
 * @param	[in]	int nID			:
 * @param	[in]	int nLevel		:
 * @param	[in]	char* szFmt		:
 * @param	[in]	...				:
 * @return　
 */
int log_write(int nID, int nLevel, const char* szFmt, ...)
{
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = _write(nID, nLevel, szFmt, arg);
	va_end(arg);
	return ret;
}

/**
 * @fn
 * @fn		_log_write
 * @brief
 * @param	[in]	int nLevel		:
 * @param	[in]	char* szFmt		:
 * @param	[in]	...				:
 * @return　
 */
int _log_write(int nLevel, const char* szFmt, ...)
{
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = _write(0, nLevel, szFmt, arg);
	va_end(arg);
	return ret;
}


//void test03()
//{
//	_log_start("D:\\Work\\VS2017\\Projects\\MFCApplication3\\Debug\\test_a.log");
//	_log_write(INF, "%d%s", 654321, "test1");
//	_log_write(WAR, "%s%d", "abcDEF", 357);
//	_log_write(ERR, "%g%d%d", 9999.888, 123, 456);
//	_log_end();
//
//	//int id1 = log_start(0, "D:\\Work\\VS2017\\Projects\\MFCApplication3\\Debug\\test1.log");
//	//int id2 = log_start(-1, "D:\\Work\\VS2017\\Projects\\MFCApplication3\\Debug\\test2.log");
//	//log_write(id1, INF, "%d", 123);
//	//log_write(id1, WAR, "%s", "ABC");
//	//log_write(id1, ERR, "%g", 4.56);
//	//log_write(id2, INF, "%d", 987);
//	//log_write(id2, WAR, "%s", "XYZ");
//	//log_write(id2, ERR, "%g", 65.4);
//	//log_end(id1);
//	//log_end(id2);
//	//log_end(-1);
//}
