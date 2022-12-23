//-----------------------------------------------------------------------------
// File : asdxLight.h
// Desc : Light Data.
// Copyright(c) Project Asura.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxMath.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// DirectionalLight class
///////////////////////////////////////////////////////////////////////////////
class DirectionalLight
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    DirectionalLight() = default;

    DirectionalLight(const Vector3& dir, const Vector3& color, float intensity = 1.0f)
    : m_Direction   (dir)
    , m_Color       (color)
    , m_Intensity   (intensity)
    { /* DO_NOTHING */ }

    void SetDirection(const Vector3& value)
    { m_Direction = value; }

    void SetColor(const Vector3& value)
    { m_Color = value; }

    void SetIntensity(float value)
    { m_Intensity = value; }

    const Vector3& GetDirection() const
    { return m_Direction; }

    const Vector3 GetLightVector() const
    { return -m_Direction; }

    const Vector3& GetColor() const
    { return m_Color; }

    float GetIntensity() const
    { return m_Intensity; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    Vector3 m_Direction = Vector3(0.0f, -1.0f, 0.0f);   // 照射方向.
    Vector3 m_Color     = Vector3(1.0f, 1.0f, 1.0f);    // ライトカラー.
    float   m_Intensity = 1.0f;                         // 強度.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// PointLight class
///////////////////////////////////////////////////////////////////////////////
class PointLight
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    PointLight() = default;

    PointLight(const Vector3& center, float radius, const Vector3& color,float intensity = 1.0f)
    : m_Center      (center)
    , m_Radius      (radius)
    , m_Color       (color)
    , m_Intensity   (intensity)
    { /* DO_NOTHING */ }

    void SetCenter(const Vector3& value)
    { m_Center = value; }

    void SetRadius(float value)
    { m_Radius = value; }

    void SetColor(const Vector3& value)
    { m_Color = value; }

    void SetIntensity(float value)
    { m_Intensity = value; }

    const Vector3& GetCenter() const
    { return m_Center; }

    float GetRadius() const
    { return m_Radius; }

    const Vector3& GetColor() const
    { return m_Color; }

    float GetIntensity() const
    { return m_Intensity; }

    // 点の包含.
    bool Contains(const Vector3& point) const
    {
        auto d2 = Vector3::DistanceSq(point, m_Center);
        auto r2 = m_Radius * m_Radius;
        return (d2 <= r2);
    }

    // BoundingBoxの包含.
    bool Contains(const Vector3& mini, const Vector3& maxi) const
    {
        auto p  = Vector3::Max(mini, Vector3::Min(m_Center, maxi));
        auto d2 = Vector3::DistanceSq(p, m_Center);
        auto r2 = m_Radius * m_Radius;
        return (d2 <= r2);
    }

    // BoundingSphereの包含.
    bool Contains(const Vector3& center, float radius) const
    {
        auto d2 = Vector3::DistanceSq(center, m_Center);
        auto r2 = (m_Radius + radius) * (m_Radius + radius);
        return (d2 <= r2);
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    Vector3     m_Center    = Vector3(0.0f, 0.0f, 0.0f);    // 中心座標.
    float       m_Radius    = 1.0f;                         // 半径.
    Vector3     m_Color     = Vector3(1.0f, 1.0f, 1.0f);    // ライトカラー.
    float       m_Intensity = 1.0f;                         // 強度.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// SpotLight class
///////////////////////////////////////////////////////////////////////////////
class SpotLight
{
    //=========================================================================
    // list of friend classes and methods.
    //=========================================================================
    /* NOTHING */

public:
    //=========================================================================
    // public variables.
    //=========================================================================
    /* NOTHING */

    //=========================================================================
    // public methods.
    //=========================================================================
    SpotLight() = default;

    SpotLight(
        const Vector3&  position,
        const Vector3&  forward,
        float           radius,
        float           innerAngle,
        float           outerAngle,
        const Vector3&  color,
        float           intensity = 1.0f)
    : m_Position    (position)
    , m_Forward     (forward)
    , m_Radius      (radius)
    , m_InnerAngle  (innerAngle)
    , m_OuterAngle  (outerAngle)
    , m_Color       (color)
    , m_Intensity   (intensity)
    { /* DO_NOTHING */ }

    void SetPosition(const Vector3& value)
    { m_Position = value; }

    void SetForward(const Vector3& value)
    { m_Forward = value; }

    void SetRadius(float value)
    { m_Radius = value; }

    void SetInnerAngle(float value)
    { m_InnerAngle = value; }

    void SetOuterAngle(float value)
    { m_OuterAngle = value; }

    void SetColor(const Vector3& value)
    { m_Color = value; }

    void SetIntensity(float value)
    { m_Intensity = value; }

    const Vector3& GetPosition() const
    { return m_Position; }

    const Vector3& GetForward() const
    { return m_Forward; }

    Vector3 GetLightVector() const
    { return -m_Forward; }

    float GetRadius() const
    { return m_Radius; }

    float GetInnerAngle() const
    { return m_InnerAngle; }

    float GetOuterAngle() const
    { return m_OuterAngle; }

    const Vector3& GetColor() const
    { return m_Color; }

    float GetIntensity() const
    { return m_Intensity; }

    // 点の包含.
    bool Contains(const Vector3& point) const
    {
        auto v = point - m_Position;
        auto lenSq = Vector3::Dot(v, v);
        auto v1Len = Vector3::Dot(v, m_Forward);
        auto distClosestPoint = cos(m_OuterAngle) * sqrt(lenSq - v1Len * v1Len) - v1Len * sin(m_OuterAngle);

        auto angleCull = distClosestPoint > 0.0f;
        auto frontCull = v1Len > m_Radius;
        auto backCull  = v1Len < 0.0f;
        return !(angleCull || frontCull || backCull);
    }

    // BoundingBoxの包含.
    bool Contains(const Vector3& mini, const Vector3& maxi) const
    {
        auto center = (maxi + mini) * 0.5f;
        auto extent = (maxi - mini) * 0.5f;
        auto r = Vector3::Dot(extent, extent);
        auto v = center - m_Position;
        auto lenSq = Vector3::Dot(v, v);
        auto v1Len = Vector3::Dot(v, m_Forward);
        auto distClosestPoint = cos(m_OuterAngle) * sqrt(lenSq - v1Len * v1Len) - v1Len * sin(m_OuterAngle);
        auto angleCull = distClosestPoint > r;
        auto frontCull = v1Len > (r + m_Radius);
        auto backCull  = v1Len < -r;
        return !(angleCull || frontCull || backCull);
    }

    // BoundingSphereの包含.
    bool Contains(const Vector3& center, float radius) const
    {
        auto v = center - m_Position;
        auto lenSq = Vector3::Dot(v, v);
        auto v1Len = Vector3::Dot(v, m_Forward);
        auto distClosestPoint = cos(m_OuterAngle) * sqrt(lenSq - v1Len * v1Len) - v1Len * sin(m_OuterAngle);

        auto angleCull = distClosestPoint > radius;
        auto frontCull = v1Len > (radius + m_Radius);
        auto backCull  = v1Len < -radius;
        return !(angleCull || frontCull || backCull);
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    Vector3     m_Position      = Vector3(0.0f, 0.0f, 0.0f);    // 位置座標.
    float       m_Radius        = 1.0f;                         // 半径.
    Vector3     m_Forward       = Vector3(0.0f, -1.0f, 0.0f);   // 照射方向.
    float       m_InnerAngle    = ToRadian(45.0f);              // 内角(rad)
    float       m_OuterAngle    = ToRadian(60.0f);;             // 外角(rad)
    Vector3     m_Color         = Vector3(1.0f, 1.0f, 1.0f);    // ライトカラー.
    float       m_Intensity     = 1.0f;                         // 強度.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx
