#pragma once

#include "stdafx.h"

class CMyHelper
{
public:
	CMyHelper();
	~CMyHelper();

	static int GetRadioValue(CButton* acRadio[], int count);
	static int GetRadioValue(CWnd* cWnd, int anID[], int count);

	static int CEditWriteText(CEdit &cEdit, CString &cStrText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);
	static int CEditWriteText(CEdit &cEdit, LPCTSTR lpszText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);
	static int CEditWriteText(CWnd* cWnd, int nID, CString &cStrText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);
	static int CEditWriteText(CWnd* cWnd, int nID, LPCTSTR lpszText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);

private:
	static int ceditWriteText(CEdit &cEdit, CString &cStrText, BOOL bAppend, BOOL bNewLine);
	static int ceditWriteText(CWnd* cWnd, int nID, CString &cStrText, BOOL bAppend, BOOL bNewLine);

public:
	static int OpenFileDialog(CStringArray &cStrArrPath, BOOL bMulti = FALSE, int nFiles = 99);
	static CString Debug_ByteStr(void* ptr, int nByteLen);
	static int CalcBCC(BYTE* pbyData, int nLen);


};


CMyHelper::CMyHelper()
{
	ASSERT(FALSE);		// new‹ÖŽ~
}


CMyHelper::~CMyHelper()
{
}


//
//
//
int CMyHelper::GetRadioValue(CButton* apcRadio[], int count)
{
	int ret = -1;

	if (apcRadio == NULL) {
		return -1;
	}

	for (int i = 0; i < count; i++) {
		if (apcRadio[i] == NULL) {
			return -1;
		}

		if (apcRadio[i]->GetCheck() == BST_CHECKED) {
			ret = i;
		}
	}

	return ret;
}
//
//
//
int CMyHelper::GetRadioValue(CWnd* cWnd, int anID[], int count)
{
	int ret = -1;

	if (cWnd == NULL || anID == NULL) {
		return -1;
	}

	CButton* pcb;

	for (int i = 0; i < count; i++) {
		pcb = (CButton*)(cWnd->GetDlgItem(anID[i]));
		if (pcb == NULL) {
			return -1;
		}

		if (pcb->GetCheck() == BST_CHECKED) {
			ret = i;
		}
	}

	return ret;
}


//
//
//
int CMyHelper::CEditWriteText(CEdit &cEdit, CString &cStrText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return ceditWriteText(cEdit, cStrText, bAppend, bNewLine);
}
int CMyHelper::CEditWriteText(CEdit &cEdit, LPCTSTR lpszText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return ceditWriteText(cEdit, CString(lpszText), bAppend, bNewLine);
}
int CMyHelper::CEditWriteText(CWnd* cWnd, int nID, CString &cStrText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return ceditWriteText(cWnd, nID, cStrText, bAppend, bNewLine);
}
int CMyHelper::CEditWriteText(CWnd* cWnd, int nID, LPCTSTR lpszText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return ceditWriteText(cWnd, nID, CString(lpszText), bAppend, bNewLine);
}
//
//
//
int CMyHelper::ceditWriteText(CEdit &cEdit, CString &cStrText, BOOL bAppend, BOOL bNewLine)
{
	int ret = 0;

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

	return ret;
}
//
//
//
int CMyHelper::ceditWriteText(CWnd* cWnd, int nID, CString &cStrText, BOOL bAppend, BOOL bNewLine)
{
	if (cWnd == NULL) {
		return -1;
	}

	int ret = 0;

	CEdit* pcEdit = (CEdit*)(cWnd->GetDlgItem(nID));

	if (pcEdit == NULL) {
		return -1;
	}

	CString cStrWork = _T("");

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

	return ret;
}


//
int CMyHelper::OpenFileDialog(CStringArray &cStrArrPath, BOOL bMulti /*=FALSE*/, int nFiles /*=99*/)
{
	//const int MAX_CFileDialog_FILE_COUNT = 99;
	const int FILE_LIST_BUFFER_SIZE = ((nFiles * (MAX_PATH + 1) + 1));
	CString cStrTmp;
	CString cStrDir;

	TCHAR* pc;
	TCHAR* pcBuffEnd;
	TCHAR* pcBuffStart;

	CFileDialog cFileDlg(TRUE);
	OPENFILENAME& stOFN = cFileDlg.GetOFN();

	//TRY
	//{
		pc = cStrTmp.GetBuffer(FILE_LIST_BUFFER_SIZE);
		if (bMulti) {
			stOFN.Flags |= OFN_ALLOWMULTISELECT;
		}
		stOFN.lpstrFile = pc;
		stOFN.nMaxFile = FILE_LIST_BUFFER_SIZE;

		cFileDlg.DoModal();
		cStrTmp.ReleaseBuffer();

		pcBuffEnd = pc + FILE_LIST_BUFFER_SIZE - 2;
		pcBuffStart = pc;

		//TRACE("%s", (LPCTSTR)Debug_ByteStr(pc, FILE_LIST_BUFFER_SIZE - 2));

		while ((pc < pcBuffEnd) && (*pc)) {
			pc++;
		}
		if (pc > pcBuffStart) {
			cStrDir.Format(_T("%s"), pcBuffStart);
			pc++;

			while ((pc < pcBuffEnd) && (*pc)) {
				pcBuffStart = pc;

				while ((pc < pcBuffEnd) && (*pc)) {
					pc++;
				}
				if (pc > pcBuffStart) {
					cStrTmp.Format(_T("%s\\%s"), cStrDir, pcBuffStart);
					cStrArrPath.Add(cStrTmp);
				}
				pc++;
			}
		}

		if (cStrArrPath.GetCount() <= 0 && 0 < cStrDir.GetLength()) {
			cStrArrPath.Add(cStrDir);
		}

		TRACE("cStrArrPath.GetCount()=%d", cStrArrPath.GetCount());
	//}
	//CATCH(CException, pcEx)
	//{
	//	pcEx->ReportError();
	//	return -1;
	//}
	//END_CATCH

	return cStrArrPath.GetCount();
}


//#define		HEX2CHR(c)		((0xA <= (c)) ? ((c) + 0x37) : ((c) + 0x30))

//
//
//
CString CMyHelper::Debug_ByteStr(void* ptr, int nByteLen)
{
	if (ptr == NULL) {
		return CString("");
	}

	CString cStr = "";
	BYTE* p = (BYTE*)ptr;
	//TCHAR buff[] = { 0, 0, 0, 0 };
	TCHAR buff[4] = {};

	for (int i = 0; i < nByteLen; i++) {
		//buff[0] = HEX2CHR((*(p + i) >> 4) & 0xF);
		//buff[1] = HEX2CHR(*(p + i) & 0xF);
		snprintf(buff, sizeof(buff), "%02X ", *(p + i));
		cStr += CString(buff);
	}
	return cStr;
}


//
//
//
int CMyHelper::CalcBCC(BYTE* pbyData, int nLen)
{
	if (pbyData == NULL) {
		return 0;
	}

	BYTE by = 0;
	for (int i = 0; i < nLen; i++) {
		by ^= *(pbyData + i);
	}
	return by;
}
