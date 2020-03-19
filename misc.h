/**
 * @file	misc.h
 * @brief	雑関数
 * @author	?
 * @date	?
 */
#pragma once

#pragma warning(push)
#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <io.h>
#include <Windows.h>
#include <tchar.h>


#ifndef DISABLE_C4996
	#define DISABLE_C4996		__pragma(warning(push))	__pragma(warning(disable:4996))
#endif
#ifndef ENABLE_C4996
	#define ENABLE_C4996		__pragma(warning(pop))
#endif


#define MEM_DUMP(pvoid, n, arr)			mem_dump(pvoid, n, arr, sizeof(arr))
#define FMT_STR(arr, fmt, ...)			fmt_str(arr, sizeof(arr), fmt, __VA_ARGS__)
#define STR_TIME_NOW(buff)				str_time_now(buff, sizeof(buff))
#define GET_FILENAME(path, buff)		get_filename(path, buff, sizeof(buff))

#define HEX2CR(val)						((0xA <= (val)) ? ((val) + 0x31) : ((val) + 0x30))
#define COUNT_OF_ARRAY(arr, type)		(sizeof(arr)/sizeof(type))

#define COMPL2(val)						((~(val)) + 1)

#define PRINTLN(fmt, ...)				printf(fmt "\r\n", args);

//#define DEBUG_PRINT(fmt, ...)			printf("%s: " fmt "\r\n", __FUNCTION__, __VA_ARGS__)

/*
 * mem_dump2用のアスキーコードデータ
 */
static const char* aszAscii[] = {
	"NUL", "SOH", "STX", "ETX", "EOT", "ENG", "ACK", "BEL", " BS", " HT", " LF", " VT", " FF", " CR", " SO", " SI",
	"DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB", "CAN", " EM", "SUB", "ESC", " FS", " GS", " RS", " US",
	" SP", " ! ", " \" ", " # ", " $ ", " % ", " & ", " ' ", " ( ", " ) ", " * ", " + ", " , ", " - ", " . ", " / ",
	" 0 ", " 1 ", " 2 ", " 3 ", " 4 ", " 5 ", " 6 ", " 7 ", " 8 ", " 9 ", " : ", " ; ", " < ", " = ", " > ", " ? ",
	" @ ", " A ", " B ", " C ", " D ", " E ", " F ", " G ", " H ", " I ", " J ", " K ", " L ", " M ", " N ", " O ",
	" P ", " Q ", " R ", " S ", " T ", " U ", " V ", " W ", " X ", " Y ", " Z ", " [ ", " \\ ", " ] ", " ^ ", " _ ",
	" ` ", " a ", " b ", " c ", " d ", " e ", " f ", " g ", " h ", " i ", " j ", " k ", " l ", " m ", " n ", " o ",
	" p ", " q ", " r ", " s ", " t ", " u ", " v ", " w ", " x ", " y ", " z ", " { ", " | ", " } ", " ~ ", "DEL",
	"...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...",
	"...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...",
	"...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...",
	"...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...",
	"...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...",
	"...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...",
	"...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...",
	"...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "...", "..."
};


int				_mem_dump(void* pData, int nByteLen, char* pszDump, int nDumpLen);
const char*		mem_dump(void* pData, int nByteLen, char* pszDump, int nDumpLen);
int				_mem_dump2(void* pData, int nByteLen, char* pszDump, int nDumpLen);
const char*		mem_dump2(void* pData, int nByteLen, char* pszDump, int nDumpLen);
int				_fmt_str(char* szBuff, int n, const char* szFmt, ...);
const char*		fmt_str(char* szBuff, int n, const char* szFmt, ...);
int				split_str(const char* szSrc, char* szDelim, char* szDest, int nDest, char* apToken[], int nToken);
const char*		str_time_now(char* szBuff, int nSize);
const char*		get_filename(const char* szPath, char* szBuff, int nSize);
long			get_filesize(const char* szPath);
int				calc_bcc(void* pData, int nByteLen);
int				calc_lrc(void* pData, int nByteLen);
int				calc_crc16(void* pData, int nByteLen);
int				get_error_msg(DWORD dwError, LPTSTR lpszDest, int nDestSize);


/**
 * @fn		_mem_dump
 * @brief	対象のメモリデータを16進表記で出力先バッファに出力
 * @param	[in]	void* pData		: 対象のデータ
 * @param	[in]	int nByteLe		: 対象のデータの大きさ(バイト単位)
 * @param	[out]	char* pszDump	: 出力先バッファ領域
 * @param	[in]	int nDumpLen	: 出力先バッファ領域の大きさ
 * @return	0:成功, -1:失敗
 */
int _mem_dump(void* pData, int nByteLen, char* pszDump, int nDumpLen)
{
	if (pData == NULL || pszDump == NULL /*|| nDumpLen < (nByteLen * 3 + 1)*/) {
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
	// 出力先バッファにデータが収まりきらない場合は省略表示
	if (nDumpLen < (nByteLen * 3 + 1) && 3 <= bp) {
		strncpy(pszDump + (bp - 3), "...", 3);
	}
	return 0;
}
/**
 * @fn		mem_dump
 * @brief	対象のメモリデータを16進表記で出力先バッファに出力
 * @param	[in]	void* pData		: 対象のデータ
 * @param	[in]	int nByteLe		: 対象のデータの大きさ(バイト単位)
 * @param	[out]	char* pszDump	: 出力先バッファ領域
 * @param	[in]	int nDumpLen	: 出力先バッファ領域の大きさ
 * @return	出力先バッファへのポインタ (失敗時は"(NULL)"文字列が返る)
 */
const char*  mem_dump(void* pData, int nByteLen, char* pszDump, int nDumpLen)
{
	if (_mem_dump(pData, nByteLen, pszDump, nDumpLen) < 0) {
		return "(NULL)";
	}
	return pszDump;
}

/**
 * @fn		_mem_dump2
 * @brief	対象のメモリデータを16進表記で出力先バッファに出力
 * @param	[in]	void* pData		: 対象のデータ
 * @param	[in]	int nByteLe		: 対象のデータの大きさ(バイト単位)
 * @param	[out]	char* pszDump	: 出力先バッファ領域
 * @param	[in]	int nDumpLen	: 出力先バッファ領域の大きさ
 * @return	0:成功, -1:失敗
 */
int _mem_dump2(void* pData, int nByteLen, char* pszDump, int nDumpLen)
{
	if (pData == NULL || pszDump == NULL /*|| nDumpLen < (nByteLen * 8 + 1)*/) {
		return -1;
	}

	unsigned char* p = (unsigned char*)pData;
	char buff[9] = { 0 };
	int bp = 0;
	unsigned char uch = 0;

	memset(pszDump, 0, nDumpLen);

	for (int i = 0; i < nByteLen && bp < nDumpLen - 8; i++) {
		uch = *(p + i);
		snprintf(buff, sizeof(buff), "%02X[%s] ", uch, aszAscii[uch]);
		strncpy(pszDump + bp, buff, strlen(buff));
		bp += strlen(buff);
	}
	// 出力先バッファにデータが収まりきらない場合は省略表示
	if (nDumpLen < (nByteLen * 8 + 1) && 8 <= bp) {
		strncpy(pszDump + (bp - 8), "...", 8);
	}
	return 0;
}
/**
 * @fn		mem_dump2
 * @brief	対象のメモリデータを16進表記で出力先バッファに出力
 * @param	[in]	void* pData		: 対象のデータ
 * @param	[in]	int nByteLe		: 対象のデータの大きさ(バイト単位)
 * @param	[out]	char* pszDump	: 出力先バッファ領域
 * @param	[in]	int nDumpLen	: 出力先バッファ領域の大きさ
 * @return	出力先バッファへのポインタ (失敗時は"(NULL)"文字列が返る)
 */
const char*  mem_dump2(void* pData, int nByteLen, char* pszDump, int nDumpLen)
{
	if (_mem_dump2(pData, nByteLen, pszDump, nDumpLen) < 0) {
		return "(NULL)";
	}
	return pszDump;
}

/**
 * @fn		_fmt_str
 * @brief	書式と書式パラメータより文字列を作成、出力先バッファに出力
 * @param	[out]	char* szBuff	: 出力先バッファ領域
 * @param	[in]	int n			: 出力先バッファ領域のサイズ
 * @param	[in]	char* szFmt		: 出力書式
 * @param	[in]	...				: 出力書式パラメータ
 * @return	0:成功, -1:失敗
 */
int _fmt_str(char* szBuff, int n, const char* szFmt, ...)
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
 * @fn		fmt_str
 * @brief	書式と書式パラメータより文字列を作成、出力先バッファに出力
 * @param	[out]	char* szBuff	: 出力先バッファ領域
 * @param	[in]	int n			: 出力先バッファ領域のサイズ
 * @param	[in]	char* szFmt		: 出力書式
 * @param	[in]	...				: 出力書式パラメータ
 * @return	出力先バッファへのポインタ (失敗時は"(NULL)"文字列が返る)
 */
const char* fmt_str(char* szBuff, int n, const char* szFmt, ...)
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
	if (szSrc == NULL || szDelim == NULL || szDest == NULL || apToken == NULL || nDest < 0) {
		return -1;
	}
	if (nDest <= (int)strlen(szSrc)) {
		return -1;
	}

	char* tp = NULL;
	int cnt = 0;

	strncpy(szDest, szSrc, nDest);

	tp = strtok(szDest, szDelim);
	while (tp != NULL) {
		if (nToken <= cnt) {
			//return -1;
			break;
		}
		apToken[cnt] = tp;
		cnt++;
		tp = strtok(NULL, szDelim);
	}

	return cnt;
}


/**
 * @fn		str_time_now
 * @brief	現在日時の文字列を得る
 * @param	[OUT]	char* szBuff	: 日時文字列を格納するバッファ領域(最低25byte)
 * @param	[IN]	int nSize		: バッファ領域のサイズ
 * @return	バッファ領域へのポインタ(処理失敗時は"(NULL)"の文字が返る
 */
const char* str_time_now(char* szBuff, int nSize)
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
 * @fn		get_filename
 * @brief	フルパスよりファイル名のみを取得
 * @param	[IN]	char* szPath	: ファイルパス文字列
 * @param	[OUT]	char* szBuff	: ファイル名を格納するバッファ領域
 * @param	[IN]	int nSize		: バッファ領域のサイズ
 * @return	バッファ領域へのポインタ(処理失敗時は"(NULL)"の文字が返る
 */
const char* get_filename(const char* szPath, char* szBuff, int nSize)
{
	if (szPath == NULL || szBuff == NULL) {
		return "(NULL)";
	}

	//char szDrv[MAX_PATH+1];
	//char szDir[MAX_PATH+1];
	char szFname[MAX_PATH+1];
	char szExt[MAX_PATH+1];

	//_splitpath(szPath, szDrv, szDir, szFname, szExt);
	_splitpath(szPath, NULL, NULL, szFname, szExt);
	snprintf(szBuff, nSize, "%s%s", szFname, szExt);

	return szBuff;
}

/**
 * @fn		get_filesize
 * @brief	ファイルサイズ取得(最大2GB)
 * @param	[IN]	char* szPath	: ファイルパス文字列
 * @return	ファイルサイズ
 */
long get_filesize(const char* szPath)
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

	if (fclose(fp) != 0) {
		//if (errno != 0) perror(NULL);
		return -1;
	}
	return fsize;
}


/**
 * @fn		calc_bcc
 * @brief	BCC計算
 * @param	[IN]	void* pData			: 計算対象データへの参照
 * @param	[IN]	int nByteLen		: 対象データのバイト数
 * @return	BCC計算値
 */
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


int calc_lrc(void* pData, int nByteLen)
{
	if (pData == NULL) {
		return -1;
	}

	unsigned char* pbyData = (unsigned char*)pData;
	unsigned int sum = 0;
	unsigned char byVal = 0;

	for (int i = 0; i < nByteLen; i++) {
		sum += *(pbyData + i);
	}
	//byVal = (unsigned char)((~sum + 1) & 0xFF);
	byVal = (unsigned char)(COMPL2(sum) & 0xFF);
	return byVal;
}


int calc_crc16(void* pData, int nByteLen)
{
	if (pData == NULL) {
		return -1;
	}

	unsigned char* pbyData = (unsigned char*)pData;
	unsigned int sum = 0;
	unsigned char byVal = 0;

	//for (int i = 0; i < nByteLen; i++) {
	//	sum += *(pbyData + i);
	//}
	//byVal = (unsigned char)((~sum + 1) & 0xFF);
	return byVal;
}


/* Table of CRC values for high-order byte */
static const unsigned char auchCRCHi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
};
/* Table of CRC values for low-order byte */
static const unsigned char auchCRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
	0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
	0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
	0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
	0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
	0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
	0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
	0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
	0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
	0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
	0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
	0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
	0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40,
};

unsigned short CalcCRC16swap(
	unsigned char* puchMsg, /* message to calculate */
	unsigned short usDataLen) /* quantity of bytes in message */
{
	unsigned char uchCRCHi = 0xFF; /* high byte of CRC initialized */
	unsigned char uchCRCLo = 0xFF; /* low byte of CRC initialized */
	unsigned int uIndex; /* will index into CRC lookup table */
	while (usDataLen--) /* pass through message buffer */
	{ /* calculate the CRC */
		uIndex = uchCRCHi ^ *puchMsg++;
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
		uchCRCLo = auchCRCLo[uIndex];
	}
	return (uchCRCHi << 8 | uchCRCLo);
}


int get_error_msg(DWORD dwError, LPTSTR lpszDest, int nDestSize)
{
	DWORD dwFlasg = FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS;
	LPTSTR lpMsg = NULL;

	DWORD dwRet = FormatMessage(dwFlasg
		, NULL
		, dwError
		, LANG_USER_DEFAULT
		, lpMsg
		, 0
		, NULL);

	if (dwRet == 0 || lpMsg == NULL) {
		_tcsncpy(lpszDest, _T("(NULL)"), nDestSize);
	}
	else {
		_tcsncpy(lpszDest, lpMsg, nDestSize);
		LocalFree(lpMsg);
	}
	return dwRet;
}


//void test()
//{
//	char chBuff[256];
//	int i[] = { 0x12345678, 0x90ABCDEF };
//	//printf(_mem_dump(i, sizeof(i), chBuff, 256));
//	printf(MEM_DUMP(i, sizeof(i), chBuff));
//	printf("\r\n");
//	printf(FMT_STR(chBuff, "%s,%d,%d", "ABC", 1, 2));
//
//	printf("\r\n[");
//	printf(fmt_str(NULL, sizeof(chBuff), "%s", "ABC", 1, 2));
//	printf("]\r\n");
//
//	// char* cp = &"";
//	// printf("aaa:%d,{%s}", *cp, cp);
//
//	printf("\r\n[");
//	printf(fmt_str(NULL, sizeof(NULL), "%s", "ABC", 1, 2));
//	printf("]\r\n");
//	printf("\r\n[");
//	printf(fmt_str(chBuff, sizeof(chBuff), NULL, "ABC", 1, 2));
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

#pragma warning(pop)
