#pragma once

#include <std_include.h>
#include <set>
#include <algorithm>
#include <execution>
#include <irradiancegrid/fwd.h>
#include <irradiancegrid/CellSample.hpp>
#include <irradiancegrid/Cell.hpp>
#include <irradiancegrid/GridData.hpp>

inline void Grid::Draw(RadianceSphere* radianceSphere) const {
	const CellSamplesContainer::SamplesVector& samplesMap = _gridData->GetCellSamples().GetVector();
	for (const auto& it : samplesMap)
	{
		it->Draw(radianceSphere);
	}
}

template<class Iterator>
void Grid::Update(const Iterator& begin, const Iterator& end, RadianceSampler* sampler)
{

	// We need to update only the samples count which may be have changed.
	// The transform change is handled by the listener
	_gridData->GetInfos().WriteSamplesCount(sampler->SamplesCount());

	// We first gave to update our subgrids structure and then we have to trim the sample indexes to respect the size of the irradiance buffer
	// This is necessary when for example, in the frame "X" there are two active subgrids
	// and then at frame "X + 1" the first subgrid is removed, leaving some unused space in the buffer
	// Optimization: for the most frames the subgrid structures may not change so we can avoid to 
	// to this control every update call
	if (_mainSubgrid->UpdateSubGridStructure(begin, end)) {
		_gridData->GetCellSamples().Trim();

		UpdateSubGridsInfos();
	}


#if DEBUG
	_gridData->AssertSubGrid();
#endif

	// We can now sample our radiance
	VariableShaderBuffer<glm::vec4>& irradianceBuffer = _gridData->GetIrradianceBuffer();
	EnsureBuffersCapacity(sampler, irradianceBuffer);

	const CellSamplesContainer::SamplesVector& samplesMap = _gridData->GetCellSamples().GetVector();
	if (_parallelUpdate) {
		std::for_each(std::execution::par_unseq, samplesMap.cbegin(), samplesMap.cend(),
			[sampler, &irradianceBuffer](const std::shared_ptr<GridCellSample>& it) {
			it->Update(sampler, irradianceBuffer);
		}
		);
	}
	else
	{
		for (const auto& it : samplesMap)
		{
			it->Update(sampler, irradianceBuffer);
		}
	}

#if DEBUG
	// Just in case of debug let's check that out entire irradiance buffer has some "valid" values
	for (int i = 0; i < irradianceBuffer.GetVectorLength(); i++)
	{
		glm::vec4* irradianceValue = irradianceBuffer.GetVectorPtr() + i;
		assert(irradianceValue->x >= 0.0f);
		assert(irradianceValue->y >= 0.0f);
		assert(irradianceValue->z >= 0.0f);
		assert(irradianceValue->w == 0.0f);
	}
#endif

	// We finally store the irradiance and the transform matrix in the gpu buffers
	irradianceBuffer.Write();
}
