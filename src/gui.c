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
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "gui.h"
#include "manager.h"
#include "settings.h"

struct ImGuiContext  *ctx;
struct ImGuiIO       *io;
struct ImPlotContext *plot_ctx;

char buffer[1024];

#define FPS_BUFFER_SIZE     512
#define FPS_AVG_BUFFER_SIZE 512

float fps_buffer[FPS_BUFFER_SIZE];
float fps_index[FPS_BUFFER_SIZE];
float fps_avg_buffer[FPS_AVG_BUFFER_SIZE];
float max_fps       = 0;
int   fps_pivot     = 0;
int   fps_avg_pivot = 0;

void gui_init() {
    ctx      = igCreateContext(NULL);
    io       = igGetIO();
    plot_ctx = ImPlot_CreateContext();

    const char *glsl_version = "#version 460 core";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    igStyleColorsDark(NULL);

    memset(fps_buffer, 0, sizeof(float));

    for (int i = 0; i < FPS_BUFFER_SIZE; ++i) {
        fps_index[i] = (float)i;
    }
}

void gui_terminate() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(ctx);
    ImPlot_DestroyContext(plot_ctx);
}

void gui_new_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
}

void gui_render() {
    gui_new_frame();

    gui_update_fps();
    gui_update_camera();

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

void update_rolling_fps_avg() {
    float ms  = manager->delta_time;
    float fps = 1.0f / ms;

    if (fps_avg_pivot >= FPS_AVG_BUFFER_SIZE) {
        fps_avg_pivot = 0;
    }

    fps_avg_buffer[fps_avg_pivot] = fps;
    fps_avg_pivot++;

    float avg_fps = 0;

    max_fps = 0;
    for (int i = 0; i < FPS_AVG_BUFFER_SIZE; ++i) {
        avg_fps += fps_avg_buffer[i] / FPS_AVG_BUFFER_SIZE;
        max_fps = fmax(max_fps, fps_avg_buffer[i]);
    }

    if (fps_pivot >= FPS_BUFFER_SIZE) {
        fps_pivot = 0;
    }

    fps_buffer[fps_pivot] = avg_fps;
    fps_pivot++;
}

void gui_update_fps() {
    if (!igBegin("Window", NULL, 0))
        return igEnd();

    float ms  = manager->delta_time;
    float fps = 1.0 / ms;

    update_rolling_fps_avg();

    snprintf(buffer, sizeof(buffer), "FPS: %6.2f", fps);
    igText(buffer);

    snprintf(buffer, sizeof(buffer), " ms: %8.6f", ms);
    igText(buffer);

    ImVec2 size            = {200, 100};
    ImVec4 plot_color_line = {1, 1, 0, 1};
    ImVec4 plot_color_fill = {1, 1, 0, 0.25};

    if (ImPlot_BeginPlot("fps", size, 0)) {
        ImPlot_SetupAxesLimits(0, FPS_BUFFER_SIZE, 0, max_fps * 1.2, ImGuiCond_Always);
        ImPlot_SetupAxes("time", "fps", 0, 0);
        ImPlot_PushStyleColor_Vec4(ImPlotCol_Line, plot_color_line);
        ImPlot_PushStyleColor_Vec4(ImPlotCol_Line, plot_color_fill);
        ImPlotAxisFlags axis_flags = ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_Lock |
                                     ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels;
        ImPlot_PlotLine_FloatPtrFloatPtr("f(x)", fps_index, fps_buffer, FPS_BUFFER_SIZE, axis_flags, 0, 4);
        ImPlot_PopStyleColor(2);

        ImPlot_EndPlot();
    }

    igEnd();
}

void gui_update_camera() {
    Camera *camera = manager->camera;

    assert(camera);

    igBegin("Camera", NULL, 0);

    if (camera->orthographic)
        snprintf(buffer, sizeof(buffer), "mode: ORTHOGRAPHIC");
    else
        snprintf(buffer, sizeof(buffer), "mode: PROJECTION");
    igText(buffer);

    if (manager->incremental_rendering)
        snprintf(buffer, sizeof(buffer), "incremental_rendering: true");
    else
        snprintf(buffer, sizeof(buffer), "incremental_rendering: false");
    igText(buffer);

    snprintf(buffer, sizeof(buffer), "position: %4.2f %4.2f %4.2f", camera->camera_pos[0], camera->camera_pos[1],
             camera->camera_pos[2]);
    igText(buffer);

    snprintf(buffer, sizeof(buffer), "target: %4.2f %4.2f %4.2f", camera->camera_target[0], camera->camera_target[1],
             camera->camera_target[2]);
    igText(buffer);

    snprintf(buffer, sizeof(buffer), "yaw, pitch: %4.2f %4.2f", camera->yaw, camera->pitch);
    igText(buffer);

    snprintf(buffer, sizeof(buffer), "fov: %4.2f", camera->zoom);
    igText(buffer);

#ifdef __SHOW_MVP
    {
        igSeparator();
        mat4 *m = &camera->view;
        igText("view matrix:");
        for (int i = 0; i < 4; i++) {
            snprintf(buffer, sizeof(buffer), "%4.2f %4.2f %4.2f %4.2f", *m[i][0], *m[i][1], *m[i][2], *m[i][3]);
            igText(buffer);
        }
    }

    {
        igSeparator();
        mat4 *m = &camera->projection;
        igText("projection matrix:");
        for (int i = 0; i < 4; i++) {
            snprintf(buffer, sizeof(buffer), "%4.2f %4.2f %4.2f %4.2f", *m[i][0], *m[i][1], *m[i][2], *m[i][3]);
            igText(buffer);
        }
    }
#endif

    igEnd();
}
