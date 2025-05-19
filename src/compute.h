/*
 * Copyright (C) 2025  Renan S. Silva, aka h3nnn4n
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

#ifndef SRC_COMPUTE_H_
#define SRC_COMPUTE_H_

#include "attractor.h"

#include <pthread.h>

typedef enum {
    COMPUTE_STATE_PAUSED,
    COMPUTE_STATE_RUNNING,
    COMPUTE_STATE_DIE,
} ComputeState;

typedef struct {
    ComputeState state;
    Attractor   *attractor;

    pthread_t thread;
} Compute;

Compute *compute_init(Attractor *attractor);
void     compute_destroy(Compute *compute);
void     compute_pause(Compute *compute);
void     compute_resume(Compute *compute);
void     compute_tick(Compute *compute);
void     compute_loop(Compute *compute);

void compute_clean_attractor(Compute *compute);
void compute_reset_attractor(Compute *compute);

#endif // SRC_COMPUTE_H_
