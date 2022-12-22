//-----------------------------------------------------------------------------
// File : asdxHelper2D.h
// Desc : 2D Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// DIR2D_TYPE enum
///////////////////////////////////////////////////////////////////////////////
enum DIR2D_TYPE
{
    DIR2D_NONE,   // 方向無し.
    DIR2D_LEFT,   // 左.
    DIR2D_RIGHT,  // 右.
    DIR2D_UP,     // 上.
    DIR2D_DOWN,   // 下.
};

///////////////////////////////////////////////////////////////////////////////
// Int2 structure
///////////////////////////////////////////////////////////////////////////////
struct Int2
{
    int x = 0;
    int y = 0;

    Int2() = default;

    Int2(int nx, int ny)
    : x(nx), y(ny)
    { /* DO_NOTHING */ }

    Int2 operator + (const Int2& value) const
    { return Int2(x + value.x, y + value.y); }

    Int2 operator - (const Int2& value) const
    { return Int2(x - value.x, y - value.y); }

    Int2 operator * (const Int2& value) const
    { return Int2(x * value.x, y * value.y); }

    Int2 operator / (const Int2& value) const
    { return Int2(x / value.x, y / value.y); }

    Int2 operator + (int value) const
    { return Int2(x + value, y + value); }

    Int2 operator - (int value) const
    { return Int2(x - value, y - value); }

    Int2 operator * (int value) const
    { return Int2(x * value, y * value); }

    Int2 operator / (int value) const
    { return Int2(x / value, y / value); }

    Int2& operator += (const Int2& value)
    {
        x += value.x;
        y += value.y;
        return *this;
    }

    Int2& operator -= (const Int2& value)
    {
        x -= value.x;
        y -= value.y;
        return *this;
    }

    Int2& operator *= (const Int2& value)
    {
        x *= value.x;
        y *= value.y;
        return *this;
    }

    Int2& operator /= (const Int2& value)
    {
        x /= value.x;
        y /= value.y;
        return *this;
    }

    Int2& operator += (int value)
    {
        x += value;
        y += value;
        return *this;
    }

    Int2& operator -= (int value)
    {
        x -= value;
        y -= value;
        return *this;
    }

    Int2& operator *= (int value)
    {
        x *= value;
        y *= value;
        return *this;
    }

    Int2& operator /= (int value)
    {
        x /= value;
        y /= value;
        return *this;
    }

    static Int2 Max(const Int2& lhs, const Int2& rhs)
    {
        return Int2(
            (lhs.x > rhs.x) ? lhs.x : rhs.x,
            (lhs.y > rhs.y) ? lhs.y : rhs.y);
    }

    static Int2 Min(const Int2& lhs, const Int2& rhs)
    {
        return Int2(
            (lhs.x < rhs.x) ? lhs.x : rhs.x,
            (lhs.y < rhs.y) ? lhs.y : rhs.y);
    }

    static Int2 Clamp(const Int2& value, const Int2& mini, const Int2& maxi)
    { return Max(mini, Min(maxi, value)); }

    static Int2 GetDir(DIR2D_TYPE type)
    {
        static const Int2 kDirection[] = {
            Int2( 0,  0),   // DIR2D_NONE
            Int2(-1,  0),   // DIR2D_LEFT
            Int2( 1,  0),   // DIR2D_RIGHT
            Int2( 0,  1),   // DIR2D_UP
            Int2( 0, -1)    // DIR2D_DOWN
        };
        return kDirection[type];
    }
};

///////////////////////////////////////////////////////////////////////////////
// Box2 structure
//////////////////////////////////////////////////////////////////////////////
struct Box2
{
    Int2 pos;   // 左上原点.
    Int2 size;  // サイズ.

    Box2() = default;

    Box2(int x, int y, int w, int h)
    : pos (x, y)
    , size(w, h)
    { /* DO_NOTHING */ }

    Box2(const Int2& p, const Int2& s)
    : pos(p)
    , size(s)
    { /* DO_NOTHING */ }

    static Box2 Merge(const Box2& lhs, const Box2& rhs)
    {
        return Box2(
            Int2::Min(lhs.pos,  rhs.pos),
            Int2::Max(lhs.size, rhs.size));
    }

    static bool Contains(const Box2& lhs, const Box2& rhs)
    {
        return lhs.pos.x < (rhs.pos.x + rhs.size.x)
            && rhs.pos.x < (lhs.pos.x + lhs.size.x)
            && lhs.pos.y < (rhs.pos.y + rhs.size.y)
            && rhs.pos.y < (lhs.pos.y + lhs.size.y);
    }

    static bool Contains(const Int2& point, const Box2& box)
    {
        return (box.pos.x <= point.x && point.x <= (box.pos.x + box.size.x))
            && (box.pos.y <= point.y && point.y <= (box.pos.y + box.size.y));
    }
};

} // namespace asdx

