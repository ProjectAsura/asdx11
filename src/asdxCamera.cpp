//-----------------------------------------------------------------------------
// File : asdxCamera.cpp
// Desc : Camera Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxCamera.h>
#include <asdxLogger.h>


namespace /* anonymos */ {

//-----------------------------------------------------------------------------
//      角度を求めます.
//-----------------------------------------------------------------------------
static inline 
float GetAngle( float sin, float cos )
{
    float angle = asinf( sin );

    if ( cos < FLT_EPSILON )
    { angle = asdx::F_PI - angle; }

    return angle;
}

//-----------------------------------------------------------------------------
//      指定された位置とターゲットから角度に変換します.
//-----------------------------------------------------------------------------
static inline 
void ToAngle
( 
    const asdx::Vector3& dir,
    asdx::Vector2&       angle
)
{
    asdx::Vector3 v1;
    v1.x = -dir.x;
    v1.y = 0.0f;
    v1.z = -dir.z;

    asdx::Vector3 v2 = v1;
    v2.Normalize();

    angle.x = GetAngle( v2.x, v2.z );

    auto dist = v1.Length();
    v1.x = dist;
    v1.y = -dir.y;
    v1.z = 0.0f;

    v2 = v1;
    v2.Normalize();

    angle.y = GetAngle( v2.y, v2.x );
}

//-----------------------------------------------------------------------------
//      指定された角度からベクトルを求めます.
//-----------------------------------------------------------------------------
static inline
void ToVector
(
    const asdx::Vector2& angle,
    asdx::Vector3* pLookDir,
    asdx::Vector3* pUpward
)
{
    auto sinH = sinf( angle.x );
    auto cosH = cosf( angle.x );

    auto sinV = sinf( angle.y );
    auto cosV = cosf( angle.y );

   if ( pLookDir )
   {
       (*pLookDir).x = -cosV * sinH;
       (*pLookDir).y = -sinV;
       (*pLookDir).z = -cosV * cosH;
   }

   if  ( pUpward )
   {
       (*pUpward).x = -sinV * sinH;
       (*pUpward).y = cosV;
       (*pUpward).z = -sinV * cosH;
   }
}

} // namespace /* anonymos */ 


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Camera class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Camera::Camera()
: m_Param()
, m_Preset()
, m_View( Matrix::CreateIdentity() )
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Camera::~Camera()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      位置を設定します.
//-----------------------------------------------------------------------------
void Camera::SetPosition( const asdx::Vector3& position )
{
    m_Param.Position = position;

    ClampDist();
    ComputeAngle();
}

//-----------------------------------------------------------------------------
//      注視点を設定します.
//-----------------------------------------------------------------------------
void Camera::SetTarget( const asdx::Vector3& target )
{
    m_Param.Target = target;

    ClampDist();
    ComputeAngle();
}

//-----------------------------------------------------------------------------
//      上向きベクトルを設定します.
//-----------------------------------------------------------------------------
void Camera::SetUpward( const asdx::Vector3& upward )
{ m_Param.Upward = upward; }

//-----------------------------------------------------------------------------
//      ツイスト角を設定します.
//-----------------------------------------------------------------------------
void Camera::SetTwist( const float twist )
{ m_Param.Twist = twist; }

//-----------------------------------------------------------------------------
//      可動距離範囲を設定します.
//-----------------------------------------------------------------------------
void Camera::SetRange( const float minDist, const float maxDist )
{
    m_Param.MinDist = minDist;
    m_Param.MaxDist = maxDist;

    ClampDist();
    ComputeAngle();
}

//-----------------------------------------------------------------------------
//      パラメータを保存します.
//-----------------------------------------------------------------------------
void Camera::Preset()
{ m_Preset = m_Param; }

//-----------------------------------------------------------------------------
//      パラメータをリセットします.
//-----------------------------------------------------------------------------
void Camera::Reset()
{
    m_Param = m_Preset;

    ClampDist();
    ComputeAngle();
}

//-----------------------------------------------------------------------------
//      ビュー行列を取得します.
//-----------------------------------------------------------------------------
const asdx::Matrix& Camera::GetView() const
{ return m_View; }

//-----------------------------------------------------------------------------
//      位置を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector3& Camera::GetPosition() const
{ return m_Param.Position; }

//-----------------------------------------------------------------------------
//      注視点を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector3& Camera::GetTarget() const
{ return m_Param.Target; }

//-----------------------------------------------------------------------------
//      上向きベクトルを取得します.
//-----------------------------------------------------------------------------
const asdx::Vector3& Camera::GetUpward() const
{ return m_Param.Upward; }

//-----------------------------------------------------------------------------
//      ツイスト角を取得します.
//-----------------------------------------------------------------------------
float Camera::GetTwist() const
{ return m_Param.Twist; }

//-----------------------------------------------------------------------------
//      可動最小距離を取得します.
//-----------------------------------------------------------------------------
float Camera::GetMinDist() const
{ return m_Param.MinDist; }

//-----------------------------------------------------------------------------
//      可動最大距離を取得します.
//-----------------------------------------------------------------------------
float Camera::GetMaxDist() const
{ return m_Param.MaxDist; }

//-----------------------------------------------------------------------------
//      角度を計算します.
//-----------------------------------------------------------------------------
void Camera::ComputeAngle()
{
    Vector3 dir = m_Param.Position - m_Param.Target;
    dir.Normalize();

    ToAngle( dir, m_Param.Rotate );
    ToAngle( dir, m_Param.PanTilt );

    Vector3 lookDir;
    Vector3 upward;
    ToVector( m_Param.Rotate, &lookDir, &upward );

    m_Param.Upward = upward;

    ComputePosition();
    ComputeTarget();
}

//-----------------------------------------------------------------------------
//      位置座標を計算します.
//-----------------------------------------------------------------------------
void Camera::ComputePosition()
{
    // ターゲットまでの距離を算出.
    float dist = Vector3::Distance( m_Param.Position, m_Param.Target );

    Vector3 lookDir;
    Vector3 upward;

    ToVector( m_Param.Rotate, &lookDir, &upward );

    m_Param.Position.x = m_Param.Target.x + dist * lookDir.x;
    m_Param.Position.y = m_Param.Target.y + dist * lookDir.y;
    m_Param.Position.z = m_Param.Target.z + dist * lookDir.z;

    m_Param.Upward = upward;
}

//-----------------------------------------------------------------------------
//      注視点を計算します.
//-----------------------------------------------------------------------------
void Camera::ComputeTarget()
{
    // ターゲットとの距離を算出.
    float dist = Vector3::Distance( m_Param.Target, m_Param.Position );

    Vector3 lookDir;
    Vector3 upward;

    ToVector( m_Param.PanTilt, &lookDir, &upward );

    m_Param.Target.x = m_Param.Position.x - dist * lookDir.x;
    m_Param.Target.y = m_Param.Position.y - dist * lookDir.y;
    m_Param.Target.z = m_Param.Position.z - dist * lookDir.z;

    m_Param.Upward = upward;
}

//-----------------------------------------------------------------------------
//      可動距離範囲内に制限します.
//-----------------------------------------------------------------------------
void Camera::ClampDist()
{
    // 距離を算出.
    float dist = Vector3::Distance( m_Param.Position, m_Param.Target );

    // 方向ベクトル
    asdx::Vector3 dir = m_Param.Position - m_Param.Target;

    // 最大距離を超えないように制限.
    if ( dist > m_Param.MaxDist )
    {
        // ゼロ除算対策.
        if ( dir.LengthSq() > FLT_EPSILON )
        { dir.Normalize(); }

        m_Param.Position = m_Param.Target + dir * m_Param.MaxDist;
    }

    // 最小距離を下回らないように制限.
    if ( dist < m_Param.MinDist )
    {
        // ゼロ除算対策.
        if ( dir.LengthSq() > FLT_EPSILON )
        { dir.Normalize(); }

        m_Param.Position = m_Param.Target + dir * m_Param.MinDist;
    }
}

//-----------------------------------------------------------------------------
//      カメラ基底ベクトルのX軸を取得します.
//-----------------------------------------------------------------------------
asdx::Vector3 Camera::GetAxisX() const
{ return asdx::Vector3(m_View._11, m_View._21, m_View._31); }

//-----------------------------------------------------------------------------
//      カメラ基底ベクトルのY軸を取得します.
//-----------------------------------------------------------------------------
asdx::Vector3 Camera::GetAxisY() const
{ return asdx::Vector3(m_View._12, m_View._22, m_View._32); }

//-----------------------------------------------------------------------------
//      カメラ基底ベクトルのZ軸を取得します.
//-----------------------------------------------------------------------------
asdx::Vector3 Camera::GetAxisZ() const
{ return asdx::Vector3(m_View._13, m_View._23, m_View._33); }

//-----------------------------------------------------------------------------
//      ビュー行列を更新します.
//-----------------------------------------------------------------------------
void Camera::Update()
{
    asdx::Vector3 upward = m_Param.Upward;

    // ツイスト角がゼロでない場合.
    if ( fabs( m_Param.Twist ) > FLT_EPSILON )
    {
        // 視線ベクトルを作成.
        asdx::Vector3 dir = m_Param.Target - m_Param.Position;
        if ( dir.LengthSq() > FLT_EPSILON )
        { dir.Normalize(); }

        // 視線ベクトル軸とした回転行列を作成.
        asdx::Matrix rotate = Matrix::CreateFromAxisAngle( dir, m_Param.Twist );

        // アップベクトルを回転.
        upward = Vector3::Transform( upward, rotate );
    }

    // ビュー行列を更新.
    m_View = Matrix::CreateLookAt( m_Param.Position, m_Param.Target, upward );
}

//-----------------------------------------------------------------------------
//      カメライベントを基にビュー行列を更新します.
//-----------------------------------------------------------------------------
void Camera::UpdateByEvent( const CameraEvent& camEvent )
{
    auto isProcess = false;

    // 回転処理.
    if ( camEvent.Flags & CameraEvent::EVENT_ROTATE )
    {
        // 回転角を加算.
        m_Param.Rotate += camEvent.Rotate;

        // 90度制限.
        if ( m_Param.Rotate.y > 1.564f )
        { m_Param.Rotate.y = 1.564f; }
        if ( m_Param.Rotate.y < -1.564f )
        { m_Param.Rotate.y = -1.564f; }

        ComputePosition();

        Vector3 dir = m_Param.Position - m_Param.Target;
        dir.Normalize();
        ToAngle( dir, m_Param.Rotate );
        ToAngle( dir, m_Param.PanTilt );

        isProcess = true;
    }

    // ドリー処理.
    if ( camEvent.Flags & CameraEvent::EVENT_DOLLY )
    {
        // 視線ベクトルを作成.
        asdx::Vector3 dir = m_Param.Position - m_Param.Target;
        float dist = Vector3::Distance( m_Param.Position, m_Param.Target );

        // 正規化.
        if ( dist > FLT_EPSILON )
        {
            float invDist = 1.0f / dist;
            dir.x *= invDist;
            dir.y *= invDist;
            dir.z *= invDist;
        }

        // ドリー量を加算.
        dist += camEvent.Dolly;

        // 可動距離範囲内に制限.
        if ( m_Param.MinDist > dist )
        { dist = m_Param.MinDist; }
        if ( m_Param.MaxDist < dist )
        { dist = m_Param.MaxDist; }

        // 位置ベクトルを更新.
        m_Param.Position = m_Param.Target + ( dir * dist );

        isProcess = true;
    }

    // トラック処理.
    if ( camEvent.Flags & CameraEvent::EVENT_TRUCK )
    {
        m_Param.Position += camEvent.Truck;
        m_Param.Target   += camEvent.Truck;

        isProcess = true;
    }

    // パン・チルト処理.
    if ( camEvent.Flags & CameraEvent::EVENT_PANTILT )
    {
        // パン・チルト角を加算.
        m_Param.PanTilt += camEvent.PanTilt;

        // 縦90度制限.
        if ( m_Param.PanTilt.y > 1.564f )
        { m_Param.PanTilt.y = 1.564f; }
        if ( m_Param.PanTilt.y < -1.564f )
        { m_Param.PanTilt.y = -1.564f; }

        ComputeTarget();

        Vector3 dir = m_Param.Position - m_Param.Target;
        dir.Normalize();
        ToAngle( dir, m_Param.Rotate );
        ToAngle( dir, m_Param.PanTilt );

        isProcess = true;
    }

    // ツイスト処理.
    if ( camEvent.Flags & CameraEvent::EVENT_TWIST )
    { 
        m_Param.Twist += camEvent.Twist;
        isProcess = true;
    }

    // リセット処理.
    if ( camEvent.Flags & CameraEvent::EVENT_RESET )
    {
        Reset();
        isProcess = true;
    }

    // ビュー行列を更新.
    if ( isProcess )
    {
        Update();
    }
}


///////////////////////////////////////////////////////////////////////////////
// ViewerCamera class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ViewerCamera::ViewerCamera()
: Camera        ()
, m_MouseLeft   ()
, m_MouseRight  ()
, m_MouseMiddle ()
, m_Type        (CONTROL_TYPE_TARGET)
{
    ResetGain();
    m_ProjParam.AspectRatio = 1.0f;
    m_ProjParam.FieldOfView = F_PIDIV4;
    m_ProjPreset = m_ProjParam;
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ViewerCamera::~ViewerCamera()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      初期設定を行います.
//-----------------------------------------------------------------------------
void ViewerCamera::Init
(
    asdx::Vector3 cameraPos,
    asdx::Vector3 cameraAim,
    asdx::Vector3 cameraUp,
    float         fieldOfView,
    float         aspectRatio,
    float         nearClip,
    float         farClip
)
{
    SetPosition( cameraPos );
    SetTarget  ( cameraAim );
    SetUpward  ( cameraUp );
    SetRange   ( nearClip, farClip );

    Preset();
    Update();
    m_MouseLeft  .Reset();
    m_MouseRight .Reset();
    m_MouseMiddle.Reset();

    m_ProjParam.FieldOfView = fieldOfView;
    m_ProjParam.AspectRatio = aspectRatio;
    m_ProjPreset            = m_ProjParam;

    m_Proj = Matrix::CreatePerspectiveFieldOfView(
        m_ProjParam.FieldOfView,
        m_ProjParam.AspectRatio,
        nearClip,
        farClip);
}

//-----------------------------------------------------------------------------
//      マウスの処理です.
//-----------------------------------------------------------------------------
void ViewerCamera::OnMouse
(
    int  x,
    int  y,
    int  wheelDelta,
    bool isLeftButtonDown,
    bool isRightButtonDown,
    bool isMiddleButtonDown,
    bool isSideButton1Down,
    bool isSideButton2Down
)
{
    m_MouseLeft  .Update( x, y, isLeftButtonDown );
    m_MouseRight .Update( x, y, isRightButtonDown );
    m_MouseMiddle.Update( x, y, isMiddleButtonDown );

    CameraEvent e = MakeEventFromMouse( wheelDelta );
    UpdateByEvent( e );
}

//-----------------------------------------------------------------------------
//      キーの処理です.
//-----------------------------------------------------------------------------
void ViewerCamera::OnKey( uint32_t nChar, bool isKeyDown, bool isAltDown )
{
    CameraEvent e = MakeEventFromKey( nChar, isKeyDown, isAltDown );
    UpdateByEvent( e );

    if (e.Flags & CameraEvent::EVENT_RESET)
    { m_ProjParam = m_ProjPreset; }
}

//-----------------------------------------------------------------------------
//      射影行列を取得します.
//-----------------------------------------------------------------------------
const asdx::Matrix& ViewerCamera::GetProj() const
{ return m_Proj; }

//-----------------------------------------------------------------------------
//      マウス入力からカメライベントを生成します.
//-----------------------------------------------------------------------------
CameraEvent ViewerCamera::MakeEventFromMouse( int wheelDelta )
{
    CameraEvent result;
    uint32_t flags = 0;

    // 移動軸.
    auto forward = GetAxisZ();
    auto right   = GetAxisX();
    auto upward  = GetAxisY();

    // 注視点カメラの場合.
    if (m_Type == CONTROL_TYPE_TARGET)
    {
        // ホイールでドリー.
        if ( wheelDelta > 0 )
        {
            flags |= CameraEvent::EVENT_DOLLY;
            result.Dolly = m_Gain[m_Type].Wheel;
        }
        else if ( wheelDelta < 0 )
        {
            flags |= CameraEvent::EVENT_DOLLY;
            result.Dolly = -m_Gain[m_Type].Wheel;
        }

        // 左ボタンドラッグで回転処理.
        if ( m_MouseLeft.isClick && m_MouseLeft.isPrevClick && ( !m_MouseRight.isClick ) && ( !m_MouseMiddle.isClick ) )
        {
            flags |= CameraEvent::EVENT_ROTATE;

            // 動いた差分を回転量とする.
            result.Rotate.x = -( m_MouseLeft.X - m_MouseLeft.prevX ) * m_Gain[m_Type].Rotate;
            result.Rotate.y = -( m_MouseLeft.Y - m_MouseLeft.prevY ) * m_Gain[m_Type].Rotate;
        }

        // 右ボタンドラッグでドリー.
        if ( ( !m_MouseLeft.isClick ) && m_MouseRight.isClick && m_MouseRight.isPrevClick && ( !m_MouseMiddle.isClick ) )
        {
            flags |= CameraEvent::EVENT_DOLLY;

            // 動いた差分をドリー量とする.
            result.Dolly = ( -( m_MouseRight.Y - m_MouseRight.prevY ) +
                             -( m_MouseRight.X - m_MouseRight.prevX ) ) * m_Gain[m_Type].Dolly;
        }

        // 中ボタンドラッグでトラック処理.
        if ( m_MouseMiddle.isClick && m_MouseMiddle.isPrevClick )
        {
            flags |= CameraEvent::EVENT_TRUCK;

            // 方向ベクトル算出.
            asdx::Vector3 dir = GetTarget() - GetPosition();
            if ( dir.LengthSq() != 0.0f )
            { dir.Normalize(); }

            // 右ベクトル算出.
            asdx::Vector3 right = Vector3::Cross( GetUpward(), dir );
            if ( right.LengthSq() != 0.0f )
            { right.Normalize(); }

            asdx::Vector3 upward = GetUpward();

            // 動いた差分を算出.
            float rightGain  = ( m_MouseMiddle.X - m_MouseMiddle.prevX ) * m_Gain[m_Type].Move;
            float upwardGain = ( m_MouseMiddle.Y - m_MouseMiddle.prevY ) * m_Gain[m_Type].Move;

            // 係数をかける.
            right.x  *= rightGain;
            right.y  *= rightGain;
            right.z  *= rightGain;
            upward.x *= upwardGain;
            upward.y *= upwardGain;
            upward.z *= upwardGain;

            // カメラ空間で平行に動かす.
            result.Truck.x = ( right.x + upward.x );
            result.Truck.y = ( right.y + upward.y );
            result.Truck.z = ( right.z + upward.z );
        }
    }
    // フリーカメラの場合.
    else if (m_Type == CONTROL_TYPE_FREE)
    {
        // ホイールで前後移動.
        if (wheelDelta != 0)
        {
            flags |= CameraEvent::EVENT_TRUCK;

            auto move = forward * m_Gain[m_Type].Wheel * float(wheelDelta);
            move.y = 0.0f;
            result.Truck += move;
        }

        // 右ボタンで上下移動とカニ歩き.
        if ( m_MouseRight.isClick && m_MouseRight.isPrevClick )
        {
            flags |= CameraEvent::EVENT_TRUCK;

            result.Truck += upward * m_Gain[m_Type].Move * float(m_MouseRight.Y - m_MouseRight.prevY);
            result.Truck += right  * m_Gain[m_Type].Move * float(m_MouseRight.X - m_MouseRight.prevX);
        }

        // 左ボタンで首振り.
        if ( m_MouseLeft.isClick && m_MouseLeft.isPrevClick )
        {
            flags |= CameraEvent::EVENT_PANTILT;

            // 動いた差分をパン・チルト角とする.
            result.PanTilt.x = ( m_MouseLeft.X - m_MouseLeft.prevX ) * m_Gain[m_Type].Rotate;
            result.PanTilt.y = ( m_MouseLeft.Y - m_MouseLeft.prevY ) * m_Gain[m_Type].Rotate;

            // 縦の首振りを抑える.
            const auto eps = 0.01f;
            if (result.PanTilt.y > asdx::F_PIDIV2 - eps)
            { result.PanTilt.y = asdx::F_PIDIV2 - eps; }
            else if (result.PanTilt.y < -asdx::F_PIDIV2 + eps)
            { result.PanTilt.y = -asdx::F_PIDIV2 + eps; }
        }
    }

    // フラグを設定.
    result.Flags = flags;

    return result;
}

//-----------------------------------------------------------------------------
//      キー入力からカメライベントを生成する.
//-----------------------------------------------------------------------------
CameraEvent ViewerCamera::MakeEventFromKey( uint32_t nChar, bool isKeyDown, bool isAltDown )
{
    CameraEvent result;

    switch( nChar )
    {
    // Fキーでリセット.
    case 0x46:
        if ( isKeyDown )
        { result.Flags |= CameraEvent::EVENT_RESET; }
        break;
    }

    return result;
}

//-----------------------------------------------------------------------------
//      調整係数をリセットします.
//-----------------------------------------------------------------------------
void ViewerCamera::ResetGain()
{
    m_Gain[0].Dolly  = 0.5f;
    m_Gain[0].Rotate = 0.01f;
    m_Gain[0].Move   = 1.0f;
    m_Gain[0].Wheel  = 20.0f;

    m_Gain[1].Dolly  = 0.5f;
    m_Gain[1].Rotate = 0.01f;
    m_Gain[1].Move   = 1.0f;
    m_Gain[1].Wheel  = 1.0f;
}

//-----------------------------------------------------------------------------
//      垂直画角を設定します.
//-----------------------------------------------------------------------------
void ViewerCamera::SetFieldOfView(float value)
{
    m_ProjParam.FieldOfView = value;
    m_Proj = Matrix::CreatePerspectiveFieldOfView(
        m_ProjParam.FieldOfView,
        m_ProjParam.AspectRatio,
        GetMinDist(),
        GetMaxDist());
}

//-----------------------------------------------------------------------------
//      アスペクト比を設定します.
//-----------------------------------------------------------------------------
void ViewerCamera::SetAspectRatio(float value)
{
    m_ProjParam.AspectRatio = value;
    m_Proj = Matrix::CreatePerspectiveFieldOfView(
        m_ProjParam.FieldOfView,
        m_ProjParam.AspectRatio,
        GetMinDist(),
        GetMaxDist());
}

} // namespace asdx

