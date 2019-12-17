/**
 * @file	misc.h
 * @brief	雑関数
 * @author	?
 * @date	?
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <Windows.h>


#define MEM_DUMP(pvoid, n, arr)			mem_dump(pvoid, n, arr, sizeof(arr))
#define _MEM_DUMP(pvoid, n, arr)		_mem_dump(pvoid, n, arr, sizeof(arr))

#define FMT_STR(arr, fmt, ...)			fmt_str(arr, sizeof(arr), fmt, __VA_ARGS__)
#define _FMT_STR(arr, fmt, ...)			_fmt_str(arr, sizeof(arr), fmt, __VA_ARGS__)

#define _STR_TIME_NOW(buff)				_str_time_now(buff, sizeof(buff))
#define _GET_FILENAME(path, buff)		_get_filename(path, buff, sizeof(buff))

#define DEBUG_PRINT(fmt, ...)			printf("%s: " fmt "\r\n", __FUNCTION__, __VA_ARGS__)


/**
 * @fn		mem_dump
 * @brief	対象のメモリデータを16進表記で出力先バッファに出力
 * @param	[in]	void* pData		: 対象のデータ
 * @param	[in]	int nByteLe		: 対象のデータの大きさ(バイト単位)
 * @param	[out]	char* pszDump	: 出力先バッファ領域
 * @param	[in]	int nDumpLen	: 出力先バッファ領域の大きさ
 * @return	0:成功, -1:失敗
 */
int mem_dump(void* pData, int nByteLen, char* pszDump, int nDumpLen)
{
	if (pData == NULL || pszDump == NULL || nDumpLen < (nByteLen * 3 + 1)) {
		return -1;
	}

	unsigned char* p = (unsigned char*)pData;
	char buff[4] = { 0,0,0,0 };
	int bp = 0;

	memset(pszDump, 0, nDumpLen);

	for (int i = 0; i < nByteLen && bp < nDumpLen - 3; i++) {
		snprintf(buff, sizeof(buff), "%02X ", *(p + i));
		strncpy(pszDump + bp, buff, strlen(buff));
		bp += strlen(buff);
	}
	return 0;
}
/**
 * @fn		_mem_dump
 * @brief	対象のメモリデータを16進表記で出力先バッファに出力
 * @param	[in]	void* pData		: 対象のデータ
 * @param	[in]	int nByteLe		: 対象のデータの大きさ(バイト単位)
 * @param	[out]	char* pszDump	: 出力先バッファ領域
 * @param	[in]	int nDumpLen	: 出力先バッファ領域の大きさ
 * @return	出力先バッファへのポインタ (失敗時は"(NULL)"文字列が返る)
 */
const char*  _mem_dump(void* pData, int nByteLen, char* pszDump, int nDumpLen)
{
	if (mem_dump(pData, nByteLen, pszDump, nDumpLen) < 0) {
		return "(NULL)";
	}
	return pszDump;
}


/**
 * @fn		fmt_str
 * @brief	書式と書式パラメータより文字列を作成、出力先バッファに出力
 * @param	[out]	char* szBuff	: 出力先バッファ領域
 * @param	[in]	int n			: 出力先バッファ領域のサイズ
 * @param	[in]	char* szFmt		: 出力書式
 * @param	[in]	...				: 出力書式パラメータ
 * @return	0:成功, -1:失敗
 */
int fmt_str(char* szBuff, int n, const char* szFmt, ...)
{
	if (szBuff == NULL || szFmt == NULL) {
		return -1;
	}

	va_list arg;
	va_start(arg, szFmt);
	int ret = vsnprintf(szBuff, n, szFmt, arg);
	va_end(arg);

	if (ret < 0 || n <= ret) {
		return -1;
	}
	return 0;
}
/**
 * @fn		_fmt_str
 * @brief	書式と書式パラメータより文字列を作成、出力先バッファに出力
 * @param	[out]	char* szBuff	: 出力先バッファ領域
 * @param	[in]	int n			: 出力先バッファ領域のサイズ
 * @param	[in]	char* szFmt		: 出力書式
 * @param	[in]	...				: 出力書式パラメータ
 * @return	出力先バッファへのポインタ (失敗時は"(NULL)"文字列が返る)
 */
const char* _fmt_str(char* szBuff, int n, const char* szFmt, ...)
{
	if (szBuff == NULL || szFmt == NULL) {
		return "(NULL)";
	}

	va_list arg;
	va_start(arg, szFmt);
	int ret = vsnprintf(szBuff, n, szFmt, arg);
	va_end(arg);

	if (ret < 0 || n <= ret) {
		return "(NULL)";
	}
	return szBuff;
}


/**
 * @fn		split_str
 * @brief	文字列分割 (分割後のデータおよび各要素へのポインタを返す)
 * @param	[in]	char* szSrc		: 分割する元文字列
 * @param	[in]	char* szDelim	: 区切り文字
 * @param	[out]	char* szDest	: 分割後のデータを出力する領域(区切り文字は'\0'に置き換えられる)
 * @param	[in]	int nDest		: 分割数
 * @param	[out]	char* apToken[]	: 分割後の各要素へのポインタを出力する配列
 * @param	[in]	int nToken		: 分割後の各要素へのポインタを出力する配列のサイズ
 * @return	1～:分割した個数, -1:失敗
 * @remarks	apTokenの各要素はszDestの対応するアドレスを指す, 分割数がnToken以上だと失敗
 */
int split_str(const char* szSrc, char* szDelim, char* szDest, int nDest, char* apToken[], int nToken)
{
	if (szSrc == NULL || szDelim == NULL || szDest == NULL || apToken == NULL) {
		return -1;
	}
	if (nDest <= strlen(szSrc)) {
		return -1;
	}

	char* tp = NULL;
	int cnt = 0;

	strncpy(szDest, szSrc, nDest);

	tp = strtok(szDest, szDelim);
	while (tp != NULL) {
		if (nToken <= cnt) {
			return -1;
		}
		apToken[cnt] = tp;
		cnt++;
		tp = strtok(NULL, szDelim);
	}

	return cnt;
}


/**
 * @fn		_str_time_now
 * @brief	現在日時の文字列を得る
 * @param	[OUT]	char* szBuff	: 日時文字列を格納するバッファ領域
 * @param	[IN]	int nSize		: バッファ領域のサイズ
 * @return	バッファ領域へのポインタ(処理失敗時は"(NULL)"の文字が返る
 */
const char* _str_time_now(char* szBuff, int nSize)
{
	if (szBuff == NULL) {
		return "(NULL)";
	}
	SYSTEMTIME stTime;
	GetLocalTime((LPSYSTEMTIME)&stTime);
	snprintf(szBuff, nSize,
		"%04d/%02d/%02d, %02d:%02d:%02d.%03d"
		, stTime.wYear
		, stTime.wMonth
		, stTime.wDay
		, stTime.wHour
		, stTime.wMinute
		, stTime.wSecond
		, stTime.wMilliseconds);
	return szBuff;
}


/**
* @fn		_get_filename
* @brief	フルパスよりファイル名のみを取得
* @param	[IN]	char* szPath	: ファイルパス文字列
* @param	[OUT]	char* szBuff	: ファイル名を格納するバッファ領域
* @param	[IN]	int nSize		: バッファ領域のサイズ
* @return	バッファ領域へのポインタ(処理失敗時は"(NULL)"の文字が返る
*/
const char* _get_filename(const char* szPath, char* szBuff, int nSize)
{
	if (szPath == NULL || szBuff == NULL) {
		return "(NULL)";
	}

	char szDrv[MAX_PATH+1];
	char szDir[MAX_PATH+1];
	char szFname[MAX_PATH+1];
	char szExt[MAX_PATH+1];

	_splitpath(szPath, szDrv, szDir, szFname, szExt);
	snprintf(szBuff, nSize, "%s%s", szFname, szExt);

	return szBuff;
}


///**
// * @fn		curr_time
// * @brief	
// * @param	
// * @return	
// * @remarks	
// */
//int curr_time(struct tm* tm)
//{
//	time_t t;
//	time(&t);
//	if (localtime_s(tm, &t) != 0) {
//		return -1;
//	}
//	return 0;
//}


int calc_bcc(void* pData, int nByteLen)
{
	if (pData == NULL) {
		return -1;
	}

	unsigned char* pbyData = (unsigned char*)pData;
	unsigned char byVal = 0;
	for (int i = 0; i < nByteLen; i++) {
		byVal ^= *(pbyData + i);
	}
	return byVal;
}


//void test()
//{
//	char chBuff[256];
//	int i[] = { 0x12345678, 0x90ABCDEF };
//	//printf(_mem_dump(i, sizeof(i), chBuff, 256));
//	printf(_MEM_DUMP(i, sizeof(i), chBuff));
//	printf("\r\n");
//	printf(_FMT_STR(chBuff, "%s,%d,%d", "ABC", 1, 2));
//
//	printf("\r\n[");
//	printf(_fmt_str(NULL, sizeof(chBuff), "%s", "ABC", 1, 2));
//	printf("]\r\n");
//
//	// char* cp = &"";
//	// printf("aaa:%d,{%s}", *cp, cp);
//
//	printf("\r\n[");
//	printf(_fmt_str(NULL, sizeof(NULL), "%s", "ABC", 1, 2));
//	printf("]\r\n");
//	printf("\r\n[");
//	printf(_fmt_str(chBuff, sizeof(chBuff), NULL, "ABC", 1, 2));
//	printf("]\r\n");
//
//	char* szSrc = "one,two,three,four";
//	char szDest[128];
//	char* apToken[10] = { 0,0,0,0,0,0,0,0,0,0 };
//	int token_num = 0;
//
//	if (0 < (token_num = split_str(szSrc, ",", szDest, sizeof(szDest), apToken, 10)))
//	{
//		for (int i = 0; i < token_num; i++) {
//			printf("[%s]\r\n", apToken[i]);
//		}
//	}
//
//	//char *sz0 = "ABCDEFGH";
//	//char *p = sz0;
//
//	//for (int i = 0; i < 7; i++) {
//	//	printf("%s\r\n", p++);
//	//	printf("%s\r\n", sz0);
//	//}
//
//	//char* ap[4];
//	//char** pp = ap;
//	//char* sz0 = "one";
//	//char* sz1 = "two";
//	//char* sz2 = "three";
//	//char* sz3 = "four";
//	//
//	//*pp = sz0;
//	//*(pp + 1) = sz1;
//	//*(pp + 2) = sz2;
//	//*(pp + 3) = sz3;
//
//	//for (int i = 0; i < 4; i++) {
//	//	printf("%s\r\n", ap[i]);
//	//}
//}
