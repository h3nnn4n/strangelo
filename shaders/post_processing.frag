#version 460 core

in vec2 TexCoord; 
out vec4 FragColor;

uniform sampler2D texture1;

// Gamma correction parameters
uniform float gamma;

// Brightness and contrast parameters
uniform float brightness;
uniform float contrast;

// Threshold for determining what's considered "black" background
const float BLACK_THRESHOLD = 0.005;

void main()
{
    // Sample the texture
    vec4 color = texture(texture1, TexCoord);
    
    // Check if this pixel is part of the background (black)
    float luminance = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    
    if (luminance <= BLACK_THRESHOLD) {
        // This is background - leave it untouched
        FragColor = color;
        return;
    }
    
    // Process non-background pixels
    
    // Step 1: Apply gamma correction to RGB channels
    vec3 corrected = pow(color.rgb, vec3(1.0 / gamma));
    
    // Step 2: Apply brightness adjustment
    corrected = corrected + brightness;
    
    // Step 3: Apply contrast adjustment (using midpoint 0.5)
    corrected = (corrected - 0.5) * contrast + 0.5;
    
    // Clamp values to valid range
    corrected = clamp(corrected, 0.0, 1.0);
    
    // Output the final processed color with original alpha
    FragColor = vec4(corrected, color.a);
}