#pragma once

#include <std_include.h>
#include <SemisphereMap.hpp>
#include <buffers/GpuBuffer.hpp>
#include <buffers/VertexArray.hpp>
#include <dbg/DbgSphere.hpp>

/// <summary>
/// Provides a sampling direction list build from a unit hemisphere with a certain resolution
/// </summary>
class UnitHemisphereDirections
{
private:
	/// <summary>
	/// Number of cell per row/column to consider
	/// </summary>
	GLint _resolution;
	/// <summary>
	/// Cell lentgh
	/// </summary>
	float _cellSegmentSize;
	/// <summary>
	/// Half cell length
	/// </summary>
	float _cellHalf;

	/// <summary>
	/// Map function for the creation of the semisphere
	/// </summary>
	std::function<glm::vec3(const glm::vec2&)> _mapFunction;

	/// <summary>
	/// List of available sampling point given the specified resolution
	/// </summary>
	vector<glm::vec3> _samplingDirections;

#if DEBUG
	Shader _shader;
	VertexArray _dbgVao;
	GpuBufferT<GL_ARRAY_BUFFER> _dbgVbo;
	GpuBufferT<GL_ELEMENT_ARRAY_BUFFER> _dbgEbo;
	DbgSphere _dbgSphere;

	void BuildDebugInfo();
#endif

	void BuildSamplingPoints() {
		_samplingDirections.clear();

		// We then need the real 2D sampling point in the center of each cell
		// In this first the se sampling point is equals to the lower left point of the cell
		vector<glm::vec2> sampling2DPoints;
		sampling2DPoints.reserve(_resolution * _resolution);
		// Columns point preparation and sampling point
		for (int col = 0; col <= _resolution; col++)
			for (int row = 0; row <= _resolution; row++)
			{
				glm::vec2 point = glm::vec2(col, row);
				if (row < _resolution && col < _resolution)
					sampling2DPoints.push_back(point);
			}

		// We now builds the unit hemisphere to get the sampling directions
		for (auto i = 0; i < sampling2DPoints.size(); i++)
		{
			// We move our sampling point to the center of the cell
			glm::vec2 data = sampling2DPoints[i] * _cellSegmentSize + _cellHalf;
			glm::vec3 point = _mapFunction(data);


			// We save the sampling point and the negative-y sampling point
			_samplingDirections.insert(_samplingDirections.begin() + i, point);
			_samplingDirections.push_back(glm::vec3(point.x, -point.y, point.z));
		}

#if DEBUG
		for (int i = 0; i < _samplingDirections.size() / 2; i++)
		{
			const glm::vec3& upperHalfVector = _samplingDirections[i];
			const glm::vec3& lowerHalfVector = _samplingDirections[i + _samplingDirections.size() / 2];

			assert(upperHalfVector.x == lowerHalfVector.x);
			assert(-upperHalfVector.y == lowerHalfVector.y);
			assert(upperHalfVector.z == lowerHalfVector.z);
		}

		BuildDebugInfo();
#endif
	}

	/// <summary>
	/// Identity Map funtion to display the grid instead of the hemisphere
	/// </summary>
	static glm::vec3 XYDebugMap(const glm::vec2& data) {
		return glm::vec3(data.x, data.y, 0.0f);
	}
public:
	NO_COPY_AND_ASSIGN(UnitHemisphereDirections);

	UnitHemisphereDirections()
		: _mapFunction(PointToSemisphere)
#if DEBUG
		, _shader("shaders/hemi.vert", "shaders/hemi.frag")
#endif	
	{
		// Let's use the default resolution of 9x9 grid
		SetResolution(9);
	}

	int GetResolution() const { return _resolution; }
	float GetHalfCell() const { return _cellHalf; }

	void SetResolution(int rows) {
		_resolution = rows;
		_cellSegmentSize = 1.0f / ((float)_resolution);
		_cellHalf = _cellSegmentSize / 2.0f;
		BuildSamplingPoints();
	}

	/// <summary>
	/// Return the list of sampling direction
	/// </summary>
	const std::vector<glm::vec3>& GetSamplingDirections() const { return _samplingDirections; }

#if DEBUG
	void Draw(const glm::vec3& position);
#endif // DEBUG
};

#if DEBUG
void UnitHemisphereDirections::BuildDebugInfo() {
	// We build the 2d point in the grid necessary to draw the grid columns lines and row lines
	// This lines are only for debug purposes
	vector<glm::vec2> dbgColumnsLines2DPoints, dbgRowsLines2DPoints;
	dbgColumnsLines2DPoints.reserve(_resolution * _resolution);
	dbgRowsLines2DPoints.reserve(_resolution * _resolution);

	// Columns point preparation and sampling point
	for (int col = 0; col <= _resolution; col++) {
		for (int row = 0; row <= _resolution; row++)
		{
			dbgColumnsLines2DPoints.push_back(glm::vec2(col, row));
			dbgRowsLines2DPoints.push_back(glm::vec2(row, col));
		}

	}

	int dbgPointsCount = dbgColumnsLines2DPoints.size() + dbgRowsLines2DPoints.size();
	// Now we can compute our (debug) mapped vertices (xyzw coords)
	glm::vec3* dbgVertices = new glm::vec3[dbgPointsCount];
	for (int i = 0, j = 0; i < dbgPointsCount; i++, j += 1)
	{
		// We have to normalze our point with the resolution we used
		glm::vec2 data = (i < dbgColumnsLines2DPoints.size() ?
			dbgColumnsLines2DPoints[i] :
			dbgRowsLines2DPoints[i - dbgColumnsLines2DPoints.size()]) * _cellSegmentSize;
		glm::vec3 semisphere = _mapFunction(data);

		dbgVertices[j] = semisphere;
	}

	// And finally we have to build the indexes array
	GLushort* dbgElements = new GLushort[dbgPointsCount];
	for (int i = 0; i < dbgPointsCount; i++)
	{
		dbgElements[i] = i;
	}


	_dbgVao.Bind();
	_dbgVbo.Bind();
	glBufferData(GL_ARRAY_BUFFER, dbgPointsCount * sizeof(glm::vec3), dbgVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(0);

	_dbgEbo.Bind();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, dbgPointsCount * sizeof(GLushort), dbgElements, GL_STATIC_DRAW);
	_dbgVao.Unbind();

	delete[] dbgVertices;
	delete[] dbgElements;
}

void UnitHemisphereDirections::Draw(const glm::vec3& position)
{
	_shader.Use();
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);

	// we pass projection and view matrices to the Shader Program of the plane
	_shader.SetUniform("modelMatrix", transform);

	_dbgVao.Bind();
	// We have to loop for each column and for each row
	for (int rowCol = 0; rowCol < (_resolution + 1) * 2; rowCol++)
	{
		// 
		for (int cell = 0; cell < _resolution; cell++)
		{
			glDrawElements(GL_LINES, 2, GL_UNSIGNED_SHORT, (GLvoid*)((rowCol * (_resolution + 1) + cell) * sizeof(GLushort)));
		}
	}
	_dbgVao.Unbind();

	_dbgSphere.SetColor(glm::vec3(1.0f, 0.5f, 0.2f));
	for (int i = 0; i < _samplingDirections.size(); i++)
	{
		const glm::vec3& samplingPoint = _samplingDirections[i];
		_dbgSphere.Draw(position + samplingPoint, (1.0f / _resolution) / 10.0f);
	}

	_dbgSphere.SetColor(glm::vec3(0.2f, 0.5f, 1.0f));
	_dbgSphere.Draw(position, 0.025f);
}
#endif

