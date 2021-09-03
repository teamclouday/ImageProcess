#version 450 core

layout (local_size_x=1, local_size_y=1, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (rgba32f, binding=1) writeonly uniform image2D imageOut;

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    
    const mat3 kernelX = mat3(
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0
    );
    const mat3 kernelY = mat3(
        -1.0, -2.0, -1.0,
         0.0,  0.0,  0.0,
         1.0,  2.0,  1.0
    );
    // get image
    mat3 img;
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            vec3 pixel = imageLoad(imageIn, baseUV + ivec2(i - 1, j - 1)).rgb;
            img[i][j] = length(pixel);
        }
    }
    // compute gradients
    float gX = 0.0;
    float gY = 0.0;
    for(int i = 0; i < 3; i++)
    {
        gX += dot(kernelX[i], img[i]);
        gY += dot(kernelY[i], img[i]);
    }
    // compute gradient magnitude
    float mag = sqrt(gX * gX + gY * gY);
    mag = smoothstep(0.5, 1.0, mag);
    imgColor = mix(imgColor, vec3(1.0), mag);

    imageStore(imageOut, baseUV, vec4(vec3(mag), 1.0));
}
