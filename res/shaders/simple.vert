#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent;
in layout(location = 4) vec3 biTangent;

uniform mat4 VP;
uniform mat4 model;
uniform mat3 normalMatrix;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec3 fragPos;
out layout(location = 2) vec2 textureCoordinates_out;
out layout(location = 3) mat3 TBN;

void main()
{   
    vec3 tangentM = normalize(normalMatrix * tangent);
    vec3 biTangentM = normalize(normalMatrix * biTangent);
    vec3 normalM = normalize(normalMatrix * normal_in);

    TBN = mat3(normalize(tangentM), normalize(biTangentM), normalize(normalM) );
    // fragPos = position; 
    // normal_out = normal_in; 
    fragPos = vec3(model * vec4(position, 1.0));
    
    // mat3 normalMatrix = transpose(inverse(mat3(normalMatrix)));
    // normal_out = normalMatrix * normal_in;
    normal_out = normalM;


   
    textureCoordinates_out = textureCoordinates_in;
    

    gl_Position = VP * model * vec4(position, 1.0f);
}
