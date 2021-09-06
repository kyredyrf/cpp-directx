#include "string-generator.h"

#include <conio.h>

std::string StringGenerator::SPrintf(const char* format, ...)
{
	std::string s;

	va_list argList;
	va_start(argList, format);
	s = VSPrintf(format, argList);
	va_end(argList);

	return std::move(s);
}

std::string StringGenerator::VSPrintf(const char* format, va_list argList)
{
	std::string s(16, ' ');

	auto ret = _vsnprintf_s(s.data(), s.size(), s.size() - 1, format, argList);
	while (ret < 0)
	{
		s.resize(s.size() * 2);
		ret = _vsnprintf_s(s.data(), s.size(), s.size() - 1, format, argList);
	}

	s = s.substr(0, ret);

	return std::move(s);
}

std::wstring StringGenerator::SPrintf(const wchar_t* format, ...)
{
	std::wstring s;

	va_list argList;
	va_start(argList, format);
	s = VSPrintf(format, argList);
	va_end(argList);

	return std::move(s);
}

std::wstring StringGenerator::VSPrintf(const wchar_t* format, va_list argList)
{
	std::wstring s(16, L' ');

	auto ret = _vsnwprintf_s(s.data(), s.size(), s.size() - 1, format, argList);
	while (ret < 0)
	{
		s.resize(s.size() * 2);
		ret = _vsnwprintf_s(s.data(), s.size(), s.size() - 1, format, argList);
	}

	s = s.substr(0, ret);

	return std::move(s);
}
