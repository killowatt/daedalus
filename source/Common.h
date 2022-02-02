#pragma once

#include <cstdlib>
#include <cstdio>

template <typename... Args>
inline void LogVK(const char* format, Args... args)
{
	std::printf("[VK] ");
	std::printf(format, args...);
	std::printf("\n");
}

namespace Debug
{
	template <typename... Args>
	static inline void PrintLine(const char* format, Args... args)
	{
		std::printf(format, args...);
		std::printf("\n");
	}
}

#define LOG_VK(format, ...)	\
	LogVK(format, ##__VA_ARGS__)

#define CRITICAL_ERROR(format, ...) \
	{ \
		Debug::PrintLine(format, ##__VA_ARGS__); \
		std::abort(); \
	} \

#define CRITICAL_ASSERT(condition, format, ...) \
	{ \
		if (!(condition)) \
		{ \
			Debug::PrintLine(format, ##__VA_ARGS__); \
			std::abort(); \
		} \
	}
	