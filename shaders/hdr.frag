// Example code from https://learnopengl.com/Advanced-Lighting/HDR

#version 410 core

in vec2 iterpolatedUVCoords;
out vec4 FragColor;

uniform sampler2D hdrBuffer;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;

    vec3 hdrColor = texture(hdrBuffer, iterpolatedUVCoords).rgb;
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, 1.0);
}