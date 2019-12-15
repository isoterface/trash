/**
 * @file	IniFile.h
 * @brief	INI�t�@�C���A�N�Z�X
 * @author	?
 * @date	?
 */
#pragma once

#include "windows.h"
#include "shlwapi.h"
#include "tchar.h"
#include "errno.h"

#pragma comment(lib, "shlwapi.lib")

#define		INI_KEY_BUFF_SIZE		(64)

/**
 * @class	CPathInfo
 * @brief	�t�@�C���p�X����͗p�⏕�N���X
 */
static class CPathInfo
{
public:
	TCHAR szFullPath[MAX_PATH + 1];		//! �t���p�X���
	TCHAR szDrive[MAX_PATH + 1];		//! �h���C�u
	TCHAR szDir[MAX_PATH + 1];			//! �t�H���_�p�X
	TCHAR szFileBase[MAX_PATH + 1];		//! �t�@�C����
	TCHAR szFileExt[MAX_PATH + 1];		//! �t�@�C���g���q

public:
	/**
	 * �R���X�g���N�^
	 */
	CPathInfo()
	{
		ClearAll();
	}
	/**
	 * @fn		ClearAll
	 * @brief	�p�X��񏉊���
	 */
	void ClearAll()
	{
		memset(szFullPath, 0, sizeof(szFullPath));
		memset(szDrive, 0, sizeof(szDrive));
		memset(szDir, 0, sizeof(szDir));
		memset(szFileBase, 0, sizeof(szFileBase));
		memset(szFileExt, 0, sizeof(szFileExt));
	}
	/**
	 * @fn		SplitPath
	 * @brief	�����Ɏw�肳�ꂽ�p�X���e�P�ʂɕ���
	 * @param	[in]	LPCTSTR lpszPath	: �t�@�C���p�X
	 * @return	TRUE:����, FALSE:���s
	 */
	BOOL SplitPath(LPCTSTR lpszPath)
	{
		if (lpszPath == NULL) {
			return FALSE;
		}

		ClearAll();
		_tcsncpy(szFullPath, lpszPath, MAX_PATH);
		_tsplitpath(szFullPath, szDrive, szDir, szFileBase, szFileExt);
		return TRUE;
	}
	/**
	 * @fn		SplitPath
	 * @brief	�����o�ϐ� szFullPath �ɕێ����Ă���p�X�����e�P�ʂɕ���
	 * @return	TRUE:����
	 */
	BOOL SplitPath()
	{
		_tsplitpath(szFullPath, szDrive, szDir, szFileBase, szFileExt);
		return TRUE;
	}
};


/**
 * @class	CIniFile
 * @brief	INI�t�@�C���A�N�Z�X�N���X
 */
class CIniFile
{
private:
	TCHAR	m_szFile[MAX_PATH + 1];			//! INI�t�@�C���p�X
	BOOL	m_bInit;						//! �������ς݃t���O

public:
	CIniFile(LPCTSTR lpszPath, BOOL bCreate = TRUE);
	~CIniFile();

	BOOL	SetPath(LPCTSTR lpszPath, BOOL bCreate = TRUE);
	BOOL	GetPath(LPTSTR lpszBuff, DWORD dwSize);

	DWORD	ReadString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszDefault, LPTSTR lpszStr, DWORD dwSize);
	LONG	ReadInt(LPCTSTR lpszSection, LPCTSTR lpszKey, INT nDefault);
	UINT	ReadHex(LPCTSTR lpszSection, LPCTSTR lpszKey, UINT uiDefault);
	DOUBLE	ReadDouble(LPCTSTR lpszSection, LPCTSTR lpszKey, DOUBLE dDefault);

	BOOL	WriteString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszStr);
	BOOL	WriteInt(LPCTSTR lpszSection, LPCTSTR lpszKey, LONG nVal);
	BOOL	WriteHex(LPCTSTR lpszSection, LPCTSTR lpszKey, UINT uiVal);
	BOOL	WriteDouble(LPCTSTR lpszSection, LPCTSTR lpszKey, DOUBLE dVal);
};


/**
 * @brief	�R���X�g���N�^
 * @param	[in]	LPCTSTR lpszPath	: INI�t�@�C���p�X
 * @param	[in]	BOOL bCreate		: �Ώ�INI�t�@�C�������݂��Ȃ��ꍇ�ɍ쐬���邩�ǂ���(TRUE:�쐬����, FALSE:�쐬���Ȃ�)
 */
CIniFile::CIniFile(LPCTSTR lpszPath, BOOL bCreate/*=TRUE*/) :
	m_bInit(FALSE)
{
	// �p�X�w�� �ȉ��̌`�����l��
	// ��΁@CIniFile cIni0(_T("C:\\My Documents\\Program\\Config\\IniFile.ini"));
	// ���΁@CIniFile cIni1(_T("Config\\IniFile.ini"));
	// �t�@�C���̂݁@CIniFile cIni2(_T("IniFile.ini"));
	// TODO:���΃p�X�w�莞�A".\Dir\File..."�Ƃ����`���͍l�����Ă��Ȃ�

	SetPath(lpszPath, bCreate);
}


/**
 * @brief	�f�X�g���N�^
 */
CIniFile::~CIniFile()
{
}


/**
 * @fn		SetPath
 * @brief	INI�t�@�C���̃p�X��ݒ肷��
 * @param	[in]	LPCTSTR lpszPath	: INI�t�@�C���p�X
 * @param	[in]	BOOL bCreate		: �Ώ�INI�t�@�C�������݂��Ȃ��ꍇ�ɍ쐬���邩�ǂ���(TRUE:�쐬����, FALSE:�쐬���Ȃ�)
 * @return�@TRUE:����, FALSE:���s
 */
BOOL CIniFile::SetPath(LPCTSTR lpszPath, BOOL bCreate/*=TRUE*/)
{
	if (lpszPath == NULL) {
		return FALSE;
	}

	LPCTSTR lpszWork = lpszPath;
	CPathInfo cPathIni;
	CPathInfo cPathCurr;
	TCHAR szWork[MAX_PATH + 1];
	memset(szWork, 0, sizeof(szWork));

	if (!cPathIni.SplitPath(lpszWork)) {
		return FALSE;
	}

	if (_tcslen(cPathIni.szFileBase) <= 0) {
		return FALSE;		// �G���[�F�t�@�C�����w��Ȃ�
	}
	if (_tcslen(cPathIni.szDrive) <= 0) {
		// ���΃p�X�w��F���s�t�@�C���̃p�X�����Q��
		if (::GetModuleFileName(NULL, cPathCurr.szFullPath, sizeof(cPathCurr.szFullPath)) == 0) {
			return FALSE;		// ���s�t�@�C���̃p�X�擾���s
		}
		cPathCurr.SplitPath();

		// ���s�t�@�C���p�X�Ƒ��΃p�X�ƃt�@�C��������������
		_sntprintf(szWork, MAX_PATH, _T("%s%s%s%s%s")
			, cPathCurr.szDrive
			, cPathCurr.szDir
			, cPathIni.szDir
			, cPathIni.szFileBase
			, cPathIni.szFileExt);
		lpszWork = szWork;
	}

	if (::PathIsDirectory(lpszWork)) {
		return FALSE;		// �w��p�X�̓t�@�C���ł͂Ȃ�
	}
	if (!::PathFileExists(lpszWork) && !bCreate) {
		return FALSE;		// �V�K�쐬�s��
	}

	memset(m_szFile, 0, sizeof(m_szFile));
	_tcsncpy(m_szFile, lpszWork, MAX_PATH);
	m_bInit = TRUE;

	return TRUE;
}


/**
 * @fn		GetPath
 * @brief	�Ώ�INI�t�@�C���̃p�X���擾����
 * @param	[in]	LPTSTR lpszBuff		: �擾��o�b�t�@�̈�
 * @param	[in]	DWORD dwSize		: �o�b�t�@�̈�T�C�Y
 * @return�@TRUE:����, FALSE:���s
 */
BOOL CIniFile::GetPath(LPTSTR lpszBuff, DWORD dwSize)
{
	if (!m_bInit) {
		return FALSE;
	}
	if (_tcslen(m_szFile) <= 0) {
		return FALSE;
	}
	_tcsncpy(lpszBuff, m_szFile, dwSize);

	return TRUE;
}


/**
 * @fn		ReadString
 * @brief	�ΏۃL�[�̕������ǂݍ���
 * @param	[in]	LPCTSTR lpszSection		: �Z�N�V����
 * @param	[in]	LPCTSTR lpszKey			: �L�[
 * @param	[in]	LPCTSTR lpszDefault		: �f�t�H���g�l
 * @param	[out]	LPTSTR lpszStr			: �ǂݍ��ސ�̃o�b�t�@�̈�
 * @param	[in]	DWORD dwSize			: �o�b�t�@�̈�T�C�Y
 * @return	0:���sor�ǂݎ�蕶����Ȃ�, 1�`:�ǂݎ����������
 */
DWORD CIniFile::ReadString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszDefault, LPTSTR lpszStr, DWORD dwSize)
{
	if (!m_bInit) {
		return 0;
	}
	if (lpszSection == NULL || lpszKey == NULL || lpszDefault == NULL || lpszStr == NULL) {
		return 0;
	}

	DWORD dwCount = ::GetPrivateProfileString(lpszSection, lpszKey, lpszDefault, lpszStr, dwSize, m_szFile);

	return dwCount;
}


/**
 * @fn		ReadInt
 * @brief	�ΏۃL�[�̐����l��ǂݍ���(�����Ή�)
 * @param	[in]	LPCTSTR lpszSection		: �Z�N�V����
 * @param	[in]	LPCTSTR lpszKey			: �L�[
 * @param	[in]	INT nDefault			: �f�t�H���g�l
 * @return	�ǂݎ�������l(���s�̏ꍇ��0���Ԃ�)
 */
LONG CIniFile::ReadInt(LPCTSTR lpszSection, LPCTSTR lpszKey, INT nDefault)
{
	if (!m_bInit) {
		return 0;
	}
	if (lpszSection == NULL || lpszKey == NULL) {
		return 0;
	}

	TCHAR szBuff[INI_KEY_BUFF_SIZE];
	TCHAR** endptr = NULL;
	LONG lRet = 0;

	memset(szBuff, 0, sizeof(szBuff));

	if (ReadString(lpszSection, lpszKey, _T(""), szBuff, INI_KEY_BUFF_SIZE) == 0) {
		return nDefault;
	}

	errno = 0;
	lRet = _tcstol(szBuff, endptr, 10);
	if (endptr != NULL || errno == ERANGE) {
		return nDefault;
	}

	return lRet;
}


/**
 * @fn		ReadHex
 * @brief	�ΏۃL�[��16�i���l��ǂݍ���
 * @param	[in]	LPCTSTR lpszSection		: �Z�N�V����
 * @param	[in]	LPCTSTR lpszKey			: �L�[
 * @param	[in]	UINT uiDefault			: �f�t�H���g�l
 * @return	�ǂݎ�������l(���s�̏ꍇ��0���Ԃ�)
 */
UINT CIniFile::ReadHex(LPCTSTR lpszSection, LPCTSTR lpszKey, UINT uiDefault)
{
	if (!m_bInit) {
		return 0;
	}
	if (lpszSection == NULL || lpszKey == NULL) {
		return 0;
	}

	TCHAR szBuff[INI_KEY_BUFF_SIZE];
	TCHAR** endptr = NULL;
	UINT uiRet = 0;

	memset(szBuff, 0, sizeof(szBuff));

	if (ReadString(lpszSection, lpszKey, _T(""), szBuff, INI_KEY_BUFF_SIZE) == 0) {
		return uiDefault;
	}

	if (_tcslen(szBuff) < 3 || szBuff[0] != '0' || szBuff[1] != 'x') {
		return 0;		// 16�i�ړ����ł͂Ȃ�
	}

	errno = 0;
	uiRet = _tcstoul(&szBuff[2], endptr, 16);
	if (endptr != NULL || errno == ERANGE) {
		return uiDefault;
	}

	return uiRet;
}


/**
 * @fn		ReadDouble
 * @brief	�ΏۃL�[�̎����l��ǂݍ���
 * @param	[in]	LPCTSTR lpszSection		: �Z�N�V����
 * @param	[in]	LPCTSTR lpszKey			: �L�[
 * @param	[in]	DOUBLE dDefault			: �f�t�H���g�l
 * @return	�ǂݎ�������l(���s�̏ꍇ��0���Ԃ�)
 */
DOUBLE CIniFile::ReadDouble(LPCTSTR lpszSection, LPCTSTR lpszKey, DOUBLE dDefault)
{
	// �ۂߒ���

	if (!m_bInit) {
		return 0;
	}
	if (lpszSection == NULL || lpszKey == NULL) {
		return 0;
	}

	TCHAR szBuff[INI_KEY_BUFF_SIZE];
	TCHAR** endptr = NULL;
	DOUBLE dRet = 0;

	memset(szBuff, 0, sizeof(szBuff));

	if (ReadString(lpszSection, lpszKey, _T(""), szBuff, INI_KEY_BUFF_SIZE) == 0) {
		return dDefault;
	}

	errno = 0;
	dRet = _tcstod(szBuff, endptr);
	if (endptr != NULL || errno == ERANGE) {
		return dDefault;
	}

	return dRet;
}


/**
 * @fn		WriteString
 * @brief	�ΏۃL�[�ɕ�����f�[�^��ۑ�����
 * @param	[in]	LPCTSTR lpszSection		: �Z�N�V����
 * @param	[in]	LPCTSTR lpszKey			: �L�[
 * @param	[in]	LPCTSTR lpszStr			: ������f�[�^
 * @return TRUE:����, FALSE:���s
 */
BOOL CIniFile::WriteString(LPCTSTR lpszSection, LPCTSTR lpszKey, LPCTSTR lpszStr)
{
	if (!m_bInit) {
		return FALSE;
	}
	if (lpszSection == NULL || lpszKey == NULL || lpszStr == NULL) {
		return FALSE;
	}

	if (::WritePrivateProfileString(lpszSection, lpszKey, lpszStr, m_szFile) == 0) {
		return FALSE;
	}

	return TRUE;
}
/**
 * @fn		WriteInt
 * @brief	�ΏۃL�[�ɐ����l�f�[�^��ۑ�����
 * @param	[in]	LPCTSTR lpszSection		: �Z�N�V����
 * @param	[in]	LPCTSTR lpszKey			: �L�[
 * @param	[in]	LONG lVal				: �����l�f�[�^
 * @return TRUE:����, FALSE:���s
 */
BOOL CIniFile::WriteInt(LPCTSTR lpszSection, LPCTSTR lpszKey, LONG lVal)
{
	TCHAR szBuff[INI_KEY_BUFF_SIZE];

	memset(szBuff, 0, sizeof(szBuff));
	_sntprintf(szBuff, INI_KEY_BUFF_SIZE, _T("%ld"), lVal);

	return WriteString(lpszSection, lpszKey, szBuff);
}
/**
 * @fn		WriteHex
 * @brief	�ΏۃL�[��16�i���l�f�[�^��ۑ�����
 * @param	[in]	LPCTSTR lpszSection		: �Z�N�V����
 * @param	[in]	LPCTSTR lpszKey			: �L�[
 * @param	[in]	UINT uiVal				: 16�i���l�f�[�^
 * @return TRUE:����, FALSE:���s
 */
BOOL CIniFile::WriteHex(LPCTSTR lpszSection, LPCTSTR lpszKey, UINT uiVal)
{
	TCHAR szBuff[INI_KEY_BUFF_SIZE];

	memset(szBuff, 0, sizeof(szBuff));
	_sntprintf(szBuff, INI_KEY_BUFF_SIZE, _T("0x%X"), uiVal);

	return WriteString(lpszSection, lpszKey, szBuff);
}
/**
 * @fn		WriteHex
 * @brief	�ΏۃL�[�Ɏ����l�f�[�^��ۑ�����
 * @param	[in]	LPCTSTR lpszSection		: �Z�N�V����
 * @param	[in]	LPCTSTR lpszKey			: �L�[
 * @param	[in]	DOUBLE dVal				: �����l�f�[�^
 * @return TRUE:����, FALSE:���s
 */
BOOL CIniFile::WriteDouble(LPCTSTR lpszSection, LPCTSTR lpszKey, DOUBLE dVal)
{
	// �ۂߒ���

	TCHAR szBuff[INI_KEY_BUFF_SIZE];

	memset(szBuff, 0, sizeof(szBuff));
	_sntprintf(szBuff, INI_KEY_BUFF_SIZE, _T("%lf"), dVal);

	return WriteString(lpszSection, lpszKey, szBuff);
}

//void CMFDlgApp7Dlg::TestFunc03()
//{
//	CIniFile cIni(_T("TestIni.ini"));
//
//	cIni.WriteString(_T("SECT_STR"), _T("KEY1"), _T("STR1"));
//	cIni.WriteInt(_T("SECT_INT"), _T("KEY1"), LONG_MAX);
//	cIni.WriteInt(_T("SECT_INT"), _T("KEY2"), LONG_MIN);
//	cIni.WriteHex(_T("SECT_HEX"), _T("KEY1"), UINT_MAX);
//	cIni.WriteHex(_T("SECT_HEX"), _T("KEY2"), 0);
//	cIni.WriteDouble(_T("SECT_DOUBLE"), _T("KEY1"), 1.23456789);
//	cIni.WriteDouble(_T("SECT_DOUBLE"), _T("KEY2"), -1.23456789);
//
//	TCHAR szBuff[256];
//	memset(szBuff, 0, sizeof(szBuff));
//
//	cIni.ReadString(_T("SECT_STR"), _T("KEY1"), _T("ReadString error!"), szBuff, sizeof(szBuff));
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, szBuff);
//
//	CString cs;
//	LONG i1 = cIni.ReadInt(_T("SECT_INT"), _T("KEY1"), 0);
//	LONG i2 = cIni.ReadInt(_T("SECT_INT"), _T("KEY2"), 0);
//	cs.Format(_T("ReadInt i1 = %ld"), i1);
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, cs);
//	cs.Format(_T("ReadInt i2 = %ld"), i2);
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, cs);
//
//	UINT h1 = cIni.ReadHex(_T("SECT_HEX"), _T("KEY1"), 0xAAAA);
//	UINT h2 = cIni.ReadHex(_T("SECT_HEX"), _T("KEY2"), 0xAAAA);
//	cs.Format(_T("ReadHex h1 = %X"), h1);
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, cs);
//	cs.Format(_T("ReadHex h2 = %X"), h2);
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, cs);
//
//	DOUBLE d1 = cIni.ReadDouble(_T("SECT_DOUBLE"), _T("KEY1"), 0);
//	DOUBLE d2 = cIni.ReadDouble(_T("SECT_DOUBLE"), _T("KEY2"), 0);
//	cs.Format(_T("ReadDouble d1 = %lf"), d1);
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, cs);
//	cs.Format(_T("ReadDouble d2 = %lf"), d2);
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, cs);
//}
