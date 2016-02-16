/*******************************
filename: EncodingAdapter.hpp
author: wuxian
create time: 20150802
last modified time: 20151126
********************************/

#ifndef _ENCODINGADAPTER_HPP_
#define _ENCODINGADAPTER_HPP_
#include <string>
#include <windows.h>
#include "../src/TransCode.hpp"
using std::string;

class EncodingAdapter {
public:
	static bool UseSmartFlowOp;
	static string GBKToUTF8(const std::string& strGBK);
	static string UTF8ToGBK(const std::string& strUTF8);
	static string UnicodeToUTF8(const CppJieba::Unicode& strUnicode) { return CppJieba::TransCode::encode(strUnicode);  }
	static CppJieba::Unicode UTF8ToUnicode(const string& strUtf8) { return CppJieba::TransCode::decode(strUtf8); }
	static INT IsTextUTF8(const char* lpstrInputStream, INT iLen);
	static int isUtf8(const string& s) { return EncodingAdapter::IsTextUTF8(s.c_str(), s.length()); }
	static string SmartToUTF8(const std::string& str) { if (isUtf8(str)) return str; else return GBKToUTF8(str); }
	static string SmartToGBK(const std::string& str) { if (isUtf8(str)) return UTF8ToGBK(str); else return str; }
	static bool isEnglish(const CppJieba::Unicode& unicode);
};

bool EncodingAdapter::UseSmartFlowOp = true;

string EncodingAdapter::GBKToUTF8(const std::string& strGBK)
{
	string strOutUTF8 = "";
	WCHAR * str1;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char * str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	strOutUTF8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return strOutUTF8;
}

string EncodingAdapter::UTF8ToGBK(const std::string& strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
	unsigned short * wszGBK = new unsigned short[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, (LPWSTR)wszGBK, len);

	len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
	char *szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
	//strUTF8 = szGBK;  
	std::string strTemp(szGBK);
	delete[]szGBK;
	delete[]wszGBK;
	return strTemp;
}

INT EncodingAdapter::IsTextUTF8(const char* lpstrInputStream, INT iLen)
{
	INT   i;
	DWORD cOctets;  // octets to go in this UTF-8 encoded character
	UCHAR chr;
	BOOL  bAllAscii = TRUE;
	cOctets = 0;
	for (i = 0; i < iLen; i++) {
		chr = *(lpstrInputStream + i);
		if ((chr & 0x80) != 0) bAllAscii = FALSE;
		if (cOctets == 0)  {
			// 7 bit ascii after 7 bit ascii is just fine.  Handle start of encoding case.
			if (chr >= 0x80) {
				// count of the leading 1 bits is the number of characters encoded
				do {
					chr <<= 1;
					cOctets++;
				} while ((chr & 0x80) != 0);
				cOctets--;                        // count includes this character
				if (cOctets == 0) return FALSE;  // must start with 11xxxxxx
			}
		}
		else {
			// non-leading bytes must start as 10xxxxxx
			if ((chr & 0xC0) != 0x80) {
				return FALSE;
			}
			cOctets--;                           // processed another octet in encoding
		}
	}
	// End of text.  Check for consistency.
	if (cOctets > 0) {   // anything left over at the end is an error
		return FALSE;
	}
	if (bAllAscii) {     // Not utf-8 if all ascii.  Forces caller to use code pages for conversion
		return FALSE;
	}
	return TRUE;
}

bool EncodingAdapter::isEnglish(const CppJieba::Unicode& unicode) {
	for(size_t i = 0; i < unicode.size(); i++) {
		if(!(unicode[i] >= 'a' && unicode[i] <= 'z' || 
			unicode[i] >= 'A' && unicode[i] <= 'Z' || 
			unicode[i] >= '0' && unicode[i] <= '9')) {
			return 0;
		}
	}
	return 1;
}

namespace std {
	ostream& operator << (ostream& os, const string& s) {
		if (EncodingAdapter::UseSmartFlowOp && &os == _Ptr_cout) {
			string gs = EncodingAdapter::SmartToGBK(s);
			os << gs.c_str();
		}
		else {
			os << s.c_str();
		}
		return os;
	}

	ostream& operator << (ostream& os, const char cs[]) {
		if (EncodingAdapter::UseSmartFlowOp && &os == _Ptr_cout) {
			const string s(cs);
			os << s;
		}
		else
			for (unsigned int i = 0; i < strlen(cs); i++) os << cs[i];
		return os;
	}

	ostream& operator << (ostream& os, const CppJieba::Unicode& s) {
		if (EncodingAdapter::UseSmartFlowOp && &os == _Ptr_cout) {
			string gs = EncodingAdapter::UTF8ToGBK(EncodingAdapter::UnicodeToUTF8(s));
			os << gs.c_str();
		}
		else {
			for (unsigned int i = 0; i < s.size(); i++)
				os << s[i];
		}
		return os;
	}
}
#endif