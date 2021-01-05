/*
18_skybox.frag: fragment shader for the visualization of the cube map as environment map

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

#version 410 core

// output shader variable
out vec4 colorFrag;

void main()
{
	 // we sample the cube map
    colorFrag = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
