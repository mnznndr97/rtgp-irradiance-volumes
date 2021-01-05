#pragma once

#include <std_include.h>
#include <Transform.hpp>
#include <cmath>
#include <dbg/DbgLine.hpp>

struct BCube {

private:
	BCube(const glm::vec3& cubeMin, const glm::vec3& cubeMax) : Min(cubeMin), Max(cubeMax) {
		/*assert(cubeMin.x <= cubeMax.x);
		assert(cubeMin.y <= cubeMax.y);
		assert(cubeMin.z <= cubeMax.z);*/
	}

public:
	glm::vec3 Min;
	glm::vec3 Max;

	BCube() : Min(glm::vec3(0.0f)), Max(glm::vec3(0.0f)) {
	}

	bool IsIntersecting(const BCube& other) const
	{
		// collision x-axis?
		bool collisionX = Max.x >= other.Min.x && other.Max.x >= Min.x;
		// collision y-axis?
		bool collisionY = Max.y >= other.Min.y && other.Max.y >= Min.y;
		// collision z-axis?
		bool collisionZ = Max.z >= other.Min.z && other.Max.z >= Min.z;
		return collisionX && collisionY && collisionZ;
	}

	/* Static section */
private:
	static void UpdateMinMax(const Mesh& mesh, glm::vec3& minValues, glm::vec3& maxValues) {
		for (const Vertex& vertex : mesh.vertices)
		{
			if (vertex.Position.x < minValues.x) minValues.x = vertex.Position.x;
			if (vertex.Position.x > maxValues.x) maxValues.x = vertex.Position.x;
			if (vertex.Position.y < minValues.y) minValues.y = vertex.Position.y;
			if (vertex.Position.y > maxValues.y) maxValues.y = vertex.Position.y;
			if (vertex.Position.z < minValues.z) minValues.z = vertex.Position.z;
			if (vertex.Position.z > maxValues.z) maxValues.z = vertex.Position.z;
		}
	}
public:
	static BCube FromMinMax(const glm::vec3& minV, const glm::vec3& maxV) {
		return BCube(minV, maxV);
	}

	template<class Iterator>
	static BCube FromMeshes(const Iterator& begin, const Iterator& end) {
		return FromMeshes(begin, end, false);
	}

	template<class Iterator>
	static BCube FromMeshes(const Iterator& begin, const Iterator& end, bool considerRotation)
	{
		glm::vec3 minValues = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 maxValues = glm::vec3(std::numeric_limits<float>::min());

		for (Iterator it = begin; it != end; ++it)
		{
			const Mesh& mesh = *it;
			UpdateMinMax(mesh, minValues, maxValues);
		}

		if (considerRotation)
		{
			// We get the absolute min and max and multiply them by sqrt(2) as they
			// where 45° rotated to obtain a sligth larger AABB that should support all y rotations
			// An OBB should be used though

			minValues = glm::vec3(min(min(minValues.x, minValues.y), minValues.z)) * sqrt(2.0f);
			maxValues = glm::vec3(max(max(maxValues.x, maxValues.y), maxValues.z)) * sqrt(2.0f);
		}	
		return BCube(minValues, maxValues);
	}

	static void Draw(const BCube& cube, const DbgLine& line) {
		line.Draw(cube.Min, glm::vec3(cube.Min.x, cube.Min.y, cube.Max.z));
		line.Draw(cube.Min, glm::vec3(cube.Min.x, cube.Max.y, cube.Min.z));
		line.Draw(cube.Min, glm::vec3(cube.Max.x, cube.Min.y, cube.Min.z));
		line.Draw(cube.Max, glm::vec3(cube.Max.x, cube.Max.y, cube.Min.z));
		line.Draw(cube.Max, glm::vec3(cube.Max.x, cube.Min.y, cube.Max.z));
		line.Draw(cube.Max, glm::vec3(cube.Min.x, cube.Max.y, cube.Max.z));

		line.Draw(glm::vec3(cube.Max.x, cube.Min.y, cube.Max.z), glm::vec3(cube.Min.x, cube.Min.y, cube.Max.z));
		line.Draw(glm::vec3(cube.Max.x, cube.Min.y, cube.Max.z), glm::vec3(cube.Max.x, cube.Min.y, cube.Min.z));

		line.Draw(glm::vec3(cube.Min.x, cube.Max.y, cube.Max.z), glm::vec3(cube.Min.x, cube.Min.y, cube.Max.z));
		line.Draw(glm::vec3(cube.Min.x, cube.Max.y, cube.Max.z), glm::vec3(cube.Min.x, cube.Max.y, cube.Min.z));

		line.Draw(glm::vec3(cube.Max.x, cube.Max.y, cube.Min.z), glm::vec3(cube.Max.x, cube.Min.y, cube.Min.z));
		line.Draw(glm::vec3(cube.Max.x, cube.Max.y, cube.Min.z), glm::vec3(cube.Min.x, cube.Max.y, cube.Min.z));
	}
};

BCube operator>>(const BCube& cube, const TransformParams& p)
{
	glm::mat4 matrix = p.AAMatrix();
	return BCube::FromMinMax(cube.Min >> matrix, cube.Max >> matrix);
}