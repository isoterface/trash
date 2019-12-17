/**
* @file		SimpleThread.h
* @brief	スレッド処理
* @author	?
* @date		?
*/
#pragma once

#include <windows.h>
#include "misc.h"


/**
 * @class	CThread
 * @brief	スレッド処理
 */
class CThread
{
private:
	HANDLE					m_hThread;

	LPSECURITY_ATTRIBUTES	m_lpThreadAttributes;
	DWORD					m_dwStackSize;
	LPTHREAD_START_ROUTINE	m_lpStartAddress;
	LPVOID					m_lpParameter;
	DWORD					m_dwCreationFlags;
	DWORD					m_dwThreadId;

public:
	CThread();
	~CThread();

	BOOL					SetArgs(LPSECURITY_ATTRIBUTES lpThreadAttributes,
		DWORD dwStackSize,
		LPTHREAD_START_ROUTINE lpStartAddress,
		LPVOID lpParameter,
		DWORD dwCreationFlags);
	BOOL					SetThreadProc(LPTHREAD_START_ROUTINE lpStartAddress);
	BOOL					SetThreadParam(LPVOID lpParameter);

	BOOL					Start();
	BOOL					Join(DWORD dwWait = INFINITE);
	static BOOL				JoinAll(CThread acThread[], int nCount, DWORD dwWait = INFINITE);

	HANDLE					GetHandle();

private:
	void					clearHandle();
};


/**
 * コンストラクタ
 */
CThread::CThread()
{
	m_hThread = NULL;

	m_lpThreadAttributes = NULL;
	m_dwStackSize = 0;
	m_lpStartAddress = NULL;
	m_lpParameter = NULL;
	m_dwCreationFlags = CREATE_SUSPENDED;	//0;
	m_dwThreadId = 0;
}

/**
 * デストラクタ
 */
CThread::~CThread()
{
	if (m_hThread != NULL) {
		Join();
	}
}

/**
* @fn
* @brief
* @param	[in]
* @return
*/
BOOL CThread::SetArgs(LPSECURITY_ATTRIBUTES lpThreadAttributes,
	DWORD dwStackSize,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	DWORD dwCreationFlags)
{
	if (lpStartAddress == NULL) {
		DEBUG_PRINT("ThreadProc is NULL.");
		return FALSE;
	}

	m_lpThreadAttributes = lpThreadAttributes;
	m_dwStackSize = dwStackSize;
	m_lpStartAddress = lpStartAddress;
	m_lpParameter = lpParameter;
	m_dwCreationFlags = dwCreationFlags;

	return TRUE;
}

/**
* @fn
* @brief
* @param	[in]
* @return
*/
BOOL CThread::Start()
{
	if (m_lpStartAddress == NULL) {
		DEBUG_PRINT("ThreadProc is NULL.");
		return FALSE;
	}
	m_hThread = ::CreateThread(
		m_lpThreadAttributes,
		m_dwStackSize,
		m_lpStartAddress,
		m_lpParameter,
		m_dwCreationFlags,
		&m_dwThreadId);
	if (m_hThread == NULL) {
		DEBUG_PRINT("CreateThread failed.");
		return FALSE;
	}
	if (::ResumeThread(m_hThread) == -1) {		// 0xFFFFFFFF:失敗
		DEBUG_PRINT("ResumeThread failed.");
		return FALSE;
	}
	return TRUE;
}

/**
* @fn
* @brief
* @param	[in]
* @return
*/
BOOL CThread::Join(DWORD dwWait/*=INFINITE*/)
{
	DWORD dwErr = 0;
	DWORD dwRet = ::WaitForSingleObject(m_hThread, dwWait);
	if (dwRet != WAIT_OBJECT_0) {
		switch (dwRet) {
		case WAIT_ABANDONED:
			DEBUG_PRINT("WaitForSingleObject failed.(WAIT_ABANDONED).");
			break;
		case WAIT_TIMEOUT:
			DEBUG_PRINT("WaitForSingleObject failed.(WAIT_TIMEOUT)");
			break;
		case WAIT_FAILED:
			dwErr = ::GetLastError();
			DEBUG_PRINT("WaitForSingleObject failed.(WAIT_FAILED(%d))", dwErr);
			break;
		default:
			DEBUG_PRINT("WaitForSingleObject failed.(%d)", dwRet);
			break;
		}
		return FALSE;
	}

	BOOL bClose = FALSE;
	if ((bClose = ::CloseHandle(m_hThread)) == TRUE) {
		clearHandle();
	}
	if (!bClose) {
		DEBUG_PRINT("CloseHandle failed.");
	}
	return bClose;
}

/**
* @fn
* @brief
* @param	[in]
* @return
*/
BOOL CThread::JoinAll(CThread acThread[], int nCount, DWORD dwWait/*=INFINITE*/)
{
	if (acThread == NULL) {
		DEBUG_PRINT("ThreadList is NULL.");
		return FALSE;
	}

	HANDLE* pHandle = new HANDLE[nCount];
	if (pHandle == NULL) {
		DEBUG_PRINT("new HANDLE ptr create failed.");
		return FALSE;
	}

	for (int i = 0; i < nCount; i++) {
		pHandle[i] = acThread[i].GetHandle();
	}

	DWORD dwErr = 0;
	DWORD dwRet = ::WaitForMultipleObjects(
		nCount,
		pHandle,
		TRUE,
		dwWait);
	if (dwRet != WAIT_OBJECT_0) {
		switch (dwRet) {
		case WAIT_ABANDONED_0:
			DEBUG_PRINT("WaitForMultipleObjects failed.(WAIT_ABANDONED).");
			break;
		case WAIT_TIMEOUT:
			DEBUG_PRINT("WaitForMultipleObjects failed.(WAIT_TIMEOUT)");
			break;
		case WAIT_FAILED:
			dwErr = ::GetLastError();
			DEBUG_PRINT("WaitForMultipleObjects failed.(WAIT_FAILED(%d))", dwErr);
			break;
		default:
			DEBUG_PRINT("WaitForMultipleObjects failed.(%d)", dwRet);
			break;
		}
		return FALSE;
	}

	BOOL bRet = TRUE;
	BOOL bClose = TRUE;
	for (int i = 0; i < nCount; i++) {
		if ((bRet = ::CloseHandle(pHandle[i])) == TRUE) {
			acThread[i].clearHandle();
		}
		bClose &= bRet;
	}
	if (!bClose) {
		DEBUG_PRINT("CloseHandle failed.");
	}
	delete[] pHandle;

	return bClose;
}

/**
* @fn
* @brief
* @param	[in]
* @return
*/
BOOL CThread::SetThreadProc(LPTHREAD_START_ROUTINE lpStartAddress)
{
	if (lpStartAddress == NULL) {
		DEBUG_PRINT("ThreadFunc is NULL.");
		return FALSE;
	}
	m_lpStartAddress = lpStartAddress;
	return TRUE;
}

/**
* @fn
* @brief
* @param	[in]
* @return
*/
BOOL CThread::SetThreadParam(LPVOID lpParameter)
{
	if (lpParameter == NULL) {
		DEBUG_PRINT("ThreadParam is NULL.");
		return FALSE;
	}
	m_lpParameter = lpParameter;
	return TRUE;
}

/**
* @fn
* @brief
* @param	[in]
* @return
*/
HANDLE CThread::GetHandle()
{
	return m_hThread;
}

void CThread::clearHandle()
{
	m_hThread = NULL;
}

// *** sample ***
//DWORD WINAPI MyThreadFunction(LPVOID lpParam)
//{
//	HANDLE hStdout;
//	PMYDATA pDataArray;
//
//	TCHAR msgBuf[BUF_SIZE];
//	size_t cchStringSize;
//	DWORD dwChars;
//
//	// Make sure there is a console to receive output results. 
//
//	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
//	if (hStdout == INVALID_HANDLE_VALUE)
//		return 1;
//
//	// Cast the parameter to the correct data type.
//	// The pointer is known to be valid because 
//	// it was checked for NULL before the thread was created.
//
//	pDataArray = (PMYDATA)lpParam;
//
//	// Print the parameter values using thread-safe functions.
//
//	StringCchPrintf(msgBuf, BUF_SIZE, TEXT("Parameters = %d, %d\n"),
//		pDataArray->val1, pDataArray->val2);
//	StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);
//	WriteConsole(hStdout, msgBuf, (DWORD)cchStringSize, &dwChars, NULL);
//
//	return 0;
//}