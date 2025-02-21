#version 330 core

layout (location = 0) in vec3 aPos;      // Position
layout (location = 1) in vec2 aTexCoord; // Texture coordinates


void main()
{
    gl_Position = vec4(aPos, 1.0); // Convert to screen space
}
