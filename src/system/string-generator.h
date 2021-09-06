#pragma once

#include <cstdarg>
#include <string>

class StringGenerator
{
public:
	static std::string SPrintf(const char* format, ...);
	static std::string VSPrintf(const char* format, va_list argList);

public:
	static std::wstring SPrintf(const wchar_t* format, ...);
	static std::wstring VSPrintf(const wchar_t* format, va_list argList);
};
