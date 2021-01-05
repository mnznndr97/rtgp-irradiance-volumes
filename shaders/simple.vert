#version 420 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;

// ViewMatrices binding is always at index 0
layout (std140, binding = 0) uniform ViewMatrices
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

uniform mat4 modelMatrix;

void main()
{
		// we apply the transformations to the vertex
		vec4 realPos =  vec4(position, 1.0);

        vec4 pos = projectionMatrix * viewMatrix * modelMatrix * realPos;
		// we want to set the Z coordinate of the projected vertex at the maximum depth (i.e., we want Z to be equal to 1.0 after the projection divide)-> we set Z equal to W (because in the projection divide, after clipping, all the components will be divided by W).
		// This means that, during the depth test, the fragments of the environment map will have maximum depth (see comments in the code of the main application)
		gl_Position = pos;
}
