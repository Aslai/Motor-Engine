#ifndef MOTOR_H_PLATFORM_HPP
#define MOTOR_H_PLATFORM_HPP

#include <windows.h>
#include <string>

#define constexpr
#define noexcept

namespace Motor{
#ifdef UNICODE
	typedef std::basic_string<wchar_t> TString;
#else
	typedef std::string TString;
#endif
	std::string TStrToUTF8(const TString & str);
	TString TStrFromUTF8(const std::string & str);
}

#endif
