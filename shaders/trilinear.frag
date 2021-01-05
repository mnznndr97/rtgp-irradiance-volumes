#version 430 core

vec2 SemisphereToPoint(vec3 direction);
vec3 GetIrradiance(vec3 pos, vec3 normal, float reflectance);

uniform int debugColor;

// output shader variable
out vec4 colorFrag;
out vec3 test;

in vec3 interpNormal;
in vec3 interpPosition;

// No interpolation


void main()
{
    // To check if irradiance is queried correctly
    if (debugColor != 0) {
        colorFrag = vec4(GetIrradiance(interpPosition, interpNormal, 3.14f), 1.0f);
    } else {
	    vec3 baseColor = vec3(0.5f);
	    vec3 irradiance = GetIrradiance(interpPosition, interpNormal, 0.8);
	    colorFrag = vec4(baseColor + irradiance, 1.0f);
    }
}
