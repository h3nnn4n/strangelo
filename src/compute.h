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

#ifndef _COMPUTE_H
#define _COMPUTE_H

#include <cglm/call.h>
#include <cglm/cglm.h>
#include <glad/glad.h>

typedef struct {
    char              shader_path[256];
    unsigned long int id;
} compute_t;

compute_t *build_compute_shader(char *shader_path);
void       compute_use(compute_t *compute);
void       compute_set_int(compute_t *compute, char *name, int value);
void       compute_set_float(compute_t *compute, char *name, float value);
void       compute_set_vec3f(compute_t *compute, char *name, float v1, float v2, float v3);
void       compute_set_vec3(compute_t *compute, char *name, vec3 *v);
void       check_compile_errors(GLuint shader, const char *type);

#endif
