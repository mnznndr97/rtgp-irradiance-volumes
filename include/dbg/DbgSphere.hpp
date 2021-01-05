#pragma once
#include <std_include.h>
#include <Transform.hpp>

class DbgSphere
{
private:
	glm::vec3 _color = glm::vec3(1.0f, 0.5f, 0.2f);
	Model _sphere;
	Shader _shader;

	void DrawImpl(const glm::vec3& p, float scale, const TransformParams& transform)
	{
		_shader.Use();

		glm::vec3 pointT = p >> transform;
		TransformParams sphereT(pointT, glm::vec3(scale), 0);

		_shader.SetUniform("modelMatrix", sphereT.Matrix());
		_shader.SetUniform("color", _color);
		_sphere.Draw();
	}
public:
	DbgSphere();
	~DbgSphere();

	const glm::vec3& GetColor() const { return _color; }
	void  SetColor(const glm::vec3& value) { _color = value; }



	void Draw(const glm::vec3& p, float scale) const {
		Draw(p, scale, TransformParams());
	}

	void Draw(const glm::vec3& p, float scale, const TransformParams& t) const {
		// Very ugly but this is just for debug
		const_cast<DbgSphere*>(this)->DrawImpl(p, scale, t);
	}
};

DbgSphere::DbgSphere() : _sphere("models/sphere.obj"), _shader("shaders/simple.vert", "shaders/simple.frag")
{
}

DbgSphere::~DbgSphere()
{
}
