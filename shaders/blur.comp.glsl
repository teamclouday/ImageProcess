#version 450 core

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (rgba32f, binding=1) writeonly uniform image2D imageOut;

uniform float quality = 6.0;
uniform float directions = 12.0;
uniform float radius = 5.0;

// https://xorshaders.weebly.com/tutorials/blur-shaders-5-part-2
void main()
{
    ivec2 texSize = imageSize(imageIn);
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0), texSize);
    const float PI_2 = 6.283185307179586;
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    // go around 2pi for each direction
    // for each direction, compute quality number pixels
    // on the radius
    // then do the average
    for(float d = 0.0; d < PI_2; d+=PI_2/directions)
    {
        for(float i = 1.0/quality; i <= 1.0; i+=1.0/quality)
        {
            ivec2 uv = ivec2(vec2(cos(d),sin(d))*radius*i);
            imgColor += imageLoad(imageIn, baseUV + uv).rgb;
        }
    }
    imgColor /= quality * directions + 1.0;
    imageStore(imageOut, baseUV, vec4(imgColor, 1.0));
}
