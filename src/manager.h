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

#ifndef SRC_MANAGER_H_
#define SRC_MANAGER_H_

#include <stdbool.h>
#include <stdint.h>

#include "attractor.h"
#include "compute.h"
#include "rendering.h" // For ScalingMethod enum

typedef struct {
    /////////////////
    // Timer Stuff
    //
    float    delta_time;
    float    current_time;
    float    current_frame_time;
    float    last_frame_time;
    uint64_t frame_count;

    /////////////////
    // Rendering
    //
    bool            incremental_rendering;
    ToneMappingMode tone_mapping_mode;
    float           exposure;
    uint32_t        render_texture;
    float           gamma;
    float           brightness;
    float           contrast;

    // Image processing settings
    ScalingMethod scaling_method;
    bool          freeze_movement;

    // Scaling parameters
    float power_exponent;    // For power scaling (0.1 to 5.0)
    float sigmoid_midpoint;  // For sigmoid scaling (0.0 to 1.0)
    float sigmoid_steepness; // For sigmoid scaling (0.1 to 10.0)

    uint32_t *texture_data;
    float    *texture_data_gl;

    // Unique value counts
    uint32_t unique_clifford_values;
    uint32_t unique_texture_data_values;
    uint32_t unique_texture_data_gl_values;

    float border_size_percent;

    //////////////////
    // Attractors
    //
    Attractor *attractor;

    /////////////////
    // GUI
    //
    bool hide_ui;

    /////////////////
    // Compute
    //
    uint32_t  compute_count;
    Compute **computes;
} Manager;

extern Manager *manager;

Manager *init_manager();

void Manager_tick_timer(Manager *manager);

void blit_attractor_to_texture(Manager *manager);

void manager_init_compute(Manager *manager);
void manager_destroy_compute(Manager *manager);
void manager_pause_compute(Manager *manager);
void manager_resume_compute(Manager *manager);
void manager_clean_attractor(Manager *manager);
void manager_reset_attractor(Manager *manager);

void manager_compute_iterate_until_timeout(Manager *manager, float timeout);

void manager_propagate_attractor(Manager *manager);

#endif // SRC_MANAGER_H_
