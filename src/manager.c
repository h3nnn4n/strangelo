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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

#include "manager.h"
#include "rendering.h"
#include "settings.h"

Manager *manager;

Manager *init_manager() {
    Manager *_manager = malloc(sizeof(Manager));

    memset(_manager, 0, sizeof(Manager));

    _manager->incremental_rendering = true;
    _manager->tone_mapping_mode     = 6; // Uchimura
    _manager->exposure              = 0.75f;
    _manager->gamma = 2.2f;
    _manager->brightness = 0.0f;
    _manager->contrast = 1.0f;
    _manager->freeze_movement = false;

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

    _manager->texture_data_gl = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4 * sizeof(unsigned char));
    for (int i = 0; i < WINDOW_WIDTH; i++) {
        for (int j = 0; j < WINDOW_HEIGHT; j++) {
            _manager->texture_data_gl[i * WINDOW_HEIGHT + j] = 0;
            _manager->texture_data_gl[i * WINDOW_HEIGHT + j] = 0;
            _manager->texture_data_gl[i * WINDOW_HEIGHT + j] = 0;
            _manager->texture_data_gl[i * WINDOW_HEIGHT + j] = 255;
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

        manager->texture_data_gl[i * 4 + 0] = 0;
        manager->texture_data_gl[i * 4 + 1] = 0;
        manager->texture_data_gl[i * 4 + 2] = 0;
        manager->texture_data_gl[i * 4 + 3] = 255;
    }

    // Reset histogram data
    memset(manager->histogram, 0, sizeof(manager->histogram));
    memset(manager->histogram_normalized, 0, sizeof(manager->histogram_normalized));
}

void copy_clifford_to_texture(Manager *manager) {
    uint32_t  border_size_x = WINDOW_WIDTH * manager->border_size_percent;
    uint32_t  border_size_y = WINDOW_HEIGHT * manager->border_size_percent;
    Clifford *clifford      = manager->clifford;

    // Copy the clifford buffer to the high res texture data, centered
    for (int i = 0; i < clifford->width; i++) {
        for (int j = 0; j < clifford->height; j++) {
            int      index         = i + j * (clifford->width);
            int      texture_index = ((border_size_x + i) + (border_size_y + j) * WINDOW_WIDTH) * 4;
            uint32_t density       = clifford->density_map[index];

            manager->texture_data[texture_index + 0] = density;
            manager->texture_data[texture_index + 1] = density;
            manager->texture_data[texture_index + 2] = density;
            manager->texture_data[texture_index + 3] = 255;
        }
    }
}

void normalize_texture(Manager *manager) {
    uint32_t max_value = 0;
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        if (manager->texture_data[i * 4 + 0] > max_value) {
            max_value = manager->texture_data[i * 4 + 0];
        }
    }

    // Reset histogram data
    memset(manager->histogram, 0, sizeof(manager->histogram));

    // Copy the high res texture data to the low res texture data
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        unsigned char value                 = (manager->texture_data[i * 4 + 0] / (float)max_value) * 255.0f;
        manager->texture_data_gl[i * 4 + 0] = value;
        manager->texture_data_gl[i * 4 + 1] = value;
        manager->texture_data_gl[i * 4 + 2] = value;
        manager->texture_data_gl[i * 4 + 3] = 255;

        // Count pixel values for histogram
        manager->histogram[value]++;
    }

    // Normalize histogram values for display
    int max_bin_count = 0;
    for (int i = 0; i < 256; i++) {
        if (manager->histogram[i] > max_bin_count) {
            max_bin_count = manager->histogram[i];
        }
    }

    // Normalize to 0.0-1.0 range
    if (max_bin_count > 0) {
        for (int i = 0; i < 256; i++) {
            manager->histogram_normalized[i] = (float)manager->histogram[i] / max_bin_count;
        }
    } else {
        memset(manager->histogram_normalized, 0, sizeof(manager->histogram_normalized));
    }
}

// Maybe should be somewhere else since this is a rendering function?
void blit_clifford_to_texture(Manager *manager) {
    clean_texture(manager);

    copy_clifford_to_texture(manager);

    normalize_texture(manager);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 manager->texture_data_gl);
}