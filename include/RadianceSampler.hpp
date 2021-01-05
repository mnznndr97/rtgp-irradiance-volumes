#pragma once

#include <std_include.h>
#include <vector>
#include <immintrin.h>
#include <limits>
#include <functional>

#include <UnitHemisphereDirections.h>
#include <pool/SimpleArrayPool.hpp>

#include <SceneObject.hpp>


/// <summary>
/// Component that samples the irradiance of a given scene context/part using a given resolution
/// </summary>
/// <remarks>
/// All the vector used by this class are of type glm::vec4. This should be more efficient
/// (mostly for the CPU Intrinscs to provide a direct fit) and provides a consistent view for 
/// the shaders due to the vec4 aligment in uniforms/interface blocks
/// </remars>
class RadianceSampler
{
private:
	/// <summary>
	/// Direction hemisphere for the sampling 
	/// </summary>
	UnitHemisphereDirections _directionsSampler;
	/// <summary>
	/// Array pool that wll be used to avoid allocation at each sampling call
	/// </summary>
	SimpleArrayPool<glm::vec4>* _directionRadianceArrayPool;
	std::function<void(glm::vec4*)> _rentInitializer;
	const glm::vec4 _zeroVector = glm::vec4(0.0f);
	vector<const SceneObject*> _samplingObjects;

	void ApplyRadianceAttenuation(glm::vec3& radiance, const RayHit& rayHit) {
		// NB. In the radiance paper there is no mention over the radiance attenuation 
		// based on the sample distance but only by the specular coeff in case of reflection
		// So this attenuation may not be needed

		// Two simple factors
		/*
		float attFactor = 0.5f;
		float attFactor = (1.0f) / (1.0f + pow(rayHit.Distance(), 2.0f));
		*/

		// From attenuation section in https://learnopengl.com/Lighting/Light-casters
		float attFactor = (1.0f) / (1.0f + (0.7f * rayHit.Distance()) + (1.8f * pow(rayHit.Distance(), 2.0f)));
		assert(attFactor <= 1.0f); // (We should never "amplify" the radiance)
		radiance *= attFactor;
	}

	void InitializeRentedBuffer(glm::vec4* buffer);

	/// <summary>
	/// Calculates the irradiance with using the SIMD intrinsics
	/// to avoid the glm::vec4 overhead
	/// </summary>
	void ComputeIrradianceFast(const glm::vec4* dirRadianceSource, int samplesCount, glm::vec4* resultBuffer) const;

	/// <summary>
	/// Basic irradiance calculation
	/// </summary>
	void ComputeIrradiance(const glm::vec4* dirRadianceSource, int samplesCount, glm::vec4* resultBuffer) const;

#ifdef DEBUG
	/// <summary>
	/// Another testing function for our implementation of
	/// inverse-mapping function
	/// </summary>
	void TestInverseMappingFunction(const Ray& samplingRay) const
	{
		const float resolution = _directionsSampler.GetResolution();
		int offset = 0;
		glm::vec3 mapDirection = samplingRay.Direction();
		if (mapDirection.y < 0) {
			mapDirection.y = -mapDirection.y;
			offset = resolution * resolution;
		}
		glm::vec2 pt = SemisphereToPoint(mapDirection);
		int ptX = (int)(floor(pt.x * resolution));
		int ptY = (int)(floor(pt.y * resolution));
		int storageIndex = offset + ptX * resolution + ptY;

		assert(storageIndex >= 0 && storageIndex < resolution* resolution * 2);
	}
#endif // DEBUG

	template<class Iterator>
	void SampleDataInDirection(const Ray& samplingRay,
		const Iterator& iteratorStart, const Iterator& iteratorEnd,
		int sampleIndex,
		glm::vec4* destBuffer) const {
#if DEBUG
		TestInverseMappingFunction(samplingRay);
#endif

		Surface* hittedSurface = nullptr;
		float currentMinDistance = std::numeric_limits<float>().max();
		for (Iterator it = iteratorStart; it != iteratorEnd; ++it) {
			const SceneObject* object = *it;
			RayHit hitInfo = object->IsHitByRay(samplingRay);
			if (!hitInfo.IsHit()) {
				// No hit with the surface 
				continue;
			}
			if (hitInfo.Distance() > currentMinDistance)
			{
				// There is another surface nea the sampling point. Ignore;
				continue;
			}
			currentMinDistance = hitInfo.Distance();
			// Here we are assuming that the object implements a precise hit algorithm
			// that gives as the precise surface of the hitting point
			hittedSurface = hitInfo.Surface();
		}

		if (!hittedSurface) {
			// Ambient component (in our case a zero vector)
			destBuffer[(sampleIndex * 2) + 1] = _zeroVector;
		}
		else if (hittedSurface->GetRadiance().has_value()) {
			const glm::vec3& radiance = hittedSurface->GetRadiance().value().Value;
			
			// Paper does' t mention an attenuation by distance
			// This may comes due to the fact that if a hitted surface is "small" and very far
			// It may hit only a few samples so it won't "participate" much in the irradiance
			// integral
			// ApplyRadianceAttenuation(radiance, hitInfo);

			// Let's avoid a construction of a vec4 here
			glm::vec4* destination = destBuffer + ((sampleIndex * 2) + 1);
			glm::vec3* destination3 = reinterpret_cast<glm::vec3*>(destination);
			*destination3 = radiance;
			destination->w = 0.0f;
		}
		else
		{
			// other cases nt taken in in account
			destBuffer[(sampleIndex * 2) + 1] = _zeroVector;
		}
	}

	template<class Iterator>
	void SampleData(const glm::vec3& samplingPoint,
		const Iterator& iteratorStart, const Iterator& iteratorEnd,
		glm::vec4* resultBuffer) const {

		const vector<glm::vec3>& directions = _directionsSampler.GetSamplingDirections();
		int samplesCount = directions.size();
		int requiredBufferSize = samplesCount * 2;

		// To avoid allocating a new array each time, we use a pool
		// This is necessary to provide thread safeness
		glm::vec4* dirRadiancePoolRent = _directionRadianceArrayPool->Rent(requiredBufferSize, _rentInitializer);

		int sampleIndex = 0;
		for (const glm::vec3& direction : directions) {
			Ray ray(samplingPoint, direction);

			SampleDataInDirection(ray, iteratorStart, iteratorEnd, sampleIndex, dirRadiancePoolRent);
			++sampleIndex;
		}

#if DEBUG
		if (_directionsSampler.GetResolution() < 15)
			ComputeIrradiance(dirRadiancePoolRent, samplesCount, resultBuffer);
		else
			ComputeIrradianceFast(dirRadiancePoolRent, samplesCount, resultBuffer);
#else
		ComputeIrradianceFast(dirRadiancePoolRent, samplesCount, resultBuffer);
#endif
		// Always return the pooled array
		_directionRadianceArrayPool->Return(dirRadiancePoolRent);
	}

public:
	RadianceSampler() : _directionRadianceArrayPool(nullptr)
	{
		// Setup for the array pool initializer delegate
		_rentInitializer = std::bind(&RadianceSampler::InitializeRentedBuffer, this, std::placeholders::_1);
		SetResolution(9);
	}

	~RadianceSampler() {
		delete _directionRadianceArrayPool;
	}

	void Sample(const glm::vec3& samplingPoint, glm::vec4* resultBuffer) const {
		// Here we assume resultBuffer is big enught
		SampleData(samplingPoint, _samplingObjects.cbegin(), _samplingObjects.cend(), resultBuffer);
	}

	int SamplesCount() const { return (_directionsSampler.GetResolution() * 2 * _directionsSampler.GetResolution()); }
	int GetResolution() const { return _directionsSampler.GetResolution(); }

	/// <summary>
	/// Entry point to obtain the list of object to perform ray casting
	/// </summary>
	/// <remarks>
	/// This may be substituted with a class that manages a better algorithm to search for object intersection
	/// in a scene
	/// </remarks>
	vector<const SceneObject*>& GetSamplingObjects() { return _samplingObjects; }

	void SetResolution(int resolution) {
		_directionsSampler.SetResolution(resolution);

		// To avoid problem, let's recreate the entire pool
		// (resolution should not change frequently so this operation should not be
		// problematic in term of performance
		delete _directionRadianceArrayPool;
		_directionRadianceArrayPool = new SimpleArrayPool<glm::vec4>();
	}
};

void RadianceSampler::InitializeRentedBuffer(glm::vec4* buffer)
{
	// With this little trick we spend only little time writing the sampling direction in our buffer
	// This is done because
	// 1) The buffer is constructed to provided a better data locality, so direction and relative radiance
	// are next to each other in memory (cache mechanism should work better)
	// 2) If the resolution of the sampling is not changed frequenty, the array pool after the first init will reuse the already
	// initialized array

	const vector<glm::vec3>& directions = _directionsSampler.GetSamplingDirections();
	for (int i = 0; i < directions.size(); ++i)
	{
		glm::vec4* dirPtr = buffer + (i * 2);
		glm::vec3* dir3Ptr = reinterpret_cast<glm::vec3*>(dirPtr);

		// Let's avoid the construction of a vec4 element to write in the buffer
		*dir3Ptr = directions[i];
		dirPtr->w = 0.0f;
	}
}

void RadianceSampler::ComputeIrradianceFast(const glm::vec4* dirRadianceSource, int samplesCount, glm::vec4* resultBuffer) const {
	// Same irradiance calculation but exploiting CPU intrinsics to avoid glm:: copy/construction/casts overhead

	const float mulConst = 4.0f * (float)M_PI / samplesCount;
	const glm::vec4* dirRadBuffer = dirRadianceSource;

	// Let's prepare our mult constant and zero vector
	__m128 multConst = _mm_set_ps(0.0f, mulConst, mulConst, mulConst);
	__m128 zero = _mm_setzero_ps();
	for (int i = 0; i < samplesCount; i++) {
		int mainDirectionIndex = (2 * i);

		// Direct load on 128bit register (with glm::vec3 this is not possible)
		__m128 mainDir = _mm_load_ps((float*)(dirRadBuffer + mainDirectionIndex));
		__m128 irradiance = _mm_setzero_ps();
		for (int j = 0; j < samplesCount; ++j) {
			int index = j * 2;
			__m128 direction = _mm_load_ps((float*)(dirRadBuffer + index));
			__m128 radiance = _mm_load_ps((float*)(dirRadBuffer + index + 1));
			// Masked dot product with only our three components
			__m128 dotProduct = _mm_max_ps(_mm_dp_ps(direction, mainDir, 0b01110111), zero);

			radiance = _mm_mul_ps(radiance, dotProduct);
			irradiance = _mm_add_ps(irradiance, radiance);
		}
		irradiance = _mm_mul_ps(irradiance, multConst);

		float* storagePtr = reinterpret_cast<float*>(resultBuffer + i);
		_mm_store_ps(storagePtr, irradiance);

		// Irradiance in xyz should always be >= zero (in w is always 0.0)
		assert(storagePtr[0] >= 0.0f);
		assert(storagePtr[1] >= 0.0f);
		assert(storagePtr[2] >= 0.0f);
		assert(storagePtr[3] == 0.0f);
	}
}

void RadianceSampler::ComputeIrradiance(const glm::vec4* dirRadianceSource, int samplesCount, glm::vec4* resultBuffer) const {
	const float mulConst = 4.0f * (float)M_PI / samplesCount;

	const glm::vec4* dirRadBuffer = dirRadianceSource;
	for (int i = 0; i < samplesCount; ++i) {
		int mainDirectionIndex = (2 * i);
		const glm::vec3& mainDirection = dirRadBuffer[mainDirectionIndex];

		glm::vec3 irr(0.0f);
		for (int j = 0; j < samplesCount; ++j) {
			const glm::vec3& dir = dirRadBuffer[(j * 2)];
			const glm::vec3& rad = dirRadBuffer[(j * 2) + 1];

			float radianceWeight = max(0.0f, glm::dot(mainDirection, dir));
			irr += (rad * radianceWeight);
		}

		irr *= mulConst;
		resultBuffer[i] = glm::vec4(irr, 0.0f);

		// Irradiance in xyz should always be >= zero (in w is always 0.0)
		assert(irr.x >= 0.0f);
		assert(irr.y >= 0.0f);
		assert(irr.z >= 0.0f);
	}
}