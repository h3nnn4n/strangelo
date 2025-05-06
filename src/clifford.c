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

#include <GLFW/glfw3.h>

void iterate_clifford_impl(Attractor *attractor, uint32_t num_iterations, float x, float y) {
    // xn + 1 = sin(a yn) + c cos(a xn)
    // yn + 1 = sin(b xn) + d cos(b yn)

    float a = attractor->parameters[0];
    float b = attractor->parameters[1];
    float c = attractor->parameters[2];
    float d = attractor->parameters[3];

    // HACK: The attractor engine shouldn't have to care about rendering
    const float margin = 0.05; // 5% margin
    const float min_x  = (-1 - fabs(c)) * (1 + margin);
    const float max_x  = (1 + fabs(c)) * (1 + margin);
    const float min_y  = (-1 - fabs(d)) * (1 + margin);
    const float max_y  = (1 + fabs(d)) * (1 + margin);

    for (uint32_t i = 0; i < num_iterations; i++) {
        float x_new = sin(a * y) + c * cos(a * x);
        float y_new = sin(b * x) + d * cos(b * y);

        x = x_new;
        y = y_new;

        // Normalize to fit the texture from 0,0 to width,height
        uint32_t scaled_x = (uint32_t)((x + max_x) / (max_x - min_x) * attractor->width);
        uint32_t scaled_y = (uint32_t)((y + max_y) / (max_y - min_y) * attractor->height);

        attractor->density_map[scaled_x + scaled_y * attractor->width] += 1;
    }
}

// Wrapper function to match the expected function signature in AttractorFunctions
void iterate_clifford(Attractor *attractor, uint32_t num_iterations) {
    iterate_clifford_impl(attractor, num_iterations, random() * 2 - 1, random() * 2 - 1);
}

void randomize_clifford(Attractor *attractor) {
    attractor->parameters[0] = random() * 4 - 2;
    attractor->parameters[1] = random() * 4 - 2;
    attractor->parameters[2] = random() * 4 - 2;
    attractor->parameters[3] = random() * 4 - 2;
}