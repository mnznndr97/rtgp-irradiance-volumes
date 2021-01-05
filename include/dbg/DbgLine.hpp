#pragma once

#include <std_include.h>
#include <buffers/VertexArray.hpp>
#include <buffers/GpuBuffer.hpp>
#include <Transform.hpp>
#include <dbg/DbgSphere.hpp>


class DbgLine {
private:
	glm::vec3 _color = glm::vec3(1.0f, 0.5f, 0.2f);
	Shader _shader;

	VertexArray _vao;
	GpuBufferT<GL_ARRAY_BUFFER> _vbo;
	GpuBufferT<GL_ELEMENT_ARRAY_BUFFER> _ebo;
	DbgSphere _sphere;
	const TransformParams _emptyParams;

	void DrawImpl(const glm::vec3& p1, const glm::vec3& p2, const TransformParams& t) {
		glm::vec3 dummyVertex[] = { p1, p2 };
		_vbo.Bind();
		glBufferData(GL_ARRAY_BUFFER, sizeof(dummyVertex), dummyVertex, GL_DYNAMIC_DRAW);
		_vbo.Unbind();

		_shader.Use();
		_shader.SetUniform("modelMatrix", t.Matrix());

		_vao.Bind();
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_SHORT, 0);
		_vao.Unbind();

		_sphere.Draw(p1, 0.05f, t);
		_sphere.Draw(p2, 0.05f, t);
	}

public:
	NO_COPY_AND_ASSIGN(DbgLine);

	DbgLine() : _shader("shaders/simple.vert", "shaders/simple.frag") {
		glm::vec3 dummyVertex[] = { glm::vec3(0.0f), glm::vec3(0.0f) };
		GLushort elements[] = { 0, 1 };

		_vao.Bind();
		_vbo.Bind();

		// The vertex will be changed frequently
		glBufferData(GL_ARRAY_BUFFER, sizeof(dummyVertex), dummyVertex, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		_ebo.Bind();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
		_vao.Unbind();
	}

	void Draw(const glm::vec3& p1, const glm::vec3& p2) const {
		Draw(p1, p2, _emptyParams);
	}

	void Draw(const glm::vec3& p1, const glm::vec3& p2, const TransformParams& t) const {
		// Very ugly but this is just for debug
		const_cast<DbgLine*>(this)->DrawImpl(p1, p2, t);
	}

	~DbgLine() {
		_shader.Delete();
	}
};