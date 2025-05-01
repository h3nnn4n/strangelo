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
#include "utils.h"

#include <math.h>
#include <stdio.h>

Clifford *make_clifford(uint32_t width, uint32_t height, float a, float b, float c, float d) {
    Clifford *clifford = malloc(sizeof(Clifford));

    clifford->a = a;
    clifford->b = b;
    clifford->c = c;
    clifford->d = d;

    clifford->width  = width;
    clifford->height = height;
    clifford->buffer = malloc(width * height * sizeof(uint32_t));

    for (int i = 0; i < width * height; i++) {
        clifford->buffer[i] = 0;
    }

    return clifford;
}

void destroy_clifford(Clifford *clifford) { free(clifford->buffer); }

void update_clifford(Clifford *clifford, float a, float b, float c, float d) {
    clifford->a = a;
    clifford->b = b;
    clifford->c = c;
    clifford->d = d;

    reset_clifford(clifford);
}

void iterate_clifford(Clifford *c, uint32_t num_iterations, float x, float y) {
    // xn + 1 = sin(a yn) + c cos(a xn)
    // yn + 1 = sin(b xn) + d cos(b yn)
    const float min_x = -1 - fabs(c->c);
    const float max_x = 1 + fabs(c->c);
    const float min_y = -1 - fabs(c->d);
    const float max_y = 1 + fabs(c->d);

    for (uint32_t i = 0; i < num_iterations; i++) {
        float x_new = sin(c->a * y) + c->c * cos(c->a * x);
        float y_new = sin(c->b * x) + c->d * cos(c->b * y);

        x = x_new;
        y = y_new;

        // Normalize to fit the texture from 0,0 to width,height
        uint32_t scaled_x = (uint32_t)((x + max_x) / (max_x - min_x) * c->width);
        uint32_t scaled_y = (uint32_t)((y + max_y) / (max_y - min_y) * c->height);

        c->buffer[scaled_x + scaled_y * c->width] += 1;
    }
}

void randomize_clifford(Clifford *clifford) {
    float a = random() * 4 - 2;
    float b = random() * 4 - 2;
    float c = random() * 4 - 2;
    float d = random() * 4 - 2;

    update_clifford(clifford, a, b, c, d);
}

void randomize_until_chaotic(Clifford *clifford) {
    do {
        randomize_clifford(clifford);
        iterate_clifford(clifford, 25000, random(), random());
    } while (get_occupancy(clifford) < 0.01);
}

void reset_clifford(Clifford *c) {
    for (int i = 0; i < c->width * c->height; i++) {
        c->buffer[i] = 0;
    }
}

float get_occupancy(Clifford *c) {
    float occupancy = 0;

    for (int i = 0; i < c->width * c->height; i++) {
        if (c->buffer[i] > 0) {
            occupancy++;
        }
    }

    return occupancy / (c->width * c->height);
}
