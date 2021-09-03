#version 450 core

layout (local_size_x=1, local_size_y=1, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (rgba32f, binding=1) writeonly uniform image2D imageOut;

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    const vec3 multiplier = vec3(0.3, 0.59, 0.11);
    float gray = clamp(dot(imgColor, multiplier), 0.0, 1.0);
    imageStore(imageOut, baseUV, vec4(vec3(gray), 1.0));
}
