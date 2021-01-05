#pragma once

/// <remarks>
/// This is not used for the moment. May be removed
/// </remarks>
class Interlocked {
private:
	Interlocked() {

	}
public:
	static int Add(int* ptr, int value);

	static int Increment(int* ptr);
	static int Decrement(int* ptr);
};