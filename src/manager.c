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

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <GLFW/glfw3.h>

#include "attractor.h"
#include "manager.h"
#include "rendering.h"
#include "settings.h"

Manager *manager;

Manager *init_manager() {
    Manager *_manager = malloc(sizeof(Manager));

    memset(_manager, 0, sizeof(Manager));

    _manager->compute_count = 8;

    _manager->incremental_rendering = true;
    _manager->tone_mapping_mode     = 1; // ACES
    _manager->exposure              = 0.75f;
    _manager->gamma                 = 2.2f;
    _manager->brightness            = 0.0f;
    _manager->contrast              = 1.0f;
    _manager->scaling_method        = POWER_SCALING;
    _manager->freeze_movement       = false;

    // Initialize scaling parameters with sensible defaults
    _manager->power_exponent    = 0.5f; // Square root by default
    _manager->sigmoid_midpoint  = 0.5f; // Midpoint at 50% of max value
    _manager->sigmoid_steepness = 3.0f; // Medium steepness

    _manager->border_size_percent = 0.05f;

    _manager->texture_data = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4 * sizeof(uint32_t));
    for (int i = 0; i < WINDOW_WIDTH; i++) {
        for (int j = 0; j < WINDOW_HEIGHT; j++) {
            _manager->texture_data[i * WINDOW_HEIGHT + j] = 0;
            _manager->texture_data[i * WINDOW_HEIGHT + j] = 0;
            _manager->texture_data[i * WINDOW_HEIGHT + j] = 0;
            _manager->texture_data[i * WINDOW_HEIGHT + j] = 255;
        }
    }

    _manager->texture_data_gl = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4 * sizeof(float));
    for (int i = 0; i < WINDOW_WIDTH; i++) {
        for (int j = 0; j < WINDOW_HEIGHT; j++) {
            _manager->texture_data_gl[i * WINDOW_HEIGHT + j] = 0.0f;
            _manager->texture_data_gl[i * WINDOW_HEIGHT + j] = 0.0f;
            _manager->texture_data_gl[i * WINDOW_HEIGHT + j] = 0.0f;
            _manager->texture_data_gl[i * WINDOW_HEIGHT + j] = 1.0f;
        }
    }

    return _manager;
}

void Manager_tick_timer(Manager *manager) {
    manager->current_time = glfwGetTime();

    manager->last_frame_time    = manager->current_frame_time;
    manager->current_frame_time = manager->current_time;
    manager->delta_time         = manager->current_frame_time - manager->last_frame_time;

    manager->frame_count++;
}

void merge_attractors_data(Manager *manager) {
    for (int i = 0; i < manager->compute_count; i++) {
        Attractor *attractor = manager->computes[i]->attractor;

        for (int j = 0; j < attractor->width * attractor->height; j++) {
            manager->attractor->density_map[j] += attractor->density_map[j];
        }
    }
}

void blit_attractor_to_texture(Manager *manager) {
    clean_texture_data(manager->texture_data, manager->texture_data_gl, WINDOW_WIDTH, WINDOW_HEIGHT);

    merge_attractors_data(manager);

    copy_attractor_to_texture_data(manager->attractor, manager->texture_data, WINDOW_WIDTH, WINDOW_HEIGHT,
                                   manager->border_size_percent);

    normalize_texture_data(manager->texture_data, manager->texture_data_gl, WINDOW_WIDTH, WINDOW_HEIGHT,
                           manager->scaling_method, manager->power_exponent, manager->sigmoid_midpoint,
                           manager->sigmoid_steepness);

    render_texture_to_gl(manager->texture_data_gl, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void manager_init_compute(Manager *manager) {
    manager->computes = malloc(manager->compute_count * sizeof(Compute *));

    for (int i = 0; i < manager->compute_count; i++) {
        Attractor *attractor =
            make_attractor(ATTRACTOR_TYPE_CLIFFORD, (1.0f - manager->border_size_percent) * WINDOW_WIDTH,
                           (1.0f - manager->border_size_percent) * WINDOW_HEIGHT);
        manager->computes[i] = compute_init(attractor);
    }
}

void manager_destroy_compute(Manager *manager) {
    for (int i = 0; i < manager->compute_count; i++) {
        compute_destroy(manager->computes[i]);
    }
}

void manager_pause_compute(Manager *manager) {
    for (int i = 0; i < manager->compute_count; i++) {
        compute_pause(manager->computes[i]);
    }
}

void manager_resume_compute(Manager *manager) {
    for (int i = 0; i < manager->compute_count; i++) {
        compute_resume(manager->computes[i]);
    }
}

void manager_compute_iterate_until_timeout(Manager *manager, float timeout) {
    float start_time = glfwGetTime();

    manager_resume_compute(manager);

    while (glfwGetTime() - start_time < timeout) {
        // Sleep for 1ms
        struct timespec ts = {0, 1000000};
        nanosleep(&ts, NULL);
    }

    manager_pause_compute(manager);
}

void manager_clean_attractor(Manager *manager) {
    clean_attractor(manager->attractor);

    for (int i = 0; i < manager->compute_count; i++) {
        compute_clean_attractor(manager->computes[i]);
    }
}

void manager_reset_attractor(Manager *manager) {
    reset_attractor(manager->attractor);

    for (int i = 0; i < manager->compute_count; i++) {
        compute_reset_attractor(manager->computes[i]);
    }
}

void manager_propagate_attractor(Manager *manager) {
    for (int i = 0; i < manager->compute_count; i++) {
        for (int j = 0; j < manager->attractor->num_parameters; j++) {
            manager->computes[i]->attractor->parameters[j] = manager->attractor->parameters[j];
        }
    }
}
