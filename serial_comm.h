#pragma once

#include <Windows.h>

#define RX_BUFF		(1024)		// ��M�o�b�t�@�T�C�Y
#define TX_BUFF		(1024)		// ���M�o�b�t�@�T�C�Y


HANDLE open_serial(const char* szPort, int nBaud, int nDataBit, int nParity, int nStopBit);
static void SetDCB(DCB* pDCB);
int close_serial(HANDLE hComm);
int clear_serial(HANDLE hComm);
int serial_send(HANDLE hComm, unsigned char* puchData, int nLength);
int serial_recv(HANDLE hComm, unsigned char* puchBuff, int nLength, int nTimeout);
int check_buff_count_rx(HANDLE hComm);
int check_buff_count_tx(HANDLE hComm);


HANDLE open_serial(const char* szPort, int nBaud, int nDataBit, int nParity, int nStopBit)
{
	HANDLE hComm;

	// COM�|�[�g�n���h���擾
	hComm = CreateFile(szPort
		, (GENERIC_READ | GENERIC_WRITE)
		, 0
		, NULL
		, OPEN_EXISTING
		, FILE_ATTRIBUTE_NORMAL
		, NULL);

	if (hComm == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	// ����M�o�b�t�@������
	if (!SetupComm(hComm, RX_BUFF, TX_BUFF)) {
		CloseHandle(hComm);
		return NULL;
	}

	// ����M�o�b�t�@�̃f�[�^����
	if (!PurgeComm(hComm, (PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))) {
		CloseHandle(hComm);
		return NULL;
	}

	// �ʐM�����̐ݒ�
	DCB stDCB;
	if (!GetCommState(hComm, &stDCB)) {
		CloseHandle(hComm);
		return NULL;
	}

	stDCB.BaudRate = nBaud;			// 9600;
	stDCB.ByteSize = nDataBit;		// 8;
	stDCB.fParity = nParity;		// NOPARITY;
	stDCB.StopBits = nStopBit;		// ONESTOPBIT;

	if (!SetCommState(hComm, &stDCB)) {
		CloseHandle(hComm);
		return NULL;
	}

	// �^�C���A�E�g���Ԃ̐ݒ�
	COMMTIMEOUTS stCommTimeout;
	//if (!GetCommTimeouts(hComm, &stCommTO)) {
	//	CloseHandle(hComm);
	//	return NULL;
	//}

	// ��M�C���^�[�o������
	// ReadIntervalTimeout�ɐݒ肷��
	// ReadFile�֐���1����������M����ۂɂ͌��ʂȂ�
	// �[���ɐݒu����ƁA��M�C���^�[�o�����Ԃ͎g���Ȃ�
	stCommTimeout.ReadIntervalTimeout = 500;
	// ��M�g�[�^������
	// ReadTotalTimeoutMultiplier * (��M�o�C�g��)+ReadTotalTimeoutConstant
	// ReadTotalTimeoutMultiplier �� ReadTotalTimeoutConstant ���[���̂Ƃ��A��M�g�[�^�����Ԃ͎g���Ȃ�
	stCommTimeout.ReadTotalTimeoutMultiplier = 0;
	stCommTimeout.ReadTotalTimeoutConstant = 500;
	// ���M�g�[�^������
	// WriteTotalTimeoutMultiplier * (���M�o�C�g��)+WriteTotalTimeoutConstant
	// WriteTotalTimeoutMultiplier �� WriteTotalTimeoutConstant ���[���̂Ƃ��A���M�g�[�^�����Ԃ͎g���Ȃ�
	stCommTimeout.WriteTotalTimeoutMultiplier = 0;
	stCommTimeout.WriteTotalTimeoutConstant = 500;

	if (!SetCommTimeouts(hComm, &stCommTimeout)) {
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
	// �n�[�h�E�F�A�t���[����
	pDCB->fOutxCtsFlow = FALSE;
	pDCB->fOutxDsrFlow = FALSE;
	pDCB->fDtrControl = DTR_CONTROL_DISABLE;
	pDCB->fRtsControl = RTS_CONTROL_DISABLE;
	// �\�t�g�E�F�A�t���[����
	pDCB->fOutX = FALSE;
	pDCB->fInX = FALSE;
	pDCB->fTXContinueOnXoff = TRUE;
	pDCB->XonLim = 512;
	pDCB->XoffLim = 512;
	pDCB->XonChar = 0x11;
	pDCB->XoffChar = 0x13;
	// ���̑�
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
	// ����M�o�b�t�@�̃f�[�^����
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


