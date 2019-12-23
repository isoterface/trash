/**
 * @file	SimpleLog.h
 * @brief	���O�o��
 * @author	?
 * @date	?
 */
#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <io.h>
#include <windows.h>
#include <Shlwapi.h>

#define MAX_ID								(5)
#define MAX_LOG_TEXT						(256)
#define MAX_FILE_SIZE						(1024)		// 1kByte�P��
#define MAX_LOG_BACKUP						(3)

#define LOG_START(id, path)					CLog::Start(id, path)
#define LOG_END(id)							CLog::End(id)		// LOG_END()�ł���
#define LOG_WRITE(id, level, fmt, ...)		CLog::Write(id, level, fmt, __VA_ARGS__)
#define LOG_DEBUG(id, level, fmt, ...)		CLog::Debug(id, level, __FILE__, __LINE__, __FUNCTION__, fmt, __VA_ARGS__)


 /**
  * @enum	_LOG_LEVEL
  * @brief	���O���x����`
  */
typedef enum _LOG_LEVEL {
	ERR = 1,
	WAR = 2,
	INF = 3,
	DBG = 4
} LOG_LEVEL;


/**
 * @class	CLog
 * @brief	���O�o�̓N���X
 */
class CLog
{
private:
	// �X�^�e�B�b�N�����o�ϐ��i�錾�̂݁A���̂̊m�ۂ�cpp���ōs���j
	static CRITICAL_SECTION	m_stCS[MAX_ID];						//! ���OID���Ƃ̔r���I�u�W�F�N�g
	static char				m_szLogPath[MAX_ID][MAX_PATH + 1];	//! ���O�t�@�C���p�X
	static BOOL				m_bUsed[MAX_ID];					//! ���OID�g�p���
	// ���O�t�@�C�����
	static char				m_szDir[MAX_ID][MAX_PATH + 1];		//! �h���C�u�A�f�B���N�g����
	static char				m_szFname[MAX_ID][MAX_PATH + 1];	//! �t�@�C�����i�g���q�����j
	static char				m_szFext[MAX_ID][MAX_PATH + 1];		//! �g���q��
	static int				m_nFileSize[MAX_ID];				//! ���O�t�@�C���T�C�Y
	static int				m_nLogBackup[MAX_ID];				//! �t�@�C���o�b�N�A�b�v��

public:
	CLog();
	//~CLog();

	static int				Start(int nID, const char* szPath);
	static int				End(int nID);
	static int				End();
	static int				Write(int nID, int nLevel, const char* szFmt, ...);
	static int				Debug(int nID, int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...);

private:
	// �ȉ��̃I�[�o�[���[�h�̓}�N������Ăяo�����ۂɎ��ʂ��ł��Ȃ�
	static int				Start(const char* szPath);
	static int				Write(int nLevel, const char* szFmt, ...);
	static int				Debug(int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...);

private:
	static int				write(int nID, int nLevel, const char* szFmt, va_list arg);
	static int				debug(int nID, int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, va_list arg);
	static const char*		logLevel(int nLevel);
	static int				getId(int nID);
	static const char*		getFnameFromPath(const char* szPath, char* szBuff, int nSize);

	static void				lockInit(int nID);
	static void				lockDelete(int nID);
	static void				lock(int nID);
	static void				unlock(int nID);

	static int				copyFilePath(int nID, const char* szPath);
	static int				getBackupName(int nID, int nBkNo, char* szBuff, int nSize);
	static int				backupFile(int nID);
	static long				getFileSize(const char* szPath);
};


//! �X�^�e�B�b�N�����o�ϐ��̎��̂̊m�ہA������
//! ���OID���Ƃ̔r���I�u�W�F�N�g
CRITICAL_SECTION CLog::m_stCS[MAX_ID];
//! ���O�t�@�C���p�X
char CLog::m_szLogPath[MAX_ID][MAX_PATH + 1];
//! ���OID�g�p���
int CLog::m_bUsed[MAX_ID] = {
	FALSE, FALSE, FALSE, FALSE, FALSE
};

//! �h���C�u�A�f�B���N�g����
char CLog::m_szDir[MAX_ID][MAX_PATH + 1];
//! �t�@�C�����i�g���q�����j
char CLog::m_szFname[MAX_ID][MAX_PATH + 1];
//! �g���q��
char CLog::m_szFext[MAX_ID][MAX_PATH + 1];
//! ���O�t�@�C���T�C�Y
int CLog::m_nFileSize[MAX_ID] = {
	MAX_FILE_SIZE, MAX_FILE_SIZE, MAX_FILE_SIZE, MAX_FILE_SIZE, MAX_FILE_SIZE
};
//! �t�@�C���o�b�N�A�b�v��
int CLog::m_nLogBackup[MAX_ID] = {
	MAX_LOG_BACKUP, MAX_LOG_BACKUP, MAX_LOG_BACKUP, MAX_LOG_BACKUP, MAX_LOG_BACKUP
};


/**
 * �R���X�g���N�^
 */
CLog::CLog()
{
	// �C���X�^���X�͐������Ȃ�
	assert(FALSE);
}

//CLog::~CLog()
//{
//}

/**
 * @fn		Start
 * @brief	�w�胍�O�t�@�C���ւ̃��O�o�͂��J�n����
 * @param	[in]	int nID				: ���O�o�͎��̎w�胍�OID((0)�`(MAX_ID-1), -1��ID�������w��)
 * @param	[in]	const char* szPath	: ���O�o�͐�t�@�C���p�X
 * @return	(0)�`(MAX_ID-1):����(�Ȍ�̃��O�o�͎͂擾����ID���g�p����), -1:���s
 * @remarks	���r�������̓��OID���Ƃ̂��߁A�قȂ郍�OID�œ������O�t�@�C�����w�肵���ꍇ�̓���͕ۏ؂���Ȃ�
 */
int CLog::Start(int nID, const char* szPath)
{
	if (szPath == NULL) {
		return -1;
	}

	int id = getId(nID);
	if (id < 0) {
		return -1;
	}

	// ���O�t�@�C���Ƀo�b�N�A�b�v�ԍ���t�^
	copyFilePath(nID, szPath);

	//memset(m_szLogPath[id], 0, sizeof(m_szLogPath[id]));
	//strncpy(m_szLogPath[id], szPath, sizeof(m_szLogPath[id]));

	//::InitializeCriticalSection(&(m_stCS[id]));
	lockInit(id);

	m_bUsed[id] = TRUE;

	return id;
}

/**
 * @fn		Start
 * @brief	�w�胍�O�t�@�C���ւ̃��O�o�͂��J�n����(ID�͎����w��)
 * @param	[in]	const char* szPath	: ���O�o�͐�t�@�C���p�X
 * @return	(0)�`(MAX_ID-1):����(�Ȍ�̃��O�o�͎͂擾����ID���g�p����), -1:���s
 */
int CLog::Start(const char* szPath)
{
	if (szPath == NULL) {
		return -1;
	}
	return CLog::Start(-1, szPath);		// ����Ăяo���ł���Ύ����I�� ID=0 �����蓖�Ă���
}

/**
 * @fn		End
 * @brief
 * @param	[in]
 * @return
 */
int CLog::End(int nID)
{
	if (nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	// �w��ID�̂݊J��
	if (m_bUsed[nID] == TRUE) {
		//::DeleteCriticalSection(&(m_stCS[nID]));
		lockDelete(nID);
		m_bUsed[nID] = FALSE;
	}
	return 0;
}

/**
 * @fn		End
 * @brief
 * @param	[in]
 * @return
 */
int CLog::End()
{
	// �SID�J��
	for (int i = 0; i < MAX_ID; i++) {
		if (m_bUsed[i] == TRUE) {
			//::DeleteCriticalSection(&(m_stCS[i]));
			lockDelete(i);
			m_bUsed[i] = FALSE;
		}
	}
	return 0;
}

/**
 * @fn		Write
 * @brief
 * @param	[in]
 * @return
 */
int CLog::Write(int nID, int nLevel, const char* szFmt, ...)
{
	if (szFmt == NULL) {
		return -1;
	}

	// TODO: �Ăь��Ń��b�N����H
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = write(nID, nLevel, szFmt, arg);
	va_end(arg);
	return ret;
}

/**
 * @fn		Write
 * @brief
 * @param	[in]
 * @return
 */
int CLog::Write(int nLevel, const char* szFmt, ...)
{
	if (szFmt == NULL) {
		return -1;
	}

	// TODO: �Ăь��Ń��b�N����H
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = write(0, nLevel, szFmt, arg);
	va_end(arg);
	return ret;
}

/**
 * @fn		Debug
 * @brief
 * @param	[in]
 * @return
 */
int CLog::Debug(int nID, int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...)
{
	if (szFmt == NULL || szFile == NULL || szFunc == NULL) {
		return -1;
	}

	// TODO: �Ăь��Ń��b�N����H
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = debug(nID, nLevel, szFile, nLine, szFunc, szFmt, arg);
	va_end(arg);
	return ret;
}

/**
 * @fn		Debug
 * @brief
 * @param	[in]
 * @return
 */
int CLog::Debug(int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, ...)
{
	if (szFmt == NULL || szFile == NULL || szFunc == NULL) {
		return -1;
	}

	// TODO: �Ăь��Ń��b�N����H
	int ret = 0;
	va_list arg;
	va_start(arg, szFmt);
	ret = debug(0, nLevel, szFile, nLine, szFunc, szFmt, arg);
	va_end(arg);
	return ret;
}

/**
 * @fn		write
 * @brief
 * @param	[in]
 * @return
 */
int CLog::write(int nID, int nLevel, const char* szFmt, va_list arg)
{
	if (szFmt == NULL) {
		return -1;
	}

	if (nID < 0 || MAX_ID <= nID || m_bUsed[nID] == FALSE) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	//::EnterCriticalSection(&(m_stCS[nID]));
	lock(nID);

	backupFile(nID);

	char szBuff0[MAX_LOG_TEXT];
	char szBuff1[MAX_LOG_TEXT * 2];
	vsnprintf(szBuff0, sizeof(szBuff0), szFmt, arg);

	errno = 0;
	FILE *fp = fopen(m_szLogPath[nID], "ab+");
	if (fp == NULL) {
		if (errno != 0) perror(NULL);
		return -1;
	}

	SYSTEMTIME stTime;
	::GetLocalTime((LPSYSTEMTIME)&stTime);
	snprintf(szBuff1, sizeof(szBuff1),
		"%04d/%02d/%02d, %02d:%02d:%02d.%03d, %s, " \
		"%s\r\n"
		, stTime.wYear
		, stTime.wMonth
		, stTime.wDay
		, stTime.wHour
		, stTime.wMinute
		, stTime.wSecond
		, stTime.wMilliseconds
		, logLevel(nLevel)
		, szBuff0);
	fputs(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		//::LeaveCriticalSection(&(m_stCS[nID]));
		unlock(nID);
		return -1;
	}
	//::LeaveCriticalSection(&(m_stCS[nID]));
	unlock(nID);
	return 0;
}

/**
 * @fn		debug
 * @brief
 * @param	[in]
 * @return
 */
int CLog::debug(int nID, int nLevel, const char* szFile, int nLine, const char* szFunc, const char* szFmt, va_list arg)
{
	if (szFmt == NULL || szFile == NULL || szFunc == NULL) {
		return -1;
	}

	if (nID < 0 || MAX_ID <= nID || m_bUsed[nID] == FALSE) {
#if _DEBUG
		assert(FALSE);
#endif
		return -1;
	}

	//::EnterCriticalSection(&(m_stCS[nID]));
	lock(nID);

	backupFile(nID);

	char szFilename[MAX_PATH];
	memset(szFilename, 0, sizeof(szFilename));

	char szBuff0[MAX_LOG_TEXT];
	char szBuff1[MAX_LOG_TEXT * 2];
	vsnprintf(szBuff0, sizeof(szBuff0), szFmt, arg);

	errno = 0;
	FILE *fp = fopen(m_szLogPath[nID], "ab+");
	if (fp == NULL) {
		if (errno != 0) perror(NULL);
		return -1;
	}

	SYSTEMTIME stTime;
	::GetLocalTime((LPSYSTEMTIME)&stTime);
	snprintf(szBuff1, sizeof(szBuff1),
		"%04d/%02d/%02d, %02d:%02d:%02d.%03d, %s, " \
		"%s(%d), %s, " \
		"%s\r\n"
		, stTime.wYear
		, stTime.wMonth
		, stTime.wDay
		, stTime.wHour
		, stTime.wMinute
		, stTime.wSecond
		, stTime.wMilliseconds
		, logLevel(nLevel)
		, getFnameFromPath(szFile, szFilename, sizeof(szFilename))
		, nLine
		, szFunc
		, szBuff0);
	fputs(szBuff1, fp);

	if (fclose(fp) != 0) {
		if (errno != 0) perror(NULL);
		//::LeaveCriticalSection(&(m_stCS[nID]));
		unlock(nID);
		return -1;
	}
	//::LeaveCriticalSection(&(m_stCS[nID]));
	unlock(nID);
	return 0;
}

/**
 * @fn		logLevel
 * @brief	���O���x���ɑΉ����閼�̂��擾
 * @param	[in]	int nLevel	: ���O���x��(LOG_LEVEL)
 * @return	���O���x������
 */
const char* CLog::logLevel(int nLevel)
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
 * @fn		getId
 * @brief	�g�p�\�ȃ��OID���擾����
 * @param	[in]	int nID		: �g�p���郍�OID (-1:�ŋ󂢂Ă���ID���擾)
 * @return	-1:���OID�擾���s(�󂫖���), 0�`:�g�p�\�ȃ��OID
 */
int CLog::getId(int nID)
{
	if (nID < -1 || MAX_ID <= nID) {
		return -1;
	}

	int id = -1;

	if (0 <= nID) {
		// �w��ID�͎g�p�\��
		if (m_bUsed[nID] == FALSE) {
			id = nID;
		}
	}
	else {
		// -1:�󂢂Ă���ID���猟��
		for (int i = 0; i < MAX_ID; i++) {
			if (m_bUsed[i] == FALSE) {
				id = i;
				break;
			}
		}
	}
	return id;
}

/**
 * @fn		getFilename
 * @brief	�t���p�X���t�@�C�����݂̂��擾
 * @param	[IN]	char* szPath	: �t�@�C���p�X������
 * @param	[OUT]	char* szBuff	: �t�@�C�������i�[����o�b�t�@�̈�
 * @param	[IN]	int nSize		: �o�b�t�@�̈�̃T�C�Y
 * @return	�o�b�t�@�̈�ւ̃|�C���^(�������s����"(NULL)"�̕������Ԃ�
 */
const char* CLog::getFnameFromPath(const char* szPath, char* szBuff, int nSize)
{
	if (szPath == NULL || szBuff == NULL) {
		return "(NULL)";
	}

	//char szDrv[MAX_PATH + 1];
	//char szDir[MAX_PATH + 1];
	char szFname[MAX_PATH + 1];
	char szExt[MAX_PATH + 1];

	//_splitpath(szPath, szDrv, szDir, szFname, szExt);
	_splitpath(szPath, NULL, NULL, szFname, szExt);
	snprintf(szBuff, nSize, "%s%s", szFname, szExt);

	return szBuff;
}

/**
 * @fn		_LockInit
 * @brief	�r������������
 * @param	[in]	int nID		 : ���OID
 */
inline void CLog::lockInit(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::InitializeCriticalSection(&(m_stCS[nID]));
	}
}

/**
 * @fn		_LockDelete
 * @brief	�r������������
 * @param	[in]	int nID		 : ���OID
 */
inline void CLog::lockDelete(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::DeleteCriticalSection(&(m_stCS[nID]));
	}
}

/**
 * @fn		_Lock
 * @brief	�r������������
 * @param	[in]	int nID		 : ���OID
 */
inline void CLog::lock(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::EnterCriticalSection(&(m_stCS[nID]));
	}
}

/**
 * @fn		_Unlock
 * @brief	�r������������
 * @param	[in]	int nID		 : ���OID
 */
inline void CLog::unlock(int nID)
{
	if (0 <= nID && nID < MAX_ID) {
		::LeaveCriticalSection(&(m_stCS[nID]));
	}
}

/**
 * @fn		copyFilePath
 * @brief	�w�胍�O�t�@�C�����Ƀo�b�N�A�b�v�ԍ���t�^����
 * @param	[in]	int nID				: ���OID
 * @param	[in]	const char* szPath	: ���O�o�͐�t�@�C���p�X
 */
int CLog::copyFilePath(int nID, const char* szPath)
{
	if (szPath == NULL || nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	char tmpDrive[MAX_PATH + 1];
	char tmpDir[MAX_PATH + 1];
	char tmpFname[MAX_PATH + 1];
	char tmpFext[MAX_PATH + 1];

	_splitpath(szPath, tmpDrive, tmpDir, tmpFname, tmpFext);

	memset(m_szDir[nID], 0, sizeof(m_szDir[nID]));
	memset(m_szFname[nID], 0, sizeof(m_szFname[nID]));
	memset(m_szFext[nID], 0, sizeof(m_szFext[nID]));
	memset(m_szLogPath[nID], 0, sizeof(m_szLogPath[nID]));

	snprintf(m_szDir[nID], sizeof(m_szDir[nID]), "%s%s", tmpDrive, tmpDir);
	snprintf(m_szFname[nID], sizeof(m_szFname[nID]), "%s", tmpFname);
	snprintf(m_szFext[nID], sizeof(m_szFext[nID]), "%s", tmpFext);

	// �t�@�C����_0.�g���q
	//snprintf(m_szLogPath[nID], sizeof(m_szLogPath[nID])
	//	, "%s%s_%d%s"
	//	, m_szDir[nID], m_szFname[nID], 0, m_szFext[nID]);
	int ret = getBackupName(nID, 0, m_szLogPath[nID], sizeof(m_szLogPath[nID]));

	return ret;
}

/**
 * @fn		createBackupName
 * @brief	�w�胍�O�t�@�C�����Ƀo�b�N�A�b�v�ԍ���t�^����
 * @param	[in]	int nID			: ���OID
 * @param	[in]	int nBkNo		: �o�b�N�A�b�v�ԍ�
 * @param	[in]	char* szBuff	: �o�͐�o�b�t�@�̈�
 * @param	[in]	int nSize		: �o�b�t�@�̈�̃T�C�Y
 * @return	0:����, -1:���s
 */
int CLog::getBackupName(int nID, int nBkNo, char* szBuff, int nSize)
{
	if (szBuff == NULL || nID < 0 || MAX_ID <= nID || nBkNo < 0 || m_nLogBackup[nID] < nBkNo)
	{
		return -1;
	}
	snprintf(szBuff, nSize, "%s%s_%d%s", m_szDir[nID], m_szFname[nID], nBkNo, m_szFext[nID]);
	return 0;
}

/**
 * @fn		backupFile
 * @brief	���O�t�@�C���o�b�N�A�b�v����
 * @param	[in]	int nID		:���OID
 * @return	0:����, -1:���s
 */
int CLog::backupFile(int nID)
{
	if (nID < 0 || MAX_ID <= nID) {
		return -1;
	}

	//lock(nID);

	long fsize = 0;
	if ((fsize = getFileSize(m_szLogPath[nID])) < 0) {
		//unlock(nID);
		return 0;
	}

	char szBefore[MAX_PATH + 1];
	char szAfter[MAX_PATH + 1];

	if (m_nFileSize[nID] <= fsize) {
		// �t�@�C���o�b�N�A�b�v����
		// �ł��Â��t�@�C���͍폜
		getBackupName(nID, m_nLogBackup[nID], szBefore, sizeof(szBefore));
		if (::PathFileExists(szBefore) && !::PathIsDirectory(szBefore)) {
			::DeleteFile(szBefore);
		}
		// �t�@�C�������l�[���A1���Â��炷
		for (int nBkNo = m_nLogBackup[nID] - 1; 0 <= nBkNo; nBkNo--) {
			getBackupName(nID, nBkNo, szBefore, sizeof(szBefore));
			if (::PathFileExists(szBefore) && !::PathIsDirectory(szBefore)) {
				getBackupName(nID, nBkNo + 1, szAfter, sizeof(szBefore));
				::MoveFile(szBefore, szAfter);
			}
		}
	}

	//unlock(nID);
	return 0;
}

/**
 * @fn		getFileSize
 * @brief	�t�@�C���T�C�Y�擾(2GB�܂�)
 * @param	[in]	char* szPath		: �t�@�C���p�X
 * @return	�t�@�C���T�C�Y(-1:���s)
 */
long CLog::getFileSize(const char* szPath)
{
	if (szPath == NULL) {
		return -1;
	}

	//errno = 0;
	FILE *fp = fopen(szPath, "rb");
	if (fp == NULL) {
		//if (errno != 0) perror(NULL);
		return -1;
	}

	long fsize = _filelength(_fileno(fp));
	fsize >>= 10;		// fsize/1024

	if (fclose(fp) != 0) {
		//if (errno != 0) perror(NULL);
		return -1;
	}
	return fsize;
}


//LOG_START(0, "log_test_0.log");
//LOG_START(1, "log_test_1.log");
//
//LOG_WRITE(0, INF, "start %d.", 0);
//LOG_WRITE(1, INF, "start %d.", 1);
//LOG_DEBUG(0, INF, "end %d.", 0);
//LOG_DEBUG(1, INF, "end 1.");
//
//LOG_END(1);
//LOG_END();
