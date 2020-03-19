#if !defined(__UNICODE_H_INCLUDED_7552370787048739__)
#define __UNICODE_H_INCLUDED_7552370787048739__

#include <algorithm>
#include <string>

namespace mll
{

namespace utils
{

struct _ch_unicode_to_ascii_func
{
	char operator()(const wchar_t& wch) const
	{
		return static_cast<char>(wch&0x7F);
	}
};

struct _ch_ascii_to_unicode_func
{
	wchar_t operator()(const char& ch) const
	{
		return static_cast<wchar_t>(ch&0x7F);
	}
};

///	перевод unicode строки в ascii строку
inline std::string to_ascii(const std::wstring& s)
{
	std::string result;
	result.resize(s.size());
	std::transform(s.begin(),s.end(),result.begin(),_ch_unicode_to_ascii_func());
	return result;
}

///	перевод ascii строки в unicode строку
inline std::wstring to_unicode(const std::string& s)
{
	std::wstring result;
	result.resize(s.size());
	std::transform(s.begin(),s.end(),result.begin(),_ch_ascii_to_unicode_func());
	return result;
}

}

}

#endif // !defined(__UNICODE_H_INCLUDED_7552370787048739__)