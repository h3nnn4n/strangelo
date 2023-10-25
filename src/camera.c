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

#include <assert.h>
#include <stdlib.h>

#include <glad/glad.h>

#include <cglm/call.h>
#include <cglm/cglm.h>

#include <stb.h>

#include "camera.h"
#include "manager.h"
#include "settings.h"
#include "utils.h"

Camera *make_camera() {
    Camera *camera = malloc(sizeof(Camera));

    vec3 camera_pos    = {0, 0, -3};
    vec3 camera_front  = {0, 0, 1};
    vec3 camera_target = {0, 0, 0};
    vec3 camera_up     = {0, 1, 0};
    vec3 camera_right  = {0, 0, 0};
    mat4 projection    = GLM_MAT4_IDENTITY_INIT;

    glm_vec3_copy(camera_pos, camera->camera_pos);
    glm_vec3_copy(camera_front, camera->camera_front);
    glm_vec3_copy(camera_target, camera->camera_target);
    glm_vec3_copy(camera_up, camera->camera_up);
    glm_vec3_copy(camera_right, camera->camera_right);

    glm_mat4_copy(projection, camera->projection);

    camera->pitch = 0;
    camera->yaw   = 90;
    camera->zoom  = 45;

    update_camera_target(camera, 0, 0);
    update_camera_projection_matrix(camera);
    update_camera_position_matrix(camera);

    return camera;
}

void destroy_camera(Camera *camera) {
    assert(camera);

    free(camera);
}

void update_camera_target(Camera *camera, float xoffset, float yoffset) {
    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera->yaw += xoffset;
    camera->pitch += yoffset;

    if (camera->pitch > 89.0f)
        camera->pitch = 89.0f;

    if (camera->pitch < -89.0f)
        camera->pitch = -89.0f;

    camera->camera_front[0] = cos(deg2rad(camera->yaw)) * cos(deg2rad(camera->pitch));
    camera->camera_front[1] = sin(deg2rad(camera->pitch));
    camera->camera_front[2] = sin(deg2rad(camera->yaw)) * cos(deg2rad(camera->pitch));

    glm_vec3_normalize(camera->camera_front);

    update_camera_position_matrix(camera);
}

void update_camera_fov(Camera *camera, float xoffset, float yoffset) {
    camera->zoom -= yoffset;

    if (camera->zoom < 1.0f)
        camera->zoom = 1.0f;

    if (camera->zoom > 90.0f)
        camera->zoom = 90.0f;

    update_camera_projection_matrix(camera);
}

void update_camera_position(Camera *camera, Direction direction) {
    const float camera_speed = 6.0f * manager->delta_time;

    vec3 tmp;

    switch (direction) {
        case UP:
            glm_vec3_scale(camera->camera_up, camera_speed, tmp);
            glm_vec3_add(camera->camera_pos, tmp, camera->camera_pos);
            break;

        case DOWN:
            glm_vec3_scale(camera->camera_up, camera_speed, tmp);
            glm_vec3_sub(camera->camera_pos, tmp, camera->camera_pos);
            break;

        case FRONT:
            glm_vec3_scale(camera->camera_front, camera_speed, tmp);
            glm_vec3_add(camera->camera_pos, tmp, camera->camera_pos);
            break;

        case BACK:
            glm_vec3_scale(camera->camera_front, camera_speed, tmp);
            glm_vec3_sub(camera->camera_pos, tmp, camera->camera_pos);
            break;

        case LEFT:
            glm_vec3_crossn(camera->camera_front, camera->camera_up, tmp);
            glm_vec3_scale(tmp, camera_speed, tmp);
            glm_vec3_sub(camera->camera_pos, tmp, camera->camera_pos);
            break;

        case RIGHT:
            glm_vec3_crossn(camera->camera_front, camera->camera_up, tmp);
            glm_vec3_scale(tmp, camera_speed, tmp);
            glm_vec3_add(camera->camera_pos, tmp, camera->camera_pos);
            break;
    }

    update_camera_target(camera, 0, 0);
    update_camera_position_matrix(manager->camera);
}

void update_camera_projection_matrix(Camera *camera) {
    glm_perspective(deg2rad(camera->zoom), aspect_ratio, near_plane, far_plane, camera->projection);
}

void update_camera_position_matrix(Camera *camera) {
    vec3 camera_direction;

    glm_vec3_sub(camera->camera_pos, camera->camera_target, camera_direction);
    glm_vec3_normalize(camera_direction);

    glm_vec3_cross(GLM_YUP, camera_direction, camera->camera_right);
    glm_vec3_normalize(camera->camera_right);

    mat4 identity = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_copy(identity, camera->view);

    glm_vec3_add(camera->camera_pos, camera->camera_front, camera->camera_target);
    glm_lookat(camera->camera_pos, camera->camera_target, camera->camera_up, camera->view);

    // FIXME: Having this here isn't the clearest thing ever
    glClearTexImage(manager->render_texture, 0, GL_RGBA, GL_FLOAT, NULL);
}
