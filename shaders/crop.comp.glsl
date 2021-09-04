#version 450 core

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (rgba32f, binding=1) writeonly uniform image2D imageOut;

// if falls into a given area
// render black
void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0,0), imageSize(imageIn));
    const ivec4 area = ivec4(
        0, 500, // upper left corner
        500, 200 // width height
    );
    if(baseUV.x - area.x > 0 && baseUV.x - area.x < area.z &&
        baseUV.y - area.y > 0 && baseUV.y - area.y < area.w)
    {
        imageStore(imageOut, baseUV, vec4(vec3(0.0), 1.0));
    }
    else
    {
        vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
        imageStore(imageOut, baseUV, vec4(imgColor, 1.0));
    }
}
