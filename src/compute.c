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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stb.h>

#include <glad/glad.h>

#include "compute.h"

compute_t *build_compute_shader(char *shader_path) {
    compute_t *compute = malloc(sizeof(compute_t));

    memcpy(compute->shader_path, shader_path, strlen(shader_path));

    const char *shader_code = stb_file(shader_path, NULL);

    // compute shader
    compute->compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute->compute, 1, (char const *const *)&shader_code, NULL);
    glCompileShader(compute->compute);
    check_compile_errors(compute->compute, "COMPUTE");

    // shader Program
    compute->id = glCreateProgram();
    glAttachShader(compute->id, compute->compute);
    glLinkProgram(compute->id);
    check_compile_errors(compute->id, "PROGRAM");

    return compute;
}

void compute_use(compute_t *compute) { glUseProgram(compute->id); }

void check_compile_errors(GLuint shader, const char *type) {
    GLint  success;
    GLchar infoLog[2048];

    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(shader, 2048, NULL, infoLog);
            printf("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n", type, infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(shader, 2048, NULL, infoLog);
            printf("ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n", type, infoLog);
        }
    }
}
