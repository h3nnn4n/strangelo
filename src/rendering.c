/*
 * Copyright (C) 2023  Renan S. Silva, aka h3nnn4n
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>

#include "attractor.h"
#include "manager.h"
#include "rendering.h"
#include "settings.h"

void set_shader_storage_buffer(uint32_t binding_id, uint32_t size, void *data) {
    GLuint ssbo;

    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_id, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void clear_texture(uint32_t texture_id) { glClearTexImage(texture_id, 0, GL_RGBA, GL_FLOAT, NULL); }

float sigmoid_normalize(float x, float midpoint, float steepness) {
    // Shift and scale to center sigmoid around midpoint
    float shifted = (x - midpoint) * steepness;
    // Apply sigmoid function: 1/(1+e^-x)
    float result = 1.0f / (1.0f + expf(-shifted));
    return result;
}

void normalize_attractor_to_texture_gl(Attractor *attractor, float *texture_data_gl, uint32_t width, uint32_t height,
                                       float border_size_percent, ScalingMethod scaling_method, float power_exponent,
                                       float sigmoid_midpoint, float sigmoid_steepness) {
    // Clear the texture data first
    for (uint32_t i = 0; i < width * height * 4; i++) {
        texture_data_gl[i] = 0.0f;
    }

    uint32_t border_size_x = width * border_size_percent;
    uint32_t border_size_y = height * border_size_percent;

    // First pass: find max value for normalization
    uint32_t max_value = 0;
    for (int i = 0; i < attractor->width * attractor->height; i++) {
        if (attractor->density_map[i] > max_value) {
            max_value = attractor->density_map[i];
        }
    }

    if (max_value == 0)
        max_value = 1;

    // Second pass: copy and normalize in one step
    for (int i = 0; i < attractor->width; i++) {
        for (int j = 0; j < attractor->height; j++) {
            int      index         = i + j * (attractor->width);
            int      texture_index = ((border_size_x + i) + (border_size_y + j) * width);
            uint32_t density       = attractor->density_map[index];

            if (density > 0) {
                float normalized = (float)density / max_value;

                switch (scaling_method) {
                    case LINEAR_SCALING: break;

                    case LOG_SCALING:
                        normalized = logf(1.0f + normalized * 9.0f) / logf(10.0f); // Maps [0,1] to [0,1]
                        break;

                    case POWER_SCALING: normalized = powf(normalized, power_exponent); break;

                    case SIGMOID_SCALING:
                        normalized = sigmoid_normalize(normalized, sigmoid_midpoint, sigmoid_steepness);
                        break;

                    case SQRT_SCALING: normalized = sqrtf(normalized); break;

                    default: break;
                }

                texture_data_gl[texture_index * 4 + 0] = normalized;
                texture_data_gl[texture_index * 4 + 1] = normalized;
                texture_data_gl[texture_index * 4 + 2] = normalized;
                texture_data_gl[texture_index * 4 + 3] = 1.0f;
            }
        }
    }
}

void render_texture_to_gl(float *texture_data_gl, uint32_t width, uint32_t height) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, texture_data_gl);
}

void apply_coloring(float *texture_data_gl, uint32_t width, uint32_t height, ColoringInfo coloring_info) {
    float min_value = INFINITY;
    float max_value = -INFINITY;
    float threshold = 0.01f; // Values below this fraction of max are considered "low"

    // First pass: find max value to determine scaling threshold
    for (int i = 0; i < width * height; i++) {
        float value = texture_data_gl[i * 4 + 0];
        if (value > 0.0f && value > max_value) {
            max_value = value;
        }
    }

    // Second pass: find min value, excluding very low values
    float cutoff = max_value * threshold;
    for (int i = 0; i < width * height; i++) {
        float value = texture_data_gl[i * 4 + 0];
        if (value > cutoff && value < min_value) {
            min_value = value;
        }
    }

    if (min_value == INFINITY) {
        min_value = 0.0f;
    }

    float range = max_value - min_value;
    if (range <= 0.0f) {
        range = 1.0f;
    }

    for (int i = 0; i < width * height; i++) {
        float value = texture_data_gl[i * 4 + 0];

        if (value <= 0.0f) {
            texture_data_gl[i * 4 + 0] = 0.0f;
            texture_data_gl[i * 4 + 1] = 0.0f;
            texture_data_gl[i * 4 + 2] = 0.0f;
            texture_data_gl[i * 4 + 3] = 0.0f;
            continue;
        }

        float normalized = (value - min_value) / range;
        normalized       = fmaxf(0.0f, fminf(1.0f, normalized));

        float r = coloring_info.starting.r + (coloring_info.ending.r - coloring_info.starting.r) * normalized;
        float g = coloring_info.starting.g + (coloring_info.ending.g - coloring_info.starting.g) * normalized;
        float b = coloring_info.starting.b + (coloring_info.ending.b - coloring_info.starting.b) * normalized;

        // Create a smooth transparency curve
        // Values at min are nearly transparent, values at max are fully opaque
        float alpha = powf(normalized, 2.5f);

        texture_data_gl[i * 4 + 0] = r;
        texture_data_gl[i * 4 + 1] = g;
        texture_data_gl[i * 4 + 2] = b;
        texture_data_gl[i * 4 + 3] = alpha;
        texture_data_gl[i * 4 + 3] = 0.0f;
    }
}