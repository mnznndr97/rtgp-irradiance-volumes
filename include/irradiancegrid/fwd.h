#pragma once

#include <std_include.h>
#include <unordered_map>
#include <memory>
#include <queue>
#include <cassert>
#include "../../RadianceSphere.hpp"
#include <RadianceSampler.hpp>
#include <irradiancegrid/GridInfoUniform.hpp>
#include <irradiancegrid/CellSample.hpp>
#include <irradiancegrid/CellsSamplesContainer.hpp>
#include <irradiancegrid/GridData.hpp>
#include <BCube.hpp>
#include <Transform.hpp>

/* Forwar declaration header to solve classes circular dependencies */

class GridCell;
class Grid;
class SubGrid;

/// <summary>
/// Represents a grid volume cell
/// </summary>
class GridCell {
private:
	/// <summary>
	/// Root grid of the volume tree
	/// </summary>
	SubGrid* _parent;
	/// <summary>
	/// Bounding cube associated with the cell
	/// </summary>
	BCube _boundingCube;
	BCube _transformedBoundingCube;
	CallbackRegistration _registration;
	/// <summary>
	/// Grid cell samples associated with the bounding cube associated sampling point
	/// </summary>
	std::shared_ptr<GridCellSample> _cellSamples[8];
	/// <summary>
	///	Grid cell position in the subgrid
	/// </summary>
	glm::ivec3 _gridCellPosition;
	SubGrid* _subGrid = nullptr;
	void OnTranformParamsChanged(const TransformParams& p);
public:
	GridCell(SubGrid* parentGrid, const BCube& cellBoundingCube, glm::ivec3 gridPosition);
	void SetIrradianceOffset();

	void CreateSubGrid();
	void DeleteSubGrid();

	int GetCellIndex();

	SubGrid* AssociatedSubGrid() { return _subGrid; }
	const BCube& GetBoundingCube() const { return _boundingCube; }

	const BCube& GetTransformedBoundingCube() const { return _transformedBoundingCube; }

	~GridCell();
};

class SubGrid {
private:
	const int _level;
	GridData* const _gridData;

	/// <summary>
	/// Associated grid cells array
	/// </summary>
	std::unique_ptr<GridCell>* _cells = nullptr;
	int _subGridIndex;

	int _cachedGridSize;
	bool* _cellsThatNeedSubGrid = nullptr;
	bool* _cellsThatHaveSubGrid = nullptr;

	/// <summary>
	/// Create the cells for the grid from the main grid resolution info
	/// </summary>
	void BuildGridCells(const BCube& gridCube);
	template<class Iterator>
	void CalculateSubGridsState(const Iterator& begin, const Iterator& end);
	bool UpdateSubGrids();
public:
	NO_COPY_AND_ASSIGN(SubGrid);

	SubGrid(const BCube& cube, int level, GridData* gridData);

	template<class Iterator>
	bool UpdateSubGridStructure(const Iterator& begin, const Iterator& end);
	void CorrectIndexes();

	int GetLevel() const { return _level; }
	GridData* GetGridData() const { return _gridData; }
	int GetIndex() const { return _subGridIndex; }
	void SetIndex(int index) { _subGridIndex = index; }

	~SubGrid() {
		_gridData->ReturnSubgridIndex(_subGridIndex);
		delete[] _cells;
		delete[] _cellsThatNeedSubGrid;
		delete[] _cellsThatHaveSubGrid;
	}
};

// <summary>
/// Main volume grid
/// </summary>
class Grid {
private:
	/// <summary>
	/// Number of subdivisions per coordinate
	/// </summary>
	glm::ivec3 _cellsPerCoordinate;
	/// <summary>
	/// Data associated with the grid
	/// </summary>
	GridData* _gridData = nullptr;

	const BCube _boundingCube;
	BCube _transformedBoundingCube;

	SubGrid* _mainSubgrid;

	bool _parallelUpdate = false;
	CallbackRegistration _transformCallback;

	void EnsureBuffersCapacity(RadianceSampler* sampler, VariableShaderBuffer<glm::vec4>& irradianceBuffer) {
		// We have to ensure that the irradiance buffer is big enough.
		// We have to store the data for each sample point we have saved in our map
		int samplesCount = sampler->SamplesCount();
		GLsizeiptr requiredVectorSize = samplesCount * _gridData->GetCellSamples().GetVector().size();

		// If buffer is already big enough, exit
		if (irradianceBuffer.GetVectorLength() >= requiredVectorSize) return;
		irradianceBuffer.SetVectorLength(requiredVectorSize);
	}

	void OnTranformChanged(const TransformParams& p) {
		_transformedBoundingCube = _boundingCube >> p;
	}

	void UpdateSubGridsInfos() {
		// We have to update the information about the sample indexes for each cell in each subgrid
		_gridData->GetInfos().EnsureCellVerticesMappingSpace(_gridData->GetSubGridCount());
		// We have to ensure that our subgrid buffer info is big enougth
		_gridData->_subGridsInfoBuffer.SetVectorLength(_gridData->GetSubGridCount());
		// We have to refresh and writes all the corrected indeces
		_mainSubgrid->CorrectIndexes();

		// We writes -1 to the end of the vector. In this way the shader can exit the loop
		_gridData->_subGridsInfoBuffer.GetVectorPtr()[_gridData->GetSubGridCount() - 1] = -1;

		// We finally writes all the data to the GPU
		_gridData->_subGridsInfoBuffer.Write();
		_gridData->GetInfos().WriteSampleIndexes();
	}

public:
	/// <summary>
	/// Main constructor for the grid. Create a new grid for the specified BoundingCube
	/// </summary>
	Grid(const BCube& gridBoundingCube) : _boundingCube(gridBoundingCube), _mainSubgrid(nullptr) {
		// We set the default grid division (2x2x2)
		SetGridDivision(glm::ivec3(2, 2, 2));

		_transformCallback = _gridData->RegisterTransformChanged(std::bind(&Grid::OnTranformChanged, this, std::placeholders::_1));
	}

	/// <summary>
	/// Returns the transform associated with the grid
	/// </summary>
	const TransformParams GetTransform() const { return _gridData->GetTransform(); }
	void SetTransform(const TransformParams& p) {
		_gridData->SetTransform(p);
	}

	const glm::ivec3& GetGridDivision() const { return _cellsPerCoordinate; }
	bool IsParallelUpdateEnabled() const { return _parallelUpdate; }
	void SetParallelUpdate(bool enabled) { _parallelUpdate = enabled; }
	bool IsDebugColorEnabled() const { return _gridData->GetCellSamples().IsDebugColorEnabled(); }
	void SetDebugColorEnabled(bool value) { _gridData->GetCellSamples().SetDebugColorEnabled(value); }

	void Draw(RadianceSphere* radianceSphere) const;

	void SetGridDivision(const glm::ivec3& numCellsPerDimension)
	{
		TransformParams oldTransform;
		int oldMaxGridLevel = 0;
		bool oldIsDebug = false;
		if (_gridData) {
			oldTransform = _gridData->GetTransform();
			oldMaxGridLevel = _gridData->GetMaxSubGridLevel();
			oldIsDebug = _gridData->GetCellSamples().IsDebugColorEnabled();
		}

		delete _mainSubgrid;
		delete _gridData;

		// Cleanup of all previous data (subgrids, cell, gridata)
		// We have to rebuild our root grid data
		_cellsPerCoordinate = numCellsPerDimension;
		_gridData = new GridData(_boundingCube.Min, _boundingCube.Max);
		// We set the division info in the grid uniform
		_gridData->GetInfos().WriteCellsPerDimension(numCellsPerDimension);

		// We create the new subdivison and we set the old transform
		_mainSubgrid = new SubGrid(_boundingCube, 0, _gridData);
		_gridData->SetTransform(oldTransform);
		_gridData->SetMaxSubGridLevel(oldMaxGridLevel);
		_gridData->GetCellSamples().SetDebugColorEnabled(oldIsDebug);
		UpdateSubGridsInfos();
	}

	template<class Iterator>
	void Update(const Iterator& begin, const Iterator& end, RadianceSampler* sampler);

	int GetMaxSubGridLevel() const { return _gridData->GetMaxSubGridLevel(); }

	void SetMaxSubGridLevel(int value) {
		int oldValue = _gridData->GetMaxSubGridLevel();
		_gridData->SetMaxSubGridLevel(value);

		// We have to reset the division if the level is lowered
		int updateValue = _gridData->GetMaxSubGridLevel();
		if (updateValue < oldValue) {
			SetGridDivision(_cellsPerCoordinate);
		}
	}

	~Grid()
	{
		// We have to clean our structure
		delete _mainSubgrid;
		delete _gridData;
	}
};
