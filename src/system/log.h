#pragma once

#include <source_location>

void Log(const char* format, ...);
void Log(const std::source_location& location, const char* format, ...);
void Exception(const std::source_location& location, const char* format, ...);
