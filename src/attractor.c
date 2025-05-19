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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attractor.h"
#include "clifford.h"

#include <GLFW/glfw3.h>

// Default parameter values for Clifford attractor
float clifford_default_params[4] = {-1.4f, 1.6f, 1.0f, 0.7f};

const AttractorSettings attractors[] = {
    {.type           = ATTRACTOR_TYPE_CLIFFORD,
     .name           = "Clifford",
     .description    = "A chaotic attractor defined by the equations x(n+1) = sin(a y(n)) + c cos(a x(n)), y(n+1) = "
                       "sin(b x(n)) + d cos(b y(n))",
     .num_parameters = 4,
     .default_parameters = clifford_default_params,
     .functions          = {
                  .iterate   = iterate_clifford,
                  .randomize = randomize_clifford,
     }}};

const AttractorFunctions attractor_functions = {
    .initialize = initialize_attractor,
    .destroy    = destroy_attractor,
    .update     = update_attractor,
};

Attractor *make_attractor(AttractorType type, uint32_t width, uint32_t height) {
    Attractor *attractor      = malloc(sizeof(Attractor));
    attractor->type           = type;
    attractor->width          = width;
    attractor->height         = height;
    attractor->num_parameters = attractors[type].num_parameters;

    attractor->functions = attractors[type].functions;

    attractor->density_map = malloc(width * height * sizeof(uint32_t));
    attractor->parameters  = malloc(attractor->num_parameters * sizeof(float));

    reset_attractor(attractor);

    return attractor;
}

void destroy_attractor(Attractor *attractor) {
    if (attractor->functions.destroy) {
        attractor->functions.destroy(attractor);
    }

    free(attractor->density_map);
    free(attractor->parameters);
    free(attractor);
}

void update_attractor(Attractor *attractor) {
    if (attractor->functions.update) {
        attractor->functions.update(attractor);
    }
}

void iterate_attractor(Attractor *attractor, uint32_t num_iterations) {
    if (attractor->functions.iterate) {
        attractor->functions.iterate(attractor, num_iterations);
    }
}

void iterate_until_timeout(Attractor *attractor, float timeout) {
    float start_time = glfwGetTime();

    while (glfwGetTime() - start_time < timeout) {
        iterate_attractor(attractor, 10000);
    }
}

void reset_attractor(Attractor *attractor) {
    clean_attractor(attractor);

    memcpy(attractor->parameters, attractors[attractor->type].default_parameters,
           attractors[attractor->type].num_parameters * sizeof(float));
}

void clean_attractor(Attractor *attractor) {
    memset(attractor->density_map, 0, attractor->width * attractor->height * sizeof(uint32_t));
}

float get_occupancy(Attractor *attractor) {
    float occupancy = 0;

    for (int i = 0; i < attractor->width * attractor->height; i++) {
        if (attractor->density_map[i] > 0) {
            occupancy++;
        }
    }

    return occupancy / (attractor->width * attractor->height);
}

void randomize_attractor(Attractor *attractor) {
    reset_attractor(attractor);
    attractor->functions.randomize(attractor);
}

void randomize_until_chaotic(Attractor *attractor) {
    if (attractor->functions.randomize_until_chaotic) {
        attractor->functions.randomize_until_chaotic(attractor);
        return;
    }

    do {
        randomize_attractor(attractor);
        iterate_attractor(attractor, 25000);
    } while (get_occupancy(attractor) < 0.01);
}

void initialize_attractor(Attractor *attractor) {
    if (!attractor->functions.initialize) {
        return;
    }

    attractor->functions.initialize(attractor);
}
