//-----------------------------------------------------------------------------
// File : asdxImGuiExtension.h
// Desc : ImGui Extension.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

#ifdef ASDX_ENABLE_IMGUI

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxMath.h>
#include <imgui.h>


inline ImVec2 ToImVec2(const asdx::Vector2& value)
{ return ImVec2(value.x, value.y); }

inline ImVec4 ToImVec4(const asdx::Vector4& value)
{ return ImVec4(value.x, value.y, value.z, value.w); }

inline asdx::Vector2 FromImVec2(const ImVec2& value)
{ return asdx::Vector2(value.x, value.y); }

inline asdx::Vector4 FromImVec4(const ImVec4& value)
{ return asdx::Vector4(value.x, value.y, value.z, value.w); }

inline ImVec2 operator + (const ImVec2& lhs, const ImVec2& rhs)
{ return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }

inline ImVec2 operator - (const ImVec2& lhs, const ImVec2& rhs)
{ return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

//-----------------------------------------------------------------------------
//      クリック可能なテキストを描画します.
//-----------------------------------------------------------------------------
bool ImGuiClickableText(
    const char*     text,
    const ImVec2&   size_arg    = ImVec2(0, 0),
    const ImVec4&   hoverColor  = ImVec4(1.0f, 0.5f, 0.0f, 1.0f)
);

//-----------------------------------------------------------------------------
//      キューブマップを描画します.
//-----------------------------------------------------------------------------
void ImGuiImageCube(
    ImTextureID     texture_id,
    const ImVec2&   size,
    bool            cross       = false,
    const ImVec4&   tint_col    = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
    const ImVec4&   border_col  = ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
);

#endif// ASDX_ENABLE_IMGUI