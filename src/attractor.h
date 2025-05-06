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

#ifndef SRC_ATTRACTOR_H_
#define SRC_ATTRACTOR_H_

#include <stdint.h>

typedef enum {
    ATTRACTOR_TYPE_CLIFFORD,
} AttractorType;

// Forward declaration of Attractor struct
typedef struct Attractor Attractor;

typedef struct {
    void (*initialize)(Attractor *attractor);
    void (*destroy)(Attractor *attractor);
    void (*update)(Attractor *attractor);
    void (*iterate)(Attractor *attractor, uint32_t num_iterations);
    void (*iterate_until_timeout)(Attractor *attractor, float timeout);
    void (*reset)(Attractor *attractor);
    float (*get_occupancy)(Attractor *attractor);
    void (*randomize)(Attractor *attractor);
    void (*randomize_until_chaotic)(Attractor *attractor);
} AttractorFunctions;

typedef struct {
    AttractorType type;
    char         *name;
    char         *description;
    uint32_t      num_parameters;
    float        *default_parameters;

    AttractorFunctions functions;
} AttractorSettings;

typedef struct Attractor Attractor;

struct Attractor {
    AttractorType type;

    float    *parameters;
    uint32_t  num_parameters;
    uint32_t  width;
    uint32_t  height;
    uint32_t *density_map;

    AttractorFunctions functions;
};

Attractor *make_attractor(AttractorType type, uint32_t width, uint32_t height);
void       destroy_attractor(Attractor *attractor);

void  initialize_attractor(Attractor *attractor);
void  destroy_attractor(Attractor *attractor);
void  update_attractor(Attractor *attractor);
void  iterate_attractor(Attractor *attractor, uint32_t num_iterations);
void  iterate_until_timeout(Attractor *attractor, float timeout);
void  randomize_attractor(Attractor *attractor);
void  randomize_until_chaotic(Attractor *attractor);
void  reset_attractor(Attractor *attractor);
float get_occupancy(Attractor *attractor);

#endif // SRC_ATTRACTOR_H_
