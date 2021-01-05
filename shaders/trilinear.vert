/*
17_skybox.vert: vertex shader for the visualization of the cube map as environment map

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2019/2020
Master degree in Computer Science
Universita' degli Studi di Milano
*/

/*
OpenGL coordinate system (right-handed)
positive X axis points right
positive Y axis points up
positive Z axis points "outside" the screen


                              Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
*/


#version 430 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

// ViewMatrices binding is always at index 0
layout (std140, binding = 0) uniform ViewMatrices
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
out vec3 interpNormal;
out vec3 interpPosition;

void main()
{
		// we apply the transformations to the vertex
		vec4 realPos =  vec4(position, 1.0);

        vec4 pos = projectionMatrix * viewMatrix * modelMatrix * realPos;
		// we want to set the Z coordinate of the projected vertex at the maximum depth (i.e., we want Z to be equal to 1.0 after the projection divide)-> we set Z equal to W (because in the projection divide, after clipping, all the components will be divided by W).
		// This means that, during the depth test, the fragments of the environment map will have maximum depth (see comments in the code of the main application)
		gl_Position = pos;

        interpPosition = vec3(modelMatrix * realPos);
        // Model may rotate so we need to address the change also here
        interpNormal = normalize(mat3(normalMatrix) * normal);     
}
