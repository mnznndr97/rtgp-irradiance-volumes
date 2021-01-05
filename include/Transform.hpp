#pragma once

#include <std_include.h>

struct TransformParams {
private:
	glm::vec3 _translation;
	glm::vec3 _scale;
	GLfloat _yRotation;

	glm::mat4 _matrix;
	glm::mat4 _aaMatrix;
public:

	TransformParams() : TransformParams(glm::vec3(0.0f), glm::vec3(1.0f), 0.0f) {
	}

	TransformParams(const glm::vec3& translation, const glm::vec3& scale, GLfloat rotationAngle) : _translation(translation), _scale(scale), _yRotation(rotationAngle) {
		_matrix = glm::translate(glm::mat4(1.0f), _translation);
		// In the aa we don't consider the rotation
		_aaMatrix = glm::scale(_matrix, _scale);

		_matrix = glm::rotate(_matrix, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		_matrix = glm::scale(_matrix, _scale);
	}

	const glm::vec3& Translation() const { return _translation; }
	const glm::vec3& Scale() const { return _scale; }
	const GLfloat YRotation() const { return _yRotation; }
	const glm::mat4& Matrix() const { return _matrix; }
	const glm::mat4& AAMatrix() const { return  _aaMatrix; }

	TransformParams operator+(const TransformParams& b) const {
		glm::vec3 newTranslation(_translation + b.Translation());
		glm::vec3 newScale(_scale * b._scale);
		float angle = _yRotation + b._yRotation;
		return TransformParams(newTranslation, newScale, angle);
	}
};


glm::vec3 operator>>(const glm::vec3& v, const glm::mat4& m)
{
	glm::vec4 transformedPt = m * glm::vec4(v, 1.0f);
	assert(transformedPt.w == 1.0f);
	return glm::vec3(transformedPt);
}

glm::vec3 operator>>(const glm::vec3& v, const TransformParams& p)
{
	return v >> p.Matrix();
}