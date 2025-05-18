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

void clean_texture(Manager *manager) {
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        manager->texture_data[i * 4 + 0] = 0;
        manager->texture_data[i * 4 + 1] = 0;
        manager->texture_data[i * 4 + 2] = 0;
        manager->texture_data[i * 4 + 3] = 255;

        manager->texture_data_gl[i * 4 + 0] = 0.0f;
        manager->texture_data_gl[i * 4 + 1] = 0.0f;
        manager->texture_data_gl[i * 4 + 2] = 0.0f;
        manager->texture_data_gl[i * 4 + 3] = 1.0f;
    }
}

void copy_attractor_to_texture(Manager *manager) {
    uint32_t   border_size_x = WINDOW_WIDTH * manager->border_size_percent;
    uint32_t   border_size_y = WINDOW_HEIGHT * manager->border_size_percent;
    Attractor *attractor     = manager->attractor;

    // Copy the attractor buffer to the high res texture data, centered
    for (int i = 0; i < attractor->width; i++) {
        for (int j = 0; j < attractor->height; j++) {
            int      index         = i + j * (attractor->width);
            int      texture_index = ((border_size_x + i) + (border_size_y + j) * WINDOW_WIDTH) * 4;
            uint32_t density       = attractor->density_map[index];

            manager->texture_data[texture_index + 0] = density;
            manager->texture_data[texture_index + 1] = density;
            manager->texture_data[texture_index + 2] = density;
            manager->texture_data[texture_index + 3] = 255;
        }
    }
}

/**
 * Apply a sigmoid function to normalize value
 *
 * @param x Input value in range [0, 1]
 * @param midpoint Value where sigmoid is 0.5 (range 0 to 1)
 * @param steepness Controls the steepness of the curve (higher = steeper)
 * @return Normalized value in range [0, 1]
 */
float sigmoid_normalize(float x, float midpoint, float steepness) {
    // Shift and scale to center sigmoid around midpoint
    float shifted = (x - midpoint) * steepness;
    // Apply sigmoid function: 1/(1+e^-x)
    float result = 1.0f / (1.0f + expf(-shifted));
    return result;
}

/**
 * Normalize the high-res texture data to 8-bit values using different scaling methods
 */
void normalize_texture(Manager *manager) {
    // Find maximum value in the texture data
    uint32_t max_value = 0;
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        if (manager->texture_data[i * 4 + 0] > max_value) {
            max_value = manager->texture_data[i * 4 + 0];
        }
    }

    // Avoid division by zero
    if (max_value == 0)
        max_value = 1;

    // Copy the high res texture data to the float texture data
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        uint32_t pixel_value = manager->texture_data[i * 4 + 0];

        // Normalize to [0, 1] range for processing
        float normalized = (float)pixel_value / max_value;

        // Apply selected scaling method
        switch (manager->scaling_method) {
            case LINEAR_SCALING:
                // Linear scaling (no change)
                // normalized = normalized;
                break;

            case LOG_SCALING:
                // Logarithmic scaling - compresses high values, expands low values
                // Using log(1+x) to handle x=0 case gracefully
                normalized = logf(1.0f + normalized * 9.0f) / logf(10.0f); // Maps [0,1] to [0,1]
                break;

            case POWER_SCALING:
                // Power scaling - adjustable exponent
                // Smaller exponent expands lower values, larger compresses them
                normalized = powf(normalized, manager->power_exponent);
                break;

            case SIGMOID_SCALING:
                // Sigmoid scaling - adjustable center point and steepness
                // Good for enhancing contrast around a specific intensity
                normalized = sigmoid_normalize(normalized, manager->sigmoid_midpoint, manager->sigmoid_steepness);
                break;

            case SQRT_SCALING:
                // Square root scaling - good balance of detail preservation
                normalized = sqrtf(normalized);
                break;

            default:
                // Default to linear if unknown method
                break;
        }

        // Store normalized value directly in float texture
        manager->texture_data_gl[i * 4 + 0] = normalized;
        manager->texture_data_gl[i * 4 + 1] = normalized;
        manager->texture_data_gl[i * 4 + 2] = normalized;
        manager->texture_data_gl[i * 4 + 3] = 1.0f;
    }
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
    clean_texture(manager);

    merge_attractors_data(manager);

    copy_attractor_to_texture(manager);

    normalize_texture(manager);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT,
                 manager->texture_data_gl);
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
