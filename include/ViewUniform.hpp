#pragma once

#include <std_include.h>
#include <GpuResource.hpp>
#include <buffers/UniformBuffer.hpp>

/// <summary>
/// View uniform buffer block
/// </summary>
struct ViewBuffer {
	glm::mat4 Projection;
	glm::mat4 CameraView;
};

/// <summary>
/// View uniform at the binding port 0
/// </summary>
class ViewUniform : UniformBuffer<ViewBuffer> {
private:
	ViewBuffer _underlyingData;

public:
	// View uniform cannot be copied
	NO_COPY_AND_ASSIGN(ViewUniform);

	ViewUniform() : UniformBuffer(0)
	{
		_underlyingData.Projection = glm::mat4(0.0f);
		_underlyingData.CameraView = glm::mat4(0.0f);
	}

	/// <summary>
	/// Updates the view projection
	/// </summary>
	void SetProjection(const glm::mat4& projection) {
		_underlyingData.Projection = projection;
		UpdateFieldData(_underlyingData, &ViewBuffer::Projection);
	}

	/// <summary>
	/// Updates the view camera matrix
	/// </summary>
	/// <param name="cameraView"></param>
	void SetCamera(const glm::mat4& cameraView) {
		_underlyingData.CameraView = cameraView;
		UpdateFieldData(_underlyingData, &ViewBuffer::CameraView);
	}
};
