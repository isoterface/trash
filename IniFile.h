#pragma once

#include "windows.h"
#include "shlwapi.h"
#include "tchar.h"
#include "errno.h"

#pragma comment(lib, "shlwapi.lib")

//namespace MyHelper
//{
#define		INI_KEY_BUFF_SIZE		(64)


//typedef struct {
//	TCHAR szFullPath[MAX_PATH + 1];
//	TCHAR szDrive[MAX_PATH + 1];
//	TCHAR szDir[MAX_PATH + 1];
//	TCHAR szFileBase[MAX_PATH + 1];
//	TCHAR szFileExt[MAX_PATH + 1];
//} ST_FILE_PATH_INFO;

class CPathInfo
{
public:
	TCHAR szFullPath[MAX_PATH + 1];
	TCHAR szDrive[MAX_PATH + 1];
	TCHAR szDir[MAX_PATH + 1];
	TCHAR szFileBase[MAX_PATH + 1];
	TCHAR szFileExt[MAX_PATH + 1];

public:
	CPathInfo()
	{
		ClearAll();
	}
	void ClearAll()
	{
		memset(szFullPath, 0, sizeof(szFullPath));
		memset(szDrive, 0, sizeof(szDrive));
		memset(szDir, 0, sizeof(szDir));
		memset(szFileBase, 0, sizeof(szFileBase));
		memset(szFileExt, 0, sizeof(szFileExt));
	}
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
	BOOL SplitPath()
	{
		_tsplitpath(szFullPath, szDrive, szDir, szFileBase, szFileExt);
		return TRUE;
	}

};


class CIniFile
{
private:
	TCHAR	m_szFile[MAX_PATH + 1];
	BOOL	m_bInit;

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


// パス指定 以下の形式を考慮
// 絶対　CIniFile cIni0(_T("C:\\My Documents\\Program\\Config\\IniFile.ini"));
// 相対　CIniFile cIni1(_T("Config\\IniFile.ini"));
// ファイルのみ　CIniFile cIni2(_T("IniFile.ini"));
// TODO:相対パス指定時、".\Dir\File..."という形式は考慮していない
CIniFile::CIniFile(LPCTSTR lpszPath, BOOL bCreate/*=TRUE*/) :
	m_bInit(FALSE)
{
	SetPath(lpszPath, bCreate);
}


CIniFile::~CIniFile()
{
}


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
		return FALSE;		// エラー：ファイル名指定なし
	}
	if (_tcslen(cPathIni.szDrive) <= 0) {
		// 相対パス指定：実行ファイルのパス情報を参照
		if (::GetModuleFileName(NULL, cPathCurr.szFullPath, sizeof(cPathCurr.szFullPath)) == 0) {
			return FALSE;		// 実行ファイルのパス取得失敗
		}
		cPathCurr.SplitPath();

		// 実行ファイルパスと相対パスとファイル名を結合する
		_sntprintf(szWork, MAX_PATH, _T("%s%s%s%s%s")
			, cPathCurr.szDrive
			, cPathCurr.szDir
			, cPathIni.szDir
			, cPathIni.szFileBase
			, cPathIni.szFileExt);
		lpszWork = szWork;
	}

	if (::PathIsDirectory(lpszWork)) {
		return FALSE;		// 指定パスはファイルではない
	}
	if (!::PathFileExists(lpszWork) && !bCreate) {
		return FALSE;		// 新規作成不可
	}

	memset(m_szFile, 0, sizeof(m_szFile));
	_tcsncpy(m_szFile, lpszWork, MAX_PATH);
	m_bInit = TRUE;

	return TRUE;
}


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

	lRet = _tcstol(szBuff, endptr, 10);
	if (endptr != NULL || errno == ERANGE) {
		return nDefault;
	}

	return lRet;
}


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

	uiRet = _tcstoul(szBuff, endptr, 16);
	if (endptr != NULL || errno == ERANGE) {
		return uiDefault;
	}

	return uiRet;
}


DOUBLE CIniFile::ReadDouble(LPCTSTR lpszSection, LPCTSTR lpszKey, DOUBLE dDefault)
{
	// 丸め注意

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

	dRet = _tcstod(szBuff, endptr);
	if (endptr != NULL || errno == ERANGE) {
		return dDefault;
	}

	return dRet;
}


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
BOOL CIniFile::WriteInt(LPCTSTR lpszSection, LPCTSTR lpszKey, LONG lVal)
{
	TCHAR szBuff[INI_KEY_BUFF_SIZE];

	memset(szBuff, 0, sizeof(szBuff));
	_sntprintf(szBuff, INI_KEY_BUFF_SIZE, _T("%ld"), lVal);

	return WriteString(lpszSection, lpszKey, szBuff);
}
BOOL CIniFile::WriteHex(LPCTSTR lpszSection, LPCTSTR lpszKey, UINT uiVal)
{
	TCHAR szBuff[INI_KEY_BUFF_SIZE];

	memset(szBuff, 0, sizeof(szBuff));
	_sntprintf(szBuff, INI_KEY_BUFF_SIZE, _T("%X"), uiVal);

	return WriteString(lpszSection, lpszKey, szBuff);
}
BOOL CIniFile::WriteDouble(LPCTSTR lpszSection, LPCTSTR lpszKey, DOUBLE dVal)
{
	// 丸め注意

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

//}