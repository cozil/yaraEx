#include "strutils.h"

std::wstring to_wstring(const std::string& str, const char * loc)
{
	if (loc)
		setlocale(LC_ALL, loc);

	std::vector<wchar_t> buff(str.size());
	size_t size;// = mbstowcs(buff.data(), str.data(), buff.size());
	return mbstowcs_s(&size, buff.data(), buff.size(), str.data(), _TRUNCATE) ? std::wstring(L"") :
		std::wstring(buff.data(), size);
}

std::string to_string(const std::wstring& str, const char * loc)
{
	/*
	�״ε���ǰӦ����localeΪ����locale��������������ʶ����ַ�����ֹת����
	setlocale(LC_ALL, "");
	����鿴setlocale��msdn�����ĵ���
	*/
	if (loc)
		setlocale(LC_ALL, loc);

	std::vector<char> buff(str.size() * 2);
	size_t size;// = wcstombs(buff.data(), str.data(), buff.size());
		
	return wcstombs_s(&size, buff.data(), buff.size(), str.data(), _TRUNCATE) ? std::string("") :
		std::string(buff.data(), size);
}



//unicodeתutf8
std::string wstr_to_utf8(const std::wstring& wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cvt;
	return cvt.to_bytes(wstr);
}

std::string wstr_to_utf8(const wchar_t * wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cvt;
	return cvt.to_bytes(wstr);
}

//asciiתutf8
std::string ascii_to_utf8(const std::string& astr)
{
	std::wstring wstr = to_wstring(astr, "");
	return wstr_to_utf8(wstr);
}

std::string ascii_to_utf8(const char * astr)
{
	std::wstring wstr = to_wstring(astr, "");
	return wstr_to_utf8(wstr);
}

//utf8תunicode
std::wstring utf8_to_wstr(const std::string& u8str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cvt;
	return cvt.from_bytes(u8str);
}

std::wstring utf8_to_wstr(const char * u8str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cvt;
	return cvt.from_bytes(u8str);
}

//utf8תascii
std::string utf8_to_ascii(const std::string& u8str)
{
	std::wstring wstr = utf8_to_wstr(u8str);
	return to_string(wstr, "");
}

std::string utf8_to_ascii(const char * u8str)
{
	std::wstring wstr = utf8_to_wstr(u8str);
	return to_string(wstr, "");
}
