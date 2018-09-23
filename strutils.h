#pragma once
#include <string>
#include <memory>
#include <stdarg.h>
#include <vector>
//#include <xutility>
#include <codecvt>
#include <tchar.h>

/*
字符串比较函数
compare_stringT
相关宏
compare_stringA
compare_stringW
compare_string

字符串格式化模板函数：
string_formatT
string_vformatT
相关宏：
string_formatA
string_formatW
string_format
string_vformatA
string_vformatW
string_vformat

文本/数值与十六进制格式转换模板函数：
to_hexstringT
相关宏：
to_hexstringA
to_hexstringW
to_hexstring

numeric_to_hexstringT
相关宏：
itohexstringA
itohexstringW
itohexstring
ltohexstringA
ltohexstringW
ltohexstring
lltohexstringA
lltohexstringW
lltohexstring
ftohexstringA
ftohexstringW
ftohexstring
lftohexstringA
lftohexstringW
lftohexstring

hexchar2byte
from_hexstringT
相关宏：
from_hexstringA
from_hexstringW
from_hexstring

hexstring_toT
相关宏：
hexstring_toiA
hexstring_toiW
hexstring_toi
hexstring_tolA
hexstring_tolW
hexstring_tol
hexstring_tollA
hexstirng_tollW
hexstring_toll

ansi/unicode/utf8转换
to_wstring
to_string
ascii_to_utf8
utf8_to_ascii
ucs2_to_utf8
utf8_to_ucs2


字符串trim
trim_leftT
trim_rightT
相关宏：
trim_leftA
trim_leftW
trim_left
trim_rightA
trim_rightW
trim_right
trim_stringA
trim_stringW
trim_string

字符串分割
split_stringT
相关宏:
split_stringA
split_stringW
split_string

字符串合并
join_stringT
相关宏:
join_stringA
join_stringW
join_string

格式化文本解析工具(string+separator+string)
namespace dsd {
strlenT
ReadCountT
ReadStringT
ReadFieldStringT
相关宏：
ReadCountA
ReadCountW
ReadCount
ReadStringA
ReadStringW
ReadString
ReadFieldStringA
ReadFieldStringW
ReadFieldString
}
*/


#ifndef BYTE
#define BYTE unsigned char
#endif

typedef std::basic_string<TCHAR> CStdString;

template<typename _CharT>
int compare_stringT(const std::basic_string<_CharT>& str1,
	const std::basic_string<_CharT>& str2,
	bool ignore_case = false)
{
	size_t _N0 = str1.size(), _Count = str2.size();
	size_t _Ans = ignore_case ? _memicmp(str1.c_str(), str2.c_str(), _N0 < _Count ? _N0 : _Count) :
		memcmp(str1.c_str(), str2.c_str(), _N0 < _Count ? _N0 : _Count);

	return (_Ans != 0 ? (int)_Ans : _N0 < _Count ? -1
		: _N0 == _Count ? 0 : +1);
}

#define compare_stringA		compare_stringT<char>
#define compare_stringW		compare_stringT<wchar_t>
#define compare_string		compare_stringT<TCHAR>


////////////////////////////////////////////////////////////////////////////////
//格式化字符串,fmt_str不能使用引用的方式传递，否则va_start会定位到错误的地址
template<typename _CharT>
std::basic_string<_CharT> string_formatT(const std::basic_string<_CharT> fmt_str, ...)
{
	int final_n, n = ((int)fmt_str.size()) * 2;
	std::unique_ptr<_CharT[]> formatted;
	va_list ap;
	while (1) {
		formatted.reset(new _CharT[n]);
		//memcpy(formatted.get(), fmt_str.c_str(), (n + 1) * sizeof(_CharT));
		va_start(ap, fmt_str);

		if (std::is_same<_CharT, wchar_t>::value)
			final_n = _vsnwprintf_s((wchar_t *)formatted.get(), n, _TRUNCATE, (const wchar_t *)fmt_str.c_str(), ap);
		else
			final_n = vsnprintf_s((char *)formatted.get(), n, _TRUNCATE, (const char *)fmt_str.c_str(), ap);

		va_end(ap);

		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return std::basic_string<_CharT>(formatted.get());
}


template<typename _CharT>
std::basic_string<_CharT> string_vformatT(const std::basic_string<_CharT> fmt_str, va_list ap)
{
	int final_n, n = ((int)fmt_str.size()) * 2;
	std::unique_ptr<_CharT[]> formatted;

	while (1) {
		formatted.reset(new _CharT[n]);
		if (std::is_same<_CharT, wchar_t>::value)
			final_n = _vsnwprintf_s((wchar_t *)&formatted[0], n, _TRUNCATE, (const wchar_t *)fmt_str.c_str(), ap);
		else
			final_n = vsnprintf_s((char *)&formatted[0], n, _TRUNCATE, (const char *)fmt_str.c_str(), ap);

		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return std::basic_string<_CharT>(formatted.get());
}

#define string_formatA		string_formatT<char>
#define string_formatW		string_formatT<wchar_t>
#define string_format		string_formatT<TCHAR>
#define string_vformatA		string_vformatT<char>
#define string_vformatW		string_vformatT<wchar_t>
#define string_vformat		string_vformatT<TCHAR>


////////////////////////////////////////////////////////////////////////////////
//将字节数组输出为十六进制文本格式 示例：AA BB 1F 90 ...
//buff,size - 需要转换的字节数组
//bytes_per_line - 每行输出字节数,为0不换行
//uppercase - 涉及的a-f之间的字母使用大写
//separator: 字节之间分隔符，默认为空格，如果为0则不使用

#define NO_SEPARATOR_CHAR			0
#define ConstStr(_Ty, s)			(std::is_same<_Ty,wchar_t>::value ? (const _Ty *)L#s : (const _Ty *)s)

template<typename _CharT>
std::basic_string<_CharT> to_hexstringT(
	const BYTE * buff,
	size_t size,
	size_t bytes_per_line = 0,
	bool uppercase = false,
	_CharT separator = (_CharT)NO_SEPARATOR_CHAR)
{
	std::basic_string<_CharT> out;
	const _CharT * _lf = ConstStr(_CharT, "\r\n");
	const _CharT * _upper_fmt = ConstStr(_CharT, "%02X");
	const _CharT * _lower_fmt = ConstStr(_CharT, "%02x");

	size_t unit_size = 2 + (separator == 0 ? 0 : 1);

	if (bytes_per_line != 0)
	{
		size_t lines = size / bytes_per_line + 1;
		out.reserve(lines * (bytes_per_line * unit_size + 2));
	}
	else
	{
		out.reserve(size * unit_size);
	}

	for (size_t i = 0; i < size; ++i)
	{
		if (i != 0 && bytes_per_line != 0 && (i % bytes_per_line) == 0)
			out += _lf;
		else if (i != 0 && separator != (_CharT)0)
			out += separator;

		if (uppercase)
			out += string_formatT<_CharT>(_upper_fmt, buff[i]);
		else
			out += string_formatT<_CharT>(_lower_fmt, buff[i]);
	}

	return out;
}

#define to_hexstringA		to_hexstringT<char>
#define to_hexstringW		to_hexstringT<wchar_t>
#define to_hexstring		to_hexstringT<TCHAR>


////////////////////////////////////////////////////////////////////////////////
//将数值输出为十六进制文本格式
//输出结果按big endian排列
__declspec(selectany) int _endian_test = 1;
#define IsLitteEndian()	(((char *)&_endian_test)[0] == '\x01')

template<typename _CharT, typename _IntT>
std::basic_string<_CharT> numeric_to_hexstringT(
	_IntT val,
	bool uppercase = false,
	_CharT separator = (_CharT)NO_SEPARATOR_CHAR)
{
	std::vector<BYTE> data;
	data.resize(sizeof(_IntT));
	memcpy(data.data(), &val, sizeof(_IntT));
	if (IsLitteEndian())
		std::reverse(data.begin(), data.end());
	return to_hexstringT<_CharT>(data.data(), data.size(), 0, uppercase, separator);
}

#define itohexstringA			numeric_to_hexstringT<char,int>
#define itohexstringW			numeric_to_hexstringT<wchar_t,int>
#define itohexstring			numeric_to_hexstringT<TCHAR,int>
#define ltohexstringA			numeric_to_hexstringT<char,long>
#define ltohexstringW			numeric_to_hexstringT<wchar_t,long>
#define ltohexstring			numeric_to_hexstringT<TCHAR,long>
#define lltohexstringA			numeric_to_hexstringT<char,__int64>
#define lltohexstringW			numeric_to_hexstringT<wchar_t,__int64>
#define lltohexstring			numeric_to_hexstringT<TCHAR,__int64>
#define ftohexstringA			numeric_to_hexstringT<char,float>
#define ftohexstringW			numeric_to_hexstringT<wchar_t,float>
#define ftohexstring			numeric_to_hexstringT<TCHAR,float>
#define lftohexstringA			numeric_to_hexstringT<char,double>
#define lftohexstringW			numeric_to_hexstringT<wchar_t,double>
#define lftohexstring			numeric_to_hexstringT<TCHAR,double>

//template<typename _Ty>
//std::string numeric_to_hexstringA(_Ty val, bool uppercase = false, char separator = NO_SEPARATOR_CHAR)
//{
//	std::vector<BYTE> data;
//	data.resize(sizeof(_Ty));
//	memcpy(data.data(), &val, sizeof(_Ty));
//	if (IsLitteEndian())
//		std::reverse(data.begin(), data.end());
//	return to_hexstringA(data.data(), data.size(), 0, uppercase, separator);
//}
//
//template<typename _Ty>
//std::wstring numeric_to_hexstringW(_Ty val, bool uppercase = false, char separator = NO_SEPARATOR_CHAR)
//{
//	std::vector<BYTE> data;
//	data.resize(sizeof(_Ty));
//	memcpy(data.data(), &val, sizeof(_Ty));
//	if (IsLitteEndian())
//		std::reverse(data.begin(), data.end());
//	return to_hexstringW(data.data(), data.size(), 0, uppercase, separator);
//}




////////////////////////////////////////////////////////////////////////////////
//将十六进制字符组成的文本转换为二进制数据
//如果有分隔符，则每两个字符出现一个，且不能以分隔符开头
//最后一个字符可以是分隔符也可以没有

#define HexChar2Byte(ch)	\
	(ch >= '0' && ch <= '9') ? (ch-'0') :	\
	((ch >= 'a' && ch <= 'f') ? (ch-'a'+10) : \
	((ch >= 'A' && ch <= 'F') ? (ch='A'+10) : 0xff))

inline BYTE hexchar2byte(char ch) { return HexChar2Byte(ch); }

template<typename _CharT>
std::vector<BYTE> from_hexstringT(
	const std::basic_string<_CharT>& content,
	_CharT separator = (_CharT)NO_SEPARATOR_CHAR)
{
	std::vector<BYTE> result;
	size_t unit_size = 2 + (separator != NO_SEPARATOR_CHAR ? 1 : 0);
	size_t unit_count = content.size() / unit_size;
	const _CharT * data = content.data();

	//没有以分隔符结束，剩余两个字符，可以形成一个byte
	if ((content.size() % unit_size) == 2)
		unit_count += 1;

	result.reserve(unit_count);
	for (size_t i = 0; i < unit_count; i++)
	{
		BYTE high = hexchar2byte((char)data[unit_size * i]);
		BYTE low = hexchar2byte((char)data[unit_size * i + 1]);
		if (high > 0xf || low > 0xf)
			break;

		result.push_back((high << 4) | low);

		//检查分隔符
		if (unit_size == 3 && (unit_size * i + 2) < content.size())
		{
			if (data[unit_size * i + 2] != separator)
				break;
		}
	}
	return result;
}

#define from_hexstringA			from_hexstringT<char>
#define from_hexstringW			from_hexstringT<wchar_t>
#define from_hexstring			from_hexstringT<TCHAR>


////////////////////////////////////////////////////////////////////////////////
//将表达十六进制数的文本转换为数值
template<typename _IntT, typename _CharT>
_IntT hexstring_toT(const std::basic_string<_CharT>& hex)
{
	_IntT value = 0;
	for (size_t i = 0; i < hex.size() && i < sizeof(_IntT) * 2; ++i)
	{
		_IntT b;
		_CharT ch = hex[i];

		if (ch >= '0' && ch <= '9')
			b = (_IntT)(ch - '0');
		else if (ch >= 'a' && ch <= 'f')
			b = (_IntT)(ch - 'a' + 10);
		else if (ch >= 'A' && ch <= 'F')
			b = (_IntT)(ch - 'A' + 10);
		else
			break;

		value = (value << 4) + b;
	}

	return value;
}

#define hexstring_toiA			hexstring_toT<int,char>
#define hexstring_toiW			hexstring_toT<int,wchar_t>
#define hexstring_toi			hexstring_toT<int,TCHAR>
#define hexstring_tolA			hexstring_toT<long,char>
#define hexstring_tolW			hexstring_toT<long,wchar_t>
#define hexstring_tol			hexstring_toT<long,TCHAR>
#define hexstring_tollA			hexstring_toT<__int64,char>
#define hexstirng_tollW			hexstring_toT<__int64,wchar_t>
#define hexstring_toll			hexstring_toT<__int64,TCHAR>


////////////////////////////////////////////////////////////////////////////////
//多字节与宽字节文本转换

//多字节转unicode
//loc为空时不设置locale
std::wstring to_wstring(const std::string& str, const char * loc = NULL);
//unicode转多字节
//loc为空时不设置locale
std::string to_string(const std::wstring& str, const char * loc = NULL);


//unicode转utf8
std::string wstr_to_utf8(const std::wstring& wstr);
std::string wstr_to_utf8(const wchar_t * wstr);

//ascii转utf8
std::string ascii_to_utf8(const std::string& astr);
std::string ascii_to_utf8(const char * astr);

//utf8转unicode
std::wstring utf8_to_wstr(const std::string& u8str);
std::wstring utf8_to_wstr(const char * u8str);

//utf8转ascii
std::string utf8_to_ascii(const std::string& u8str);
std::string utf8_to_ascii(const char * u8str);

#ifdef _UNICODE
#define T2UTF8		wstr_to_utf8
#define UTF8T		utf8_to_wstr
#else
#define T2UTF8		ascii_to_utf8
#define UTF8T		utf8_to_ascii
#endif



////////////////////////////////////////////////////////////////////////////////
//字符串trim
template<typename _CharT>
std::basic_string<_CharT>& trim_leftT(
	std::basic_string<_CharT>& str, _CharT ch = ' ')
{
	std::basic_string<_CharT>::const_iterator start = str.cbegin();
	size_t count = 0;
	while (start != str.cend() && *start++ == ch)
		++count;

	if (count != 0)
		str.erase(0, count);

	return str;
}

template<typename _CharT>
std::basic_string<_CharT>& trim_rightT(
	std::basic_string<_CharT>& str, _CharT ch = ' ')
{
	std::basic_string<_CharT>::const_reverse_iterator start = str.crbegin();
	size_t count = 0;
	while (start != str.crend() && *start++ == ch)
		++count;

	if (count != 0)
		str.erase(str.size() - count, count);
	return str;
}

#define trim_leftA			trim_leftT<char>
#define trim_leftW			trim_leftT<wchar_t> 
#define trim_left			trim_leftT<TCHAR> 
#define trim_rightA			trim_rightT<char> 
#define trim_rightW			trim_rightT<wchar_t>
#define trim_right			trim_rightT<TCHAR> 
#define trim_stringA		trim_stringT<char>
#define trim_stringW		trim_stringT<wchar_t> 
#define trim_string			trim_stringT<TCHAR> 

template<typename _T>
std::basic_string<_T>& trim_stringT(
	std::basic_string<_T>& str, char ch = ' ')
{
	return trim_right(trim_left(str, ch), ch);
}

template<typename _T>
std::basic_string<_T>& trim_stringT(const _T * sz, char ch = ' ')
{
	std::basic_string<_T> str(sz);
	return trim_right(trim_left(str, ch), ch);
}

////////////////////////////////////////////////////////////////////////////////
//字符串split分割

template<typename _T>
std::vector<std::basic_string<_T> > split_stringT(const std::basic_string<_T>& s, const std::basic_string<_T>& c)
{
	std::basic_string<_T>::size_type pos1, pos2;
	std::vector<std::basic_string<_T>> result;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::basic_string<_T>::npos != pos2)
	{
		result.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		result.push_back(s.substr(pos1));

	return result;
}

#define split_stringA		split_stringT<char>
#define split_stringW		split_stringT<wchar_t>
#define split_string		split_stringT<TCHAR>


template<typename _T>
std::basic_string<_T> join_stringT(const std::vector<std::basic_string<_T> >& v, const std::basic_string<_T>& c)
{
	std::basic_string<_T> result;
	for (const std::basic_string<_T>& s : v)
	{
		if (result.size()) result += c;
		result += s;
	}
	return result;
}

#define join_stringA		join_stringT<char>
#define join_stringW		join_stringT<wchar_t>
#define join_string			join_stringT<TCHAR>


////////////////////////////////////////////////////////////////////////////////
//解析 string+separator+string格式字符串

namespace dsd {

	//计算字符串长度
	template<typename _CharT>
	int strlenT(const _CharT * s)
	{
		int len = 0;
		while (s[len] != 0)	++len;
		return len;
	}

	//统计以split分隔的字符串数量
	//如果输入串长度为0则返回0，如果开头或末尾是分隔符则数量加1
	//例如： "abc," 或 ",abc" 将返回两个单元
	template<typename _CharT>
	int ReadCountT(const _CharT * stream, _CharT split)
	{
		int count = 0;
		int len = strlenT<_CharT>(stream);
		for (int i = 0; i < len; ++i)
		{
			if (stream[i] == split)
				++count;
		}

		if (len != 0) ++count;
		return count;
	}

	//在stream中读取以split分隔的第fieldId个字符串
	template<typename _CharT>
	bool ReadStringT(const _CharT * stream, _CharT split, int fieldId, std::basic_string<_CharT>& result)
	{
		int len = strlenT<_CharT>(stream);
		int begin = 0, end = -1;
		if (fieldId > 0)
		{
			for (int i = 0; i < len; ++i)
			{
				if (stream[i] == split && (--fieldId) == 0)
				{
					begin = i + 1;
					break;
				}
			}
		}

		if (fieldId == 0)
		{
			for (int i = begin; i < len; ++i)
			{
				if (stream[i] == split)
				{
					end = i;
					break;
				}
			}

			result.resize(0);
			if (end == -1)
				result.append(stream + begin);
			else
				result.append(stream + begin, end - begin);
			return true;
		}
		return false;
	}

	//stream以split分隔，分隔单元是field=value的形式。
	//根据fieldName找出其对应的value
	template<typename _CharT>
	bool ReadFieldStringT(const _CharT * stream, _CharT split, const _CharT * fieldName, std::basic_string<_CharT>& result)
	{
		int nCount = ReadCountT<_CharT>(stream, split);
		std::basic_string<_CharT> item, field;
		const _CharT subsplit = '=';

		for (int i = 0; i < nCount; ++i)
		{
			if (!ReadStringT<_CharT>(stream, split, i, item))
				return false;

			if (!ReadStringT<_CharT>(item.c_str(), subsplit, 0, field))
				return false;

			if (!field.compare(fieldName))
				return ReadStringT<_CharT>(item.c_str(), subsplit, 1, result);
		}

		return false;
	}

#define ReadCountA				ReadCountT<char>
#define ReadCountW				ReadCountT<wchar_t>
#define ReadCount				ReadCountT<TCHAR>
#define ReadStringA				ReadStringT<char>
#define ReadStringW				ReadStringT<wchar_t>
#define ReadString				ReadStringT<TCHAR>
#define ReadFieldStringA		ReadFieldStringT<char>
#define ReadFieldStringW		ReadFieldStringT<wchar_t>
#define ReadFieldString			ReadfFieldStringT<TCHAR>


	///////////////////////////////////////////////////////////////////////////
	//a - w - T 字符串类型相互转换工具

};
