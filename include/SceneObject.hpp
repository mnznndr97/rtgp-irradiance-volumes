#pragma once

#include <std_include.h>
#include <vector>

#include <Transform.hpp>
#include <Surface.hpp>
#include <BCube.hpp>

/// <summary>
/// Represents a basic object that will be placed in the scene
/// </summary>
class SceneObject {
private:
	TransformParams _transform;

protected:
	Shader _shader;

	virtual void OnTransformChanged() {
	};

public:
	NO_COPY_AND_ASSIGN(SceneObject);

	/// <remarks>
	/// This is not the best. The caller have to explicitly release the resources
	/// </remarks>
	explicit SceneObject(Shader& shader) : _shader(shader) {
	}

	explicit SceneObject(Shader&& shader) : _shader(std::move(shader)) {
	}

	virtual ~SceneObject() {
		_shader.Delete();
	}

	// A scene object has associated some 
	const TransformParams& GetTransform() const { return _transform; };
	const glm::vec3& GetPosition() const { return _transform.Translation(); };
	const glm::vec3& GetScale() const { return _transform.Scale(); };
	const GLfloat GetYRotation() const { return _transform.YRotation(); };


	void SetYRotation(GLfloat yRot) {
		_transform = TransformParams(_transform.Translation(), _transform.Scale(), yRot);
		OnTransformChanged();
	}

	void SetPosition(const glm::vec3& value) {
		_transform = TransformParams(value, _transform.Scale(), _transform.YRotation());
		OnTransformChanged();
	}

	virtual void SetScale(glm::vec3& value) {
		_transform = TransformParams(_transform.Translation(), value, _transform.YRotation());
		OnTransformChanged();
	};
	virtual void SetScale(glm::vec3&& value) {
		SetScale(value);
	};

	/// <summary>
	/// Abstract definition for a finding a object hit with a ray
	/// </summary>
	virtual RayHit IsHitByRay(const Ray& ray) const = 0;

	virtual BCube& GetBoundingCube() = 0;
	virtual BCube& GetTransformedBoundingCube() = 0;

	virtual void Draw() = 0;
};