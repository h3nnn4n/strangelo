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

#ifndef SRC_RENDERING_H_
#define SRC_RENDERING_H_

#include <stdbool.h>
#include <stdint.h>

#include "attractor.h"

typedef enum {
    LINEAR_SCALING,
    LOG_SCALING,
    POWER_SCALING,
    SIGMOID_SCALING,
    SQRT_SCALING,
} ScalingMethod;

typedef enum {
    NONE_TONE_MAPPING,
    ACES_TONE_MAPPING,
    FILMIC_TONE_MAPPING,
    LOTTES_TONE_MAPPING,
    REINHARD_TONE_MAPPING,
    REINHARD2_TONE_MAPPING,
    UCHIMURA_TONE_MAPPING,
    UNCHARTED2_TONE_MAPPING,
    UNREAL_TONE_MAPPING,
} ToneMappingMode;

void set_shader_storage_buffer(uint32_t binding_id, uint32_t size, void *data);

void  clean_texture_data(uint32_t *texture_data, float *texture_data_gl, uint32_t width, uint32_t height);
void  copy_attractor_to_texture_data(struct Attractor *attractor, uint32_t *texture_data, uint32_t width,
                                     uint32_t height, float border_size_percent);
float sigmoid_normalize(float x, float midpoint, float steepness);
void  normalize_texture_data(const uint32_t *texture_data, float *texture_data_gl, uint32_t width, uint32_t height,
                             ScalingMethod scaling_method, float power_exponent, float sigmoid_midpoint,
                             float sigmoid_steepness);
void  render_texture_to_gl(float *texture_data_gl, uint32_t width, uint32_t height);

void apply_coloring(float *texture_data_gl, uint32_t width, uint32_t height, ColoringInfo coloring_info);

#endif // SRC_RENDERING_H_
