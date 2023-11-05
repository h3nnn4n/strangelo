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

#ifndef _MANAGER_H
#define _MANAGER_H

#include <stdint.h>

#include "camera.h"

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
    bool     incremental_rendering;
    uint32_t tone_mapping_mode;
    uint32_t render_texture;
    uint32_t skybox_texture;
    uint32_t debug_texture;
    bool     ambient_light;
    uint32_t n_samples;
    uint32_t n_bounces;

    /////////////////
    // Movement
    //
    bool freeze_movement;

    /////////////////
    // GUI
    //
    bool hide_ui;

    /////////////////
    // Camera
    //
    Camera *camera;
} Manager;

extern Manager *manager;

Manager *init_manager();

void Manager_tick_timer(Manager *manager);
void Manager_set_camera(Manager *manager, Camera *camera);

#endif
