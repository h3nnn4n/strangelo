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

#ifndef _CAMERA_H
#define _CAMERA_H

#include <cglm/cglm.h>

#include "shader_c.h"
#include "utils.h"

typedef struct {
    vec3 camera_pos;
    vec3 camera_front;
    vec3 camera_target;
    vec3 camera_up;
    vec3 camera_right;

    mat4 projection;
    mat4 view;

    float pitch;
    float yaw;
    float zoom;
} Camera;

Camera *make_camera();
void    destroy_camera(Camera *camera);
void    update_camera_target(Camera *camera, float xoffset, float yoffset);
void    update_camera_fov(Camera *camera, float xoffset, float yoffset);
void    update_camera_target(Camera *camera, float xoffset, float yoffset);
void    update_camera_position(Camera *camera, Direction direction);

void update_camera_projection_matrix(Camera *camera, Shader *shader);
void update_camera_view_matrix(Camera *camera, Shader *shader);
void update_camera_position_matrix(Camera *camera);

#endif

