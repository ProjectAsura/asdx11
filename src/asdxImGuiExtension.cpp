//-----------------------------------------------------------------------------
// File : asdxImGuiExtension.cpp
// Desc : ImGui Extension.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxImGuiExtension.h>

#ifdef ASDX_ENABLE_IMGUI
#include <imgui_internal.h>


//-----------------------------------------------------------------------------
//      クリック可能なテキストを描画します.
//-----------------------------------------------------------------------------
bool ImGuiClickableText
(
    const char*   text,
    const ImVec2& size_arg,
    const ImVec4& hoverColor
)
{
    auto window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    auto& g = *GImGui;
    const auto& style = g.Style;
    const auto id = window->GetID(text);
    const auto label_size = ImGui::CalcTextSize(text, nullptr, true);

    auto pos = window->DC.CursorPos;
    auto size = ImGui::CalcItemSize(
        size_arg,
        label_size.x + style.FramePadding.x * 2.0f,
        label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    auto pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

    if (hovered)
    { ImGui::PushStyleColor(ImGuiCol_Text, hoverColor); }

    ImGui::RenderTextClipped(
        bb.Min + style.FramePadding,
        bb.Max - style.FramePadding,
        text, nullptr, &label_size, style.ButtonTextAlign, &bb);
    
    if (hovered)
    { ImGui::PopStyleColor(); }

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return pressed;
}

#endif // ASDX_ENABLE_IMGUI