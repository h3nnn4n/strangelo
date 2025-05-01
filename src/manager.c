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
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

#include "manager.h"
#include "settings.h"

Manager *manager;

Manager *init_manager() {
    Manager *_manager = malloc(sizeof(Manager));

    memset(_manager, 0, sizeof(Manager));

    _manager->incremental_rendering = true;
    _manager->ambient_light         = true;
    _manager->tone_mapping_mode     = 6; // Uchimura

    _manager->n_samples = 10;
    _manager->n_bounces = 5;

    _manager->exposure = 0.75f;

    _manager->texture_data = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4 * sizeof(unsigned char));
    for (int y = 0; y < WINDOW_HEIGHT; y++) {
        for (int x = 0; x < WINDOW_WIDTH; x++) {
            int index                         = (y * WINDOW_WIDTH + x) * 4;
            _manager->texture_data[index + 0] = 0;   // R
            _manager->texture_data[index + 1] = 0;   // G
            _manager->texture_data[index + 2] = 0;   // B
            _manager->texture_data[index + 3] = 255; // A
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

void Manager_set_camera(Manager *manager, Camera *camera) {
    assert(manager);
    assert(camera);

    manager->camera = camera;
}

// Maybe should be somewhere else since this is a rendering function?
void blit_clifford_to_texture(Manager *manager) {
    uint32_t max_value = 0;
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        if (manager->clifford->buffer[i] > max_value) {
            max_value = manager->clifford->buffer[i];
        }
    }

    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        manager->texture_data[i * 4 + 0] = manager->clifford->buffer[i] * 255 / max_value;
        manager->texture_data[i * 4 + 1] = manager->clifford->buffer[i] * 255 / max_value;
        manager->texture_data[i * 4 + 2] = manager->clifford->buffer[i] * 255 / max_value;
        manager->texture_data[i * 4 + 3] = 255;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 manager->texture_data);
}