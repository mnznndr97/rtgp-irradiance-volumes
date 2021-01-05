#include "Interlocked.hpp"


#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#else
#endif

int Interlocked::Add(int* ptr, int value)
{
	return InterlockedAdd((LONG*)ptr, value);
}

int Interlocked::Increment(int* ptr)
{
	return InterlockedIncrement((LONG*)ptr);
}

int Interlocked::Decrement(int* ptr)
{
	return InterlockedDecrement((LONG*)ptr);
}

