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

#include <pcg_variants.h>

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
        gui_update_clifford();
        gui_update_histogram();
    }

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

void gui_update_clifford() {
    if (!igBegin("Clifford", NULL, 0))
        return igEnd();

    Clifford *clifford = manager->clifford;

    snprintf(buffer, sizeof(buffer), "%2.6f %2.6f %2.6f %2.6f", clifford->a, clifford->b, clifford->c, clifford->d);
    igText(buffer);

    float old_a = clifford->a;
    float old_b = clifford->b;
    float old_c = clifford->c;
    float old_d = clifford->d;

    igSliderFloat("a", &clifford->a, -2, 2, "%2.6f", 0);
    igSliderFloat("b", &clifford->b, -2, 2, "%2.6f", 0);
    igSliderFloat("c", &clifford->c, -2, 2, "%2.6f", 0);
    igSliderFloat("d", &clifford->d, -2, 2, "%2.6f", 0);

    if (old_a != clifford->a || old_b != clifford->b || old_c != clifford->c || old_d != clifford->d) {
        reset_clifford(clifford);
    }

    ImVec2 size = {100, 0};
    if (igButton("Randomize", size)) {
        randomize_until_chaotic(clifford);
    }

    igSeparator();

    // Occupancy
    snprintf(buffer, sizeof(buffer), "Occupancy: %2.6f", get_occupancy(clifford));
    igText(buffer);

    igSeparator();

    // Post-processing parameters
    igText("Post-processing:");

    // Tone mapping combo box
    const char *tone_mapping_modes[] = {"None",        "ACES",     "Filmic",      "Lottes", "Reinhard",
                                        "Reinhard II", "Uchimura", "Uncharted 2", "Unreal"};
    int         tone_mapping_count   = 9; // Number of elements in the array above
    int         current_mode         = (int)manager->tone_mapping_mode;
    igCombo_Str_arr("Tone Mapping", &current_mode, tone_mapping_modes, tone_mapping_count, 0);
    manager->tone_mapping_mode = (uint32_t)current_mode;

    igSliderFloat("Exposure", &manager->exposure, 0.1f, 5.0f, "%2.2f", 0);
    igSliderFloat("Gamma", &manager->gamma, 0.1f, 5.0f, "%2.2f", 0);
    igSliderFloat("Brightness", &manager->brightness, -0.5f, 0.5f, "%2.2f", 0);
    igSliderFloat("Contrast", &manager->contrast, 0.5f, 2.0f, "%2.2f", 0);

    // Reset button for post-processing parameters
    ImVec2 reset_button_size = {120, 0}; // Width of 120, auto height
    if (igButton("Reset Post-Proc", reset_button_size)) {
        manager->tone_mapping_mode             = 6;     // Default: Uchimura
        manager->exposure                      = 0.75f; // Default exposure
        manager->gamma                         = 2.2f;  // Default gamma
        manager->brightness                    = 0.0f;  // Default brightness
        manager->contrast                      = 1.0f;  // Default contrast
        manager->enable_histogram_equalization = false; // Disable histogram equalization
        manager->enable_log_scaling            = true;  // Enable logarithmic scaling by default

        // Force update to apply the reset settings
        blit_clifford_to_texture(manager);
    }

    return igEnd();
}

void gui_update_histogram() {
    if (!igBegin("Histogram", NULL, 0))
        return igEnd();

    // Create x-axis indices for the histogram (0-255)
    static float x_data[256];
    static bool  initialized = false;

    if (!initialized) {
        for (int i = 0; i < 256; i++) {
            x_data[i] = (float)i;
        }
        initialized = true;
    }

    // Histogram Controls
    if (igCollapsingHeader_BoolPtr("Histogram Settings", NULL, 0)) {
        bool update_needed = false;
        
        // Add checkbox for logarithmic scaling
        if (igCheckbox("Enable Logarithmic Scaling", &manager->enable_log_scaling)) {
            update_needed = true;
        }
        igTextWrapped("Logarithmic scaling preserves more detail by compressing high values while expanding lower values. "
                     "This significantly increases the number of unique values preserved in the final image.");
        
        igSeparator();
        
        // Add checkbox for histogram equalization
        if (igCheckbox("Enable Histogram Equalization", &manager->enable_histogram_equalization)) {
            update_needed = true;
        }
        
        if (manager->enable_histogram_equalization) {
            igTextWrapped("Histogram equalization enhances image contrast by redistributing intensity values.");
        }
        
        igSeparator();
        
        // Add a button to manually apply changes right away
        ImVec2 apply_btn_size = {180, 0};
        if (igButton("Apply Changes Now", apply_btn_size) || update_needed) {
            // Force recalculation of the histogram and application of transformations
            blit_clifford_to_texture(manager);
        }
        
        igSeparator();
    }

    // Display histogram
    ImVec2 size = {300, 200};

    // Calculate histogram statistics before plotting
    int non_zero_bins  = 0;
    int first_non_zero = 255;
    int last_non_zero  = 0;

    for (int i = 0; i < 256; i++) {
        if (manager->histogram[i] > 0) {
            non_zero_bins++;
            first_non_zero = (i < first_non_zero) ? i : first_non_zero;
            last_non_zero  = (i > last_non_zero) ? i : last_non_zero;
        }
    }

    if (ImPlot_BeginPlot("Image Histogram", size, ImPlotFlags_NoMouseText)) {
        ImPlot_SetupAxesLimits(0, 255, 0, 1.05, ImGuiCond_Always);
        ImPlot_SetupAxes("Pixel Value", "Normalized Count", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);

        // Dynamic color based on equalization state
        ImVec4 line_color = manager->enable_histogram_equalization ? (ImVec4){0.0, 1.0, 0.0, 1.0}
                                                                   : // Green when equalized
                                (ImVec4){1.0, 1.0, 1.0, 1.0};        // White otherwise

        // Plot the histogram as a line
        ImPlot_PushStyleColor_Vec4(ImPlotCol_Line, line_color);
        ImPlot_PlotLine_FloatPtrFloatPtr("Luminance", x_data, manager->histogram_normalized, 256, 0, 0, 4);
        ImPlot_PopStyleColor(1);

        // Plot the histogram as filled bars
        ImVec4 fill_color = {0.5, 0.5, 0.5, 0.5}; // Semi-transparent gray
        ImPlot_PushStyleColor_Vec4(ImPlotCol_Fill, fill_color);
        ImPlot_PlotShaded_FloatPtrFloatPtrInt("##Shaded", x_data, manager->histogram_normalized, 256, 0, 0, 0, 4);
        ImPlot_PopStyleColor(1);

        ImPlot_EndPlot();
    }

    // Display statistics below the histogram
    igText("Histogram Statistics:");
    igText("  Range: [%d, %d]", 0, 255);
    if (non_zero_bins > 0) {
        igSameLine(0, 20);
        igText("Active Range: [%d, %d]", first_non_zero, last_non_zero);
    }
    igText("  Non-zero bins: %d / 256 (%.1f%%)", non_zero_bins, (float)non_zero_bins / 256.0f * 100.0f);

    // Add unique value counts section
    if (igCollapsingHeader_BoolPtr("Unique Value Analysis", NULL, 0)) {
        igText("Information density at each processing stage:");
        
        // Clifford density map
        igText("Clifford density map: %u unique values", 
               manager->unique_clifford_values);
        
        // texture_data (high resolution)
        igText("Texture data (high-res): %u unique values", 
               manager->unique_texture_data_values);
        
        // texture_data_gl (8-bit)
        igText("GL texture (8-bit): %u unique values / 256", 
               manager->unique_texture_data_gl_values);
        
        // Display percentages for better understanding
        float clifford_percentage = manager->unique_clifford_values > 0 ?
                                   ((float)manager->unique_texture_data_values / manager->unique_clifford_values) * 100.0f : 0.0f;
                                    
        float texture_data_percentage = manager->unique_texture_data_values > 0 ?
                                       ((float)manager->unique_texture_data_gl_values / manager->unique_texture_data_values) * 100.0f : 0.0f;
                                       
        igSeparator();
        
        igText("Information preserved:");
        igText("  Clifford → texture: %.1f%%", clifford_percentage);
        igText("  Texture → GL: %.1f%%", texture_data_percentage);
        igText("  Overall: %.1f%%", 
               manager->unique_clifford_values > 0 ? 
               ((float)manager->unique_texture_data_gl_values / manager->unique_clifford_values) * 100.0f : 0.0f);
        
        igSeparator();
        
        // Show analysis explanation
        igTextWrapped("Higher unique value counts indicate more detailed and precise data representation. "
                     "The reduction in unique values through the processing pipeline represents "
                     "quantization (precision loss) during rendering.");
    }

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
