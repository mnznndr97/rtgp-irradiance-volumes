#pragma once

#include <std_include.h>
#include <buffers/GpuBuffer.hpp>
#include <buffers/ShaderStorageBuffer.hpp>

/// <summary>
/// Wrapper class around a generic pointer
/// </summary>
/// <typeparam name="T">Pointer</typeparam>
template <class T>
struct __PointerHolder
{
	T* Data;

	__PointerHolder() : Data(nullptr)
	{

	}

	// Copy is not allowed
	__PointerHolder(const __PointerHolder& other) = delete;
	__PointerHolder& operator=(const __PointerHolder& other) = delete;

	__PointerHolder(__PointerHolder&& other) noexcept
		: Data(other.Data)
	{
		// We have to set to null the other pointer to avoid
		// a double free on the buffer
		other.Data = nullptr;
	}

	__PointerHolder& operator=(__PointerHolder&& other) noexcept
	{
		if (this == &other)
			return *this;
		Data = other.Data;

		other.Data = nullptr;
		return *this;
	}

	~__PointerHolder()
	{
		delete[] Data;
	}
};

/// <summary>
/// Shader buffer for a single variable-length array
/// </summary>
/// <typeparam name="P">Buffer type</typeparam>
template <class P>
class VariableShaderBuffer : ShaderStorageBuffer<__PointerHolder<P>>
{
	__PointerHolder<P> _buffer;
	GLsizeiptr _lastVectorLength;
public:
	/// <summary>
	/// Creates a new shader buffer on the specified
	/// </summary>
	/// <param name="bindingPort">Binding port for the shader storage</param>
	explicit VariableShaderBuffer(int bindingPort) : ShaderStorageBuffer<__PointerHolder<P>>(bindingPort), _buffer() ,_lastVectorLength(0)
	{
	}

	/// <summary>
	/// Return the underlying vector length
	/// </summary>
	GLsizeiptr GetVectorLength() const { return _lastVectorLength; }

	/// <summary>
	/// Returns the buffer pointer
	/// </summary>
	P* GetVectorPtr() const { return _buffer.Data; }

	/// <summary>
	/// Set new length for the underlying buffer if the new length is different from the old one
	/// </summary>
	/// <remarks>
	///	The old data are not preserved if length is changed
	/// </remarks>
	void SetVectorLength(GLsizeiptr length)
	{
		if (length == _lastVectorLength) return;

		_lastVectorLength = length;
		delete[] _buffer.Data;
		_buffer.Data = new P[length];
		
		// We have to rebind the buffer since the length is change
		this->RebindBuffer(sizeof(P) * _lastVectorLength);
	}

	/// <summary>
	/// Writes the underlying buffer to the GPU
	/// </summary>
	void Write() {
		this->template UpdatePointerFieldData<P*>(_buffer, &__PointerHolder<P>::Data, sizeof(P) * _lastVectorLength);
	}
};