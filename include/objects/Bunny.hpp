#pragma once

#include <std_include.h>	
#include <SceneObject.hpp>
#include <dbg/DbgLine.hpp>
class Bunny : public SceneObject
{

private:
	Model _model;
	BCube _boundingCube;
	BCube _transformedBoundingCube;
	bool _debugColor = false;
	DbgLine _line;
	glm::mat3 _normalMatrix;
protected:
	virtual void OnTransformChanged() override {
		_transformedBoundingCube = _boundingCube >> GetTransform();
		_normalMatrix = glm::inverseTranspose(glm::mat3(GetTransform().Matrix()));
	}
public:
	Bunny() : SceneObject(Shader("shaders/trilinear.vert", "shaders/trilinear.frag")), _model("models/bunny_lp.obj"), _normalMatrix(glm::mat3(1.0f)) {
		_boundingCube = BCube::FromMeshes(_model.meshes.cbegin(), _model.meshes.cend(), true);


		SetPosition(glm::vec3(-0.75f));
		SetScale(glm::vec3(0.2f));
	}

	virtual RayHit IsHitByRay(const Ray& ray) const override {
		// Not needed at the moment
		return RayHit();
	}

	virtual BCube& GetBoundingCube() override {
		return _boundingCube;
	}

	virtual BCube& GetTransformedBoundingCube() override {
		return _transformedBoundingCube;
	}

	void SetDebugColor(bool value) { _debugColor = value; }

	void Draw() override
	{
		_shader.Use();
		const TransformParams& transform = GetTransform();
		_shader.SetUniform("modelMatrix", transform.Matrix());
		_shader.SetUniform("normalMatrix", _normalMatrix);
		_shader.SetUniform("debugColor", (int)_debugColor);
		_model.Draw();

		//BCube::Draw(_transformedBoundingCube, _line);		
	}
};
