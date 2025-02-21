#version 430 core

layout(binding = 0) uniform sampler2D textureSampler;


layout (location = 0) in vec2 TexCoord; // Texture coordinates





out vec4 FragColor;

void main()
{
    // vec4 sampledColor = texture(textTexture, TexCoord);
    
    // if (sampledColor.a < 0.1)  // Discard transparent pixels
    //     discard;
    vec4 textureColor = texture(textureSampler, TexCoord);
    FragColor = textureColor;
}
