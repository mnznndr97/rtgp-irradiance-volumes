#pragma once

#include <std_include.h>
#include <buffers/InterfaceBlock.hpp>

/// <summary>
/// Shader storage buffer interface
/// </summary>
template <typename T>
class ShaderStorageBuffer : public InterfaceBlock<T>
{
private:
	int _shaderBufferMaxSize = 0;
public:
	static const GLuint BlockType = GL_SHADER_STORAGE_BUFFER;

	// Shaders buffers cannot be copied
	ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
	ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;

	/// <summary>
	/// Creates a new shader storage buffer
	/// </summary>
	/// <param name="bindingPort">Buffer binind port</param>
	explicit ShaderStorageBuffer(int bindingPort) : ShaderStorageBuffer(bindingPort, GL_DYNAMIC_DRAW) {
	}

	ShaderStorageBuffer(const int bindingPort, GLenum usage) : InterfaceBlock<T>(BlockType, bindingPort, usage) {
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &_shaderBufferMaxSize);
	}

	/// <summary>
	/// Returns the max size for a shader buffer
	/// </summary>
	int GetMaxSize() const { return _shaderBufferMaxSize; }

};
