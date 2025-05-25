/*
 * Copyright (C) 2023  Renan S. Silva, aka h3nnn4n
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

#include "shader_c.h"
#include "shader.hpp"

extern "C" {
Shader *newShader(const char *vertexPath, const char *fragmentPath, const char *geometryPath) {
    return new Shader(vertexPath, fragmentPath, geometryPath);
}

void Shader_reload_changes(Shader *shader) { shader->reload_changes(); }

unsigned int Shader_get_id(Shader *shader) { return shader->ID; }

void Shader_use(Shader *shader) { shader->use(); }

void Shader_set_bool(Shader *shader, const char *name, int value) { shader->setBool(name, value); }

void Shader_set_int(Shader *shader, const char *name, int value) { shader->setInt(name, value); }

void Shader_set_float(Shader *shader, const char *name, float value) { shader->setFloat(name, value); }

void Shader_set_vec3(Shader *shader, const char *name, float *value) { shader->setVec3(name, value); }

void Shader_set_vec3f(Shader *shader, const char *name, float v1, float v2, float v3) {
    shader->setVec3(name, v1, v2, v3);
}

void Shader_set_matrix4(Shader *shader, const char *name, float *value) { shader->setMatrix4(name, value); }

void Shader_destroy(Shader *shader) { delete shader; }
}
