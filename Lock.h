/**
 * @file	Lock.h
 * @brief	排他ロック
 * @author	?
 * @date	?
 */
#pragma once

#include <windows.h>


#define LOCK(obj)			obj.Lock()
#define UNLOCK(obj)			obj.Unlock()


/**
 * @class CLock
 * @brief
 */
class CLock
{
private:
	CRITICAL_SECTION	m_stCS;
	BOOL				m_bInit;

public:
	CLock();
	~CLock();

	void	Init();
	void	Exit();
	int		Loock();
	int		Unlock();
};


/**
 * コンストラクタ
 */
CLock::CLock()
	: m_bInit(FALSE)
{
	Init();
}

/**
 * デストラクタ
 */
CLock::~CLock()
{
	Exit();
}

/**
 * @fn		Init
 * @brief
 * @return
 */
void CLock::Init()
{
	if (!m_bInit) {
		InitializeCriticalSection(&m_stCS);
		m_bInit = TRUE;
	}
}

/**
 * @fn		Exit
 * @brief
 * @return
 */
void CLock::Exit()
{
	DeleteCriticalSection(&m_stCS);
	m_bInit = FALSE;
}

/**
 * @fn		Loock
 * @brief
 * @return
 */
int CLock::Loock()
{
	if (!m_bInit) {
		return -1;
	}
	EnterCriticalSection(&m_stCS);
	return 0;
}

/**
 * @fn		Unlock
 * @brief
 * @return
 */
int CLock::Unlock()
{
	if (!m_bInit) {
		return -1;
	}
	LeaveCriticalSection(&m_stCS);
	return 0;
}
