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

#ifndef SRC_SCENE_H_
#define SRC_SCENE_H_

#include <stdlib.h>

#include <cglm/cglm.h>

typedef enum {
    DIFFUSE    = 1,
    METAL      = 2,
    DIELECTRIC = 3,
    WHATEVER   = 4,
    LIGHT      = 5,
} material_type_t;

typedef struct {
    vec4            position;
    float           radius;
    vec4            albedo;
    float           roughness;
    material_type_t material_type;
} object_t;

void init_scene();

extern vec3 camera_pos;
extern vec3 camera_orientation;

#define _OVEN_SCENE
// #define _WHATEVER_SCENE
// #define _CORNELL_BOX_SCENE

#ifdef _OVEN_SCENE

#define n_spheres 2

extern vec4  positions[n_spheres];
extern float radius[n_spheres];
extern vec4  albedo[n_spheres];
extern float roughness[n_spheres];
extern int   material_type[n_spheres];
#endif // _OVEN_SCENE

#ifdef _CORNELL_BOX_SCENE

vec3 camera_pos = {47, 51, -80};

const float f = 100;

// clang-format off

const vec4 positions[] = {
    { 1e5 + f      , 40.0       , 80        , 1.0 } , // Left
    {-1e5 - f + 80 , 40.0       , 80        , 1.0 } , // Right
    { 50           , 40.0       , 1e5 + f   , 1.0 } , // Back
    { 50           , 40.0       ,-1e5 - f   , 1.0 } , // Front
    { 50           , 1e5 + f    , 80        , 1.0 } , // Top
    { 50           ,-1e5        , 80        , 1.0 } , // Bottom
    { 27           , 16.5       , 47        , 1.0 } , // Glass
    { 73           , 16.5       , 78        , 1.0 } , // Mirror
    { 73 - 0       , 16.5 * 3.0 , 78        , 1.0 } , // Mirror
    { 73 - 0       , 16.5 * 5.0 , 78        , 1.0 } , // Mirror
    { 73 - 33.0    , 16.5       , 78 + 00.0 , 1.0 } , // Diffuse
    { 73 - 33.0    , 16.5 * 3.0 , 78 + 00.0 , 1.0 } , // Diffuse
    { 73 - 33.0    , 16.5 * 5.0 , 78 + 00.0 , 1.0 } , // Diffuse
    { 50 - 10      , 600 + 100  , 60        , 1.0 } , // Light
};

// clang-format on

const vec4 albedo[] = {
    {1.00, 0.25, 0.25}, //
    {0.25, 0.25, 1.00}, //
    {0.75, 0.75, 0.75}, //
    {1.00, 1.00, 1.00}, //
    {0.75, 0.75, 0.75}, //
    {0.75, 0.75, 0.75}, //
    {1.00, 0.75, 0.75}, //
    {1.00, 1.00, 1.00}, //
    {1.00, 1.00, 1.00}, //
    {1.00, 1.00, 1.00}, //
    {1.00, 0.41, 0.41}, //
    {0.41, 1.00, 0.41}, //
    {0.41, 0.41, 1.00}, //
    {0.00, 5.00, 5.00}, //
};

const float radius[] = {
    1e5,  //
    1e5,  //
    1e5,  //
    1e5,  //
    1e5,  //
    1e5,  //
    16.5, //
    16.5, //
    16.5, //
    16.5, //
    16.5, //
    16.5, //
    16.5, //
    600,  //
};

const float roughness[] = {
    0.0, //
    0.0, //
    0.0, //
    0.0, //
    0.0, //
    0.0, //
    0.0, //
    0.2, //
    0.5, //
    1.0, //
    0.0, //
    0.0, //
    0.0, //
    0.0, //
};

// 1 = diffuse
// 2 = metal
// 3 = dielectric
// 4 = reflective
// 5 = light
const int material_type[] = {
    1, //
    1, //
    1, //
    1, //
    1, //
    1, //
    3, //
    2, //
    2, //
    2, //
    1, //
    1, //
    1, //
    5, //
};

#endif // _CORNELL_BOX_SCENE

#ifdef _WHATEVER_SCENE

vec3        camera_pos = {0.0, 0.0, 0.0};
const float r          = 750;
const float f          = 11.5;

const vec4 positions[] = {
    {0.0, -0.5, -9.0, 1.0},  // 1
    {0.0, 1.5, -9.0, 1.0},   // 2
    {2.0, 0.5, -11.0, 1.0},  // 3
    {0.0, 0.0, -11.0, 1.0},  // 4
    {0.0, 0.0, 10.0, 1.0},   // 5
    {0.0, 10.0, 0.0, 1.0},   // 6
    {0.0, -10.0, 0.0, 1.0},  // 7
    {10.0, 0.0, 0.0, 1.0},   // 8
    {-15.0, 0.0, 0.0, 1.0},  // 9
    {-25.0, 1.0, 3.0, 1.0},  // 10
    {-27.0, 0.0, -5.0, 1.0}, // 11
    {0.0, -r - f, 0.0, 1.0}, // 12
};

const vec4 albedo[] = {
    {1.0, 1.0, 1.0, 0.0},    // 1
    {0.0, 0.0, 1.0, 0.0},    // 2
    {1.0, 1.0, 1.0, 0.0},    // 3
    {0.3, 0.8, 0.2, 0.0},    // 4
    {0.4, 0.2, 0.8, 0.0},    // 5
    {0.8, 0.7, 0.2, 0.0},    // 6
    {0.2, 0.7, 0.6, 0.0},    // 7
    {0.1, 0.4, 0.3, 0.0},    // 8
    {1.0, 1.0, 1.0, 0.0},    // 9
    {0.6, 0.3, 0.1, 0.0},    // 10
    {0.76, 0.61, 0.35, 0.0}, // 11  Brass color
    {1.0, 1.0, 1.0, 0.0},    // 12
};

const float radius[] = {
    1.25, // 1
    1.25, // 2
    2.5,  // 3
    2.5,  // 4
    2.5,  // 5
    2.5,  // 6
    2.5,  // 7
    2.5,  // 8
    2.5,  // 9
    3.5,  // 10
    3.5,  // 11
    r,    // 12
};

// 1 = diffuse
// 2 = metal
// 3 = dielectric
// 4 = reflective
// 5 = light
const int material_type[] = {
    5, // 1
    3, // 2
    1, // 3
    1, // 4
    1, // 5
    1, // 6
    1, // 7
    1, // 8
    3, // 9
    1, // 10
    2, // 11
    2, // 12
};

const float roughness[] = {
    0.0, // 1
    0.0, // 2
    0.0, // 3
    0.0, // 4
    0.0, // 5
    0.0, // 6
    0.0, // 7
    0.0, // 8
    0.0, // 9
    0.0, // 10
    0.5, // 11
    0.2, // 12
};

#endif // _WHATEVER_SCENE

#endif // SRC_SCENE_H_
