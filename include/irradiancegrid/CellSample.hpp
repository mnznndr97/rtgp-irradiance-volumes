#pragma once

#include <std_include.h>
#include <unordered_map>
#include <memory>
#include <queue>
#include <cassert>
#include <RadianceSampler.hpp>
#include <irradiancegrid/GridInfoUniform.hpp>
#include <irradiancegrid/fwd.h>
#include <buffers/VariableShaderBuffer.hpp>

#ifdef DEBUG
//#define DEBUGRANDOMCOLOR
#endif // DEBUG

/// <summary>
/// Represents a sampling point in the space
/// </summary>
class GridCellSample {
private:
	/// <summary>
	/// Sample index in the grid
	/// </summary>
	int _gridSampleIndex;
	/// <summary>
	/// Associated sampling point
	/// </summary>
	glm::vec4 _samplingPoint;
	glm::vec3 _vec3SamplingPoint;
	/// <summary>
	/// Sampling point with transformed coordinates after an Update call
	/// </summary>
	glm::vec3 _transformedSamplingPoint;
	/// <summary>
	/// Number of samples taken in the point
	/// </summary>
	/// <remarks>
	/// This is usefull for the Draw() call
	/// </remarks>
	int _samplesCount = 0;

	/// <summary>
	/// Debug random color that is enabled via an application flag
	/// </summary>
	glm::vec3 _randomColor;
	bool _useRandomColor = false;

public:
	/// <summary>
	/// Create a new grid sampling point a 
	/// </summary>
	/// <param name="samplingPoint">Non transformed sampling point</param>
	GridCellSample(const glm::vec3& samplingPoint) : _gridSampleIndex(-1), _samplingPoint(glm::vec4(samplingPoint, 1.0f)), 
		_vec3SamplingPoint(samplingPoint), _transformedSamplingPoint(glm::vec3(0.0f)) {

		// If enabled we generate a fake color to better visualize and debug the trilinear interpolation
		float randMaxF = RAND_MAX;
		float fakeRandom1 = ((float)rand() / randMaxF);
		float fakeRandom2 = ((float)rand() / randMaxF);
		float fakeRandom3 = ((float)rand() / randMaxF);
		_randomColor = glm::vec3(fakeRandom1, fakeRandom2, fakeRandom3);

	}

	// GridCellSample should never be copied
	GridCellSample(const GridCellSample&) = delete;
	GridCellSample& operator=(const GridCellSample&) = delete;

	/// <summary>
	/// Return the non-transformed associated sampling point
	/// </summary>
	const glm::vec3& GetSamplingPoint() const {
		// The sampling point is a vec4 to avoid the creation of a vec4 each update call
		// but we could safely return the reference to the vec3 data here
		// ABSOLUTE WARN: On Linux :<( the trick with the vec4 doesn't work so we MUST return a valid glm::vec3 reference
		return _vec3SamplingPoint;
	}
	/// <summary>
	/// Updates the sample data relative to the sampling point
	/// </summary>
	/// <param name="sampler">Radiance sampler instance</param>
	/// <param name="irradianceBuffer">Buffer to update</param>
	void Update(RadianceSampler* sampler, VariableShaderBuffer<glm::vec4>& irradianceBuffer);
	/// <summary>
	/// Draw a radiance sphere in the transformed sampling point position 
	/// </summary>
	void Draw(RadianceSphere* radianceSphere);
	/// <summary>
	/// Returns the sample index associated with the grid structure
	/// </summary>
	int GetSampleGridIndex() const { return _gridSampleIndex; }
	void SetSampleGridIndex(int index) { _gridSampleIndex = index; }

	void UseRandomColor(bool active) { _useRandomColor = active; }

	void SetTransform(const TransformParams& t) {
		// To avoid to calculate the transformed sampling point each time, 
		// we do de calculation only one time
		_transformedSamplingPoint = _samplingPoint >> t;
	}
};

void GridCellSample::Update(RadianceSampler* sampler, VariableShaderBuffer<glm::vec4>& irradianceBuffer) {
	// The update must be performed on the irradiance buffer provided
	// So we have to ensure that the grid-sample index is correctly set
	// and doesn' t exceed the buffer length
	if (_gridSampleIndex < 0) throw std::runtime_error("Sample index must be set before update");

	int bufferLength = irradianceBuffer.GetVectorLength();
	if (bufferLength <= _gridSampleIndex) throw std::runtime_error("Invalid irradiance buffer size");

	// We have to seek our irradiance buffer span remembering that each grid sample is storing "_samplesCount" samples
	_samplesCount = sampler->SamplesCount();
	glm::vec4* basePtr = irradianceBuffer.GetVectorPtr();
	glm::vec4* dataPointer = basePtr + (_gridSampleIndex * _samplesCount);

	if (_useRandomColor) {
		for (size_t i = 0; i < sampler->SamplesCount(); i++)
		{
			dataPointer[i] = glm::vec4(_randomColor, 0.0f);
		}
	}
	else {
		// Finally we sample the irradiance in the transformed sampling point
		sampler->Sample(_transformedSamplingPoint, dataPointer);
	}
}

void GridCellSample::Draw(RadianceSphere* radianceSphere)
{
	// The draw call in this case is usefull only for debug visualization of sampled irradiance in a point
	radianceSphere->Draw(_transformedSamplingPoint, 0.30f, GetSampleGridIndex(), _samplesCount);
}