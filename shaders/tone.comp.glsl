#version 450 core

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (rgba32f, binding=1) writeonly uniform image2D imageOut;

vec3 srgb2linear(vec3 c)
{
    // return pow(c, vec3(2.2));
    bvec3 cutoff = lessThan(c, vec3(0.04045));
    vec3 higher = pow((c + vec3(0.055))/vec3(1.055), vec3(2.4));
    vec3 lower = c/vec3(12.92);
    return mix(higher, lower, cutoff);
}

vec3 linear2srgb(vec3 c)
{
    // return pow(c, vec3(0.454545454545454545));
    bvec3 cutoff = lessThan(c, vec3(0.0031308));
    vec3 higher = vec3(1.055)*pow(c, vec3(1.0/2.4)) - vec3(0.055);
    vec3 lower = c * vec3(12.92);
    return mix(higher, lower, cutoff);
}

// https://64.github.io/tonemapping/

vec3 reinhard(vec3 c)
{
    return c / (1.0 + c);
}

vec3 reinhard_ext(vec3 c, float white)
{
    vec3 n = c * (1.0 + (c / vec3(white * white)));
    return n / (1.0 + c);
}

float L(vec3 c)
{
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

vec3 luminance(vec3 c, float l_out)
{
    float l_in = L(c);
    return c * (l_in / l_out);
}

vec3 reinhard_ext_luminance(vec3 c, float white)
{
    float l_in = L(c);
    float n = l_in * (1.0 + (l_in / (white * white)));
    float l_out = n / (1.0 + l_in);
    return luminance(c, l_out);
}

vec3 reinhard_jodie(vec3 c)
{
    float l_in = L(c);
    vec3 r = reinhard(c);
    return mix(c / (1.0 + l_in), r, r);
}

// modify tone
void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0,0), imageSize(imageIn));
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    imgColor = srgb2linear(imgColor);

    // imgColor = clamp(imgColor, 0.0, 1.0);
    // imgColor = reinhard(imgColor);
    // imgColor = reinhard_ext(imgColor, 0.4);
    // imgColor = vec3(L(imgColor));
    // imgColor = luminance(imgColor, 0.3);
    // imgColor = reinhard_ext_luminance(imgColor, 0.5);
    imgColor = reinhard_jodie(imgColor);

    imgColor = linear2srgb(imgColor);
    imageStore(imageOut, baseUV, vec4(imgColor, 1.0));
}
