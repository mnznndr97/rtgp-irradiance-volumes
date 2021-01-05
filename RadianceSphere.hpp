#pragma once

#include <std_include.h>	

class RadianceSphere
{
private:
	glm::vec3 _color = glm::vec3(1.0f, 0.5f, 0.2f);
	Model _sphere;
	Shader _shader;
	bool _debugColor = false;

public:
	RadianceSphere() : _sphere("models/sphere.obj"), _shader("shaders/radiance.vert", "shaders/radiance.frag") {

	}

	~RadianceSphere()
	{

	}

	void SetDebugColor(bool value) { _debugColor = value; };

	void Draw(const glm::vec3& translation, float scale, int offset, int samples)
	{
		//UpdateRadianceBuffers(sampler);

		_shader.Use();

		glm::mat4 sphereTransform = glm::translate(glm::mat4(1.0f), translation);
		sphereTransform = glm::scale(sphereTransform, glm::vec3(scale));

		// we pass projection and view matrices to the Shader Program of the plane
		_shader.SetUniform("modelMatrix", sphereTransform);
		_shader.SetUniform("debugColor", (int)_debugColor);
		_shader.SetUniform("samplesCount", samples);
		_shader.SetUniform("sampleOffset", offset);
		_sphere.Draw();
	}
};

