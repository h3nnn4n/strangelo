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

    _manager->incremental_rendering         = true;
    _manager->tone_mapping_mode             = 6; // Uchimura
    _manager->exposure                      = 0.75f;
    _manager->gamma                         = 2.2f;
    _manager->brightness                    = 0.0f;
    _manager->contrast                      = 1.0f;
    _manager->enable_histogram_equalization = false;
    _manager->freeze_movement               = false;

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

    // Copy the high res texture data to the low res texture data
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        unsigned char value                 = (manager->texture_data[i * 4 + 0] / (float)max_value) * 255.0f;
        manager->texture_data_gl[i * 4 + 0] = value;
        manager->texture_data_gl[i * 4 + 1] = value;
        manager->texture_data_gl[i * 4 + 2] = value;
        manager->texture_data_gl[i * 4 + 3] = 255;
    }
}

void calculate_histogram(Manager *manager) {
    // Reset histogram data
    memset(manager->histogram, 0, sizeof(manager->histogram));

    // Find maximum value for scaling
    uint32_t max_value = 0;
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        if (manager->texture_data[i * 4 + 0] > max_value) {
            max_value = manager->texture_data[i * 4 + 0];
        }
    }

    // Avoid division by zero
    if (max_value == 0) {
        max_value = 1;
    }

    // Calculate histogram from high resolution texture data
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        uint32_t value = manager->texture_data[i * 4 + 0];

        // Only count non-black pixels in the histogram
        if (value > 0) {
            // Scale value to 0-255 range for the histogram
            unsigned char bin = (unsigned char)((value / (float)max_value) * 255.0f);
            manager->histogram[bin]++;
        }
    }
}

void normalize_histogram(Manager *manager) {
    // Find maximum bin count
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

void apply_histogram_equalization(Manager *manager) {
    // Calculate cumulative distribution function (CDF)
    int cdf[256] = {0};
    cdf[0]       = manager->histogram[0];

    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + manager->histogram[i];
    }

    // Find the first non-zero value in the CDF (min_cdf)
    int min_cdf = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > 0) {
            min_cdf = cdf[i];
            break;
        }
    }

    // Calculate total number of pixels (excluding black background)
    int total_pixels = cdf[255];

    // Avoid division by zero
    if (total_pixels - min_cdf <= 0) {
        return; // Nothing to equalize
    }

    // Create mapping function for histogram equalization (0.0 to 1.0 range)
    float equalize_map[256] = {0.0f};
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > min_cdf) {
            equalize_map[i] = (float)(cdf[i] - min_cdf) / (total_pixels - min_cdf);
        } else {
            equalize_map[i] = 0.0f;
        }
    }

    // Find maximum value in the texture data for scaling
    uint32_t max_value = 0;
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        if (manager->texture_data[i * 4 + 0] > max_value) {
            max_value = manager->texture_data[i * 4 + 0];
        }
    }

    // Avoid division by zero
    if (max_value == 0) {
        return; // Nothing to equalize
    }

    // Apply the mapping to the high-resolution texture data
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        uint32_t value = manager->texture_data[i * 4 + 0];

        if (value > 0) { // Only process non-black pixels
            // Calculate bin index (0-255) for current value
            unsigned char bin = (unsigned char)((value / (float)max_value) * 255.0f);

            // Apply equalization mapping and scale back to original range
            float    normalized = equalize_map[bin];
            uint32_t new_value  = (uint32_t)(normalized * max_value);

            // Update all color channels
            manager->texture_data[i * 4 + 0] = new_value;
            manager->texture_data[i * 4 + 1] = new_value;
            manager->texture_data[i * 4 + 2] = new_value;
        }
    }

    // After modifying the high-resolution data, we need to update the GL texture data
    normalize_texture(manager);
}

/**
 * Counts the number of unique values in the Clifford density map, texture_data and texture_data_gl
 * Uses a simple hashset approach for efficient counting of unique values
 */
void count_unique_values(Manager *manager) {
    // Count unique values in Clifford density map
    Clifford *clifford = manager->clifford;
    int clifford_size = clifford->width * clifford->height;
    
    // Simple approach using flags array - efficient enough for this use case
    // Create a boolean array to mark which values we've seen
    // We use calloc to ensure it's initialized to 0
    const uint32_t MAX_DENSITY = 100000; // Reasonable upper bound for Clifford density
    bool *seen_clifford = calloc(MAX_DENSITY, sizeof(bool));
    
    if (seen_clifford) {
        // Count unique values in Clifford density map
        manager->unique_clifford_values = 0;
        for (int i = 0; i < clifford_size; i++) {
            uint32_t value = clifford->density_map[i];
            if (value < MAX_DENSITY && !seen_clifford[value]) {
                seen_clifford[value] = true;
                manager->unique_clifford_values++;
            }
        }
        free(seen_clifford);
    } else {
        manager->unique_clifford_values = 0; // Memory allocation failed
    }
    
    // Count unique values in texture_data (only considering red channel)
    const uint32_t texture_size = WINDOW_WIDTH * WINDOW_HEIGHT;
    bool *seen_texture_data = calloc(MAX_DENSITY, sizeof(bool));
    
    if (seen_texture_data) {
        manager->unique_texture_data_values = 0;
        for (int i = 0; i < texture_size; i++) {
            uint32_t value = manager->texture_data[i * 4]; // Red channel
            if (value < MAX_DENSITY && !seen_texture_data[value]) {
                seen_texture_data[value] = true;
                manager->unique_texture_data_values++;
            }
        }
        free(seen_texture_data);
    } else {
        manager->unique_texture_data_values = 0; // Memory allocation failed
    }
    
    // Count unique values in texture_data_gl (only considering red channel)
    // We know the maximum possible is 256 for 8-bit values
    bool seen_texture_data_gl[256] = {false};
    
    manager->unique_texture_data_gl_values = 0;
    for (int i = 0; i < texture_size; i++) {
        unsigned char value = manager->texture_data_gl[i * 4]; // Red channel
        if (!seen_texture_data_gl[value]) {
            seen_texture_data_gl[value] = true;
            manager->unique_texture_data_gl_values++;
        }
    }
}

/**
 * Main function that orchestrates the process of converting Clifford attractor data
 * to a texture and preparing histogram data
 */
void blit_clifford_to_texture(Manager *manager) {
    // Step 1: Clean the texture data
    clean_texture(manager);

    // Step 2: Copy Clifford data to the texture
    copy_clifford_to_texture(manager);

    // Step 3: Calculate the histogram from the high-resolution texture
    calculate_histogram(manager);
    normalize_histogram(manager);

    // Step 4: Apply histogram equalization if enabled (works on high-res data)
    if (manager->enable_histogram_equalization) {
        apply_histogram_equalization(manager);

        // Recalculate histogram after equalization for display
        calculate_histogram(manager);
        normalize_histogram(manager);
    }

    // Step 5: Normalize texture for display (scales high-res to GL texture)
    normalize_texture(manager);
    
    // Step 6: Count unique values in all data structures
    count_unique_values(manager);

    // Step 7: Upload the texture to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 manager->texture_data_gl);
}