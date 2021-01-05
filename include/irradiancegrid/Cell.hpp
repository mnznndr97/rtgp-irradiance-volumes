#pragma once

#include <std_include.h>
#include <unordered_map>
#include <memory>
#include <queue>
#include <cassert>
#include <RadianceSampler.hpp>
#include <irradiancegrid/GridInfoUniform.hpp>
#include <irradiancegrid/fwd.h>
#include <irradiancegrid/CellSample.hpp> 
#include <irradiancegrid/Grid.hpp> 
#include <irradiancegrid/SubGrid.hpp> 
#include <BCube.hpp>

vector<glm::vec3> GetSpatialSamples(const BCube& _volumeCube) {
	vector<glm::vec3> result;
	// Sample order for trilinear implementation (the corner is a binary number representing the index of the 
	// sample) The bit "0" represents a coordinate in the min vector, "1" in the max vector

	/*
	*
   (011)   .+----------+ (111)
		 .' |         .|
	   .'   |  (110).' |
(010) +-----+-----+'   |
	  |     |     |    |
	  |    .+-----|----+ (101)
	  |  .' (001) |  .'
	  | '         | '
	  +-----------+
(000)              (100)

	  */


	  // Sample 000
	result.push_back(glm::vec3(_volumeCube.Min.x, _volumeCube.Min.y, _volumeCube.Min.z));
	// Sample 001
	result.push_back(glm::vec3(_volumeCube.Min.x, _volumeCube.Min.y, _volumeCube.Max.z));
	// Sample 010
	result.push_back(glm::vec3(_volumeCube.Min.x, _volumeCube.Max.y, _volumeCube.Min.z));
	// Sample 011
	result.push_back(glm::vec3(_volumeCube.Min.x, _volumeCube.Max.y, _volumeCube.Max.z));
	// Sample 100
	result.push_back(glm::vec3(_volumeCube.Max.x, _volumeCube.Min.y, _volumeCube.Min.z));
	// Sample 101
	result.push_back(glm::vec3(_volumeCube.Max.x, _volumeCube.Min.y, _volumeCube.Max.z));
	// Sample 110
	result.push_back(glm::vec3(_volumeCube.Max.x, _volumeCube.Max.y, _volumeCube.Min.z));
	// Sample 111
	result.push_back(glm::vec3(_volumeCube.Max.x, _volumeCube.Max.y, _volumeCube.Max.z));
	return result;
}


GridCell::GridCell(SubGrid* parentGrid, const BCube& cellBoundingCube, glm::ivec3 gridPosition) : _parent(parentGrid), _boundingCube(cellBoundingCube), _gridCellPosition(gridPosition)
{
	GridData* gridData = _parent->GetGridData();
	const TransformParams& transform = gridData->GetTransform();

	int i = 0;
	for (const glm::vec3& vertex : GetSpatialSamples(cellBoundingCube))
	{
		std::shared_ptr<GridCellSample> cellSampleInGrid = gridData->GetCellSamples().GetCellSample(vertex, transform);
		_cellSamples[i] = cellSampleInGrid;
		++i;
	}
	_transformedBoundingCube = _boundingCube >> transform;
	_registration = parentGrid->GetGridData()->RegisterTransformChanged(std::bind(&GridCell::OnTranformParamsChanged, this, std::placeholders::_1));
}

void GridCell::OnTranformParamsChanged(const TransformParams& p)
{
	// We update the cell bounding cube and the sampling points
	_transformedBoundingCube = _boundingCube >> p;
	for (int i = 0; i < 8; i++)
	{
		_cellSamples[i]->SetTransform(p);
	}
}

void GridCell::CreateSubGrid()
{
	_subGrid = new SubGrid(_boundingCube, _parent->GetLevel() + 1, _parent->GetGridData());
}

void GridCell::DeleteSubGrid()
{
	delete _subGrid;
	_subGrid = nullptr;
}


int GridCell::GetCellIndex()
{
	const glm::ivec3& dim = _parent->GetGridData()->GetInfos().GetData().NumCellsPerDimension;

	return (_parent->GetIndex() * dim.x * dim.y * dim.z)
		+ (_gridCellPosition.x * dim.y * dim.z)
		+ (_gridCellPosition.y * dim.z)
		+ (_gridCellPosition.z);

}

void GridCell::SetIrradianceOffset()
{
	GridInfoUniform& data = _parent->GetGridData()->GetInfos();
	int containerSubGridIndex = _parent->GetIndex();
	assert(containerSubGridIndex >= 0 && containerSubGridIndex < _parent->GetGridData()->GetSubGridCount());

	for (int sampleIndex = 0; sampleIndex < 8; sampleIndex++)
	{
		data.SetCellIrradianceOffset(containerSubGridIndex, _gridCellPosition, sampleIndex, _cellSamples[sampleIndex]->GetSampleGridIndex());
	}
}

GridCell::~GridCell()
{
	_parent->GetGridData()->UnregisterTransformChanged(_registration);
	delete _subGrid;
}