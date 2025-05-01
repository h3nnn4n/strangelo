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

#ifndef SRC_CLIFFORD_H_
#define SRC_CLIFFORD_H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

// Definition
// xn + 1 = sin(a yn) + c cos(a xn)
// yn + 1 = sin(b xn) + d cos(b yn)
// where a, b, c, d are variables that define each attractor.

typedef struct {
    float a;
    float b;
    float c;
    float d;

    uint32_t  width;
    uint32_t  height;
    uint32_t *buffer;
} Clifford;

Clifford *make_clifford(uint32_t width, uint32_t height, float a, float b, float c, float d);
void     destroy_clifford(Clifford *clifford);

void  update_clifford(Clifford *clifford, float a, float b, float c, float d);
void  iterate_clifford(Clifford *clifford, uint32_t num_iterations, float x, float y);
void  iterate_clifford_until_timeout(Clifford *clifford, float timeout);
void  randomize_clifford(Clifford *clifford);
void  randomize_until_chaotic(Clifford *clifford);
void  reset_clifford(Clifford *clifford);
float get_occupancy(Clifford *clifford);
#endif // SRC_CLIFFORD_H_