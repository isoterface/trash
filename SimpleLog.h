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


static CRITICAL_SECTION	g_cs[MAX_ID];		//! ���OID���Ƃ̔r���I�u�W�F�N�g
static char				g_szLogPath[MAX_ID][MAX_PATH];		//! ���O�t�@�C���p�X
static int				g_nUseStatus[MAX_ID] = {			//! ���OID�g�p���
	NOT_USED, NOT_USED, NOT_USED, NOT_USED, NOT_USED,
	NOT_USED, NOT_USED, NOT_USED, NOT_USED, NOT_USED };


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


class CLog
{
//private:
	//static CRITICAL_SECTION	g_cs[MAX_ID];		//! ���OID���Ƃ̔r���I�u�W�F�N�g
	//static char				g_szLogPath[MAX_ID][MAX_PATH];		//! ���O�t�@�C���p�X
	//static int				g_nUseStatus[MAX_ID] = {			//! ���OID�g�p���
	//	NOT_USED, NOT_USED, NOT_USED, NOT_USED, NOT_USED,
	//	NOT_USED, NOT_USED, NOT_USED, NOT_USED, NOT_USED };

public:
	CLog();
	~CLog();

	static int Start(int nID, const char* szPath);
	static int Start(const char* szPath);
	static int End(int nID);
	static int End();
	static int Write(int nID, int nLevel, const char* szFmt, ...);
	static int Write(int nLevel, const char* szFmt, ...);

private:
	static int write(int nID, int nLevel, const char* szFmt, va_list arg);
	static const char* log_level(int nLevel);
	static int get_id(int nID);
};


CLog::CLog()
{
}


CLog::~CLog()
{
}

int CLog::Start(int nID, const char* szPath)
{
	int id = get_id(nID);
	if (id < 0) {
		return -1;
	}

	// TODO:�p�X�`�F�b�N

	memset(g_szLogPath[id], 0, sizeof(g_szLogPath[id]));
	strncpy(g_szLogPath[id], szPath, sizeof(g_szLogPath[id]));
	InitializeCriticalSection(&(g_cs[id]));
	g_nUseStatus[id] = USED;

	return id;
}

int CLog::Start(const char* szPath)
{
	return CLog::Start(0, szPath);
}

int CLog::End(int nID)
{
	if (nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	// �w��ID�̂݊J��
	if (g_nUseStatus[nID] == USED) {
		DeleteCriticalSection(&(g_cs[nID]));
		g_nUseStatus[nID] = NOT_USED;
	}
	return 0;
}

int CLog::End()
{
	// �SID�J��
	for (int i = 0; i < MAX_ID; i++) {
		if (g_nUseStatus[i] == USED) {
			DeleteCriticalSection(&(g_cs[i]));
			g_nUseStatus[i] = NOT_USED;
		}
	}
	return 0;
}


int CLog::Write(int nID, int nLevel, const char* szFmt, ...)
{
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = write(nID, nLevel, szFmt, arg);
	va_end(arg);
	return ret;
}

int CLog::Write(int nLevel, const char* szFmt, ...)
{
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = write(0, nLevel, szFmt, arg);
	va_end(arg);
	return ret;
}


int CLog::write(int nID, int nLevel, const char* szFmt, va_list arg)
{
	if (nID < 0 || MAX_ID <= nID || g_nUseStatus[nID] != USED) {
		return -1;
	}

	char szBuff0[LOG_MAX];
	char szBuff1[LOG_MAX * 2];

	vsnprintf(szBuff0, sizeof(szBuff0), szFmt, arg);

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
 * @fn		log_level
 * @brief
 * @param	[in]	int nLevel	:
 * @return
 */
const char* CLog::log_level(int nLevel)
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
int CLog::get_id(int nID)
{
	if (nID < -1 || MAX_ID <= nID) {
		return -1;
	}

	int id = -1;

	if (0 <= nID) {
		// �w��ID�͎g�p�\��
		if (g_nUseStatus[nID] == NOT_USED) {
			id = nID;
		}
	}
	else {
		// -1:�󂢂Ă���ID���猟��
		for (int i = 0; i < MAX_ID; i++) {
			if (g_nUseStatus[i] == NOT_USED) {
				id = i;
				break;
			}
		}
	}
	return id;
}
