#pragma once

#include <std_include.h>	

class DebuggingShere
{
public:
	DebuggingShere();
	~DebuggingShere();


	const glm::vec3& GetColor() const { return _color; }
	void  SetColor(const glm::vec3& value) { _color = value; }


	void Draw(const glm::vec3& translation, float scale)
	{
		_shader.Use();

		glm::mat4 sphereTransform = glm::translate(glm::mat4(1.0f), translation);
		sphereTransform = glm::scale(sphereTransform, glm::vec3(scale));
		// we pass projection and view matrices to the Shader Program of the plane
		glUniformMatrix4fv(glGetUniformLocation(_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereTransform));
		glUniform3fv(glGetUniformLocation(_shader.Program, "color"), 1, glm::value_ptr(_color));
		_sphere.Draw();
	}
private:
	glm::vec3 _color = glm::vec3(1.0f, 0.5f, 0.2f);
	Model _sphere;
	Shader _shader;
};

DebuggingShere::DebuggingShere() : _sphere("models/sphere.obj"), _shader("shaders/simple.vert", "shaders/simple.frag")
{
}

DebuggingShere::~DebuggingShere()
{
}
