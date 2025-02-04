#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragPos;

out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 color;
};

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

#define NUM_LIGHTS 3
uniform Light lights[NUM_LIGHTS];
uniform layout(location = 5) vec3 viewPos;
uniform layout(location = 6) vec3 objectColor;

void main()
{
    
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 result = vec3(0.0);

    for(int i = 0; i < NUM_LIGHTS; i++) {
        vec3 lightDir = normalize(lights[i].position - fragPos);

        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lights[i].color;
        //vec3 diffuse = diff *vec3(0.5,0.3, 0.7);


        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = spec * lights[i].color;
        // vec3 specular = spec * vec3(0.5,0.3, 0.7);

        result += diffuse + specular;
    }
    // result = normalize(result);

    //FragColor = vec4(result * objectColor, 1.0);
    //FragColor = vec4(0.5 * normal + 0.5, 1.0);
    
    FragColor = vec4(result*0.5, 1.0);
    // FragColor = vec4(lights[1].color, 1.0);
 

}