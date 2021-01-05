#pragma once
#include <std_include.h>

#include <GpuResource.hpp>

/// <summary>
/// Functor for the allocation and deallocation for a vertex array
/// </summary>
struct VertexArrayAllocator
{
public:
	/// <summary>
	/// Creates a new vertext array
	/// </summary>
	/// <returns>New vertext array id </returns>
	GLuint CreateResource()
	{
		GLuint newBuffer = 0;
		glGenVertexArrays(1, &newBuffer);
		return  newBuffer;
	}

	void DestroyResource(GLuint buffer)
	{
		glDeleteVertexArrays(1, &buffer);
	}
};

/// <summary>
/// Represents a vertext array which is created and destroyed
/// by the VertexArrayAllocator
/// </summary>
class VertexArray : public GpuResource<GLuint, VertexArrayAllocator>
{
public:
	void Bind() {
		glBindVertexArray(Resource());
	}

	void Unbind() {
		glBindVertexArray(0);
	}
};