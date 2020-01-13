#include <stdio.h>
#include <conio.h>
#include <stdarg.h>
#include <windows.h>
#include <process.h>
#include "misc.h"
#include "simple_queue2.h"

#define RX_BUFF		(1024)		// 受信バッファサイズ
#define TX_BUFF		(1024)		// 送信バッファサイズ


int start_recvbuff_thread();
unsigned __stdcall thread_recv_buff(PVOID pParam);

int start_comm_thread();
int end_comm_thread();

unsigned __stdcall thread_serial_comm(PVOID pParam);
int ReportStatusEvent(unsigned char* bytebuff, DWORD cnt);

HANDLE open_serial_async(const char* szPort, int nBaud, int nDataBit, int nParity, int nStopBit, const char* szLogName);
static void SetDCB(DCB* pDCB);
int close_serial(HANDLE hComm, const char* szLogName);
int clear_serial(HANDLE hComm);
int serial_send(HANDLE hComm, unsigned char* puchData, int nLength, const char* szLogName);
int serial_recv(HANDLE hComm, unsigned char* puchBuff, int nLength, int nTimeout, const char* szLogName);
int check_buff_count_rx(HANDLE hComm);
int check_buff_count_tx(HANDLE hComm);
static void _com_log_output(const char* szLogName, const char* szFunc, const char* szOutput);


HANDLE g_hComm;
OVERLAPPED g_osReader = { 0 };
HANDLE g_hThreadReader;
DWORD g_dwThreadId;
char g_szLog[] = "com14.log";

HANDLE g_hThreadExitEvent;

RING_BUFFER* g_pstRecvBuff;

HANDLE g_hId_RecvBuff;
DWORD g_dwId_RecvBuff;
HANDLE g_hEvt_RecvBuff;

#define RING_BUFF_SIZE		(16)

int main(int argc, char* argv)
{
	g_pstRecvBuff = create_queue(RING_BUFF_SIZE);

	if (g_pstRecvBuff == NULL) {
		printf("RingBuffer create failed.");
		return -1;
	}

	g_hEvt_RecvBuff = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hThreadExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	g_hComm = open_serial_async("COM4"
		, CBR_9600, 8, NOPARITY, ONESTOPBIT, g_szLog);

	start_recvbuff_thread();

	if (start_comm_thread() < 0) {
		close_serial(g_hComm, NULL);
		printf("start error.");
		getch();
		return -1;
	}

	while (getch() != 'q') {
		Sleep(100);
	}

	end_comm_thread();

	printf("end.\r\n");

	getch();

	delete_queue(g_pstRecvBuff);

	return 0;
}


int start_recvbuff_thread()
{
	g_hId_RecvBuff = (HANDLE)_beginthreadex(NULL
		, 0
		, thread_recv_buff
		, NULL
		, 0
		, (unsigned int*)(&g_dwId_RecvBuff));

	if (g_hId_RecvBuff == INVALID_HANDLE_VALUE) {
		return -1;
	}

	printf("thread start.\r\n");
	return 0;
}


unsigned __stdcall thread_recv_buff(PVOID pParam)
{
	DWORD dwEvt;
	int nLength = 0;
	unsigned char buff[RING_BUFF_SIZE];
	char dumpbuff[RING_BUFF_SIZE * 8];

	while (TRUE) {
		if (WaitForSingleObject(g_hEvt_RecvBuff, 500) == WAIT_OBJECT_0) {
			nLength = queue_data_count(g_pstRecvBuff);
			if (0 < nLength) {
				queue_pop(g_pstRecvBuff, buff, nLength);
				printf("3>>> QPOP: %s.\r\n", mem_dump2(buff, nLength, dumpbuff, RING_BUFF_SIZE * 8));
			}
		}
	}
}


int start_comm_thread()
{
	g_hThreadReader = (HANDLE)_beginthreadex(NULL
		, 0
		, thread_serial_comm
		, NULL
		, 0
		, (unsigned int*)(&g_dwThreadId));

	if (g_hThreadReader == INVALID_HANDLE_VALUE) {
		return -1;
	}

	printf("thread start.\r\n");
	return 0;
}

int end_comm_thread()
{
	printf("thread terminate.\r\n");

	HANDLE hThreads[1] = { g_hThreadReader };

	SetEvent(g_hThreadExitEvent);

	// WaitCommEventの待機状態を解除
	if (SetCommMask(g_hComm, EV_ERR) == FALSE) {
		return -1;
	}

	DWORD dwRes = WaitForMultipleObjects(1, hThreads, TRUE, 2000);
	switch (dwRes) {
	case WAIT_OBJECT_0:
		break;
	case WAIT_TIMEOUT:
		if (WaitForSingleObject(g_hThreadReader, 0) == WAIT_TIMEOUT) {
			printf("end_comm_thread, WaitForSingleObject WAIT_TIMEOUT error thread end.\r\n");
			//Error
		}
		break;
	default:
		//Error
		break;
	}

	ResetEvent(g_hThreadExitEvent);

	// beginthreadexでは必要, _beginthreadでは不要(ハンドルは自動的に閉じられる)
	if (CloseHandle(g_hThreadReader) == 0) {
		// TODO:
	}
	if (CloseHandle(g_hThreadExitEvent) == 0) {
		// TODO:
	}

	return close_serial(g_hComm, g_szLog);
}


unsigned __stdcall thread_serial_comm(PVOID pParam)
{
	BOOL g_bLive = FALSE;

	const int Status_Check_Timeout = 500;
	DWORD dwRes = 0;
	DWORD dwRead = 0;
	DWORD dwCommEvent = 0;
	DWORD dwStoredFlags = 0;
	BOOL fWaitingOnRead = FALSE;
	char szError[512];

	unsigned char bytebuff[256] = { 0 };
	int cnt = 0;

	dwStoredFlags = EV_BREAK \
		| EV_CTS \
		| EV_DSR \
		| EV_ERR \
		| EV_RING \
		| EV_RLSD;
		//| EV_RXCHAR \
		//| EV_RXFLAG \
		//| EV_TXEMPTY;

	if (!SetCommMask(g_hComm, dwStoredFlags)) {
		// error setting communications mask; abort
		return -1;
	}

	g_osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_osReader.hEvent == NULL) {
		// error creating event; abort
		return -1;
	}

	char szTimeBuff[256];

	HANDLE hArray[2] = { g_osReader.hEvent, g_hThreadExitEvent };

	g_bLive = TRUE;
	while (g_bLive)
	{
		//printf("%s loop.\r\n", str_time_now(szTimeBuff, sizeof(szTimeBuff)));
		// Issue a status event check if one hasn't been issued already.
		if (!fWaitingOnRead) {
			if (!ReadFile(g_hComm, bytebuff, 512, &dwRead, &g_osReader)) {
			//if (!WaitCommEvent(g_hComm, &dwCommEvent, &g_osReader)) {
				dwRes = GetLastError();
				if (dwRes == ERROR_IO_PENDING) {
				//	fWaitingOnRead = TRUE;
				//	;
				}
				else {
					// error in WaitCommEvent; abort
					get_error_msg(dwRes, szError, sizeof(szError));
					printf("WaitCommEvent error thread end. (%X:%s)\r\n", dwRes, szError);
					//CloseHandle(g_osReader.hEvent);
					//return -1;
				}
				fWaitingOnRead = TRUE;
			}
			else {
				if (0 < dwRead) {
					// WaitCommEvent returned immediately.
					// Deal with status event as appropriate.
					printf("1>>> ");
					ReportStatusEvent(bytebuff, dwRead);
					dwCommEvent = 0;
				}
			}
		}

		if (fWaitingOnRead) {
			dwRes = WaitForMultipleObjects(2, hArray, FALSE, 500);		
			switch (dwRes) {
			case WAIT_OBJECT_0:
				if (!GetOverlappedResult(g_hComm, &g_osReader, &dwRead, FALSE)) {
					// An error occurred in the overlapped operation;
					// call GetLastError to find out what it was
					// and abort if it is fatal.

					dwRes = GetLastError();
					switch (dwRes) {
					case ERROR_IO_PENDING:
						printf("WaitForSingleObject GetOverlappedResult ERROR_IO_PENDING.\r\n");
					default:
						printf("WaitForSingleObject GetOverlappedResult error=%d.\r\n", dwRes);
						get_error_msg(dwRes, szError, sizeof(szError));
						printf("WaitCommEvent error thread end. (%X:%s)\r\n", dwRes, szError);
						dwCommEvent = 0;
						//fWaitingOnRead = FALSE;
					}
				}
				else {
					//printf("GetOverlappedResult\r\n");
					if (0 < dwRead) {
						// Status event is stored in the event flag
						// specified in the original WaitCommEvent call.
						// Deal with the status event as appropriate.
						printf("2>>> ");
						ReportStatusEvent(bytebuff, dwRead);
						dwCommEvent = 0;
						//fWaitingOnRead = FALSE;
					}
				}
				fWaitingOnRead = FALSE;
				// Set fWaitingOnStat flag to indicate that a new
				// WaitCommEvent is to be issued.
				//fWaitingOnStat = FALSE;
				break;
			case WAIT_OBJECT_0 + 1:		// ThreadExitEvent
				g_bLive = FALSE;
				break;
			case WAIT_TIMEOUT:
				// Operation isn't complete yet. fWaitingOnStatusHandle flag 
				// isn't changed since I'll loop back around and I don't want
				// to issue another WaitCommEvent until the first one finishes.
				//
				// This is a good time to do some background work.
				//DoBackgroundWork();
				//printf("wait timeout.\r\n");
				break;

			case WAIT_ABANDONED:
				printf("WaitForSingleObject WAIT_ABANDONED error thread end.\r\n");
				//CloseHandle(g_osReader.hEvent);
				return -1;

			//case WAIT_FAILED:
			//	dwRes = GetLastError();
			//	get_error_msg(dwRes, szError, sizeof(szError));
			//	printf("WaitForSingleObject WAIT_FAILED error thread end. (%X:%s)\r\n", dwRes, szError);
			//	CloseHandle(g_osReader.hEvent);
			//	return -1;

			default:
				// Error in the WaitForSingleObject; abort
				// This indicates a problem with the OVERLAPPED structure's
				// event handle.
				printf("WaitForSingleObject error thread end. res=%d.\r\n", dwRes);
				//CloseHandle(g_osReader.hEvent);
				return -1;
			}
		}
		//Sleep(1);
	}

	CloseHandle(g_osReader.hEvent);

	//_endthreadex(0);		// スレッド終了時に自動的に呼ばれる(_beginthreadでスレッド開始した場合は_endthreadが呼ばれる)

	return 0;
}

int ReportStatusEvent(unsigned char* bytebuff, DWORD cnt)
{
	//unsigned char bytebuff[256] = { 0 };
	char txtbuf[512 * 8] = { 0 };
	//int cnt = 0;

	//if (dwEvtMask == 0) {
	//	return 0;
	//}
	////if (dwEvtMask & EV_BREAK) {
	////}
	////if (dwEvtMask & EV_CTS) {
	////}
	////if (dwEvtMask & EV_DSR) {
	////}
	////if (dwEvtMask & EV_RING) {
	////}
	////if (dwEvtMask & EV_RLSD) {
	////}
	//if (dwEvtMask & EV_RXCHAR) {
	//	cnt = check_buff_count_rx(g_hComm);
	//	if (256 < cnt) {
	//		cnt = 256;
	//		printf("buffer overflow.\r\n");
	//	}
	//	serial_recv(g_hComm, bytebuff, cnt, 0, NULL);

		printf("RECV: %s.\r\n", mem_dump2(bytebuff, cnt, txtbuf, sizeof(txtbuf)));
		queue_push(g_pstRecvBuff, bytebuff, cnt);
		SetEvent(g_hEvt_RecvBuff);
	//}
	//if (dwEvtMask & EV_ERR) {
	//	// COMポート再初期化
	//}
	//if (dwEvtMask & EV_RXFLAG) {
	//}
	//if (dwEvtMask & EV_TXEMPTY) {
	//}
	return 0;
}


HANDLE open_serial_async(const char* szPort, int nBaud, int nDataBit, int nParity, int nStopBit, const char* szLogName)
{
	HANDLE hComm;

	// COMポートハンドル取得
	hComm = CreateFile(szPort
		, (GENERIC_READ | GENERIC_WRITE)
		, 0
		, NULL
		, OPEN_EXISTING
		, FILE_FLAG_OVERLAPPED		// FILE_FLAG_OVERLAPPED : FILE_ATTRIBUTE_NORMAL
		, NULL);

	if (hComm == INVALID_HANDLE_VALUE) {
		_com_log_output(szLogName, __FUNCTION__, "CreateFile failed");
		return NULL;
	}

	// 送受信バッファ初期化
	if (!SetupComm(hComm, RX_BUFF, TX_BUFF)) {
		_com_log_output(szLogName, __FUNCTION__, "SetupComm failed");
		CloseHandle(hComm);
		return NULL;
	}

	// 送受信バッファのデータ消去
	if (!PurgeComm(hComm, (PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))) {
		_com_log_output(szLogName, __FUNCTION__, "PurgeComm failed");
		CloseHandle(hComm);
		return NULL;
	}

	// 通信条件の設定
	DCB stDCB;
	if (!GetCommState(hComm, &stDCB)) {
		_com_log_output(szLogName, __FUNCTION__, "GetCommState failed");
		CloseHandle(hComm);
		return NULL;
	}

	stDCB.BaudRate = nBaud;			// CBR_9600;
	stDCB.ByteSize = nDataBit;		// 8;
	stDCB.fParity = nParity;		// NOPARITY, ODDPARITY, EVENPARITY
	stDCB.StopBits = nStopBit;		// ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS

	if (!SetCommState(hComm, &stDCB)) {
		_com_log_output(szLogName, __FUNCTION__, "SetCommState failed");
		CloseHandle(hComm);
		return NULL;
	}

	// タイムアウト時間の設定
	COMMTIMEOUTS stCTO;
	//if (!GetCommTimeouts(hComm, &stCommTO)) {
	//	CloseHandle(hComm);
	//	return NULL;
	//}

	// 受信インターバル時間
	// ReadIntervalTimeoutに設定する
	// ReadFile関数で1文字だけ受信する際には効果なし
	// ゼロに設置すると、受信インターバル時間は使われない
	stCTO.ReadIntervalTimeout = 500;
	// 受信トータル時間
	// ReadTotalTimeoutMultiplier * (受信バイト数)+ReadTotalTimeoutConstant
	// ReadTotalTimeoutMultiplier と ReadTotalTimeoutConstant がゼロのとき、受信トータル時間は使われない
	stCTO.ReadTotalTimeoutMultiplier = 0;
	stCTO.ReadTotalTimeoutConstant = 500;
	// 送信トータル時間
	// WriteTotalTimeoutMultiplier * (送信バイト数)+WriteTotalTimeoutConstant
	// WriteTotalTimeoutMultiplier と WriteTotalTimeoutConstant がゼロのとき、送信トータル時間は使われない
	stCTO.WriteTotalTimeoutMultiplier = 0;
	stCTO.WriteTotalTimeoutConstant = 500;

	if (!SetCommTimeouts(hComm, &stCTO)) {
		_com_log_output(szLogName, __FUNCTION__, "SetCommTimeouts failed");
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

int close_serial(HANDLE hComm, const char* szLogName)
{
	// 送受信バッファのデータ消去
	if (!PurgeComm(hComm, (PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))) {
		_com_log_output(szLogName, __FUNCTION__, "PurgeComm failed");
	}

	if (!CloseHandle(hComm)) {
		_com_log_output(szLogName, __FUNCTION__, "CloseHandle failed");
		// TODO:
		return -1;
	}
	return 0;
}

int clear_serial(HANDLE hComm)
{
	// 送受信バッファのデータ消去
	if (!PurgeComm(hComm, (PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))) {
		return -1;
	}
	return 0;
}

int serial_send(HANDLE hComm, unsigned char* puchData, int nLength, const char* szLogName)
{
	int nWrite = 0;
	if (!WriteFile(hComm, puchData, nLength, (LPDWORD)&nWrite, NULL)) {
		_com_log_output(szLogName, __FUNCTION__, "WriteFile failed");
		return -1;
	}
	char szDump[1024];
	_com_log_output(szLogName, __FUNCTION__, mem_dump(puchData, nLength, szDump, sizeof(szDump)));
	return nWrite;
}

int serial_recv(HANDLE hComm, unsigned char* puchBuff, int nLength, int nTimeout, const char* szLogName)
{
	int nRead = 0;
	//if (!ReadFile(hComm, puchBuff, nLength, (LPDWORD)&nRead, NULL)) {
	if (!ReadFile(hComm, puchBuff, nLength, (LPDWORD)&nRead, &g_osReader)) {
		_com_log_output(szLogName, __FUNCTION__, "ReadFile failed");
		return -1;
	}
	char szDump[1024];
	_com_log_output(szLogName, __FUNCTION__, mem_dump(puchBuff, nRead, szDump, sizeof(szDump)));
	return nRead;
}


int check_buff_count_rx(HANDLE hComm)
{
	int nCount = 0;
	DWORD dwErrorMask;
	COMSTAT stComStat;
	if (ClearCommError(hComm, &dwErrorMask, &stComStat) == 0) {
		return -1;
	}
	nCount = (int)stComStat.cbInQue;
	return nCount;
}


int check_buff_count_tx(HANDLE hComm)
{
	int nCount = 0;
	DWORD dwErrorMask;
	COMSTAT stComStat;
	if (ClearCommError(hComm, &dwErrorMask, &stComStat) == 0) {
		return -1;
	}
	nCount = (int)stComStat.cbOutQue;
	return nCount;
}


static void _com_log_output(const char* szLogName, const char* szFunc, const char* szOutput)
{
	//if (szLogName == NULL || szFunc == NULL || szOutput == NULL) {
	//	return;
	//}
	char szTime[32];
	//errno = 0;
	//FILE* fp = fopen(szLogName, "ab+");
	//if (fp == NULL) {
	//	//if (errno != 0) perror(NULL);
	//	return;
	//}
	//fprintf(fp, "%s, %s, %s.\r\n", str_time_now(szTime, sizeof(szTime)), szFunc, szOutput);
	printf("%s, %s, %s.\r\n", str_time_now(szTime, sizeof(szTime)), szFunc, szOutput);
	//fclose(fp);
}
