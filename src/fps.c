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

#include <stdio.h>
#include <string.h>

#include "fps.h"

static float fps_buffer[FPS_BUFFER_SIZE];
static float fps_index[FPS_BUFFER_SIZE];
static int   fps_pivot   = 0;
static int   fps_samples = 0;

void init_fps() {
    memset(fps_buffer, 0, sizeof(float) * FPS_BUFFER_SIZE);

    for (int i = 0; i < FPS_BUFFER_SIZE; ++i) {
        fps_index[i] = i;
    }
}

void add_fps_sample(float fps) {
    fps_buffer[fps_pivot] = fps;
    fps_pivot             = (fps_pivot + 1) % FPS_BUFFER_SIZE;
    fps_samples           = fps_samples < FPS_BUFFER_SIZE ? fps_samples + 1 : FPS_BUFFER_SIZE;
}

float get_max_fps() {
    float max_fps = 0;

    for (int i = 0; i < fps_samples; ++i) {
        if (fps_buffer[i] > max_fps) {
            max_fps = fps_buffer[i];
        }
    }

    return max_fps;
}

float *get_fps_buffer() { return fps_buffer; }
float *get_fps_index_buffer() { return fps_index; }

float get_average_fps() {
    float sum = 0;

    for (int i = 0; i < fps_samples; ++i) {
        sum += fps_buffer[i];
    }

    return sum / fps_samples;
}
