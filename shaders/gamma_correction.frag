#version 460 core

in vec2 TexCoord; 
out vec4 FragColor;

uniform sampler2D texture1;
uniform float gamma;

// Threshold for determining what's considered "black" background
const float BLACK_THRESHOLD = 0.005;

void main()
{
    vec4 color = texture(texture1, TexCoord);
    
    // Check if this pixel is part of the background (black)
    float luminance = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    
    if (luminance <= BLACK_THRESHOLD) {
        // This is background - leave it untouched
        FragColor = color;
        return;
    }
    
    // Apply gamma correction to non-black pixels
    vec3 corrected = pow(color.rgb, vec3(1.0 / gamma));
    
    FragColor = vec4(corrected, color.a);
}