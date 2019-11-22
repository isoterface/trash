#pragma once
#include <afxwin.h>

class CMyHelper
{
public:
	CMyHelper();
	~CMyHelper();
	static int WriteText(CEdit &cEdit, CString &cStrText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);
	static int WriteText(CEdit &cEdit, LPCTSTR lpszText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);
	static int WriteText(CWnd* cWnd, int nID, CString &cStrText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);
	static int WriteText(CWnd* cWnd, int nID, LPCTSTR lpszText, BOOL bAppend = TRUE, BOOL bNewLine = TRUE);

	static int GetRadioValue(CButton* acRadio[], int count);
	static int GetRadioValue(CWnd* cWnd, int anID[], int count);

private:
	static int writeText(CEdit &cEdit, CString &cStrText, BOOL bAppend, BOOL bNewLine);
	static int writeText(CWnd* cWnd, int nID, CString &cStrText, BOOL bAppend, BOOL bNewLine);

};


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


CMyHelper::CMyHelper()
{
}


CMyHelper::~CMyHelper()
{
}


int CMyHelper::WriteText(CEdit &cEdit, CString &cStrText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return writeText(cEdit, cStrText, bAppend, bNewLine);
}
int CMyHelper::WriteText(CEdit &cEdit, LPCTSTR lpszText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return writeText(cEdit, CString(lpszText), bAppend, bNewLine);
}
int CMyHelper::WriteText(CWnd* cWnd, int nID, CString &cStrText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return writeText(cWnd, nID, cStrText, bAppend, bNewLine);
}
int CMyHelper::WriteText(CWnd* cWnd, int nID, LPCTSTR lpszText, BOOL bAppend/*=TRUE*/, BOOL bNewLine/*=TRUE*/)
{
	return writeText(cWnd, nID, CString(lpszText), bAppend, bNewLine);
}


int CMyHelper::writeText(CEdit &cEdit, CString &cStrText, BOOL bAppend, BOOL bNewLine)
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


int CMyHelper::writeText(CWnd* cWnd, int nID, CString &cStrText, BOOL bAppend, BOOL bNewLine)
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
