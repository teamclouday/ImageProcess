#version 450 core

layout (location = 0) in vec2 imgUV;
layout (location = 0) out vec4 color;

uniform sampler2D image;

void main()
{
    float amount = 0.5;
    float neighbor = amount * -1.0;
    float center = amount * 4.0 + 1.0;
    vec2 offset = 1.0 / textureSize(image, 0);
    vec3 imgColor = texture(image, imgUV + vec2(0.0, offset.y)).rgb * neighbor;
    imgColor += texture(image, imgUV + vec2(-offset.x, 0.0)).rgb * neighbor;
    imgColor += texture(image, imgUV + vec2(0.0, 0.0)).rgb * center;
    imgColor += texture(image, imgUV + vec2(offset.x, 0.0)).rgb * neighbor;
    imgColor += texture(image, imgUV + vec2(0.0, -offset.y)).rgb * neighbor;
    color = vec4(imgColor, 1.0);
}