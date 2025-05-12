#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture1;

// Post-processing parameters
uniform float gamma;
uniform float brightness;
uniform float contrast;
uniform float exposure;
uniform int tone_mapping_mode;

// Tone mapping modes (must match enum in C code)
const int NONE_TONE_MAPPING = 0;
const int ACES_TONE_MAPPING = 1;
const int FILMIC_TONE_MAPPING = 2;
const int LOTTES_TONE_MAPPING = 3;
const int REINHARD_TONE_MAPPING = 4;
const int REINHARD2_TONE_MAPPING = 5;
const int UCHIMURA_TONE_MAPPING = 6;
const int UNCHARTED2_TONE_MAPPING = 7;
const int UNREAL_TONE_MAPPING = 8;

// Threshold for determining what's considered "black" background
const float BLACK_THRESHOLD = 0.005;

// ACES tone mapping curve fit (Narkowicz 2015)
vec3 ACES(vec3 color) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

// Filmic tone mapping (Jim Hejl and Richard Burgess-Dawson)
vec3 Filmic(vec3 color) {
    vec3 x = max(vec3(0.0), color - 0.004);
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

// Lottes tone mapping (Timothy Lottes, 2016)
vec3 Lottes(vec3 x) {
    const vec3 a = vec3(1.6);
    const vec3 d = vec3(0.977);
    const vec3 hdrMax = vec3(8.0);
    const vec3 midIn = vec3(0.18);
    const vec3 midOut = vec3(0.267);

    const vec3 b =
        (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
    const vec3 c =
        (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
        ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

    return pow(x, a) / (pow(x, a * d) * b + c);
}

// Reinhard tone mapping (Erik Reinhard, 2002)
vec3 Reinhard(vec3 color) {
    return color / (color + vec3(1.0));
}

// Reinhard2 tone mapping (variant with white point)
vec3 Reinhard2(vec3 color) {
    const float white = 4.0;
    const float wSquared = white * white;

    return (color * (1.0 + color/wSquared)) / (1.0 + color);
}

// Uchimura tone mapping (Yoshiharu Uchimura, 2017)
vec3 Uchimura(vec3 x) {
    const float P = 1.0;  // max display brightness
    const float a = 1.0;  // contrast
    const float m = 0.22; // linear section start
    const float l = 0.4;  // linear section length
    const float c = 1.33; // black
    const float b = 0.0;  // pedestal

    // Uchimura function calculation
    float l0 = ((P - m) * l) / a;
    float L0 = m - m / a;
    float L1 = m + (1.0 - m) / a;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = (a * P) / (P - S1);
    float CP = -C2 / P;

    vec3 w0 = vec3(1.0) - smoothstep(vec3(0.0), vec3(m), x);
    vec3 w2 = step(vec3(m + l0), x);
    vec3 w1 = vec3(1.0) - w0 - w2;

    vec3 T = vec3(m * pow(x / m, vec3(c)) + b);
    vec3 S = vec3(P - (P - S1) * exp(CP * (x - S0)));
    vec3 L = vec3(m + a * (x - m));

    return T * w0 + L * w1 + S * w2;
}

// Uncharted 2 tone mapping (John Hable, 2010)
vec3 Uncharted2(vec3 color) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;

    color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;

    return color / white;
}

// Unreal 3 tone mapping (Brian Karis, 2013)
vec3 Unreal(vec3 x) {
    return x / (x + 0.155) * 1.019;
}

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

    // Apply exposure
    vec3 hdrColor = color.rgb * exposure;

    // Apply tone mapping based on selected mode
    vec3 tonemapped;

    switch (tone_mapping_mode) {
        case NONE_TONE_MAPPING:
            tonemapped = hdrColor;
            break;
        case ACES_TONE_MAPPING:
            tonemapped = ACES(hdrColor);
            break;
        case FILMIC_TONE_MAPPING:
            tonemapped = Filmic(hdrColor);
            break;
        case LOTTES_TONE_MAPPING:
            tonemapped = Lottes(hdrColor);
            break;
        case REINHARD_TONE_MAPPING:
            tonemapped = Reinhard(hdrColor);
            break;
        case REINHARD2_TONE_MAPPING:
            tonemapped = Reinhard2(hdrColor);
            break;
        case UCHIMURA_TONE_MAPPING:
            tonemapped = Uchimura(hdrColor);
            break;
        case UNCHARTED2_TONE_MAPPING:
            tonemapped = Uncharted2(hdrColor);
            break;
        case UNREAL_TONE_MAPPING:
            tonemapped = Unreal(hdrColor);
            break;
        default:
            tonemapped = hdrColor;
            break;
    }

    // Apply gamma correction
    vec3 corrected = pow(tonemapped, vec3(1.0 / gamma));

    // Apply brightness adjustment
    corrected = corrected + brightness;

    // Apply contrast adjustment (using midpoint 0.5)
    corrected = (corrected - 0.5) * contrast + 0.5;

    // Clamp values to valid range
    corrected = clamp(corrected, 0.0, 1.0);

    // Output the final processed color with original alpha
    FragColor = vec4(corrected, color.a);

    // FragColor = texture(texture1, TexCoord);
    // FragColor = color;
}
