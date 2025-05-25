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

#ifndef SRC_GUI_H_
#define SRC_GUI_H_

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimplot.h>

extern GLFWwindow           *window;
extern struct ImGuiContext  *ctx;
extern struct ImGuiIO       *io;
extern struct ImPlotContext *plot_ctx;

int  ImGui_ImplOpenGL3_Init(const char *glsl_version);
int  ImGui_ImplGlfw_InitForOpenGL(GLFWwindow *window, bool install_callbacks);
void ImGui_ImplGlfw_Shutdown();
void ImGui_ImplOpenGL3_Shutdown();

void ImGui_ImplOpenGL3_RenderDrawData();
void ImGui_ImplOpenGL3_NewFrame();
void ImGui_ImplGlfw_NewFrame();

void gui_init();
void gui_terminate();
void gui_render();
void gui_new_frame();

void gui_update_fps();
void gui_update_clifford();
void gui_update_scaling();

#endif // SRC_GUI_H_
