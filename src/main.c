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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <cglm/call.h>
#include <cglm/cglm.h>

#include <stb_image.h>

#include <pcg_variants.h>

#include <entropy.h>

#include "clifford.h"
#include "gui.h"
#include "input_handling.h"
#include "manager.h"
#include "rendering.h"
#include "settings.h"
#include "shader_c.h"
#include "utils.h"

GLFWwindow *window;

int main(int argc, char *argv[]) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Raytracing for fun and fun", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    printf("Created window of size %dx%d\n", WINDOW_WIDTH, WINDOW_HEIGHT);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_click_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    /*glEnable(GL_DEBUG_OUTPUT);*/
    /*glDebugMessageCallback(MessageCallback, 0);*/

    stbi_set_flip_vertically_on_load(1);

    /*glCullFace(GL_BACK);*/
    /*glEnable(GL_CULL_FACE);*/
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    {
        uint64_t seeds[2];
        entropy_getbytes((void *)seeds, sizeof(seeds));
        pcg32_srandom(seeds[0], seeds[1]);
    }

    {
        gui_init();
        manager = init_manager();

        manager->clifford = make_clifford((1.0f - manager->border_size_percent) * WINDOW_WIDTH,
                                          (1.0f - manager->border_size_percent) * WINDOW_HEIGHT, -1.4, 1.6, 1.0, 0.7);
    }

    // Shaders
    Shader *shader = newShader("shaders/main.vert", "shaders/post_processing.frag", NULL);
    Shader_use(shader);
    Shader_set_int(shader, "texture1", 0);

    // Quad for fullscreen rendering
    unsigned int VAO;
    unsigned int VBO;
    float        quadVertices[] = {
        // positions        // texture Coords
        -1.0f, +1.0f, 0.0f, 0.0f, 1.0f, //
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, //
        +1.0f, +1.0f, 0.0f, 1.0f, 1.0f, //
        +1.0f, -1.0f, 0.0f, 1.0f, 0.0f, //
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenTextures(1, &manager->render_texture);
    glBindTexture(GL_TEXTURE_2D, manager->render_texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    printf("starting render loop\n");

    while (!glfwWindowShouldClose(window)) {
        // Process input
        glfwPollEvents();
        process_input(window);

        // Timer
        Manager_tick_timer(manager);

        if (manager->frame_count % 1000 == 0) {
            printf("fps: %f\n", 1.0f / manager->delta_time);
        }

        iterate_clifford_until_timeout(manager->clifford, 1 / 60.0f);

        // Main pass
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blit_clifford_to_texture(manager);

        Shader_use(shader);

        Shader_set_int(shader, "texture1", 0);
        // Set post-processing parameters
        Shader_set_float(shader, "gamma", manager->gamma);
        Shader_set_float(shader, "brightness", manager->brightness);
        Shader_set_float(shader, "contrast", manager->contrast);
        // Keep these for compatibility with potential future shaders
        Shader_set_int(shader, "tone_mapping_mode", manager->tone_mapping_mode);
        Shader_set_float(shader, "exposure", manager->exposure);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, manager->render_texture);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        // Render gui
        gui_render();

        // Draw to screen
        glfwSwapBuffers(window);
    }

    gui_terminate();
    glfwTerminate();

    return 0;
}
