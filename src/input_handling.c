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

#include "input_handling.h"
#include "manager.h"
#include "settings.h"
#include "utils.h"

int firstMouse;
int left_mouse_pressed;
int right_mouse_pressed;

int ctrl_key_pressed;
int alt_key_pressed;

int f1_key_pressed;
int f2_key_pressed;
int f3_key_pressed;
int f4_key_pressed;

vec3 mouse_world_position;

float lastX;
float lastY;

void mouse_click_callback(GLFWwindow *window, int button, int action, int mods) {
    if (manager->freeze_movement)
        return;

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
    if (manager->freeze_movement)
        return;

    if (firstMouse) {
        lastX      = xpos;
        lastY      = ypos;
        firstMouse = 0;
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (manager->freeze_movement)
        return;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // FIXME: Should update aspect_ratio and window size
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }

    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
        if (!f1_key_pressed)
            toggle(&manager->hide_ui);

        f1_key_pressed = 1;
    } else if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE) {
        f1_key_pressed = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
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

    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
        if (!f2_key_pressed) {
            toggle(&manager->freeze_movement);

            if (manager->freeze_movement)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            else
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        f2_key_pressed = 1;
    } else if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_RELEASE) {
        f2_key_pressed = 0;
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
}