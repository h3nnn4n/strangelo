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

#include "fps.h"
#include "gui.h"
#include "imgui_custom_c.h"
#include "manager.h"
#include "settings.h"

struct ImGuiContext  *ctx;
struct ImGuiIO       *io;
struct ImPlotContext *plot_ctx;

char buffer[1024];

void gui_init() {
    ctx      = igCreateContext(NULL);
    io       = igGetIO();
    plot_ctx = ImPlot_CreateContext();

    const char *glsl_version = "#version 460 core";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    igStyleColorsDark(NULL);

    init_fps();
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

    if (!manager->hide_ui) {
        gui_update_fps();
        gui_update_scene();
        gui_update_camera();
        gui_debug();
    }

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

void gui_update_scene() {
    if (!igBegin("Scene", NULL, 0))
        return igEnd();

    if (manager->ambient_light)
        snprintf(buffer, sizeof(buffer), "ambient_light: ON");
    else
        snprintf(buffer, sizeof(buffer), "ambient_light: OFF");

    toggle_button("ambient_light", buffer, &manager->ambient_light);

    igSeparator();

    // Radio button for tone mapping selection
    igText("Tone Mapping");
    igRadioButton_IntPtr("NONE", (int *)&manager->tone_mapping_mode, 0);
    igRadioButton_IntPtr("ACES Narkowicz", (int *)&manager->tone_mapping_mode, 1);
    igRadioButton_IntPtr("Filmic", (int *)&manager->tone_mapping_mode, 2);
    igRadioButton_IntPtr("Lottes", (int *)&manager->tone_mapping_mode, 3);
    igRadioButton_IntPtr("Reinhard", (int *)&manager->tone_mapping_mode, 4);
    igRadioButton_IntPtr("Reinhard 2", (int *)&manager->tone_mapping_mode, 5);
    igRadioButton_IntPtr("Uchimura", (int *)&manager->tone_mapping_mode, 6);
    igRadioButton_IntPtr("Uncharted 2", (int *)&manager->tone_mapping_mode, 7);
    igRadioButton_IntPtr("Unreal", (int *)&manager->tone_mapping_mode, 8);

    igEnd();
}

void gui_update_fps() {
    if (!igBegin("Window", NULL, 0))
        return igEnd();

    float ms          = manager->delta_time;
    float instant_fps = 1.0 / ms;
    float average_fps = 0.0f;

    add_fps_sample(instant_fps);

    static int n_samples_old = -1;
    int        n_samples     = get_fps_sample_count();
    n_samples                = (float)fmin(n_samples, (((int)(instant_fps * 1.2f) / 10.0f) * 10.0f));

    if (n_samples_old == -1) {
        n_samples_old = n_samples;
    } else if (n_samples_old != n_samples) {
        n_samples     = n_samples_old * 0.9 + n_samples * 0.1;
        n_samples_old = n_samples;
    }

    average_fps             = get_average_fps_with_sample_limit(n_samples);
    float        max_fps    = get_max_fps_with_sample_limit(n_samples);
    const float *fps_buffer = get_fps_buffer_with_sample_limit(n_samples);
    const float *fps_index  = get_fps_index_buffer();

    snprintf(buffer, sizeof(buffer), "FPS: %6.2f  (%.2f)", instant_fps, average_fps);
    igText(buffer);

    snprintf(buffer, sizeof(buffer), " ms: %9.5f", ms);
    igText(buffer);

    ImVec2 size            = {200, 100};
    ImVec4 plot_color_line = {1, 1, 0, 1};
    ImVec4 plot_color_fill = {1, 1, 0, 0.25};

    if (ImPlot_BeginPlot("fps", size, 0)) {
        ImPlot_SetupAxesLimits(0, n_samples, 0, max_fps * 1.2, ImGuiCond_Always);
        ImPlot_SetupAxes("time", "fps", 0, 0);
        ImPlot_PushStyleColor_Vec4(ImPlotCol_Line, plot_color_line);
        ImPlot_PushStyleColor_Vec4(ImPlotCol_Line, plot_color_fill);
        ImPlotAxisFlags axis_flags = ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_Lock |
                                     ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels;
        ImPlot_PlotLine_FloatPtrFloatPtr("f(x)", fps_index, fps_buffer, n_samples, axis_flags, 0, 4);
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

    if (manager->freeze_movement)
        snprintf(buffer, sizeof(buffer), "freeze_movement: true");
    else
        snprintf(buffer, sizeof(buffer), "freeze_movement: false");
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
        const mat4 *m = &camera->view;
        igText("view matrix:");
        for (int i = 0; i < 4; i++) {
            snprintf(buffer, sizeof(buffer), "%4.2f %4.2f %4.2f %4.2f", *m[i][0], *m[i][1], *m[i][2], *m[i][3]);
            igText(buffer);
        }
    }

    {
        igSeparator();
        const mat4 *m = &camera->projection;
        igText("projection matrix:");
        for (int i = 0; i < 4; i++) {
            snprintf(buffer, sizeof(buffer), "%4.2f %4.2f %4.2f %4.2f", *m[i][0], *m[i][1], *m[i][2], *m[i][3]);
            igText(buffer);
        }
    }
#endif

    igEnd();
}

void gui_debug() {
    igBegin("Debug", NULL, 0);

    igImage((ImTextureID)(intptr_t)manager->debug_texture, (ImVec2){200 * aspect_ratio, 200}, (ImVec2){0, 1},
            (ImVec2){1, 0}, (ImVec4){1, 1, 1, 1}, (ImVec4){1, 1, 1, 0});

    igEnd();
}
