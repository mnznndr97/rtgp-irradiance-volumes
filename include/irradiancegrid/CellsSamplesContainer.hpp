#pragma once

#include <std_include.h>
#include <unordered_map>
#include <set>
#include <memory>
#include <queue>
#include <cassert>
#include <functional>
#include <irradiancegrid/CellSample.hpp>

/// <summary>
/// Container for a collection of Cell Samples
/// The cell samples are freed when there are no more references
/// keeping them alive
/// </summary>
class CellSamplesContainer {
public:
	// Let' s force that out class cannot be copied
	NO_COPY_AND_ASSIGN(CellSamplesContainer);

	typedef std::unordered_map<glm::vec3, std::shared_ptr<GridCellSample>> SamplesMap;
	typedef std::priority_queue<int, std::vector<int>, std::greater<int>> IndexQueue;
	typedef std::vector<std::shared_ptr<GridCellSample>> SamplesVector;
private:
	/// <summary>
	/// Cell samples map. The key is a vertex position and the value is the corresponding <see cref="GridCellSample">
	/// </summary>
	/// <remarks>
	/// We can use the map to avoid samples data duplication on cell-shared vertices.
	/// This is possible given the fact that the grid is creating the cell vertices from the same 
	/// "parents" point and only the midpoints of a cell are calculated
	/// </remarks>
	SamplesMap _gridCellsSamplesMap;
	IndexQueue _freeSamplesIndices;
	/// <summary>
	/// For better data locality, we can use a vector that can be easly iterater
	/// when sampling and drawing the samples
	/// </summary>
	/// <remarks>
	/// The vector has some invariants:
	/// 1) Same size of map
	/// 2) Sample indexes in the array must match the array index
	/// 3) Shared pointer in the vector has > 2 users (map, vector + cells...)
	/// <remarks>
	std::vector<std::shared_ptr<GridCellSample>> _gridCellsSamplesVector;

	std::function<void(GridCellSample*)> _deleter;
	/// <summary>
	/// Cached array used in the trim function to avoid 
	/// vector allocations each time trim is called
	/// </summary>
	std::vector<glm::vec3> _keyToBeRemoveCached;
	std::vector<int> _indexToRemove;
	bool _useDebugColor = false;

	/// <summary>
	/// Searches for "holes" in the sampling indexes and fixes them
	/// </summary>
	void Align();
	void Add(std::shared_ptr<GridCellSample>& sample);
	void InitializeDeleter();
	void OnCellSampleDeleted(GridCellSample* sample);

public:
	CellSamplesContainer() {
		InitializeDeleter();
	}

	/// <summary>
	/// Returns a CellSample given a sampling point
	/// </summary>
	std::shared_ptr<GridCellSample> GetCellSample(const glm::vec3& samplingPoint, const TransformParams& t);
	/// <summary>
	/// Searches for dead cell samples and release al the associated resources
	/// </summary>
	void Trim();

	/// <summary>
	/// Returns the map of the current alive sampling point
	/// </summary>
	const SamplesVector& GetVector() const { return _gridCellsSamplesVector; }

	bool IsDebugColorEnabled() const { return _useDebugColor; }
	void SetDebugColorEnabled(bool value);


	~CellSamplesContainer() {
#if DEBUG
		// When detructing the only active reference of out shared ptr should be
		// the reference in the map
		for (auto it = _gridCellsSamplesMap.cbegin(); it != _gridCellsSamplesMap.cend(); ++it) {
			const std::shared_ptr<GridCellSample>& sharedPtrRef = it->second;
			assert(sharedPtrRef.use_count() == 2);
		}
#endif
		// We force the clear of the list to release the shared pointers
		_gridCellsSamplesVector.clear();
		_gridCellsSamplesMap.clear();
	}
};

void CellSamplesContainer::Add(std::shared_ptr<GridCellSample>& sample)
{
	// We can reuse some old index here
	int cellSampleIndex;
	if (_freeSamplesIndices.size() > 0) {
		cellSampleIndex = _freeSamplesIndices.top();
		_freeSamplesIndices.pop();

		_gridCellsSamplesVector.insert(_gridCellsSamplesVector.begin() + cellSampleIndex, sample);
	}
	else
	{
		cellSampleIndex = _gridCellsSamplesMap.size();
		_gridCellsSamplesVector.push_back(sample);
	}

	// Let's make sure point doesn't exists
	assert(_gridCellsSamplesMap.find(sample->GetSamplingPoint()) == _gridCellsSamplesMap.cend());

	sample->SetSampleGridIndex(cellSampleIndex);
	_gridCellsSamplesMap[sample->GetSamplingPoint()] = sample;

	// Let's check if the index match the size of the map and the vector
	assert(_gridCellsSamplesVector.size() == (cellSampleIndex + 1));
	assert(_gridCellsSamplesMap.size() == (cellSampleIndex + 1));
}

void CellSamplesContainer::InitializeDeleter() {
	_deleter = std::bind(&CellSamplesContainer::OnCellSampleDeleted, this, std::placeholders::_1);
}

void CellSamplesContainer::OnCellSampleDeleted(GridCellSample* sample)
{
	_freeSamplesIndices.push(sample->GetSampleGridIndex());
	delete sample;
}

void CellSamplesContainer::Align()
{
	const CellSamplesContainer::SamplesMap& samplesMap = _gridCellsSamplesMap;

	for (auto it = samplesMap.cbegin(); it != samplesMap.cend(); ++it) {
		std::shared_ptr<GridCellSample> cellSampleToCorrect = (*it).second;

		// We search in the map all the items with a "wrong" index
		int sampleIndex = cellSampleToCorrect->GetSampleGridIndex();
		if (sampleIndex < samplesMap.size()) continue;
		// Our sample index does not respect the cell samples count. We have to correct it

		// In this situation there should always be some free indexes since they have been removed by the trimming
		assert(_freeSamplesIndices.size() > 0);
		int freeIndex = _freeSamplesIndices.top();
		assert(freeIndex < samplesMap.size()); // In this situation used free indexes should always reflect the samples size
		_freeSamplesIndices.pop();

		// We have to add to the queue the old index. This is usefull for example in this situation (SubGridSamples from 0 to 9 to semplicity)
		// MainGrid Samples: 0-9, Grid1 Samples: 10-19, Grid2 Samples: 20-29. Grid2 Samples: 30-39
		// If Grid1 is deleted, samples indexes 10-19 become free and will be used to update Grid2 samples
		// and Grid2 samples need to be used to update Grid3
		// Grid3 samples will be used when a new subgrid is created but this indexes will respect the main dictionary size
		_freeSamplesIndices.push(sampleIndex);
		cellSampleToCorrect->SetSampleGridIndex(freeIndex);

		// In the free index the shared_ptr should be empty due to the trimming
		assert(_gridCellsSamplesVector[freeIndex].use_count() == 0);
		_gridCellsSamplesVector[freeIndex] = cellSampleToCorrect;
	}

	// We have now to remove the double items in the end of the array
	while (_gridCellsSamplesVector.size() > samplesMap.size()) {
		_gridCellsSamplesVector.erase(_gridCellsSamplesVector.end() - 1);
	}

#if DEBUG
	// For debug let's verify out vector invariants
	int index = 0;
	for (std::shared_ptr<GridCellSample>& sample : _gridCellsSamplesVector) {
		// Same index and usage
		// If in the grid there is some dead pointer, our heap manager should notify us
		assert(sample->GetSampleGridIndex() == index);
		assert(sample.use_count() > 2);
		++index;
	}
#endif
}

std::shared_ptr<GridCellSample> CellSamplesContainer::GetCellSample(const glm::vec3& samplingPoint, const TransformParams& t)
{
	// When building a cell, the cell will request its relatives sampling points
	// A sampling point may be shared by multiple adjacent cells so first we have to 
	// search for an already existing sampling point in out grid structure
	std::shared_ptr<GridCellSample> result;

	const auto mapSearchResult = _gridCellsSamplesMap.find(samplingPoint);
	if (mapSearchResult != _gridCellsSamplesMap.cend()) {
		result = mapSearchResult->second;
	}
	else
	{
		// The sampling point don't exists in our grid structure. Let's create a new one
		result = std::shared_ptr<GridCellSample>(new GridCellSample(samplingPoint), _deleter);
		result->SetTransform(t);
		result->UseRandomColor(_useDebugColor);
		Add(result);
	}
	return result;
}

void CellSamplesContainer::Trim()
{
	// We are assuming this is not called by multiple threads
	for (auto it = _gridCellsSamplesMap.cbegin(); it != _gridCellsSamplesMap.cend(); ++it) {
		if (it->second.use_count() == 2) {
			_indexToRemove.push_back(it->second->GetSampleGridIndex());
			_keyToBeRemoveCached.push_back(it->first);
		}
	}

	for (size_t i = 0; i < _keyToBeRemoveCached.size(); i++)
	{
		// We first reset out shared ptr in the vector so the following erase in the map will
		// call the _deleter
		std::shared_ptr<GridCellSample>& toBeReplaced = _gridCellsSamplesVector[_indexToRemove[i]];
		toBeReplaced.reset();

		_gridCellsSamplesMap.erase(_keyToBeRemoveCached[i]);
	}

	// After trimming we may have created "holes" in the sampling indexes. This is no good
	// because the sampling buffer will always relfect the number of cell samples.
	// So we MUST align the indexes
	Align();

	_keyToBeRemoveCached.clear();
	_indexToRemove.clear();
}

void CellSamplesContainer::SetDebugColorEnabled(bool value)
{
	if (_useDebugColor == value) return;
	_useDebugColor = value;

	for (auto& it : _gridCellsSamplesVector) {
		it->UseRandomColor(value);
	}
}
