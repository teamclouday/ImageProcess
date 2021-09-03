#version 450 core

layout (location = 0) in vec2 imgUV;
layout (location = 0) out vec4 color;

uniform sampler2D image;

void main()
{
    vec3 imgColor = texture(image, imgUV).rgb;
    const vec3 multiplier = vec3(0.3, 0.59, 0.11);
    float gray = clamp(dot(imgColor, multiplier), 0.0, 1.0);
    color = vec4(vec3(gray), 1.0);
}