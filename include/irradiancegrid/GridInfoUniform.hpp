#pragma once

#include <type_traits>
#include <std_include.h>
#include <buffers/ShaderStorageBuffer.hpp>

/// <summary>
/// Irradiance grid data that will be associated to a shader buffer object
/// </summary>
/// <remarks>
/// WE HAVE TO ALIGN THE VEC3 STRUCTURE TO A 16-BYTE BOUNDARY
/// due to the alignment in the shaders. 
/// https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL (Either 2N or 4N. This means that a vec3 has a base alignment of 4N)
/// https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
/// 
/// For some reasons, the alignas() operator seems to do nothing for the alignment (at least with the cl compiler)
/// </remarks>
struct IrradianceGridData
{
private:

public:
	/// <summary>
	/// Grid minimum coords
	/// </summary>
	glm::vec3 GridMin;
	float __aligment1__ = 0.0f;
	/// <summary>
	/// Grid maximum coords
	/// </summary>
	glm::vec3 GridMax;
	float __aligment2__ = 0.0f;
	/// <summary>
	/// Number of cell per dimension
	/// </summary>
	glm::ivec3 NumCellsPerDimension;
	int __aligment3__ = 0.0f;
	/// <summary>
	/// Grid transform
	/// </summary>
	glm::mat4 GridTransform;
	// This will be 8-byte aligned
	int SamplesResolution;

	/// <summary>
	/// Map for the eight vertices of a cell to the corresponding irradiance offset
	/// 
	/// The array is indexed with: SubGridOffset x CellX x CellY x CellZ x VertexIndex [0*8]
	/// The SubGridOffset is relative to the subgrid in witch the irradiance offset are calculated (SubgridOffset 0 is the root grid)
	/// CellX, Y, Z identify a cell in the indexed subgrid
	/// Vertex index represents a vertex in the cell
	///
	/// The entry in the array correspond to the offset of the irradiance calculate. For example, the first vertex sample (in the entire grid) will have a offset 0 * numSamples in the irradiance buffer, the second calculated vertex
	/// will have a offset of 1 * numSamples. So for example, the first grid in the cell [0:subgrid, 0:cellx, 0:celly, 0:celly] will index the vertex 0 to 8 with offset 0 to 8.
	/// Another grid in the cell will index the vertex it' s own vertexes along with vertexes shared with adjacent cells
	/// </summary>
	int* CellsVerticesToSamplesMap = nullptr;

	explicit IrradianceGridData() : GridMin(glm::vec3(0.0f)), GridMax(glm::vec3(0.0f)), 
	 NumCellsPerDimension(glm::ivec3(0)),
	 GridTransform(glm::mat4(0.0f)), SamplesResolution(0) {

	}

	NO_COPY_AND_ASSIGN(IrradianceGridData);
	IrradianceGridData(IrradianceGridData&& other) noexcept
		: GridMin(other.GridMin),
		GridMax(other.GridMax),
		NumCellsPerDimension(other.NumCellsPerDimension),
		GridTransform(other.GridTransform),
		SamplesResolution(other.SamplesResolution),
		CellsVerticesToSamplesMap(other.CellsVerticesToSamplesMap)
	{
	}

	IrradianceGridData& operator=(IrradianceGridData&& other) noexcept
	{
		if (this == &other)
			return *this;
		GridMin = other.GridMin;
		GridMax = other.GridMax;
		NumCellsPerDimension = other.NumCellsPerDimension;
		GridTransform = other.GridTransform;
		CellsVerticesToSamplesMap = other.CellsVerticesToSamplesMap;
		return *this;
	}

	~IrradianceGridData()
	{
		delete[] CellsVerticesToSamplesMap;
	}

	void ResizeBuffer(int size)
	{
		if (CellsVerticesToSamplesMap)
		{
			delete[] CellsVerticesToSamplesMap;
		}
		CellsVerticesToSamplesMap = new int[size];
	}
};

class GridInfoUniform : ShaderStorageBuffer<IrradianceGridData> {
private:
	IrradianceGridData _gridData;
	int yOffsetCache;
	int xOffsetCache;
	int subGridOffsetCache;
	int _lastCellsMapBufferSize = 0;
public:
	GridInfoUniform(const glm::vec3& gridMin, const glm::vec3& gridMax) : ShaderStorageBuffer(1) {
		_gridData.GridMin = gridMin;
		_gridData.GridMax = gridMax;

		yOffsetCache = 0;
		xOffsetCache = 0;
		subGridOffsetCache = 0;

		// Assert just to ensure that we have the required 16 bytes alignment with the vec3 fields
		assert(offsetof(IrradianceGridData, GridMax) % 16 == 0);
		assert(offsetof(IrradianceGridData, NumCellsPerDimension) % 16 == 0);

		WriteBaseFields();
	}

	/// <summary>
	/// Ensure that the cells vertices to samples mapping array is big enought
	/// </summary>
	void EnsureCellVerticesMappingSpace(int subGridsCount)
	{
		// To avoid too many allocation/deallocation, let' s resize the buffer only
		// if we have a bigger data size
		int irrOffsetArrayRequiredSize = subGridsCount * subGridOffsetCache;
		if (irrOffsetArrayRequiredSize <= _lastCellsMapBufferSize) return;

		int totalShaderBufferByteSize = offsetof(IrradianceGridData, CellsVerticesToSamplesMap) + (sizeof(int) * irrOffsetArrayRequiredSize);
		if (totalShaderBufferByteSize >= GetMaxSize()) throw std::out_of_range("Buffer max size exceeded");

		_lastCellsMapBufferSize = irrOffsetArrayRequiredSize;
		// We have to update our buffer map
		_gridData.ResizeBuffer(irrOffsetArrayRequiredSize);
		RebindBuffer(totalShaderBufferByteSize);

		// We have to update our initial data since we have re-bound the buffer
		WriteBaseFields();
	}

	void SetCellIrradianceOffset(int subGridIndex, const glm::ivec3& cellPosition, int sampleIndexInCell, int irradianceOffset) {
		const int sampleIndexIn1d = (subGridIndex * subGridOffsetCache) + (cellPosition.x * xOffsetCache) + (cellPosition.y * yOffsetCache) + (cellPosition.z * 8) + sampleIndexInCell;
		_gridData.CellsVerticesToSamplesMap[sampleIndexIn1d] = irradianceOffset;

		// std::cout << "Writing subgrid " << subGridIndex << "[ " << cellPosition << "] (" << sampleIndexIn1d << ") irradiance offset: " << irradianceOffset << std::endl;
	}

	void WriteBaseFields()
	{
		UpdateFieldData(_gridData, &IrradianceGridData::GridMin);
		UpdateFieldData(_gridData, &IrradianceGridData::GridMax);
		UpdateFieldData(_gridData, &IrradianceGridData::NumCellsPerDimension);
		UpdateFieldData(_gridData, &IrradianceGridData::GridTransform);
		UpdateFieldData(_gridData, &IrradianceGridData::SamplesResolution);
	}

	void WriteCellsPerDimension(const glm::ivec3& cellsPerDimension)
	{
		_gridData.NumCellsPerDimension = cellsPerDimension;
		yOffsetCache = cellsPerDimension.z * 8;
		xOffsetCache = cellsPerDimension.y * yOffsetCache;
		subGridOffsetCache = cellsPerDimension.x * xOffsetCache;
		UpdateFieldData(_gridData, &IrradianceGridData::NumCellsPerDimension);
	}

	void WriteGridTransform(const glm::mat4& transform) {
		_gridData.GridTransform = transform;
		UpdateFieldData(_gridData, &IrradianceGridData::GridTransform);
	}

	void WriteSamplesCount(int samplesCount) {
		if (samplesCount == _gridData.SamplesResolution) return;

		_gridData.SamplesResolution = samplesCount;
		UpdateFieldData(_gridData, &IrradianceGridData::SamplesResolution);
	}


	void WriteSampleIndexes() {
		UpdatePointerFieldData<int*>(_gridData, &IrradianceGridData::CellsVerticesToSamplesMap, sizeof(int) * _lastCellsMapBufferSize);
	}

	const IrradianceGridData& GetData() const { return _gridData; }
};