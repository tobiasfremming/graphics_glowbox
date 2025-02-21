#version 330 core

out vec4 FragColor;

void main()
{
    // vec4 sampledColor = texture(textTexture, TexCoord);
    
    // if (sampledColor.a < 0.1)  // Discard transparent pixels
    //     discard;

    FragColor = vec4(0.5, 0.1, 0.7, 1.0);
}
