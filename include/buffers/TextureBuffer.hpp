#pragma once

#include <std_include.h>
#include <buffers/GpuBuffer.hpp>

/// <summary>
/// Wrapper to create a texture buffer that can be passet to opengl shader
/// </summary>
/// <remarks>
/// Used at the beginning but then moved to InterfaceBlocks to more flexibility
/// </remarks>
class TextureBuffer : GpuBufferT<GL_TEXTURE_BUFFER> {
private:
	/// <summary>
	/// Max buffer data capacity
	/// </summary>
	GLint _maxBufferSize;
	/// <summary>
	/// Texture handle
	/// </summary>
	GLuint _texture;
	/// <summary>
	/// Current buffer size
	/// </summary>
	GLsizeiptr _bufferSize;

	void Create() {
		// We create an empty buffer and we associate it to the texture 
		// The data in the buffer will be update in the future

		// We have to define a buffer dimension here. We can update the dimension later based on the 
		// samples count
		Bind();
		glBufferData(GL_TEXTURE_BUFFER, sizeof(float), NULL, GL_STREAM_READ);
		Unbind();

		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_BUFFER, _texture);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, Resource());
		glBindTexture(GL_TEXTURE_BUFFER, 0);
	}

	void EnsureCapacity(GLsizeiptr requestedSize) {
		if (requestedSize != _bufferSize) {
			_bufferSize = requestedSize;

			// Buffer dimension update (in case a dynamic size changes)
			// GL_STREAM_READ is used since the buffer is often updated 
			glBufferData(GL_TEXTURE_BUFFER, _bufferSize, NULL, GL_STREAM_READ);
		}
	}
public:
	NO_COPY_AND_ASSIGN(TextureBuffer);

	TextureBuffer() {
		glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &_maxBufferSize);
		Create();
	}

	TextureBuffer(const TextureBuffer&& other) noexcept : _maxBufferSize(other._maxBufferSize), _texture(other._texture), _bufferSize(other._bufferSize) {
	}

	/// <summary>
	/// Update all the data in the buffer and ensures that the buffer has the correc dimension
	/// </summary>
	void UpdateBuffer(const void* data, GLsizeiptr dataSize) {
		if (dataSize > _maxBufferSize) throw std::runtime_error("TextureBuffer requires too much space");

		Bind();
		EnsureCapacity(dataSize);
		glBufferSubData(GL_TEXTURE_BUFFER, 0, dataSize, data);
		Unbind();
	}

	/// <summary>
	/// Updates a subset of the buffer data
	/// </summary>
	void UpdateData(const void* data, GLsizeiptr ptrOffset, GLsizeiptr dataSize) {
		if ((ptrOffset + dataSize) > _bufferSize) throw std::invalid_argument("Invalid data size or offset");

		Bind();
		glBufferSubData(GL_TEXTURE_BUFFER, ptrOffset, dataSize, data);
		Unbind();
	}

	/// <summary>
	/// Set the capacity for the buffer
	/// </summary>
	void SetCapacity(GLsizeiptr dataSize) {
		if (dataSize > _maxBufferSize) throw std::runtime_error("TextureBuffer requires too much space");

		Bind();
		EnsureCapacity(dataSize);
		Unbind();
	}

	GLuint Texture() const { return _texture; }
};