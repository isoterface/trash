/**
 * @file	MyHelper.h
 * @brief	éGä÷êî
 * @author	?
 * @date	?
 */
#pragma once

#include "stdafx.h"

//namespace MyHelper
//{
#define		HEX2CHR(c)					((0xA <= (c)) ? ((c) + 0x37) : ((c) + 0x30))
#define		COUNT_OF_ARRAY(a, t)		(sizeof(a)/sizeof(t))

class CMyHelper
{
public:
	CMyHelper();
	~CMyHelper();

	static int GetRadioValue(CButton* apcRadio[], int cntRadio);
	static int GetRadioValue(CWnd* pcWnd, int anID[], int cntRadio);

	static int CEditWriteText(CEdit &cEdit, CString &cStrText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);
	static int CEditWriteText(CEdit &cEdit, LPCTSTR lpszText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);
	static int CEditWriteText(CEdit &cEdit, LPCSTR lpszText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);

	static int CEditWriteText(CWnd* pcWnd, int nID, CString &cStrText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);
	static int CEditWriteText(CWnd* pcWnd, int nID, LPCTSTR lpszText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);
	static int CEditWriteText(CWnd* pcWnd, int nID, LPCSTR lpszText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);

private:
	static int ceditWriteText(CEdit &cEdit, CString &cStrText, BOOL bAppend, BOOL bNewLine);
	static int ceditWriteText(CWnd* pcWnd, int nID, CString &cStrText, BOOL bAppend, BOOL bNewLine);

public:
	static int OpenFileDialog(CStringArray &cStrArrPath, BOOL bMulti = FALSE, int nFiles = 99);
	static int MemDump(void* pData, int nByteLen, CString &cStrDump);
	static int MemDump(void* pData, int nByteLen, char* pszDest, int nDestLen);

};


CMyHelper::CMyHelper()
{
}


CMyHelper::~CMyHelper()
{
}


/**
 * @fn		GetRadioValue
 * @brief	
 * @param	[in]	
 * @return	
 */
int CMyHelper::GetRadioValue(CButton* apcRadio[], int cntRadio)
{
	int ret = -1;

	if (apcRadio == NULL) {
		return -1;
	}

	for (int i = 0; i < cntRadio; i++) {
		if (apcRadio[i] == NULL) {
			return -1;
		}

		if (apcRadio[i]->GetCheck() == BST_CHECKED) {
			ret = i;
		}
	}

	return ret;
}
/**
 * @fn		GetRadioValue
 * @brief
 * @param	[in]
 * @return
 */
int CMyHelper::GetRadioValue(CWnd* pcWnd, int anID[], int cntRadio)
{
	int ret = -1;

	if (pcWnd == NULL || anID == NULL) {
		return -1;
	}

	CButton* pcb;

	for (int i = 0; i < cntRadio; i++) {
		pcb = (CButton*)(pcWnd->GetDlgItem(anID[i]));
		if (pcb == NULL) {
			return -1;
		}

		if (pcb->GetCheck() == BST_CHECKED) {
			ret = i;
		}
	}

	return ret;
}


/**
 * @fn		CEditWriteText
 * @brief
 * @param	[in]
 * @return
 */
int CMyHelper::CEditWriteText(CEdit &cEdit, CString &cStrText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return ceditWriteText(cEdit, cStrText, bAppend, bNewLine);
}
int CMyHelper::CEditWriteText(CEdit &cEdit, LPCTSTR lpszText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return ceditWriteText(cEdit, CString(lpszText), bAppend, bNewLine);
}
int CMyHelper::CEditWriteText(CEdit &cEdit, LPCSTR lpszText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return ceditWriteText(cEdit, CString(lpszText), bAppend, bNewLine);
}
int CMyHelper::CEditWriteText(CWnd* pcWnd, int nID, CString &cStrText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return ceditWriteText(pcWnd, nID, cStrText, bAppend, bNewLine);
}
int CMyHelper::CEditWriteText(CWnd* pcWnd, int nID, LPCTSTR lpszText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return ceditWriteText(pcWnd, nID, CString(lpszText), bAppend, bNewLine);
}
int CMyHelper::CEditWriteText(CWnd* pcWnd, int nID, LPCSTR lpszText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return ceditWriteText(pcWnd, nID, CString(lpszText), bAppend, bNewLine);
}
/**
 * @fn		ceditWriteText
 * @brief
 * @param	[in]
 * @return
 */
int CMyHelper::ceditWriteText(CEdit &cEdit, CString &cStrText, BOOL bAppend, BOOL bNewLine)
{
	CString cStrWork = _T("");

	if (bAppend) {
		cEdit.GetWindowText(cStrWork);
	}
	if (bNewLine) {
		cStrWork += (cStrText + _T("\r\n"));
	}
	else {
		cStrWork += (cStrText);
	}
	cEdit.SetWindowText(cStrWork);

	return 0;
}
/**
 * @fn		ceditWriteText
 * @brief
 * @param	[in]
 * @return
 */
int CMyHelper::ceditWriteText(CWnd* pcWnd, int nID, CString &cStrText, BOOL bAppend, BOOL bNewLine)
{
	if (pcWnd == NULL) {
		return -1;
	}

	CEdit* pcEdit = (CEdit*)(pcWnd->GetDlgItem(nID));
	CString cStrWork = _T("");

	if (pcEdit == NULL) {
		return -1;
	}

	if (bAppend) {
		pcEdit->GetWindowText(cStrWork);
	}
	if (bNewLine) {
		cStrWork += (cStrText + _T("\r\n"));
	}
	else {
		cStrWork += (cStrText);
	}
	pcEdit->SetWindowText(cStrWork);

	return 0;
}


/**
 * @fn		OpenFileDialog
 * @brief
 * @param	[in]
 * @return
 */
int CMyHelper::OpenFileDialog(CStringArray &cStrArrPath, BOOL bMulti /*=FALSE*/, int nFiles /*=99*/)
{
	//const int MAX_CFileDialog_FILE_COUNT = 99;
	const int FILE_LIST_BUFFER_SIZE = ((nFiles * (MAX_PATH + 1) + 1));
	CString cStrTmp;
	CString cStrDir;

	TCHAR* pchPtr;
	TCHAR* pchBuffEnd;
	TCHAR* pchBuffStart;

	CFileDialog cFileDlg(TRUE);
	OPENFILENAME& stOFN = cFileDlg.GetOFN();

	pchPtr = cStrTmp.GetBuffer(FILE_LIST_BUFFER_SIZE);
	if (bMulti) {
		stOFN.Flags |= OFN_ALLOWMULTISELECT;
	}
	stOFN.lpstrFile = pchPtr;
	stOFN.nMaxFile = FILE_LIST_BUFFER_SIZE;

	cFileDlg.DoModal();
	cStrTmp.ReleaseBuffer();

	pchBuffEnd = pchPtr + FILE_LIST_BUFFER_SIZE - 2;
	pchBuffStart = pchPtr;

	while ((pchPtr < pchBuffEnd) && (*pchPtr)) {
		pchPtr++;
	}
	if (pchPtr > pchBuffStart) {
		cStrDir.Format(_T("%s"), pchBuffStart);
		pchPtr++;

		while ((pchPtr < pchBuffEnd) && (*pchPtr)) {
			pchBuffStart = pchPtr;

			while ((pchPtr < pchBuffEnd) && (*pchPtr)) {
				pchPtr++;
			}
			if (pchPtr > pchBuffStart) {
				cStrTmp.Format(_T("%s\\%s"), cStrDir, pchBuffStart);
				cStrArrPath.Add(cStrTmp);
			}
			pchPtr++;
		}
	}

	if (cStrArrPath.GetCount() <= 0 && 0 < cStrDir.GetLength()) {
		cStrArrPath.Add(cStrDir);
	}

	TRACE("cStrArrPath.GetCount()=%d", cStrArrPath.GetCount());

	return cStrArrPath.GetCount();
}


/**
 * @fn		MemDump
 * @brief
 * @param	[in]
 * @return
 */
int CMyHelper::MemDump(void* pData, int nByteLen, CString &cStrDump)
{
	if (pData == NULL) {
		return -1;
	}

	cStrDump = "";
	BYTE* p = (BYTE*)pData;
	char buff[4] = {};

	for (int i = 0; i < nByteLen; i++) {
		_snprintf(buff, sizeof(buff), "%02X ", *(p + i));
		cStrDump += CString(buff);
	}
	return 0;
}
/**
 * @fn		MemDump
 * @brief
 * @param	[in]
 * @return
 */
int CMyHelper::MemDump(void* pData, int nByteLen, char* pszDump, int nDumpLen)
{
	if (pData == NULL || pszDump == NULL || nDumpLen < (nByteLen * 3 + 1)) {
		return -1;
	}

	BYTE* p = (BYTE*)pData;
	char buff[4] = {};
	int bp = 0;

	memset(pszDump, 0, nDumpLen);

	for (int i = 0; i < nByteLen && bp < nDumpLen - 3; i++) {
		_snprintf(buff, sizeof(buff), "%02X ", *(p + i));
		strncpy(pszDump + bp, buff, strlen(buff));
		bp += strlen(buff);
	}

	return 0;
}


//LONGLONG ll = 0x1234567890ABCDEF;
//
//CString cs0, csOut;
//
//if (CMyHelper::MemDump(&ll, sizeof(LONGLONG), cs0) < 0) {
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, _T("GetDumpStr failed 1."));
//}
//else {
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, cs0);
//}
//
//char achDump[25];
//
//cs0.Format(_T("sizeof(a)=%d, sizeof(t)=%d, COUNT_OF_ARRAY=%d")
//	, sizeof(achDump), sizeof(char), COUNT_OF_ARRAY(achDump, char));
//CMyHelper::CEditWriteText(this, IDC_EDIT1, cs0);
//
//if (CMyHelper::MemDump(&ll, sizeof(LONGLONG), achDump, COUNT_OF_ARRAY(achDump, char)) < 0) {
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, _T("GetDumpStr failed 2."));
//}
//else {
//	CMyHelper::CEditWriteText(this, IDC_EDIT1, CString(achDump));
//}

//}