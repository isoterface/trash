#pragma once

#include <windows.h>
#include "misc.h"

#define QUEUE_SIZE		(16)		// 2**n
#define MAX_ID			(1)

static CRITICAL_SECTION g_stCS[MAX_ID];
static unsigned char g_byQueue[MAX_ID][QUEUE_SIZE];
static int g_nHead = 0;
static int g_nLength = 0;
static const int g_nMask = (QUEUE_SIZE - 1);


int queue_init(int nID);
int queue_end(int nID);
int queue_clear(int nID);
int queue_enqueue(int nID, const unsigned char* pbyData, int nLen);
int queue_dequeue(int nID, unsigned char* pbyBuff, int nLen);

static void lock_init(int nID);
static void lock_delete(int nID);
static void lock(int nID);
static void unlock(int nID);

void debug_print();

int queue_init(int nID)
{
	if (nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	lock_init(nID);
	queue_clear(nID);

	return 0;
}

int queue_end(int nID)
{
	if (nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	lock_delete(nID);

	return 0;
}

int queue_clear(int nID)
{
	if (nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	lock(nID);
	memset(g_byQueue[nID], 0, sizeof(g_byQueue[nID]));
	unlock(nID);

	return 0;
}

int queue_enqueue(int nID, const unsigned char* pbyData, int nLen)
{
	if (pbyData == NULL || nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	lock(nID);
	int count = 0;
	int ptr = 0;
	for (int i = 0; i < nLen; i++) {
		if (g_nLength < QUEUE_SIZE) {
			ptr = ((g_nHead + g_nLength) & g_nMask);
			g_byQueue[nID][ptr] = pbyData[i];
			g_nLength++;
			count++;
		}
		else {
			break;
		}
	}
	unlock(nID);

	debug_print();

	return count;
}


int queue_dequeue(int nID, unsigned char* pbyBuff, int nLen)
{
	if (pbyBuff == NULL || nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	lock(nID);
	int count = 0;
	for (int i = 0; i < nLen; i++) {
		if (0 < g_nLength) {
			pbyBuff[i] = g_byQueue[nID][g_nHead];
			g_byQueue[nID][g_nHead] = 0;
			g_nHead = ((g_nHead + 1) & g_nMask);
			g_nLength--;
			count++;
		}
		else {
			break;
		}
	}
	unlock(nID);

	debug_print();

	return count;
}


int queue_is_empty()
{
	return ((g_nLength == 0) ? (0) : (-1));
}

void debug_print()
{
	char szBuff[256];
	mem_dump(g_byQueue, sizeof(g_byQueue), szBuff, sizeof(szBuff));
	printf("H:%02d,L:%02d,[%s]\r\n", g_nHead, g_nLength, szBuff);
}




/**
 * @fn		_LockInit
 * @brief	”r‘¼ˆ—‰Šú‰»
 * @param	[in]	int nID		 : ƒƒOID
 */
static void lock_init(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::InitializeCriticalSection(&(g_stCS[nID]));
	}
}
/**
 * @fn		_LockDelete
 * @brief	”r‘¼ˆ—‰Šú‰»
 * @param	[in]	int nID		 : ƒƒOID
 */
static void lock_delete(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::DeleteCriticalSection(&(g_stCS[nID]));
	}
}
/**
 * @fn		_Lock
 * @brief	”r‘¼ˆ—‰Šú‰»
 * @param	[in]	int nID		 : ƒƒOID
 */
static void lock(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::EnterCriticalSection(&(g_stCS[nID]));
	}
}
/**
 * @fn		_Unlock
 * @brief	”r‘¼ˆ—‰Šú‰»
 * @param	[in]	int nID		 : ƒƒOID
 */
static void unlock(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::LeaveCriticalSection(&(g_stCS[nID]));
	}
}
