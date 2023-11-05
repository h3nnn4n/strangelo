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

#include "scene.h"

// clang-format off
vec3 camera_pos         = { -1.0 ,   2.75 ,  1.75 };
vec3 camera_orientation = { -6.0 , 270.0  , 45.0  };
// clang-format on

vec4  positions[n_spheres];
float radius[n_spheres];
vec4  albedo[n_spheres];
vec4  emission[n_spheres];
float roughness[n_spheres];
int   material_type[n_spheres];

vec4 triangle_v0[n_triangles];
vec4 triangle_v1[n_triangles];
vec4 triangle_v2[n_triangles];
vec4 triangle_albedo[n_triangles];
vec4 triangle_emission[n_triangles];

void init_scene() {
    // clang-format off
    sphere_t data[] = {
      //  position           ,   radius , albedo             , emission           , roughness , type
      { { 2.0 , 1.3 , -10.0} ,   1.5    , {1.0 , 1.0 , 1.0 } , {1.0 , 1.0 , 1.0 } , 0.0       , DIFFUSE    } ,
      { { 0.0 , 1.0 , -10.0} ,   1.0    , {1.0 , 0.2 , 0.3 } , {0.0 , 0.0 , 0.0 } , 0.0       , DIFFUSE    } ,
      { { 4.0 , 1.0 ,  -9.0} ,   1.0    , {0.3 , 0.9 , 0.1 } , {0.0 , 0.0 , 0.0 } , 0.0       , DIFFUSE    } ,
      { {-4.0 , 1.0 , -10.0} ,   1.0    , {0.0 , 0.2 , 0.9 } , {0.0 , 0.0 , 0.0 } , 0.0       , DIFFUSE    } ,
      { { 0.0 , 3.0 , -10.0} ,   1.0    , {0.7 , 0.6 , 0.2 } , {0.0 , 0.0 , 0.0 } , 0.1       , METAL      } ,
      { { 4.0 , 3.0 , -10.0} ,   1.0    , {0.3 , 0.8 , 0.2 } , {0.0 , 0.0 , 0.0 } , 0.3       , METAL      } ,
      { {-4.0 , 3.0 , -10.0} ,   1.0    , {0.3 , 0.1 , 0.8 } , {0.0 , 0.0 , 0.0 } , 0.5       , METAL      } ,
      { {-2.0 , 2.0 , -15.0} ,   3.0    , {0.7 , 0.6 , 0.3 } , {0.0 , 0.0 , 0.0 } , 0.25      , METAL      } ,
      { { 0.5 , 1.0 ,  -7.0} ,   1.0    , {1.0 , 1.0 , 1.0 } , {0.0 , 0.0 , 0.0 } , 1.1       , DIELECTRIC } ,
      { { 2.5 , 1.0 ,  -7.0} ,   1.0    , {1.0 , 1.0 , 1.0 } , {0.0 , 0.0 , 0.0 } , 1.5       , DIELECTRIC } ,
    };

    float t_s = 1.0;

    triangle_t triangles[] = {
      { { t_s , 0.0  , 0.0 } , { 0.0 ,  t_s , 0.0 } , { 0.0 , 0.0 , t_s } , { 0.2 , 0.8 , 0.3 } } ,
      { { t_s , 0.0  , 0.0 } , { 0.0 ,  0.0 ,-t_s } , { 0.0 , t_s , 0.0 } , { 0.2 , 0.8 , 0.3 } } ,
      { { 0.0 , t_s  , 0.0 } , {-t_s ,  0.0 , 0.0 } , { 0.0 , 0.0 , t_s } , { 0.2 , 0.8 , 0.3 } } ,
      { { 0.0 , 0.0  ,-t_s } , {-t_s ,  0.0 , 0.0 } , { 0.0 , t_s , 0.0 } , { 0.2 , 0.8 , 0.3 } } ,
      { { t_s , 0.0  , 0.0 } , { 0.0 ,  0.0 , t_s } , { 0.0 ,-t_s , 0.0 } , { 0.2 , 0.8 , 0.3 } } ,
      { { t_s , 0.0  , 0.0 } , { 0.0 , -t_s , 0.0 } , { 0.0 , 0.0 ,-t_s } , { 0.2 , 0.8 , 0.3 } } ,
      { { 0.0 ,-t_s  , 0.0 } , { 0.0 ,  0.0 , t_s } , {-t_s , 0.0 , 0.0 } , { 0.2 , 0.8 , 0.3 } } ,
      { { 0.0 , 0.0  ,-t_s } , { 0.0 , -t_s , 0.0 } , {-t_s , 0.0 , 0.0 } , { 0.2 , 0.8 , 0.3 } } ,
    };

    // clang-format on

    for (int i = 0; i < n_spheres; i++) {
        for (int j = 0; j < 3; j++) {
            positions[i][j] = data[i].position[j];
            albedo[i][j]    = data[i].albedo[j];
            emission[i][j]  = data[i].emission[j];
        }

        positions[i][3] = 1.0;
        albedo[i][3]    = 1.0;
        emission[i][3]  = 1.0;

        radius[i]        = data[i].radius;
        roughness[i]     = data[i].roughness;
        material_type[i] = data[i].material_type;
    }

    for (int i = 0; i < n_triangles; i++) {
        for (int j = 0; j < 3; j++) {
            triangle_v0[i][j]       = triangles[i].v0[j];
            triangle_v1[i][j]       = triangles[i].v1[j];
            triangle_v2[i][j]       = triangles[i].v2[j];
            triangle_albedo[i][j]   = triangles[i].albedo[j];
            triangle_emission[i][j] = triangles[i].emission[j];
        }

        triangle_v0[i][0] += 2.0;
        triangle_v1[i][0] += 2.0;
        triangle_v2[i][0] += 2.0;

        triangle_v0[i][1] += 4.3;
        triangle_v1[i][1] += 4.3;
        triangle_v2[i][1] += 4.3;

        triangle_v0[i][2] -= 10.0;
        triangle_v1[i][2] -= 10.0;
        triangle_v2[i][2] -= 10.0;

        triangle_v0[i][3]       = 1.0;
        triangle_v1[i][3]       = 1.0;
        triangle_v2[i][3]       = 1.0;
        triangle_albedo[i][3]   = 1.0;
        triangle_emission[i][3] = 1.0;
    }
}

#ifdef _RANDOM_SCENE
void init_scene() {
    for (int i = 0; i < n_spheres; i++) {
        for (int j = 0; j < 3; j++) {
            albedo[i][j] = fabs(ldexp(pcg32_random(), -32));
        }

        positions[i][0] = ldexp(pcg32_random(), -32) * 15.0;
        positions[i][1] = ldexp(pcg32_random(), -32) - 1.0;
        positions[i][2] = ldexp(pcg32_random(), -32) * 10.0 - 15.0;
        printf("%f %f %f\n", positions[i][0], positions[i][1], positions[i][2]);

        positions[i][3] = 1.0;
        albedo[i][3]    = 1.0;

        radius[i]        = fabs(ldexp(pcg32_random(), -32)) + 1.0;
        roughness[i]     = fabs(ldexp(pcg32_random(), -32)) + 1.0;
        material_type[i] = pcg32_boundedrand(5);
        if (material_type[i] == 4)
            material_type[i]++;
    }
}
#endif // _RANDOM_SCENE

// clang-format off
// clang-format on
