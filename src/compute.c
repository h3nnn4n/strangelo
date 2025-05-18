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

#include <pthread.h>
#include <stdlib.h>

#include "compute.h"

Compute *compute_init(Attractor *attractor) {
    Compute *compute   = malloc(sizeof(Compute));
    compute->attractor = attractor;
    compute->state     = COMPUTE_STATE_PAUSED;

    void *(*thread_func)(void *) = (void *(*)(void *))compute_loop;
    pthread_create(&compute->thread, NULL, thread_func, (void *)compute);

    return compute;
}

void compute_destroy(Compute *compute) {
    compute->state = COMPUTE_STATE_DIE;
    pthread_join(compute->thread, NULL);
    free(compute);
}

void compute_tick(Compute *compute) {
    //
}

void compute_resume(Compute *compute) { compute->state = COMPUTE_STATE_RUNNING; }

void compute_pause(Compute *compute) { compute->state = COMPUTE_STATE_PAUSED; }

void compute_loop(Compute *compute) {
    while (compute->state != COMPUTE_STATE_DIE) {
        compute_tick(compute);
    }
}