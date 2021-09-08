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

vec3 uncharted2_tonemap_partial(vec3 x)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 uncharted2_filmic(vec3 c)
{
    const float exposure_bias = 2.0;
    vec3 curr = uncharted2_tonemap_partial(c * exposure_bias);
    const vec3 W = vec3(11.2);
    vec3 white_scale = vec3(1.0) / uncharted2_tonemap_partial(W);
    return curr * white_scale;
}

vec3 rtt_and_odt_fit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 aces_fitted(vec3 c)
{
    const mat3 aces_input_matrix = mat3(
        vec3(0.59719, 0.07600, 0.02840),
        vec3(0.35458, 0.90834, 0.13383),
        vec3(0.04823, 0.01566, 0.83777)
    );
    const mat3 aces_output_matrix = mat3(
        vec3( 1.60475, -0.10208, -0.00327),
        vec3(-0.53108,  1.10813, -0.07276),
        vec3(-0.07367, -0.00605,  1.07602)
    );
    c = aces_input_matrix * c;
    c = rtt_and_odt_fit(c);
    return aces_output_matrix * c;
}

vec3 aces_approx(vec3 v)
{
    v *= 0.6;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((v*(a*v+b))/(v*(c*v+d)+e), 0.0, 1.0);
}

uniform int mode = 0;
uniform float c_reinhard_ext = 0.4;
uniform float c_luminance = 0.3;
uniform float c_reinhard_ext_luminance = 1.5;

// modify tone
void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0), imageSize(imageIn));
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    imgColor = srgb2linear(imgColor);

    if(mode == 1)
    {
        imgColor = reinhard(imgColor);
    }
    else if(mode == 2)
    {
        imgColor = reinhard_ext(imgColor, c_reinhard_ext);
    }
    else if(mode == 3)
    {
        imgColor = luminance(imgColor, c_luminance);
    }
    else if(mode == 4)
    {
        imgColor = reinhard_ext_luminance(imgColor, c_reinhard_ext_luminance);
    }
    else if(mode == 5)
    {
        imgColor = reinhard_jodie(imgColor);
    }
    else if(mode == 6)
    {
        imgColor = uncharted2_filmic(imgColor);
    }
    else if(mode == 7)
    {
        imgColor = aces_fitted(imgColor);
    }
    else if(mode == 8)
    {
        imgColor = aces_approx(imgColor);
    }

    imgColor = linear2srgb(imgColor);
    imageStore(imageOut, baseUV, vec4(imgColor, 1.0));
}
