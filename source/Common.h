#pragma once

#include <cstdlib>
#include <cstdio>

// TODO: change these to #defines so we dont have to go up the callstack?
template <typename... Args>
inline void CriticalError(const char* format, Args... args)
{
	std::printf(format, args...);
	std::abort();
}

template <typename... Args>
inline void CriticalAssert(bool condition, const char* format, Args... args)
{
	if (!condition)
		CriticalError(format, args...);
}
