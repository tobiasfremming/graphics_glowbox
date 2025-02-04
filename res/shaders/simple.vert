#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 3) mat4 VP;
uniform layout(location = 4) mat4 model;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec3 fragPos;

void main()
{
    // fragPos = position; 
    // normal_out = normal_in; 
    fragPos = vec3(model * vec4(position, 1.0));
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    normal_out = normalMatrix * normal_in;
   
    textureCoordinates_out = textureCoordinates_in;

    gl_Position = VP  * vec4(position, 1.0f);
}
