R"(
#version 450 core

layout (location = 0) in vec2 inPos;
layout (location = 0) out vec2 imgUV;

uniform float winRatio; // w / h

void main()
{
    imgUV = (inPos + 1.0) * 0.5;
    gl_Position = vec4(inPos, 0.0, 1.0);
}
)"
