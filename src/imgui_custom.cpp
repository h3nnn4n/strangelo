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

#include "imgui.h"
#include "imgui_internal.h"

#include "imgui_custom.hpp"

// https://github.com/ocornut/imgui/issues/1537#issuecomment-355562097
void ToggleButton(const char *str_id, const char *label, bool *value) {
    ImVec2      p         = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width  = height * 1.55f;
    float radius = height * 0.50f;

    if (ImGui::InvisibleButton(str_id, ImVec2(width, height))) {
        *value = !*value;
    }

    auto  white = IM_COL32(255, 255, 255, 255);
    ImU32 col_bg;
    if (ImGui::IsItemHovered()) {
        col_bg = *value ? IM_COL32(145 + 20, 211, 68 + 20, 255) : IM_COL32(218 - 20, 218 - 20, 218 - 20, 255);
    } else {
        col_bg = *value ? IM_COL32(145, 211, 68, 255) : IM_COL32(218, 218, 218, 255);
    }

    auto rect_position   = ImVec2(p.x + width, p.y + height);
    auto circle_position = ImVec2(*value ? (p.x + width - radius) : (p.x + radius), p.y + radius);

    draw_list->AddRectFilled(p, rect_position, col_bg, height * 0.5f);
    draw_list->AddCircleFilled(circle_position, radius - 1.5f, white);

    const ImVec2 text_size = ImGui::CalcTextSize(label);
    ImVec2       position  = ImGui::GetItemRectMin();
    position.x += rect_position.x * 0.6f;
    position.y += text_size.y * 0.25f;

    // draw the label.
    draw_list->AddText(position, white, label);
}
