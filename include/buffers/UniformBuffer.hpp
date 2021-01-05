#pragma once

#include <std_include.h>
#include <buffers/InterfaceBlock.hpp>

/// <summary>
/// GPU Uniform buffer interface
/// </summary>
template <typename T>
class UniformBuffer : public InterfaceBlock<T>
{
private:
public:
	// Uniform cannot be copied
	NO_COPY_AND_ASSIGN(UniformBuffer);

	/// <summary>
	/// Creates a new UniforBuffer for the T data block. Usage is set to GL_DYNAMIC_DRAW
	/// </summary>
	/// <param name="bindingPort">Uniform binding port</param>
	explicit UniformBuffer(const int bindingPort) : UniformBuffer(bindingPort, GL_DYNAMIC_DRAW) {
	}

	/// <summary>
	/// Creates a new UniforBuffer for the T data block
	/// </summary>
	/// <param name="bindingPort">uniform binding port</param>
	/// <param name="usage">Buffer usage type</param>
	UniformBuffer(const int bindingPort, GLenum usage) : InterfaceBlock<T>(GL_UNIFORM_BUFFER, bindingPort, usage) {

	}
};