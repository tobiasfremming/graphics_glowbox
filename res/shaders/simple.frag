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

vec3 reject(vec3 from, vec3 onto) {
    return from - onto*dot(from, onto)/dot(onto, onto);
}

#define NUM_LIGHTS 3
uniform Light lights[NUM_LIGHTS];
uniform vec3 ball_position;
uniform float ball_radius;

uniform layout(location = 5) vec3 viewPos;
uniform layout(location = 6) vec3 objectColor;

void main()
{   

    float l_a = 0.001; // Constant attenuation
    float l_b = 0.02; // Linear attenuation
    float l_c = 0.0001; // Quadratic attenuation
    

    vec3 emissionColor = vec3(0.1, 0.1, 0.1);
    float emissionStrength = 0.0;
    
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 result = vec3(0.0);
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(0.1,0.1, 0.1);

    for(int i = 0; i < NUM_LIGHTS; i++) {
        vec3 deltaPos = lights[i].position - fragPos;
        vec3 lightDir = deltaPos;
        vec3 normalized_light_direction = normalize(lightDir);
        float d = length(deltaPos);

        // Simple ray tracer
        // deltapos is the vector from the fragment to the light source
        // vector from the fragment to the center of the ball
        vec3 frag_to_ball = ball_position - fragPos;

        vec3 rejection = reject(frag_to_ball, lightDir);
        bool less_than_ball = length(rejection) < ball_radius;
        bool light_closer_to_wall = length(frag_to_ball) < d;
        bool over_90 = dot(frag_to_ball, lightDir) > 0;
        bool in_shadow = less_than_ball && light_closer_to_wall && over_90;

        if (in_shadow) {
            continue;
        }


        //attenuation
        float attenuation  = 1/(l_a + l_b*d + l_c*d*d);

        // diffuse, specular, and emission
        
        
        

        vec3 diffuse = vec3(0.0);
        vec3 specular = vec3(0.0);

        
        // Diffuse
        float diff = max(dot(norm, normalized_light_direction), 0.0);
        diffuse = attenuation * diff * lights[i].color;

        // Specular
        vec3 reflectDir = reflect(-normalized_light_direction, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        specular = attenuation * spec * lights[i].color;
        
        

        result +=diffuse + specular;
    }

    // emission
    vec3 emission = emissionColor * emissionStrength;
    result += emission + ambient;
    // result = normalize(result);

    //FragColor = vec4(result * objectColor, 1.0);
    //FragColor = vec4(0.5 * normal + 0.5, 1.0);
    result = result + vec3(dither(textureCoordinates));
    FragColor = vec4(result*0.5, 1.0);
    // FragColor = vec4(lights[1].color, 1.0);
 

}