#version 450 core

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (rgba32f, binding=1) writeonly uniform image2D imageOut;

uniform int pixelSize = 4;

void main()
{
    ivec2 texSize = imageSize(imageIn);
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0), texSize);
    ivec2 offset = baseUV % pixelSize;
    offset = ivec2(floor(pixelSize / 2.0)) - offset;
    vec3 imgColor = imageLoad(imageIn, baseUV + offset).rgb;
    imageStore(imageOut, baseUV, vec4(imgColor, 1.0));
}
