#pragma once

#include <std_include.h>
#include <vector>
#include <cmath>

#include <SceneObject.hpp>
#include <GpuResource.hpp>
#include <buffers/GpuBuffer.hpp>
#include <buffers/VertexArray.hpp>
#include <dbg/DbgLine.hpp>

class Wall : public SceneObject {
private:
	const RayHit s_NoHit;

	Surface* _wallSurface;

	VertexArray _vao;
	GpuBufferT<GL_ARRAY_BUFFER> _vbo;
	GpuBufferT<GL_ELEMENT_ARRAY_BUFFER> _ebo;

	glm::vec3 _minCoords;
	glm::vec3 _maxCoords;

	glm::vec3 _minCoordsT;
	glm::vec3 _maxCoordsT;

	glm::vec3 _planeP0;
	glm::vec3 _normal;

	glm::vec3 _planeP0T;
	glm::vec3 _planeNormalT;
	GLfloat _planeNormalTLength;
	GLfloat _planeDistance;
	DbgLine _dbgLine;

	BCube _emptyBCube;

	void SetupVAO(const glm::vec3 vertices[], int size) {
		_minCoords = _maxCoords = vertices[0];
		for (int i = 1; i < size; i++)
		{
			const glm::vec3& vertex = vertices[i];
			_minCoords.x = min(_minCoords.x, vertex.x);
			_minCoords.y = min(_minCoords.y, vertex.y);
			_minCoords.z = min(_minCoords.z, vertex.z);

			_maxCoords.x = max(_maxCoords.x, vertex.x);
			_maxCoords.y = max(_maxCoords.y, vertex.y);
			_maxCoords.z = max(_maxCoords.z, vertex.z);
		}

		GLushort elements[] = {
			 0, 1, 2, 1, 2, 3
		};

		_vao.Bind();
		_vbo.Bind();
		glBufferData(GL_ARRAY_BUFFER, size * sizeof(glm::vec3), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		_ebo.Bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
		_vao.Unbind();
	}

	void GenerateNormals(const glm::vec3 vertices[], int size) {
		_planeP0 = vertices[0];
		const glm::vec3& point1 = vertices[1];
		const glm::vec3& point2 = vertices[2];

		glm::vec3 vector1 = point1 - _planeP0;
		glm::vec3 vector2 = point2 - _planeP0;

		_normal = glm::normalize(glm::cross(vector1, vector2));
	}
protected:
	virtual void OnTransformChanged() override {
		const TransformParams& transform = GetTransform();
		_planeP0T = _planeP0 >> transform;
		_planeNormalT = _normal >> transform;
		_minCoordsT = _minCoords >> transform;
		_maxCoordsT = _maxCoords >> transform;

		_planeDistance = abs(glm::dot(_planeP0T, _planeNormalT));
		// after the plane distance calculation, we can normalize the
		// transformed normal ??
		_planeNormalTLength = glm::length(_planeNormalT);
	}

public:
	explicit Wall(glm::vec3 vertices[], int verticesCount, Shader parentShader) :
		SceneObject(parentShader),
		_wallSurface(nullptr)
	{
		SetupVAO(vertices, verticesCount);
		GenerateNormals(vertices, verticesCount);

		// In out case a wall is represented by a single surface
		_wallSurface = new Surface();
	}

	virtual ~Wall() override
	{
		delete _wallSurface;
		_wallSurface = nullptr;

		// ~ Not the best solution ~
		// The shader is shared with the Cube object
		// Since in our implementation the CubeWall is used only
		// in the cube, the cube will perform the shader cleanup code
	}


	virtual void Draw() override {
		_shader.Use();
		_shader.SetUniform("modelMatrix", GetTransform().Matrix());
		std::optional<RadianceP> radiance = _wallSurface->GetRadiance();
		if (radiance.has_value()) {
			_shader.SetUniform("color", radiance.value().Value);
		}

		_vao.Bind();
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, (GLvoid*)(3 * sizeof(GLushort)));
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
		_vao.Unbind();
	}

	virtual RayHit IsHitByRay(const Ray& ray) const override {
		// Ray-Plane intersection consider the ray eq and the play eq
		glm::vec3 normalT = _planeNormalT;

		float dirNormalProduct = glm::dot(ray.Direction(), normalT);
		if (dirNormalProduct == 0.0f) {
			// If the product is exactly zero, the ray is parallel to the plane.
			// We can avoid some calculations
			return s_NoHit;
		}

		// ray direction is already normalized
		float angleCos = dirNormalProduct / _planeNormalTLength;

		// In the ray direction has the same verse of the normal, we have to consider
		// the opposite plane normal to have a match
		// (here a plane has two faces so both normal are considered)
		if (angleCos > 0.0f) {
			normalT = -normalT;
			// We should also update the previous calculated dot product
			dirNormalProduct = -dirNormalProduct;

			// Just tu be sure we are doing things correctly
			assert(abs(dirNormalProduct - glm::dot(ray.Direction(), normalT)) < 0.0001);
		}

		float f = -(glm::dot(ray.Position(), normalT) + _planeDistance) / dirNormalProduct;
		if (isinf(f)) {
			// This check may be redundant
			return s_NoHit;
		}

		glm::vec3 intersectionPoint = ray.Position() + (ray.Direction() * f);
		/*
		// This may be more performant due to avoided vec3 copies
		glm::vec3 intersectionPoint = ray.Direction();
		intersectionPoint *= f;
		intersectionPoint += ray.Position();
		*/


		// If the plane is parallel to one axis, the = operator may fail. So we have to consider some epsilon to perform checks
		const float epsilon = 0.0001f;
		if ((intersectionPoint.x >= (_minCoordsT.x - epsilon) && intersectionPoint.x <= (_maxCoordsT.x + epsilon)) &&
			(intersectionPoint.y >= (_minCoordsT.y - epsilon) && intersectionPoint.y <= (_maxCoordsT.y + epsilon)) &&
			(intersectionPoint.z >= (_minCoordsT.z - epsilon) && intersectionPoint.z <= (_maxCoordsT.z + epsilon))) {

			return RayHit(_wallSurface, intersectionPoint, glm::distance(intersectionPoint, ray.Position()), LazyReflection(normalT, ray.Direction()));
		}
		else
		{
			return s_NoHit;
		}
	}

	virtual BCube& GetBoundingCube() override {
		// Not used at the moment
		return _emptyBCube;
	}

	virtual BCube& GetTransformedBoundingCube() override {
		return _emptyBCube;
	}

	Surface* GetSurface() const { return _wallSurface; }
};