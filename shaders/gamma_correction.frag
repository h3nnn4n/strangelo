#version 460 core

in vec2 TexCoord; 
out vec4 FragColor;

uniform sampler2D texture1;
uniform float gamma;

void main()
{
    vec4 color = texture(texture1, TexCoord);
    
    vec3 corrected = pow(color.rgb, vec3(1.0 / gamma));
    
    FragColor = vec4(corrected, color.a);
}