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

#include <math.h>
#include <stdio.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <cglm/call.h>
#include <cglm/cglm.h>

#include <stb_image.h>

#include "camera.h"
#include "compute.h"
#include "gui.h"
#include "input_handling.h"
#include "manager.h"
#include "settings.h"
#include "shader_c.h"

GLFWwindow *window;

const float r = 750;
const float f = 11.5;

vec4 positions[] = {
    {0.0, -0.5, -9.0, 1.0},  //
    {0.0, 1.5, -9.0, 1.0},   //
    {2.0, 0.5, -11.0, 1.0},  //
    {0.0, 0.0, -11.0, 1.0},  //
    {0.0, 0.0, 10.0, 1.0},   //
    {0.0, 10.0, 0.0, 1.0},   //
    {0.0, -10.0, 0.0, 1.0},  //
    {10.0, 0.0, 0.0, 1.0},   //
    {-10.0, 0.0, 0.0, 1.0},  //
    {0.0, -r - f, 0.0, 1.0}, //
};

vec4 albedo[] = {
    {1.0, 1.0, 1.0, 0.0}, //
    {0.0, 0.0, 1.0, 0.0}, //
    {1.0, 1.0, 1.0, 0.0}, //
    {0.3, 0.8, 0.2, 0.0}, //
    {0.4, 0.2, 0.8, 0.0}, //
    {0.8, 0.7, 0.2, 0.0}, //
    {0.2, 0.7, 0.6, 0.0}, //
    {0.1, 0.4, 0.3, 0.0}, //
    {0.6, 0.3, 0.1, 0.0}, //
    {1.0, 1.0, 1.0, 0.0}, //
};

float radius[] = {
    1.25, //
    1.25, //
    2.5,  //
    2.5,  //
    2.5,  //
    2.5,  //
    2.5,  //
    2.5,  //
    2.5,  //
    r,    //
};

// 1 = diffuse
// 2 = metal
// 3 = dielectric
// 4 = reflective
// 5 = light
int material_type[] = {
    5, //
    3, //
    1, //
    1, //
    1, //
    1, //
    1, //
    1, //
    1, //
    2, //
};

float roughness[] = {
    0.0, //
    0.0, //
    0.0, //
    0.0, //
    0.0, //
    0.0, //
    0.0, //
    0.0, //
    0.0, //
    0.2, //
};

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    /*glEnable(GL_DEBUG_OUTPUT);*/
    /*glDebugMessageCallback(MessageCallback, 0);*/

    stbi_set_flip_vertically_on_load(1);

    /*glCullFace(GL_BACK);*/
    /*glEnable(GL_CULL_FACE);*/
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    int work_grp_cnt[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    printf("max global (total) work group counts x:%i y:%i z:%i\n", work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

    int work_grp_size[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

    printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n", work_grp_size[0], work_grp_size[1],
           work_grp_size[2]);

    int work_grp_inv;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
    printf("max local work group invocations %i\n", work_grp_inv);

    {
        gui_init();
        manager        = init_manager();
        Camera *camera = make_camera();
        Manager_set_camera(manager, camera);
    }

    // Shaders
    Shader *shader = newShader("shaders/main.vert", "shaders/main.frag", NULL);
    Shader_use(shader);
    Shader_set_int(shader, "tex", 0);

    compute_t *compute_shader = build_compute_shader("shaders/raytracer.comp");
    compute_use(compute_shader);
    compute_set_float(compute_shader, "near_plane", near_plane);
    compute_set_float(compute_shader, "far_plane", far_plane);

    // Quad
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
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    // SSBOs
    GLuint ssbo_positions;
    glGenBuffers(1, &ssbo_positions);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_positions);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, ssbo_positions);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint ssbo_radius;
    glGenBuffers(1, &ssbo_radius);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_radius);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(radius), radius, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssbo_radius);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint ssbo_material_type;
    glGenBuffers(1, &ssbo_material_type);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_material_type);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(material_type), material_type, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssbo_material_type);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint ssbo_albedo;
    glGenBuffers(1, &ssbo_albedo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_albedo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(albedo), albedo, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssbo_albedo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint ssbo_roughness;
    glGenBuffers(1, &ssbo_roughness);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_roughness);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(roughness), roughness, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, ssbo_roughness);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Compute texture
    const unsigned int TEXTURE_WIDTH  = WINDOW_WIDTH;
    const unsigned int TEXTURE_HEIGHT = WINDOW_HEIGHT;
    unsigned int       texture;

    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    while (!glfwWindowShouldClose(window)) {
        // Clear the working texture
        // FIXME: Needs to be cleaned whenever we move the camera
        /*glClearTexImage(texture, 0, GL_RGBA32F, GL_FLOAT, NULL);*/

        // Process input
        glfwPollEvents();
        processInput(window);

        // Timer
        Manager_tick_timer(manager);

        if (manager->frame_count % 1000 == 0) {
            printf("fps: %f\n", 1.0f / manager->delta_time);
        }

        // Run compute shader
        compute_use(compute_shader);
        compute_set_float(compute_shader, "time", manager->current_time);
        compute_set_matrix4(compute_shader, "camera_view", &manager->camera->view);
        compute_set_bool(compute_shader, "orthographic", manager->camera->orthographic);
        compute_set_bool(compute_shader, "incremental_rendering", manager->incremental_rendering);

        glDispatchCompute(TEXTURE_WIDTH / 32, TEXTURE_HEIGHT / 32, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Main pass
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Shader_use(shader);
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

    destroy_camera(manager->camera);

    return 0;
}
