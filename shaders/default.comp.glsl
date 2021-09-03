#version 450 core

layout (local_size_x=1, local_size_y=1, local_size_z=1) in;

layout (rgba32f, binding=0) uniform image2D imageIn;
layout (rgba32f, binding=1) uniform image2D imageOut;

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    imageStore(imageOut, baseUV, vec4(imgColor, 1.0));
}
