/*
 * Copyright (C) 2025  Renan S. Silva, aka h3nnn4n
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

#include "clifford.h"

#include <math.h>

Clifford make_clifford(uint32_t width, uint32_t height, float a, float b, float c, float d) {
    Clifford clifford;
    clifford.a = a;
    clifford.b = b;
    clifford.c = c;
    clifford.d = d;

    clifford.width  = width;
    clifford.height = height;
    clifford.buffer = malloc(width * height * sizeof(uint32_t));

    return clifford;
}

void destroy_clifford(Clifford *clifford) { free(clifford->buffer); }

void iterate_clifford(Clifford *c, uint32_t num_iterations, float x, float y) {
    for (uint32_t i = 0; i < num_iterations; i++) {
        float x_new = sin(c->a * y) + c->c * cos(c->a * x);
        float y_new = sin(c->b * x) + c->d * cos(c->b * y);

        x = x_new;
        y = y_new;

        // Clamp x and y to the range [-2, 2]
        float clamped_x = fmaxf(-2.0f, fminf(2.0f, x));
        float clamped_y = fmaxf(-2.0f, fminf(2.0f, y));

        // Normalize to fit the texture from 0,0 to width,height
        uint32_t scaled_x = (uint32_t)((clamped_x + 2.0f) / 4.0f * c->width);
        uint32_t scaled_y = (uint32_t)((clamped_y + 2.0f) / 4.0f * c->height);

        c->buffer[scaled_x + scaled_y * c->width] = 255;
    }
}