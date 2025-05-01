#version 460 core

in vec2 TexCoord; 
out vec4 FragColor;

uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
    // FragColor = vec4(0.5, 0.0, 0.5, 1.0);
}