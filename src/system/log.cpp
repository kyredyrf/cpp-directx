#include "Log.h"

#include "String.h"
#include <Windows.h>

void Log(const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	const auto s2 = StringGenerator::VSPrintf(format, argList);
	va_end(argList);

	OutputDebugStringA((s2 + "\n").c_str());
}

void Log(const std::source_location& location, const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	const auto s1 = StringGenerator::SPrintf("%s(%d,%d): ", location.file_name(), location.line(), location.column());
	const auto s2 = StringGenerator::VSPrintf(format, argList);
	va_end(argList);

	const auto t = std::move(s1 + s2 + "\n");
	OutputDebugStringA(t.c_str());
}

void Exception(const std::source_location& location, const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	const auto s1 = StringGenerator::SPrintf("%s(%d,%d): ", location.file_name(), location.line(), location.column());
	const auto s2 = StringGenerator::VSPrintf(format, argList);
	va_end(argList);

	const auto t = std::move(s1 + s2 + "\n");
	OutputDebugStringA(t.c_str());
	throw new std::exception(t.c_str());
}
