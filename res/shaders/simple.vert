#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 3) mat4 MVP;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec3 fragPos;

void main()
{
    //fragPos = vec3(model * vec4(position, 1.0)); // world space position
    fragPos = position; // world space position
    normal_out = normal_in;
   
    textureCoordinates_out = textureCoordinates_in;

    gl_Position = MVP * vec4(position, 1.0f);
}
