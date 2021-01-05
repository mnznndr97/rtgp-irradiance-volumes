#pragma once

#include <std_include.h>
#include <utility>

/// <summary>
/// Represents a GPU resource. The GPU resource should not be copied.
/// Only move operation are permitted
/// </summary>
/// <typeparam name="TResource">Resource type</typeparam>
/// <typeparam name="TResourceAllocator">Resource allocator</typeparam>
template<class TResource, class TResourceAllocator>
class GpuResource
{
private:
	bool _hasValue = false;
	TResource _resource;
public:
	GpuResource(const GpuResource&) = delete;
	void operator=(const GpuResource&) = delete;

	GpuResource(GpuResource&& other) noexcept
		: _hasValue(other._hasValue),
		_resource(std::move(other._resource))
	{
		other._hasValue = false;
		other._resource = TResource(); // default value
	}

	GpuResource& operator=(GpuResource&& other) noexcept
	{
		if (this == &other)
			return *this;
		_hasValue = other._hasValue;
		_resource = std::move(other._resource);

		// We have to reset the other values
		other._hasValue = false;
		other._resource = TResource(); // default value
		return *this;
	}

	GpuResource()
	{
		TResourceAllocator allocator;
		_resource = allocator.CreateResource();
		_hasValue = true;
	}

	~GpuResource()
	{
		// We have to deallocate our resource only if we have stored a valid value
		if (_hasValue)
		{
			TResourceAllocator allocator;
			allocator.DestroyResource(_resource);
		}
	}

	// TODO: maybe return a reference?
	TResource Resource() const
	{
		return _resource;
	}
};