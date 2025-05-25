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

void apply_histogram_normalization(uint32_t *texture_data, uint32_t width, uint32_t height) {
    uint32_t histogram[256] = {0};
    uint32_t total_pixels   = width * height;

    // Build histogram
    for (int i = 0; i < total_pixels; i++) {
        uint8_t intensity = texture_data[i];
        histogram[intensity]++;
    }

    // Compute cumulative distribution function (CDF)
    uint32_t cdf[256] = {0};
    cdf[0]            = histogram[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + histogram[i];
    }

    // Normalize the CDF
    uint32_t min_cdf = cdf[0];
    uint32_t range   = total_pixels - min_cdf;
    if (range == 0)
        range = 1;

    for (int i = 0; i < 256; i++) {
        cdf[i] = ((cdf[i] - min_cdf) * 255) / range;
    }

    // Apply histogram normalization
    for (int i = 0; i < total_pixels; i++) {
        uint8_t intensity     = texture_data[i];
        uint8_t new_intensity = cdf[intensity];
        texture_data[i]       = new_intensity;
    }
}

void copy_attractor_to_texture_data(Attractor *attractor, uint32_t *texture_data, uint32_t width, uint32_t height,
                                    float border_size_percent) {
    uint32_t border_size_x = width * border_size_percent;
    uint32_t border_size_y = height * border_size_percent;

    for (int i = 0; i < attractor->width; i++) {
        for (int j = 0; j < attractor->height; j++) {
            int      index         = i + j * (attractor->width);
            int      texture_index = ((border_size_x + i) + (border_size_y + j) * width) * 4;
            uint32_t density       = attractor->density_map[index];

            texture_data[texture_index + 0] = density;
            texture_data[texture_index + 1] = density;
            texture_data[texture_index + 2] = density;
            texture_data[texture_index + 3] = 255;
        }
    }
}

float sigmoid_normalize(float x, float midpoint, float steepness) {
    // Shift and scale to center sigmoid around midpoint
    float shifted = (x - midpoint) * steepness;
    // Apply sigmoid function: 1/(1+e^-x)
    float result = 1.0f / (1.0f + expf(-shifted));
    return result;
}

void normalize_texture_data(const uint32_t *texture_data, float *texture_data_gl, uint32_t width, uint32_t height,
                            ScalingMethod scaling_method, float power_exponent, float sigmoid_midpoint,
                            float sigmoid_steepness) {
    uint32_t max_value = 0;
    for (int i = 0; i < width * height; i++) {
        if (texture_data[i * 4 + 0] > max_value) {
            max_value = texture_data[i * 4 + 0];
        }
    }

    if (max_value == 0)
        max_value = 1;

    for (int i = 0; i < width * height; i++) {
        uint32_t pixel_value = texture_data[i * 4 + 0];

        float normalized = (float)pixel_value / max_value;

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

        texture_data_gl[i * 4 + 0] = normalized;
        texture_data_gl[i * 4 + 1] = normalized;
        texture_data_gl[i * 4 + 2] = normalized;
        texture_data_gl[i * 4 + 3] = 1.0f;
    }
}

void clean_texture_data(uint32_t *texture_data, float *texture_data_gl, uint32_t width, uint32_t height) {
    for (uint32_t i = 0; i < width * height * 4; i++) {
        texture_data[i]    = 0;
        texture_data_gl[i] = 0.0f;
    }
}

void render_texture_to_gl(float *texture_data_gl, uint32_t width, uint32_t height) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, texture_data_gl);
}
