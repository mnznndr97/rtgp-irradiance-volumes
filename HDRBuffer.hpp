#pragma once

#include <std_include.h>

/// <summary>
/// Enables lightting mode
/// 
/// From https://learnopengl.com/Advanced-Lighting/HDR
/// </summary>
class HDRBuffer {
private:
	GLuint _hdrFBO;
	GLuint _colorBuffer;
	GLuint _rboDepth;

	GLuint _renderQuadVAO = 0;
	GLuint _renderQuadVBO;

	Shader _hdrShader;

	float _exposure = 1.0f;

	void GenerateBuffer(GLsizei screenWidth, GLsizei screenHeight) {
		// Create base framebuffer
		glGenFramebuffers(1, &_hdrFBO);
		// Create floating point color buffer with the same size as the screen
		glGenTextures(1, &_colorBuffer);
		glBindTexture(GL_TEXTURE_2D, _colorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// create depth buffer (renderbuffer)
		glGenRenderbuffers(1, &_rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, _rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);

		// Attach buffers to FBO
		glBindFramebuffer(GL_FRAMEBUFFER, _hdrFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBuffer, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rboDepth);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			throw std::exception();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GenerateRenderQuad() {
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &_renderQuadVAO);
		glGenBuffers(1, &_renderQuadVBO);
		glBindVertexArray(_renderQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, _renderQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glBindVertexArray(0);
	}

public:
	HDRBuffer(GLsizei screenWidth, GLsizei screenHeight) : _hdrShader("shaders/hdr.vert", "shaders/hdr.frag") {
		GenerateBuffer(screenWidth, screenHeight);
		GenerateRenderQuad();
	}

	/// <summary>
	/// Binds to the HDR framebuffer to draw on the underlying  texture
	/// </summary>
	void BindForRendering() {
		glBindFramebuffer(GL_FRAMEBUFFER, _hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	/// <summary>
	/// Binds to the default framebuffer for the HDR correction and final draw
	/// </summary>
	void Draw() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		_hdrShader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _colorBuffer);

		glUniform1f(glGetUniformLocation(_hdrShader.Program, "exposure"), 1.0f);

		glBindVertexArray(_renderQuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}

	float GetExposure() const { return _exposure; }
	void SetExposure(float value) { _exposure = value; }
};
