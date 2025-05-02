#version 460 core

in vec2 TexCoord; 
out vec4 FragColor;

uniform sampler2D texture1;
uniform float brightness; // Brightness adjustment [-1.0, 1.0]
uniform float contrast;   // Contrast adjustment [0.0, 2.0]

void main()
{
    // Sample the texture
    vec4 color = texture(texture1, TexCoord);
    
    // Apply brightness adjustment
    vec3 adjusted = color.rgb + brightness;
    
    // Apply contrast adjustment (using midpoint 0.5)
    adjusted = (adjusted - 0.5) * contrast + 0.5;
    
    // Clamp values to valid range
    adjusted = clamp(adjusted, 0.0, 1.0);
    
    // Output the adjusted color with original alpha
    FragColor = vec4(adjusted, color.a);
}