#version 450 core

layout (local_size_x=1, local_size_y=1, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (rgba32f, binding=1) writeonly uniform image2D imageOut;

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    float amount = 0.5;
    float neighbor = amount * -1.0;
    float center = amount * 4.0 + 1.0;
    ivec2 offset = ivec2(1, 1);
    vec3 imgColor = imageLoad(imageIn, baseUV + ivec2(0, offset.y)).rgb * neighbor;
    imgColor += imageLoad(imageIn, baseUV + ivec2(-offset.x, 0)).rgb * neighbor;
    imgColor += imageLoad(imageIn, baseUV + ivec2(0, 0)).rgb * center;
    imgColor += imageLoad(imageIn, baseUV + ivec2(offset.x, 0)).rgb * neighbor;
    imgColor += imageLoad(imageIn, baseUV + ivec2(0, -offset.y)).rgb * neighbor;
    imageStore(imageOut, baseUV, vec4(imgColor, 1.0));
}
