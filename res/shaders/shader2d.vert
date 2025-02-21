#version 430 core

in layout (location = 0) vec3 aPos;      // Position
in layout (location = 2) vec2 aTexCoord; // Texture coordinates
uniform mat4 OrthoProjection;
uniform mat4 model;

out layout (location = 0) vec2 TexCoord;




void main()
{
    TexCoord = aTexCoord;
    gl_Position = OrthoProjection * model * vec4(aPos, 1.0); // Convert to screen space
}
