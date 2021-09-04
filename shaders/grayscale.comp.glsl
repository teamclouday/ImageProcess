#version 450 core

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (rgba32f, binding=1) writeonly uniform image2D imageOut;

uniform int numShades = 3;

float to_gray1(vec3 c)
{
    const vec3 multiplier = vec3(0.3, 0.59, 0.11);
    return dot(c, multiplier);
}

float to_gray2(vec3 c)
{
    float f = (c.r + c.g + c.b) / 3.0;
    return f;
}

float to_gray3(vec3 c, int n)
{
    float f = (c.r + c.g + c.b) / 3.0;
    float convert = 1.0 / (n - 1.0);
    return floor(f / convert + 0.5) * convert;
}

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0), imageSize(imageIn));
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    
    // float gray = to_gray1(imgColor);
    // float gray = to_gray2(imgColor);
    float gray = to_gray3(imgColor, numShades);


    imageStore(imageOut, baseUV, vec4(vec3(gray), 1.0));
}
