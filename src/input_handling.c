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

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include "camera.h"
#include "input_handling.h"
#include "manager.h"
#include "settings.h"
#include "utils.h"

int firstMouse;
int left_mouse_pressed;
int right_mouse_pressed;

int ctrl_key_pressed;
int alt_key_pressed;

int f3_key_pressed;
int f4_key_pressed;
int f5_key_pressed;
int f9_key_pressed;

vec3 mouse_world_position;

float lastX;
float lastY;

// Code from https://antongerdelan.net/opengl/raycasting.html
void update_mouse_world_position() {
    float x = (2.0f * lastX) / WINDOW_WIDTH - 1.0f;
    float y = 1.0f - (2.0f * lastY) / WINDOW_HEIGHT;
    float z = 1.0f;

    vec3 ray_nds  = {x, y, z};
    vec4 ray_clip = {ray_nds[0], ray_nds[1], -1.0, 1.0};

    /*printf("\n");*/
    /*printf("rayclip: %5.2f %5.2f %5.2f %5.2f \n",*/
    /*ray_clip[0], ray_clip[1], ray_clip[2], ray_clip[3]);*/

    mat4 inverse_projection;
    glm_mat4_inv(manager->camera->projection, inverse_projection);

    vec4 ray_eye = GLM_VEC4_ZERO_INIT;
    glm_mat4_mulv(inverse_projection, ray_clip, ray_eye);
    ray_eye[2] = -1;
    ray_eye[3] = 0;
    /*printf(" rayeye: %5.2f %5.2f %5.2f %5.2f \n",*/
    /*ray_eye[0], ray_eye[1], ray_eye[2], ray_eye[3]);*/

    mat4 inverse_view = GLM_MAT4_ZERO_INIT;
    glm_mat4_inv(manager->camera->view, inverse_view);

    vec4 ray_world = GLM_VEC4_ZERO_INIT;
    glm_mat4_mulv(inverse_view, ray_eye, ray_world);
    glm_vec4_normalize(ray_world);

    for (int i = 0; i < 3; i++) {
        mouse_world_position[i] = ray_world[i];
    }
}

void mouse_click_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            right_mouse_pressed = 1;
        } else if (action == GLFW_RELEASE) {
            right_mouse_pressed = 0;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            left_mouse_pressed = 1;
        } else if (action == GLFW_RELEASE) {
            left_mouse_pressed = 0;
        }
    }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX      = xpos;
        lastY      = ypos;
        firstMouse = 0;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top

    lastX = xpos;
    lastY = ypos;

    update_camera_target(manager->camera, xoffset, yoffset);
    update_camera_projection_matrix(manager->camera);
    update_mouse_world_position();

    if (xpos != 0 && ypos != 0)
        glClearTexImage(manager->render_texture, 0, GL_RGBA, GL_FLOAT, NULL);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    update_camera_fov(manager->camera, xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // FIXME: Should update aspect_ratio and window size
    glViewport(0, 0, width, height);

    update_camera_projection_matrix(manager->camera);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }

    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
        if (!f3_key_pressed)
            toggle(&manager->camera->orthographic);

        f3_key_pressed = 1;
    } else if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_RELEASE) {
        f3_key_pressed = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
        if (!f4_key_pressed)
            toggle(&manager->incremental_rendering);

        f4_key_pressed = 1;
    } else if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_RELEASE) {
        f4_key_pressed = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        ctrl_key_pressed = 1;
    } else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) {
        ctrl_key_pressed = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
        alt_key_pressed = 1;
    } else if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE) {
        alt_key_pressed = 0;
    }

    handle_camera_movements(window);
}

void handle_camera_movements(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        update_camera_position(manager->camera, FRONT);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        update_camera_position(manager->camera, BACK);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        update_camera_position(manager->camera, LEFT);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        update_camera_position(manager->camera, RIGHT);
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        update_camera_position(manager->camera, DOWN);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        update_camera_position(manager->camera, UP);
    }
}
