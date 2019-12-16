#pragma once

#include <windows.h>


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
	
	BOOL					SetParameter(LPSECURITY_ATTRIBUTES lpThreadAttributes,
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
};



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


CThread::~CThread()
{
}


BOOL CThread::SetParameter(LPSECURITY_ATTRIBUTES lpThreadAttributes,
	DWORD dwStackSize,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	DWORD dwCreationFlags)
{
	if (lpStartAddress == NULL) {
		return FALSE;
	}

	m_lpThreadAttributes = lpThreadAttributes;
	m_dwStackSize = dwStackSize;
	m_lpStartAddress = lpStartAddress;
	m_lpParameter = lpParameter;
	m_dwCreationFlags = dwCreationFlags;

	return TRUE;
}

BOOL CThread::Start()
{
	if (m_lpStartAddress == NULL) {
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
		return FALSE;
	}
	::ResumeThread(m_hThread);
	return TRUE;
}

BOOL CThread::Join(DWORD dwWait/*=INFINITE*/)
{
	::WaitForSingleObject(m_hThread, dwWait);
	::CloseHandle(m_hThread);
	return TRUE;
}

BOOL CThread::JoinAll(CThread acThread[], int nCount, DWORD dwWait/*=INFINITE*/)
{
	if (acThread == NULL) {
		return FALSE;
	}

	HANDLE* pHandle = new HANDLE[nCount];
	if (pHandle == NULL) {
		return FALSE;
	}

	for (int i = 0; i < nCount; i++) {
		pHandle[i] = acThread[i].GetHandle();
	}

	::WaitForMultipleObjects(
		nCount,
		pHandle,
		TRUE,
		dwWait);

	for (int i = 0; i < nCount; i++) {
		::CloseHandle(pHandle[i]);
	}


	delete[] pHandle;

	return TRUE;
}

BOOL CThread::SetThreadProc(LPTHREAD_START_ROUTINE lpStartAddress)
{
	if (lpStartAddress == NULL) {
		return FALSE;
	}
	m_lpStartAddress = lpStartAddress;
	return TRUE;
}

BOOL CThread::SetThreadParam(LPVOID lpParameter)
{
	if (lpParameter == NULL) {
		return FALSE;
	}
	m_lpParameter = lpParameter;
	return TRUE;
}

HANDLE CThread::GetHandle()
{
	return m_hThread;
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