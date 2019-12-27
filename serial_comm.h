#pragma once

#include <stdarg.h>
#include <Windows.h>
#include "misc.h"


#define RX_BUFF		(1024)		// 受信バッファサイズ
#define TX_BUFF		(1024)		// 送信バッファサイズ


typedef struct _SERIAL_INFO {
	HANDLE hComm;
	char szPort[16];
	int nBaud;
	int nDataBit;
	int nParity;
	int nStopBit;
} SERIAL_INFO;


HANDLE open_serial(const char* szPort, int nBaud, int nDataBit, int nParity, int nStopBit, const char* szLogName);
static void SetDCB(DCB* pDCB);
int close_serial(HANDLE hComm, const char* szLogName);
int clear_serial(HANDLE hComm, const char* szLogName);
int serial_send(HANDLE hComm, unsigned char* puchData, int nLength, const char* szLogName);
int serial_recv(HANDLE hComm, unsigned char* puchBuff, int nLength, int nTimeout, const char* szLogName);
int check_buff_count_rx(HANDLE hComm);
int check_buff_count_tx(HANDLE hComm);
static void _com_log_output(const char* szLogName, const char* szFunc, const char* szFmt, ...);


HANDLE open_serial(const char* szPort, int nBaud, int nDataBit, int nParity, int nStopBit, const char* szLogName)
{
	HANDLE hComm;

	// COMポートハンドル取得
	hComm = CreateFile(szPort
		, (GENERIC_READ | GENERIC_WRITE)
		, 0
		, NULL
		, OPEN_EXISTING
		, FILE_ATTRIBUTE_NORMAL
		, NULL);

	if (hComm == INVALID_HANDLE_VALUE) {
		_com_log_output(szLogName, __FUNCTION__, "CreateFile failed.");
		return NULL;
	}

	// 送受信バッファ初期化
	if (!SetupComm(hComm, RX_BUFF, TX_BUFF)) {
		_com_log_output(szLogName, __FUNCTION__, "SetupComm failed.");
		CloseHandle(hComm);
		return NULL;
	}

	// 送受信バッファのデータ消去
	if (!PurgeComm(hComm, (PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))) {
		_com_log_output(szLogName, __FUNCTION__, "PurgeComm failed.");
		CloseHandle(hComm);
		return NULL;
	}

	// 通信条件の設定
	DCB stDCB;
	if (!GetCommState(hComm, &stDCB)) {
		_com_log_output(szLogName, __FUNCTION__, "GetCommState failed.");
		CloseHandle(hComm);
		return NULL;
	}

	stDCB.BaudRate = nBaud;			// 9600;
	stDCB.ByteSize = nDataBit;		// 8;
	stDCB.fParity = nParity;		// NOPARITY;
	stDCB.StopBits = nStopBit;		// ONESTOPBIT;

	if (!SetCommState(hComm, &stDCB)) {
		_com_log_output(szLogName, __FUNCTION__, "SetCommState failed.");
		CloseHandle(hComm);
		return NULL;
	}

	// タイムアウト時間の設定
	COMMTIMEOUTS stCommTimeout;
	//if (!GetCommTimeouts(hComm, &stCommTO)) {
	//	CloseHandle(hComm);
	//	return NULL;
	//}

	// 受信インターバル時間
	// ReadIntervalTimeoutに設定する
	// ReadFile関数で1文字だけ受信する際には効果なし
	// ゼロに設置すると、受信インターバル時間は使われない
	stCommTimeout.ReadIntervalTimeout = 500;
	// 受信トータル時間
	// ReadTotalTimeoutMultiplier * (受信バイト数)+ReadTotalTimeoutConstant
	// ReadTotalTimeoutMultiplier と ReadTotalTimeoutConstant がゼロのとき、受信トータル時間は使われない
	stCommTimeout.ReadTotalTimeoutMultiplier = 0;
	stCommTimeout.ReadTotalTimeoutConstant = 500;
	// 送信トータル時間
	// WriteTotalTimeoutMultiplier * (送信バイト数)+WriteTotalTimeoutConstant
	// WriteTotalTimeoutMultiplier と WriteTotalTimeoutConstant がゼロのとき、送信トータル時間は使われない
	stCommTimeout.WriteTotalTimeoutMultiplier = 0;
	stCommTimeout.WriteTotalTimeoutConstant = 500;

	if (!SetCommTimeouts(hComm, &stCommTimeout)) {
		_com_log_output(szLogName, __FUNCTION__, "SetCommTimeouts failed.");
		CloseHandle(hComm);
		return NULL;
	}


	return hComm;
}

static void SetDCB(DCB* pDCB)
{
	pDCB->DCBlength = sizeof(DCB);
	pDCB->fBinary = TRUE;
	//pDCB->BaudRate = 9600;
	//pDCB->ByteSize = 8;
	//pDCB->fParity = NOPARITY;
	//pDCB->StopBits = ONESTOPBIT;
	// ハードウェアフロー制御
	pDCB->fOutxCtsFlow = FALSE;
	pDCB->fOutxDsrFlow = FALSE;
	pDCB->fDtrControl = DTR_CONTROL_DISABLE;
	pDCB->fRtsControl = RTS_CONTROL_DISABLE;
	// ソフトウェアフロー制御
	pDCB->fOutX = FALSE;
	pDCB->fInX = FALSE;
	pDCB->fTXContinueOnXoff = TRUE;
	pDCB->XonLim = 512;
	pDCB->XoffLim = 512;
	pDCB->XonChar = 0x11;
	pDCB->XoffChar = 0x13;
	// その他
	pDCB->fNull = TRUE;
	pDCB->fAbortOnError = TRUE;
	pDCB->fErrorChar = FALSE;
	pDCB->ErrorChar = 0x00;
	pDCB->EofChar = 0x03;
	pDCB->EvtChar = 0x02;
}

int close_serial(HANDLE hComm)
{
	if (!CloseHandle(hComm)) {
		// TODO:
		return -1;
	}
	return 0;
}

int clear_serial(HANDLE hComm)
{
	// 送受信バッファのデータ消去
	if (!PurgeComm(hComm, (PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))) {
		// TODO:
		return -1;
	}
	return 0;
}

int serial_send(HANDLE hComm, unsigned char* puchData, int nLength)
{
	int nWrite = 0;
	if (!WriteFile(hComm, puchData, nLength, (LPDWORD)&nWrite, NULL)) {
		// TODO:
	}
	return nWrite;
}

int serial_recv(HANDLE hComm, unsigned char* puchBuff, int nLength, int nTimeout)
{
	int nRead = 0;
	if (ReadFile(hComm, puchBuff, nLength, (LPDWORD)&nRead, NULL)) {
		// TODO:
	}
	return nRead;
}


int check_buff_count_rx(HANDLE hComm)
{
	int nCount = 0;
	DWORD dwErrorMask;
	COMSTAT stComStat;
	ClearCommError(hComm, &dwErrorMask, &stComStat);
	nCount = (int)stComStat.cbInQue;
	return nCount;
}


int check_buff_count_tx(HANDLE hComm)
{
	int nCount = 0;
	DWORD dwErrorMask;
	COMSTAT stComStat;
	ClearCommError(hComm, &dwErrorMask, &stComStat);
	nCount = (int)stComStat.cbOutQue;
	return nCount;
}


static void _com_log_output(const char* szLogName, const char* szFunc, const char* szFmt, ...)
{
	char szTime[32];
	char szOutput[256];

	va_list arg;
	va_start(arg, szFmt);
	vsnprintf(szOutput, sizeof(szOutput), szFmt, arg);
	va_end(arg);

	errno = 0;
	FILE* fp = fopen(szLogName, "ab+");
	if (fp == NULL) {
		//if (errno != 0) perror(NULL);
		return;
	}
	fprintf(fp, "%s, %s, %s", str_time_now(szTime, sizeof(szTime)), szFunc, szOutput);
	fclose(fp);
}
