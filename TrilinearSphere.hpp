#pragma once

#include <std_include.h>
#include <irradiancegrid/Grid.hpp>
#include <BCube.hpp>

/// <summary>
/// Sphere that implements the irradiance interpolation
/// </summary>
class TrilinearSphere : public SceneObject
{
private:
	Model _sphere;
	BCube _boundingCube;
	BCube _transformedBoundingCube;
	bool _debugColor = false;
	glm::mat3 _normalMatrix;
protected:
	virtual void OnTransformChanged() override{
		_transformedBoundingCube = _boundingCube >> GetTransform();
		_normalMatrix = glm::inverseTranspose(glm::mat3(GetTransform().Matrix()));
	}

public:
	TrilinearSphere() : SceneObject(Shader("shaders/trilinear.vert", "shaders/trilinear.frag")), _sphere("models/sphere.obj"), _normalMatrix(glm::mat3(1.0f)) {
		_boundingCube = BCube::FromMeshes(_sphere.meshes.cbegin(), _sphere.meshes.cend());

		SetPosition(glm::vec3(-0.75f));
		SetScale(glm::vec3(0.30));
	}

	virtual RayHit IsHitByRay(const Ray& ray) const override {
		return RayHit();
	}

	virtual BCube& GetBoundingCube() override {
		return _boundingCube;
	}

	virtual BCube& GetTransformedBoundingCube() override {
		return _transformedBoundingCube;
	}

	void SetDebugColor(bool value) { _debugColor = value; }

	virtual void Draw() override
	{
		_shader.Use();
		const TransformParams& sphereTransform = GetTransform();
		_shader.SetUniform("modelMatrix", sphereTransform.Matrix());
		_shader.SetUniform("normalMatrix", _normalMatrix);
		_shader.SetUniform("debugColor", (int)_debugColor);

		_sphere.Draw();
	}
};