#pragma once

#include <std_include.h>
#include <buffers/GpuBuffer.hpp>

/// <summary>
/// OpenGl InterfaceBlock base definition
/// </summary>
/// <remarks>
/// The type should contain only primitive types and maybe pointers to primitive types.
/// In case of pointer the T size will not reflect the actual size of the object so 
/// the size MUST be corrected with the RebindBuffer() method
/// </remarks>
template <typename T>
class InterfaceBlock
{
	// We require T to be a non-pointer argument to be able get it's size
	static_assert(!std::is_pointer<T>::value, "The argument not be a pointer.");

private:
	/// <summary>
	/// Underlying interface block size
	/// </summary>
	/// <remarks>
	/// The size initially reflects the T type size. If the type contains a pointer, the underlying size
	/// should be changed via the <code>RebindBuffer()</code> function
	/// </remarks>
	GLsizeiptr _bufferSize = sizeof(T);
	/// <summary>
	/// Buffer resource
	/// </summary>
	GpuBuffer _buffer;
	/// <summary>
	/// Interface block type
	/// </summary>
	GLenum _blockType;
	/// <summary>
	/// Block drawing usage
	/// </summary>
	GLenum _usage;
	/// <summary>
	/// Block binding port
	/// </summary>
	const int _bindingPort;
	
protected:
	// We show the buffer to the children to provide more control on the data
	GpuBuffer& GetBuffer() { return _buffer; }
	
public:
	// An interface block should never be copied but only moved
	// (since it contains a GPU resource, copy is meaningless)
	InterfaceBlock(const InterfaceBlock&) = delete;
	InterfaceBlock& operator=(const InterfaceBlock&) = delete;

	InterfaceBlock(InterfaceBlock&& other) noexcept
		: _bufferSize(other._bufferSize),
		  _buffer(std::move(other._buffer)),
		  _blockType(other._blockType),
		  _usage(other._usage),
		  _bindingPort(other._bindingPort)
	{
		
	}
	
	InterfaceBlock& operator=(InterfaceBlock&& other) noexcept
	{
		if (this == &other)
			return *this;
		_bufferSize = other._bufferSize;
		_buffer = std::move(other._buffer);
		_blockType = other._blockType;
		_usage = other._usage;
		_bindingPort = other._bindingPort;
		return *this;
	}

	/// <summary>
	/// Creates a new InterfaceBlock. The block set the usage to GL_DYNAMIC_DRAW to handle frequent data updates
	/// </summary>
	/// <param name="blockType">Interface block type</param>
	/// <param name="bindingPort">Binding port</param>
	InterfaceBlock(GLenum blockType, int bindingPort) : InterfaceBlock(blockType, bindingPort, GL_DYNAMIC_DRAW) {
	}

	/// <summary>
	///	Create a new InterfaceBlock
	/// </summary>
	/// <param name="blockType">Interface block type</param>
	/// <param name="bindingPort">Binding port</param>
	/// <param name="usage">Buffer usage type</param>
	InterfaceBlock(GLenum blockType, int bindingPort, GLenum usage) : _blockType(blockType), _bindingPort(bindingPort) {
		_usage = usage;
		
		RebindBuffer(_bufferSize);
	}	
	
	GpuBuffer& Bind() {
		glBindBuffer(_blockType, _buffer.Resource());
		return _buffer;
	}

	void Unbind() {
		glBindBuffer(_blockType, 0);
	}

	/// <summary>
	/// Sets the new size for the buffer and bind the new size to the GPU
	/// </summary>
	/// <param name="totalByteSize">Buffer size</param>
	void RebindBuffer(GLsizeiptr totalByteSize)
	{
		_bufferSize = totalByteSize;
		glBindBuffer(_blockType, _buffer.Resource());
		glBufferData(_blockType, _bufferSize, nullptr, _usage);
		glBindBuffer(_blockType, 0);

		glBindBufferRange(_blockType, _bindingPort, _buffer.Resource(), 0, _bufferSize);
	}
	
	/// <summary>
	///  Updates the entire T block of data
	/// </summary>
	/// <param name="data"></param>
	void UpdateData(const T& data) {
		glBindBuffer(_blockType, _buffer.Resource());
		glBufferSubData(_blockType, 0, _bufferSize, &data);
		glBindBuffer(_blockType, 0);
	}

	/// <summary>
	/// Updates only a specified value-type member of T
	/// </summary>
	/// <typeparam name="TReturn">Member type. Must be a vlue type</typeparam>
	/// <param name="data">Data block</param>
	/// <param name="member">Member pointer</param>
	template <class TReturn>
	void UpdateFieldData(const T& data, TReturn T::*member) {
		// This will work only with "value-type" fields
		static_assert(!std::is_pointer<TReturn>(), "Member cannot return a pointer");
		
		// We need field offset, field size, field data
		std::size_t fieldOffset = reinterpret_cast<std::size_t>(&(((T*)0)->*member));
		GLsizeiptr fieldSize = sizeof(TReturn);
		const TReturn& fieldData = data.*member;

		// Let's check that the buffer is big enough to store our data
		GLsizeiptr requiredSize = fieldOffset + fieldSize;
		if (requiredSize > _bufferSize) throw std::out_of_range("Underlying buffer size is too small");

		// Finally we can write the data
		glBindBuffer(_blockType, _buffer.Resource());
		glBufferSubData(_blockType, fieldOffset, fieldSize, glm::value_ptr(fieldData));
		glBindBuffer(_blockType, 0);
	}

	/// <summary>
	/// Updates only a specified value-type member of T
	/// </summary>
	/// <typeparam name="TReturn">Member type. Must be a vlue type</typeparam>
	/// <param name="data">Data block</param>
	/// <param name="member">Member pointer</param>
	void UpdateFieldData(const T& data, int T::* member) {
		// We need field offset, field size, field data
		std::size_t fieldOffset = reinterpret_cast<std::size_t>(&(((T*)0)->*member));
		GLsizeiptr fieldSize = sizeof(int);
		int fieldData = data.*member;

		// Let's check that the buffer is big enough to store our data
		GLsizeiptr requiredSize = fieldOffset + fieldSize;
		if (requiredSize > _bufferSize) throw std::out_of_range("Underlying buffer size is too small");

		// Finally we can write the data
		glBindBuffer(_blockType, _buffer.Resource());
		glBufferSubData(_blockType, fieldOffset, fieldSize, &fieldData);
		glBindBuffer(_blockType, 0);
	}

	/// <summary>
	/// Updates only a specified ptr-type member of T
	/// </summary>
	/// <typeparam name="TReturn">Member type. Must be a pointer type</typeparam>
	/// <param name="data">Data block</param>
	/// <param name="member">Member pointer</param>
	/// <param name="ptrByteSize">Data pointer size</param>
	template <class TReturn>
	void UpdatePointerFieldData(const T& data, TReturn T::* member, GLsizeiptr ptrByteSize) {
		static_assert(std::is_pointer<TReturn>(), "Member must return a pointer");

		if(ptrByteSize <= 0) throw std::out_of_range("Invalid pointer size");
		
		// We need field offset, field size, fiels data
		std::size_t fieldOffset = reinterpret_cast<std::size_t>(&(((T*)0)->*member));
		const TReturn fieldData = data.*member;

		// Always check if our underlying buffer is big enough
		GLsizeiptr requiredSize = fieldOffset + ptrByteSize;
		if (requiredSize > _bufferSize) throw std::out_of_range("Underlying buffer size is too small");
		
		// Eventually we write the data
		glBindBuffer(_blockType, _buffer.Resource());
		glBufferSubData(_blockType, fieldOffset, ptrByteSize, fieldData);
		glBindBuffer(_blockType, 0);
	}
};
