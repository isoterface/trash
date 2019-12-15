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
	
	BOOL SetArgs(LPSECURITY_ATTRIBUTES lpThreadAttributes,
		DWORD dwStackSize,
		LPTHREAD_START_ROUTINE lpStartAddress,
		LPVOID lpParameter,
		DWORD dwCreationFlags);
	BOOL Start();
	BOOL Join();
	static BOOL JoinAll(CThread acThread[], int nCount);

	BOOL SetFunction(LPTHREAD_START_ROUTINE lpStartAddress);
	BOOL SetParameter(LPVOID lpParameter);

	HANDLE GetHandle();
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


BOOL CThread::SetArgs(LPSECURITY_ATTRIBUTES lpThreadAttributes,
	DWORD dwStackSize,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	DWORD dwCreationFlags)
{
	m_lpThreadAttributes = lpThreadAttributes;
	m_dwStackSize = dwStackSize;
	m_lpStartAddress = lpStartAddress;
	m_lpParameter = lpParameter;
	m_dwCreationFlags = dwCreationFlags;

	//if (m_lpStartAddress == NULL) {
	//	return FALSE;
	//}
	//m_hThread = ::CreateThread(
	//	m_lpThreadAttributes,
	//	m_dwStackSize,
	//	m_lpStartAddress,
	//	m_lpParameter,
	//	m_dwCreationFlags,
	//	&m_dwThreadId);
	//return (m_hThread != NULL);
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
BOOL CThread::Join()
{
	BOOL bClose = FALSE;
	::WaitForSingleObject(m_hThread, INFINITE);
	bClose &= ::CloseHandle(m_hThread);
	return bClose;
}

BOOL CThread::JoinAll(CThread acThread[], int nCount)
{
	HANDLE ahThread[10];		// TODO: ÉXÉåÉbÉhÇÃå¬êîÇ…ëŒâû

	for (int i = 0; i < nCount; i++) {
		ahThread[i] = acThread[i].GetHandle();
	}

	::WaitForMultipleObjects(
		nCount,
		ahThread,
		TRUE,
		INFINITE);
	return TRUE;
}

BOOL CThread::SetFunction(LPTHREAD_START_ROUTINE lpStartAddress)
{
	if (lpStartAddress == NULL) {
		return FALSE;
	}
	m_lpStartAddress = lpStartAddress;
	return TRUE;
}
BOOL CThread::SetParameter(LPVOID lpParameter)
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
