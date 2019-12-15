/**
 * @file	IniFile.h
 * @brief	INIファイルアクセス
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
 * @brief	ファイルパス情報解析用補助クラス
 */
static class CPathInfo
{
public:
	TCHAR szFullPath[MAX_PATH + 1];		//! フルパス情報
	TCHAR szDrive[MAX_PATH + 1];		//! ドライブ
	TCHAR szDir[MAX_PATH + 1];			//! フォルダパス
	TCHAR szFileBase[MAX_PATH + 1];		//! ファイル名
	TCHAR szFileExt[MAX_PATH + 1];		//! ファイル拡張子

public:
	/**
	 * コンストラクタ
	 */
	CPathInfo()
	{
		ClearAll();
	}
	/**
	 * @fn		ClearAll
	 * @brief	パス情報初期化
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
	 * @brief	引数に指定されたパスを各単位に分割
	 * @param	[in]	LPCTSTR lpszPath	: ファイルパス
	 * @return	TRUE:成功, FALSE:失敗
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
	 * @brief	メンバ変数 szFullPath に保持しているパス情報を各単位に分割
	 * @return	TRUE:成功
	 */
	BOOL SplitPath()
	{
		_tsplitpath(szFullPath, szDrive, szDir, szFileBase, szFileExt);
		return TRUE;
	}
};


/**
 * @class	CIniFile
 * @brief	INIファイルアクセスクラス
 */
class CIniFile
{
private:
	TCHAR	m_szFile[MAX_PATH + 1];			//! INIファイルパス
	BOOL	m_bInit;						//! 初期化済みフラグ

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
 * @brief	コンストラクタ
 * @param	[in]	LPCTSTR lpszPath	: INIファイルパス
 * @param	[in]	BOOL bCreate		: 対象INIファイルが存在しない場合に作成するかどうか(TRUE:作成する, FALSE:作成しない)
 */
CIniFile::CIniFile(LPCTSTR lpszPath, BOOL bCreate/*=TRUE*/) :
	m_bInit(FALSE)
{
	// パス指定 以下の形式を考慮
	// 絶対　CIniFile cIni0(_T("C:\\My Documents\\Program\\Config\\IniFile.ini"));
	// 相対　CIniFile cIni1(_T("Config\\IniFile.ini"));
	// ファイルのみ　CIniFile cIni2(_T("IniFile.ini"));
	// TODO:相対パス指定時、".\Dir\File..."という形式は考慮していない

	SetPath(lpszPath, bCreate);
}


/**
 * @brief	デストラクタ
 */
CIniFile::~CIniFile()
{
}


/**
 * @fn		SetPath
 * @brief	INIファイルのパスを設定する
 * @param	[in]	LPCTSTR lpszPath	: INIファイルパス
 * @param	[in]	BOOL bCreate		: 対象INIファイルが存在しない場合に作成するかどうか(TRUE:作成する, FALSE:作成しない)
 * @return　TRUE:成功, FALSE:失敗
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


/**
 * @fn		GetPath
 * @brief	対象INIファイルのパスを取得する
 * @param	[in]	LPTSTR lpszBuff		: 取得先バッファ領域
 * @param	[in]	DWORD dwSize		: バッファ領域サイズ
 * @return　TRUE:成功, FALSE:失敗
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
 * @brief	対象キーの文字列を読み込む
 * @param	[in]	LPCTSTR lpszSection		: セクション
 * @param	[in]	LPCTSTR lpszKey			: キー
 * @param	[in]	LPCTSTR lpszDefault		: デフォルト値
 * @param	[out]	LPTSTR lpszStr			: 読み込む先のバッファ領域
 * @param	[in]	DWORD dwSize			: バッファ領域サイズ
 * @return	0:失敗or読み取り文字列なし, 1～:読み取った文字数
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
 * @brief	対象キーの整数値を読み込む(負数対応)
 * @param	[in]	LPCTSTR lpszSection		: セクション
 * @param	[in]	LPCTSTR lpszKey			: キー
 * @param	[in]	INT nDefault			: デフォルト値
 * @return	読み取った数値(失敗の場合は0が返る)
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
 * @brief	対象キーの16進数値を読み込む
 * @param	[in]	LPCTSTR lpszSection		: セクション
 * @param	[in]	LPCTSTR lpszKey			: キー
 * @param	[in]	UINT uiDefault			: デフォルト値
 * @return	読み取った数値(失敗の場合は0が返る)
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
		return 0;		// 16進接頭辞ではない
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
 * @brief	対象キーの実数値を読み込む
 * @param	[in]	LPCTSTR lpszSection		: セクション
 * @param	[in]	LPCTSTR lpszKey			: キー
 * @param	[in]	DOUBLE dDefault			: デフォルト値
 * @return	読み取った数値(失敗の場合は0が返る)
 */
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

	errno = 0;
	dRet = _tcstod(szBuff, endptr);
	if (endptr != NULL || errno == ERANGE) {
		return dDefault;
	}

	return dRet;
}


/**
 * @fn		WriteString
 * @brief	対象キーに文字列データを保存する
 * @param	[in]	LPCTSTR lpszSection		: セクション
 * @param	[in]	LPCTSTR lpszKey			: キー
 * @param	[in]	LPCTSTR lpszStr			: 文字列データ
 * @return TRUE:成功, FALSE:失敗
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
 * @brief	対象キーに整数値データを保存する
 * @param	[in]	LPCTSTR lpszSection		: セクション
 * @param	[in]	LPCTSTR lpszKey			: キー
 * @param	[in]	LONG lVal				: 整数値データ
 * @return TRUE:成功, FALSE:失敗
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
 * @brief	対象キーに16進数値データを保存する
 * @param	[in]	LPCTSTR lpszSection		: セクション
 * @param	[in]	LPCTSTR lpszKey			: キー
 * @param	[in]	UINT uiVal				: 16進数値データ
 * @return TRUE:成功, FALSE:失敗
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
 * @brief	対象キーに実数値データを保存する
 * @param	[in]	LPCTSTR lpszSection		: セクション
 * @param	[in]	LPCTSTR lpszKey			: キー
 * @param	[in]	DOUBLE dVal				: 実数値データ
 * @return TRUE:成功, FALSE:失敗
 */
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
