#pragma once

#include <std_include.h>
#include <set>
#include <irradiancegrid/fwd.h>
#include <irradiancegrid/CellSample.hpp>
#include <irradiancegrid/Cell.hpp>
#include <irradiancegrid/GridData.hpp>

SubGrid::SubGrid(const BCube& cube, int level, GridData* gridData) : _level(level), _gridData(gridData) {

	_subGridIndex = _gridData->GetNewSubgridIndex();
	assert((_level == 0 && _subGridIndex == 0) || (_level > 0 && _subGridIndex > 0));

	const glm::ivec3 cellsPerCoordinate = _gridData->GetInfos().GetData().NumCellsPerDimension;
	_cachedGridSize = cellsPerCoordinate.x * cellsPerCoordinate.y * cellsPerCoordinate.z;

	_cellsThatNeedSubGrid = new bool[_cachedGridSize];
	_cellsThatHaveSubGrid = new bool[_cachedGridSize];
	BuildGridCells(cube);
}

void SubGrid::BuildGridCells(const BCube& gridCube)
{
	// Let's build the points array that are needed to build the various cells bounding box
	// By using the exact same float value for a vertex coord shared by adiacent cells we can use the 
	// hashmap to avoid duplication of sample data on the same vertex
	const glm::ivec3 cellsPerCoordinate = _gridData->GetInfos().GetData().NumCellsPerDimension;
	const glm::vec3 stepPerCoors = (gridCube.Max - gridCube.Min) / glm::vec3(cellsPerCoordinate);

	float* xPoint = new float[cellsPerCoordinate.x + 1];
	xPoint[0] = gridCube.Min.x;
	for (int x = 1; x < cellsPerCoordinate.x; x++)
	{
		xPoint[x] = xPoint[x - 1] + stepPerCoors.x;
	}
	xPoint[cellsPerCoordinate.x] = gridCube.Max.x;

	float* yPoint = new float[cellsPerCoordinate.y + 1];
	yPoint[0] = gridCube.Min.y;
	for (int y = 1; y < cellsPerCoordinate.y; y++)
	{
		yPoint[y] = yPoint[y - 1] + stepPerCoors.y;
	}
	yPoint[cellsPerCoordinate.y] = gridCube.Max.y;

	float* zPoint = new float[cellsPerCoordinate.z + 1];
	zPoint[0] = gridCube.Min.z;
	for (int z = 1; z < cellsPerCoordinate.z; z++)
	{
		zPoint[z] = zPoint[z - 1] + stepPerCoors.z;
	}
	zPoint[cellsPerCoordinate.z] = gridCube.Max.z;

	_cells = new std::unique_ptr<GridCell>[_cachedGridSize];
	// Now we can build our cells
	for (int x = 0; x < cellsPerCoordinate.x; x++)
	{
		for (int y = 0; y < cellsPerCoordinate.y; y++)
		{
			for (int z = 0; z < cellsPerCoordinate.z; z++)
			{
				glm::vec3 minVector(xPoint[x], yPoint[y], zPoint[z]);
				glm::vec3 maxVector(xPoint[x + 1], yPoint[y + 1], zPoint[z + 1]);

				BCube cellBoundingCube = BCube::FromMinMax(minVector, maxVector);
				glm::ivec3 cellPositionVec(x, y, z);
				int cellIndex = (x * cellsPerCoordinate.y * cellsPerCoordinate.z) + (y * cellsPerCoordinate.z) + z;

				_cells[cellIndex] = std::make_unique<GridCell>(this, cellBoundingCube, cellPositionVec);
			}
		}
	}

	delete[] xPoint;
	delete[] yPoint;
	delete[] zPoint;
}

template<class Iterator>
void SubGrid::CalculateSubGridsState(const Iterator& begin, const Iterator& end)
{
	// First we iterate one time to check if a subgrid already exists
	for (int i = 0; i < _cachedGridSize; i++)
	{
		std::unique_ptr<GridCell>& cellPtr = _cells[i];
		_cellsThatHaveSubGrid[i] = cellPtr->AssociatedSubGrid();
	}

	/* FOR THE FUTURE: Here the code is absolutely not optimized since it searches in all the objects
	* proided in the iterator.
	*
	* For our purpose is still ok, since we are considering the performance and the correctness of the
	* sampling algorithm and the quering
	*/

	int cellsThatNeedsCounts = 0;
	bool firstTime = true;
	// If there are a lot of objects but all the cells already needs a grid, we can exit prematurely
	for (Iterator it = begin; it != end && cellsThatNeedsCounts < _cachedGridSize; ++it)
	{
		SceneObject* object = *it;
		for (int i = 0; i < _cachedGridSize; i++)
		{
			// Simple check -> We have already seen that the cell needs a subgrid.
			// We have to make sure to skip other intersection and to avoid reset to false the flag
			if (!firstTime && _cellsThatNeedSubGrid[i]) continue;

			std::unique_ptr<GridCell>& cellPtr = _cells[i];
			bool cellIsNeeded = cellPtr->GetTransformedBoundingCube().IsIntersecting(object->GetTransformedBoundingCube());
			// We directly write the result to avoid memset the entire pointer to 0 every time
			_cellsThatNeedSubGrid[i] = cellIsNeeded;

			cellsThatNeedsCounts += (1 * (int)cellIsNeeded);
		}
		firstTime = false;
	}

	assert(cellsThatNeedsCounts <= _cachedGridSize);
}

bool SubGrid::UpdateSubGrids()
{
	bool somethingChanged = false;
	for (int i = 0; i < _cachedGridSize; i++)
	{
		std::unique_ptr<GridCell>& cellThatNeedSubgrid = _cells[i];
		if (_cellsThatNeedSubGrid[i] || !_cellsThatHaveSubGrid[i]) continue;

		cellThatNeedSubgrid->DeleteSubGrid();
		somethingChanged = true;
	}

	// We now checks for new subgrids
	for (int i = 0; i < _cachedGridSize; i++)
	{
		std::unique_ptr<GridCell>& cellThatNeedSubgrid = _cells[i];
		if (!_cellsThatNeedSubGrid[i] || _cellsThatHaveSubGrid[i]) continue;

		cellThatNeedSubgrid->CreateSubGrid();
		somethingChanged = true;
	}
	return somethingChanged;
}

template<class Iterator>
bool SubGrid::UpdateSubGridStructure(const Iterator& begin, const Iterator& end)
{
	if (_level >= _gridData->GetMaxSubGridLevel()) return false;

	// First we need to check the state of the grid cells (subgrid already present/absent, subgrid needed/not need)
	// In case present-not needed, we need to delete
	// In case absent-needed, we need to create a new subgrid
	CalculateSubGridsState(begin, end);
	bool result = UpdateSubGrids();
	bool subResult = false;

	for (size_t i = 0; i < _cachedGridSize; i++)
	{
		SubGrid* subGrid = _cells[i]->AssociatedSubGrid();
		if (subGrid) {
			bool subGridResult = subGrid->UpdateSubGridStructure(begin, end);
			subResult |= subGridResult;
		}
	}
	return result || subResult;
}

void SubGrid::CorrectIndexes() {

	_gridData->CorrectSubGridIndex(_subGridIndex);

	for (int cellIndex = 0; cellIndex < _cachedGridSize; cellIndex++)
	{
		// index may have not been corrected but the buffer may have been re-bounded so better always rewrite the
		// indexes
		std::unique_ptr<GridCell>& cell = _cells[cellIndex];
		cell->SetIrradianceOffset();

		SubGrid* cellSubGrid = cell->AssociatedSubGrid();
		if (cellSubGrid) {
			// If there is a subgrid we correct it and the we write that the specified subgrid index is related to the cell index
			cellSubGrid->CorrectIndexes();
			_gridData->_subGridsInfoBuffer.GetVectorPtr()[cellSubGrid->GetIndex() - 1] = cell->GetCellIndex();
		}
	}
}
