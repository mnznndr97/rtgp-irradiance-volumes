#version 430 core
const float PI = 3.14159265359f;

// FWD declaration
vec3 radianceSimple(vec3 direction, int samples, int sampleOffset);

uniform int samplesCount;
uniform int sampleOffset;
uniform int debugColor;

// output shader variable
out vec4 colorFrag;
in vec3 interpNormal;


void main()
{
    
    // To check if irradiance is queried correctly
    if (debugColor != 0) {
        colorFrag = vec4(radianceSimple(interpNormal, samplesCount, sampleOffset), 1.0f);
        
    } else
    {
        vec3 baseColor = vec3(0.3f);
        vec3 irradianceColor = radianceSimple(interpNormal, samplesCount, sampleOffset);
        float reflectance = 0.5f;
	    irradianceColor = (reflectance * irradianceColor / PI);
        colorFrag = vec4(baseColor + irradianceColor, 1.0f);
    }
}
