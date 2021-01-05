#pragma once

#include <std_include.h>
#include <GpuResource.hpp>

/// <summary>
/// Functor for the allocation and deallocation for a gpu buffer resource
/// </summary>
struct GpuBufferAllocator
{
public:
	/// <summary>
	/// Creates
	/// </summary>
	/// <returns>New buffer id </returns>
	GLuint CreateResource()
	{
		GLuint newBuffer = 0;
		glGenBuffers(1, &newBuffer);
		return  newBuffer;
	}

	void DestroyResource(GLuint buffer)
	{
		glDeleteBuffers(1, &buffer);
	}
};

/// <summary>
/// Represents a GPU buffer of type GLuint which is created and destroyed
/// by the GpuBufferAllocator
/// </summary>
class GpuBuffer : public GpuResource<GLuint, GpuBufferAllocator>
{

};

template<GLenum BT>
class GpuBufferT : public GpuResource<GLuint, GpuBufferAllocator>
{
public:
	void Bind() {
		glBindBuffer(BT, Resource());
	}

	void Unbind() {
		glBindBuffer(BT, 0);
	}
};