#version 450 core

layout (location = 0) in vec2 imgUV;
layout (location = 0) out vec4 color;

uniform sampler2D image;

void main()
{
    vec3 imgColor = texture(image, imgUV).rgb;
    
    vec2 c = 1.0 / textureSize(image, 0);
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
            vec3 pixel = texture(image, imgUV + c * vec2(i - 1, j - 1)).rgb;
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

    color = vec4(vec3(mag), 1.0);
    // color = vec4(imgColor, 1.0);
}