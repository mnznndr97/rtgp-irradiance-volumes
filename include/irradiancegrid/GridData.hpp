#pragma once

#include <std_include.h>
#include <unordered_map>
#include <memory>
#include <queue>
#include <cassert>
#include <functional>
#include <unordered_set>
#include <RadianceSampler.hpp>
#include <irradiancegrid/GridInfoUniform.hpp>
#include <irradiancegrid/fwd.h>
#include <irradiancegrid/CellSample.hpp>
#include <buffers/VariableShaderBuffer.hpp>

typedef std::function<void(const TransformParams&)> TransformCallback;

struct CallbackRegistration {
private:
	unsigned long MapId = 0L;
	TransformCallback Callback = nullptr;

public:
	friend class GridData;
};

/// <summary>
/// Container for all the associated grid data 
/// </summary>
class GridData
{
private:
	GridInfoUniform _gridInfo;
	/// <summary>
	/// Buffer for the sampled irradiance
	/// </summary>
	VariableShaderBuffer<glm::vec4> _irradianceBuffer;

	/// <summary>
	/// Transform associated with the grid
	/// </summary>
	TransformParams _gridTransform;

	unsigned long _progressiveCallbackId;
	/// <summary>
	/// List of callback to listen to transform changes and update the bounding cube
	/// </summary>
	/// <remarks>
	/// This may be usefull if the transform is changed rarely during the application lifetime.
	/// This can significantly reduce the number and computation costs of the 
	/// construction of a transformed bounding cube
	/// </remarks>
	std::unordered_map<unsigned long, CallbackRegistration> _transformChangedCallbacks;

	int _maxSubGridLevel;

	CellSamplesContainer _cellsSamples;


	int _subGridCount;
	std::priority_queue<int, std::vector<int>, std::greater<int>> _subgridFreeIndeces;

public:
	VariableShaderBuffer<int> _subGridsInfoBuffer;

	GridData(const glm::vec3& gridMin, const glm::vec3& gridMax) :
		_gridInfo(gridMin, gridMax), _irradianceBuffer(2), _progressiveCallbackId(0),
		_maxSubGridLevel(0), _subGridCount(0),
		_subGridsInfoBuffer(3) {
	}

	/* Grid index */

	int GetNewSubgridIndex() {
		int result = -1;
		if (_subgridFreeIndeces.empty()) {
			result = _subGridCount;
		}
		else {
			result = _subgridFreeIndeces.top();
			_subgridFreeIndeces.pop();
		}

		// WARN: here the index cal also be < than the count since if we delete a grid of index 1, a grind of index 2 is 
		// still alive and the we recreate a new grid, the new grid will be of index 1. This avoid us the correction
		// of the index each time one grid is deleted
		assert(result <= _subGridCount);
		++_subGridCount;

		return result;
	}

	void ReturnSubgridIndex(int index) {
		_subGridCount--;
		_subgridFreeIndeces.push(index);
	}

	bool CorrectSubGridIndex(int& subGridIndex)
	{
		bool indexOutOfBound = subGridIndex >= _subGridCount;
		bool freshIndexIsAvailable = !_subgridFreeIndeces.empty() && _subgridFreeIndeces.top() < subGridIndex;
		if (indexOutOfBound || freshIndexIsAvailable) {
			int oldIndex = subGridIndex;

			// in this situation we should always be able to pop an old indices
			subGridIndex = _subgridFreeIndeces.top();
			assert(subGridIndex < _subGridCount&& subGridIndex < oldIndex);
			// We fix the queue
			_subgridFreeIndeces.pop();
			_subgridFreeIndeces.push(oldIndex);
			return true;
		}
		return false;
	}

	int GetSubGridCount() const {
		return _subGridCount;
	}

#if DEBUG
	void AssertSubGrid() const {
		if (!_subgridFreeIndeces.empty()) {
			// If we have some index in the queue, the index should be equals to the next grid index
			// relative to the count
			assert(_subgridFreeIndeces.top() == _subGridCount);
		}
	}
#endif

	/* Grid Infos */

	GridInfoUniform& GetInfos() { return _gridInfo; }

	/* IrradianceBuffer */

	VariableShaderBuffer<glm::vec4>& GetIrradianceBuffer() { return _irradianceBuffer; }

	/* Cell samples related */
	CellSamplesContainer& GetCellSamples() { return _cellsSamples; }

	/* Transform-related function */

	/// <summary>
	/// Register a new transform-changed listener
	/// </summary>
	CallbackRegistration RegisterTransformChanged(TransformCallback&& callback) {
		return RegisterTransformChanged(callback);
	}

	/// <summary>
	/// Register a new transform-changed listener
	/// </summary>
	CallbackRegistration RegisterTransformChanged(TransformCallback& callback) {
		// As it turned out, is not so easy to calculate a hash for a std:func so
		// we need to use a simple id mechanism
		// Hopefully we won't generate 2^64 callbacks;

		unsigned long callbackId = ++_progressiveCallbackId;
		CallbackRegistration registration;
		registration.MapId = callbackId;
		registration.Callback = callback;

		_transformChangedCallbacks.insert(std::pair<unsigned long, CallbackRegistration>(callbackId, registration));
		return registration;
	}

	/// <summary>
	/// Deregister a transform-changed listener
	/// </summary>
	void UnregisterTransformChanged(CallbackRegistration& callback) {
		bool result = _transformChangedCallbacks.erase(callback.MapId);
		assert(result);
	}

	/// <summary>
	/// Returns the current grid transform
	/// </summary>
	const TransformParams GetTransform() const { return _gridTransform; }
	/// <summary>
	/// Sets a new grid transforms and notifies all the listeners
	/// </summary>
	void SetTransform(const TransformParams& p) {
		_gridTransform = p;

		// Let's notify our listeners
		for (const auto& transformChangedCallback : _transformChangedCallbacks)
		{
			const CallbackRegistration& registration = transformChangedCallback.second;
			registration.Callback(p);
		}

		// Let's update the radiance uniform
		_gridInfo.WriteGridTransform(p.Matrix());
	}

	/* Subgrid level related Api */

	int GetMaxSubGridLevel() const { return _maxSubGridLevel; }

	void SetMaxSubGridLevel(int value) {
		value = max(value, 0);
		_maxSubGridLevel = value;
	}
};

