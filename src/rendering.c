/*
 * Copyright (C) 2023  Renan S. Silva, aka h3nnn4n
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the “Software”), to deal in the Software without restriction, including without limitation the
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>

#include "manager.h"
#include "rendering.h"

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
        uint8_t intensity = (texture_data[i] >> 24) & 0xFF; // Assuming intensity is stored in the red channel
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
        uint8_t intensity     = (texture_data[i] >> 24) & 0xFF;
        uint8_t new_intensity = cdf[intensity];
        texture_data[i] =
            (new_intensity << 24) | (new_intensity << 16) | (new_intensity << 8) | (texture_data[i] & 0xFF);
    }
}

void apply_tone_mapping(uint32_t *texture_data, uint32_t width, uint32_t height) {
    for (int i = 0; i < width * height; i++) {
        texture_data[i] = apply_gamma_correction(texture_data[i], 2.2f);
    }
}

uint32_t apply_gamma_correction(uint32_t color, float gamma) {
    float inv_gamma = 1.0f / gamma;

    // Extract the color components
    uint8_t r = (color >> 24) & 0xFF;
    uint8_t g = (color >> 16) & 0xFF;
    uint8_t b = (color >> 8) & 0xFF;
    uint8_t a = color & 0xFF;

    // Apply gamma correction
    r = (uint8_t)(pow(r / 255.0f, inv_gamma) * 255.0f);
    g = (uint8_t)(pow(g / 255.0f, inv_gamma) * 255.0f);
    b = (uint8_t)(pow(b / 255.0f, inv_gamma) * 255.0f);

    // Reassemble the color
    return (r << 24) | (g << 16) | (b << 8) | a;
}