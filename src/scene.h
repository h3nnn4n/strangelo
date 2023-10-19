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

#ifndef SRC_SCENE_H_
#define SRC_SCENE_H_

#include <stdlib.h>

#include <cglm/cglm.h>

const float r = 750;
const float f = 11.5;

const vec4 positions[] = {
    {0.0, -0.5, -9.0, 1.0},  // 1
    {0.0, 1.5, -9.0, 1.0},   // 2
    {2.0, 0.5, -11.0, 1.0},  // 3
    {0.0, 0.0, -11.0, 1.0},  // 4
    {0.0, 0.0, 10.0, 1.0},   // 5
    {0.0, 10.0, 0.0, 1.0},   // 6
    {0.0, -10.0, 0.0, 1.0},  // 7
    {10.0, 0.0, 0.0, 1.0},   // 8
    {-15.0, 0.0, 0.0, 1.0},  // 9
    {-25.0, 1.0, 3.0, 1.0},  // 10
    {-27.0, 0.0, -5.0, 1.0}, // 11
    {0.0, -r - f, 0.0, 1.0}, // 12
};

const vec4 albedo[] = {
    {1.0, 1.0, 1.0, 0.0},    // 1
    {0.0, 0.0, 1.0, 0.0},    // 2
    {1.0, 1.0, 1.0, 0.0},    // 3
    {0.3, 0.8, 0.2, 0.0},    // 4
    {0.4, 0.2, 0.8, 0.0},    // 5
    {0.8, 0.7, 0.2, 0.0},    // 6
    {0.2, 0.7, 0.6, 0.0},    // 7
    {0.1, 0.4, 0.3, 0.0},    // 8
    {1.0, 1.0, 1.0, 0.0},    // 9
    {0.6, 0.3, 0.1, 0.0},    // 10
    {0.76, 0.61, 0.35, 0.0}, // 11  Brass color
    {1.0, 1.0, 1.0, 0.0},    // 12
};

const float radius[] = {
    1.25, // 1
    1.25, // 2
    2.5,  // 3
    2.5,  // 4
    2.5,  // 5
    2.5,  // 6
    2.5,  // 7
    2.5,  // 8
    2.5,  // 9
    3.5,  // 10
    3.5,  // 11
    r,    // 12
};

// 1 = diffuse
// 2 = metal
// 3 = dielectric
// 4 = reflective
// 5 = light
const int material_type[] = {
    5, // 1
    3, // 2
    1, // 3
    1, // 4
    1, // 5
    1, // 6
    1, // 7
    1, // 8
    3, // 9
    1, // 10
    2, // 11
    2, // 12
};

const float roughness[] = {
    0.0, // 1
    0.0, // 2
    0.0, // 3
    0.0, // 4
    0.0, // 5
    0.0, // 6
    0.0, // 7
    0.0, // 8
    0.0, // 9
    0.0, // 10
    0.5, // 11
    0.2, // 12
};

const int n_spheres = sizeof(radius) / sizeof(float);

#endif
