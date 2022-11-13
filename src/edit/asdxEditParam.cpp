//-----------------------------------------------------------------------------
// File : asdxEditParam.cpp
// Desc : Edit Parameter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxDeviceContext.h>
#include <asdxMisc.h>
#include <edit/asdxEditParam.h>
#include <edit/asdxParamHistory.h>
#include <edit/asdxAppHistoryMgr.h>
#include <edit/asdxLocalization.h>


#ifdef ASDX_ENABLE_IMGUI
#include <imgui.h>
#endif//ASDX_ENABLE_IMGUI


#ifndef ASDX_UNUSED
#define ASDX_UNUSED(x) ((void)x)
#endif//ASDX_UNUSED


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const asdx::Localization kTagNoTexture(u8"テクスチャ無し", u8"NO TEXTURE");
static const asdx::Localization kTagLoad(u8"設定", u8"Load");
static const asdx::Localization kTagDelete(u8"破棄", u8"Delete");


///////////////////////////////////////////////////////////////////////////////
// ParamHistory
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class ParamHistory : public asdx::IHistory
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

    //-------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //-------------------------------------------------------------------------
    ParamHistory(T* target, const T& value)
    : m_pTarget (target)
    , m_Curr    (value)
    , m_Prev    (*target)
    { /* DO_NOTHING */ }

    ParamHistory(T* target, const T& nextValue, const T& prevValue)
    : m_pTarget (target)
    , m_Curr    (nextValue)
    , m_Prev    (prevValue)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //! @brief      やり直します.
    //-------------------------------------------------------------------------
    void Redo() override
    { *m_pTarget = m_Curr; }

    //-------------------------------------------------------------------------
    //! @brief      元に戻します.
    //-------------------------------------------------------------------------
    void Undo() override
    { *m_pTarget = m_Prev; }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    T*  m_pTarget;  //!< 変更対象.
    T   m_Prev;     //!< 変更前の値.
    T   m_Curr;     //!< 変更後の値.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// TextureHistory class
///////////////////////////////////////////////////////////////////////////////
class Texture2DHistory : public asdx::IHistory
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

    //-------------------------------------------------------------------------
    //      コンストラクタです.
    //-------------------------------------------------------------------------
    Texture2DHistory
    (
        asdx::Texture2D*    pTexture,
        const std::string&  nextValue,
        const std::string&  prevValue
    )
    : m_pTexture    (pTexture)
    , m_NextPath    (nextValue)
    , m_PrevPath    (prevValue)
    { /* DO_NOTHING */ }

    //-------------------------------------------------------------------------
    //      やり直しを実行します.
    //-------------------------------------------------------------------------
    void Redo() override
    {
        if (m_pTexture == nullptr)
        { return; }

        if (m_NextPath == "" || m_NextPath.empty())
        {
            m_pTexture->Release();
        }
        else
        {
            asdx::ResTexture res;
            if (!res.LoadFromFileA(m_NextPath.c_str()))
            { return; }

            m_pTexture->Release();
            
            auto pDevice  = asdx::DeviceContext::Instance().GetDevice();
            auto pContext = asdx::DeviceContext::Instance().GetContext();
            if (!m_pTexture->Create(pDevice, pContext, res))
            { return; }
        }
    }

    //-------------------------------------------------------------------------
    //      元に戻すを実行します.
    //-------------------------------------------------------------------------
    void Undo() override
    {
        if (m_pTexture == nullptr)
        { return; }

        if (m_PrevPath == "" || m_PrevPath.empty())
        {
            m_pTexture->Release();
        }
        else
        {
            asdx::ResTexture res;
            if (!res.LoadFromFileA(m_PrevPath.c_str()))
            { return; }

            m_pTexture->Release();

            auto pDevice  = asdx::DeviceContext::Instance().GetDevice();
            auto pContext = asdx::DeviceContext::Instance().GetContext();
            if (!m_pTexture->Create(pDevice, pContext, res))
            { return; }
        }
    }

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    asdx::Texture2D*    m_pTexture  = nullptr;
    std::string         m_NextPath;
    std::string         m_PrevPath;

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

//-----------------------------------------------------------------------------
//      ComboBox用ゲッターです.
//-----------------------------------------------------------------------------
static bool LocalizationGetter(void* data, int idx, const char** out_text)
{
    auto items = reinterpret_cast<asdx::Localization*>(data);
    if (items == nullptr)
    { return false; }

    *out_text = items[idx].c_str();
    return true;
}

} // namespace 


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// EditBool class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditBool::EditBool(bool value)
: m_Value(value)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditBool::SetValue(bool value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (value == m_Value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditBool::CreateHistory(bool value)
{ return new ParamHistory<bool>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
bool EditBool::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      チェックボックスを描画します.
//-----------------------------------------------------------------------------
void EditBool::DrawCheckbox(const char* tag)
{
#ifdef ASDX_ENABLE_IMGUI
    auto prev = m_Value;
    if (ImGui::Checkbox(tag, &m_Value))
    { AppHistoryMgr::GetInstance().Add(new ParamHistory<bool>(&m_Value, m_Value, prev), false); }
#else
    ASDX_UNUSED(tag);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditInt class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditInt::EditInt(int value)
: m_Value   (value)
, m_Prev    (value)
, m_Dragged (false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditInt::SetValue(int value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
int EditInt::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditInt::CreateHistory(int value)
{ return new ParamHistory<int>(&m_Value, m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawSlider(const char* tag, int step, int mini, int maxi)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::DragInt(tag, &m_Value, float(step), mini, maxi);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging();
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(step);
    ASDX_UNUSED(mini);
    ASDX_UNUSED(maxi);
#endif
}

//-----------------------------------------------------------------------------
//      エディットボックスとして描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawEditBox(const char* label)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::InputInt(label, &m_Value, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);
    if (flag)
    { AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, m_Value, m_Prev), false); }
#else
    ASDX_UNUSED(label);
#endif
}

//-----------------------------------------------------------------------------
//      コンボボックスを描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawCombo(const char* tag, int count, const char** items)
{
#ifdef ASDX_ENABLE_IMGUI
    auto value = m_Value;
    if (ImGui::Combo(tag, &value, items, count))
    { AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, value, m_Prev)); }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(count);
    ASDX_UNUSED(items);
#endif
}

//-----------------------------------------------------------------------------
//      コンボボックスを描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawCombo(const char* tag, bool (*items_getter)(void* data, int idx, const char** out_text), int count)
{
#ifdef ASDX_ENABLE_IMGUI
    auto value = m_Value;
    if (ImGui::Combo(tag, &value, items_getter, &value, count))
    { AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, value, m_Prev)); }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(items_getter);
    ASDX_UNUSED(count);
#endif
}

//-----------------------------------------------------------------------------
//      コンボボックスを描画します.
//-----------------------------------------------------------------------------
void EditInt::DrawCombo(const char* tag, int count, const Localization* items)
{
#ifdef ASDX_ENABLE_IMGUI
    auto data = const_cast<Localization*>(items);
    auto user = reinterpret_cast<void*>(data);

    auto value = m_Value;
    if (ImGui::Combo(tag, &value, LocalizationGetter, user, count))
    { AppHistoryMgr::GetInstance().Add(new ParamHistory<int>(&m_Value, value, m_Prev)); }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(count);
    ASDX_UNUSED(items);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// EditFloat class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat::EditFloat(float value)
: m_Value  (value)
, m_Prev   (0.0f)
, m_Dragged(false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditFloat::SetValue(float value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
float EditFloat::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat::CreateHistory(float value)
{ return new ParamHistory<float>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat::DrawSlider(const char* tag, float step, float mini, float maxi)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::DragFloat(tag, &m_Value, step, mini, maxi, "%.6f");

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<float>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<float>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging();
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(step);
    ASDX_UNUSED(mini);
    ASDX_UNUSED(maxi);
#endif
}

//-----------------------------------------------------------------------------
//      エディットボックスとして描画します.
//-----------------------------------------------------------------------------
void EditFloat::DrawEditBox(const char* label)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::InputFloat(label, &m_Value, 1.0f, 100.0f, "%.6f", ImGuiInputTextFlags_EnterReturnsTrue);
    if (flag)
    { AppHistoryMgr::GetInstance().Add(new ParamHistory<float>(&m_Value, m_Value, m_Prev), false); }
#else
    ASDX_UNUSED(label);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditFloat2 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat2::EditFloat2(float x, float y)
: m_Value  (x, y)
, m_Prev   (x, y)
, m_Dragged(false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditFloat2::SetValue(const asdx::Vector2& value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector2& EditFloat2::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat2::CreateHistory(const asdx::Vector2& value)
{ return new ParamHistory<asdx::Vector2>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat2::DrawSlider(const char* tag, float step, float mini, float maxi)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::DragFloat2(tag, m_Value, step, mini, maxi);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector2>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector2>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging();
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(step);
    ASDX_UNUSED(mini);
    ASDX_UNUSED(maxi);
#endif
}

//-----------------------------------------------------------------------------
//      エディットボックスとして描画します.
//-----------------------------------------------------------------------------
void EditFloat2::DrawEditBox(const char* label)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::InputFloat2(label, m_Value, "%.6f", ImGuiInputTextFlags_EnterReturnsTrue);
    if (flag)
    { AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector2>(&m_Value, m_Value, m_Prev), false);  }
#else
    ASDX_UNUSED(label);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// EditFloat3 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat3::EditFloat3(float x, float y, float z)
: m_Value   (x, y, z)
, m_Prev    (x, y, z)
, m_Dragged (false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditFloat3::SetValue(const asdx::Vector3& value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector3& EditFloat3::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat3::CreateHistory(const asdx::Vector3& value)
{ return new ParamHistory<asdx::Vector3>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat3::DrawSlider(const char* tag, float step, float mini, float maxi)
{
#ifdef ASDX_ENABLE_IMGUI
    auto prev = m_Value;
    auto flag = ImGui::DragFloat3(tag, m_Value, step, mini, maxi);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging();
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(step);
    ASDX_UNUSED(mini);
    ASDX_UNUSED(maxi);
#endif
}

//-----------------------------------------------------------------------------
//      エディットボックスとして描画します.
//-----------------------------------------------------------------------------
void EditFloat3::DrawEditBox(const char* label)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::InputFloat3(label, m_Value, "%.6f", ImGuiInputTextFlags_EnterReturnsTrue);
    if (flag)
    { AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false); }
#else
    ASDX_UNUSED(label);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditFloat4 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditFloat4::EditFloat4(float x, float y, float z, float w)
: m_Value   (x, y, z, w)
, m_Prev    (x, y, z, w)
, m_Dragged (false)
{ /* DO_NOTHING */ }


//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditFloat4::SetValue(const asdx::Vector4& value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector4& EditFloat4::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditFloat4::CreateHistory(const asdx::Vector4& value)
{ return new ParamHistory<asdx::Vector4>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditFloat4::DrawSlider(const char* tag, float step, float mini, float maxi)
{
#ifdef ASDX_ENABLE_IMGUI
    auto prev = m_Value;
    auto flag = ImGui::DragFloat4(tag, m_Value, step, mini, maxi);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging();
    }
#else
    ASDX_UNUSED(tag);
    ASDX_UNUSED(step);
    ASDX_UNUSED(mini);
    ASDX_UNUSED(maxi);
#endif
}

//-----------------------------------------------------------------------------
//      エディットボックスとして描画します.
//-----------------------------------------------------------------------------
void EditFloat4::DrawEditBox(const char* label)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::InputFloat4(label, m_Value, "%.6f", ImGuiInputTextFlags_EnterReturnsTrue);
    if (flag)
    { AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false); }
#else
    ASDX_UNUSED(label);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// EditColor3 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditColor3::EditColor3(float r, float g, float b)
: m_Value   (r, g, b)
, m_Prev    (r, g, b)
, m_Dragged (false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditColor3::SetValue(const asdx::Vector3& value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector3& EditColor3::GetValue() const
{ return m_Value; }
//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditColor3::CreateHistory(const asdx::Vector3& value)
{ return new ParamHistory<asdx::Vector3>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditColor3::DrawPicker(const char* tag)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::ColorEdit3(tag, m_Value, ImGuiColorEditFlags_Float);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging(0);
    }
#else
    ASDX_UNUSED(tag);
#endif
}

//-----------------------------------------------------------------------------
//      カラーホイールを描画します.
//-----------------------------------------------------------------------------
void EditColor3::DrawWheel(const char* tag)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::ColorPicker3(
        tag,
        m_Value,
        ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_PickerHueBar);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector3>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging(0);
    }
#else
    ASDX_UNUSED(tag);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// EditColor4 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditColor4::EditColor4(float r, float g, float b, float a)
: m_Value   (r, g, b, a)
, m_Prev    (r, g, b, a)
, m_Dragged (false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditColor4::SetValue(const asdx::Vector4& value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//-----------------------------------------------------------------------------
const asdx::Vector4& EditColor4::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditColor4::CreateHistory(const asdx::Vector4& value)
{ return new ParamHistory<asdx::Vector4>(&m_Value, value); }

//-----------------------------------------------------------------------------
//      スライダーを描画します.
//-----------------------------------------------------------------------------
void EditColor4::DrawPicker(const char* tag)
{
#ifdef ASDX_ENABLE_IMGUI
    auto prev = m_Value;
    auto flag = ImGui::ColorEdit4(tag, m_Value, ImGuiColorEditFlags_Float);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging(0);
    }
#else
    ASDX_UNUSED(tag);
#endif
}

//-----------------------------------------------------------------------------
//      カラーホイールを描画します.
//-----------------------------------------------------------------------------
void EditColor4::DrawWheel(const char* tag)
{
#ifdef ASDX_ENABLE_IMGUI
    auto flag = ImGui::ColorPicker4(
        tag,
        m_Value,
        ImGuiColorEditFlags_Float | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_PickerHueBar| ImGuiColorEditFlags_AlphaBar);

    if (!ImGui::IsMouseDragging(0) && !ImGui::IsMouseDown(0))
    {
        if (!m_Dragged && !ImGui::IsItemActive())
        {
            // 一度もドラッグされておらず，アイテムもいじられていない場合.
            m_Prev = m_Value;
        }
        else if (m_Dragged)
        {
            // マウスドラッグ終了時.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false);
            m_Dragged = false;
        }
        else if (flag)
        {
            // キーボード入力.
            AppHistoryMgr::GetInstance().Add(new ParamHistory<asdx::Vector4>(&m_Value, m_Value, m_Prev), false);
        }
    }
    else if (ImGui::IsItemActive())
    {
        m_Dragged = ImGui::IsMouseDragging(0);
    }
#else
    ASDX_UNUSED(tag);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// EditBit32 class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditBit32::EditBit32(uint32_t value)
: m_Value   (value)
, m_Prev    (value)
, m_Dragged (false)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      値を設定します.
//-----------------------------------------------------------------------------
void EditBit32::SetValue(uint32_t value, bool history)
{
    if (!history)
    {
        m_Value = value;
        return;
    }

    if (m_Value == value)
    {
        return;
    }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
}

//-----------------------------------------------------------------------------
//      値を取得します.
//----------------------------------------------------------------------------
uint32_t EditBit32::GetValue() const
{ return m_Value; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
asdx::IHistory* EditBit32::CreateHistory(uint32_t value)
{ return new ParamHistory<uint32_t>(&m_Value, m_Value, value); }

//-----------------------------------------------------------------------------
//      チェックボックスを描画します.
//-----------------------------------------------------------------------------
void EditBit32::DrawCheckBox(const char* tag)
{
#ifdef ASDX_ENABLE_IMGUI
    bool bit[32];
    for(auto i=0; i<32; ++i)
    {
        bit[i] = (m_Value & (0x1 << i)) == (0x1 << i);
    }

    auto changed = false;

    int idx = 1;
    uint32_t next_value = 0;

    char buf[16];
    char bin[33];

    ImGui::PushID(tag);
    if (ImGui::TreeNode(tag))
    {
        for(auto i=0; i<4; ++i)
        {
            sprintf_s(buf, "%02d", idx); idx++;
            changed |= ImGui::Checkbox(buf, &bit[8 * i + 0]);
            ImGui::SameLine();

            sprintf_s(buf, "%02d", idx); idx++;
            changed |= ImGui::Checkbox(buf, &bit[8 * i + 1]);
            ImGui::SameLine();

            sprintf_s(buf, "%02d", idx); idx++;
            changed |= ImGui::Checkbox(buf, &bit[8 * i + 2]);
            ImGui::SameLine();

            sprintf_s(buf, "%02d", idx); idx++;
            changed |= ImGui::Checkbox(buf, &bit[8 * i + 3]);
            ImGui::SameLine();

            sprintf_s(buf, "%02d", idx); idx++;
            changed |= ImGui::Checkbox(buf, &bit[8 * i + 4]);
            ImGui::SameLine();

            sprintf_s(buf, "%02d", idx); idx++;
            changed |= ImGui::Checkbox(buf, &bit[8 * i + 5]);
            ImGui::SameLine();

            sprintf_s(buf, "%02d", idx); idx++;
            changed |= ImGui::Checkbox(buf, &bit[8 * i + 6]);
            ImGui::SameLine();

            sprintf_s(buf, "%02d", idx); idx++;
            changed |= ImGui::Checkbox(buf, &bit[8 * i + 7]);
        }

        for (auto i = 0; i<32; ++i)
        {
            if (bit[i])
                next_value |= (0x1 << i);

            bin[32 - i - 1] = (bit[i]) ? '1' : '0';
        }
        bin[32] = '\0';

        ImGui::Text(u8"[DEC] : %u",   m_Value);         // 10進数表示.
        ImGui::Text(u8"[HEX] : %08x", next_value);      // 16進数表示.
        ImGui::Text(u8"[BIN]  : %s",  bin);             // 2進数表示.

        ImGui::TreePop();
    }

    ImGui::PopID();

    if (changed)
    {
        AppHistoryMgr::GetInstance().Add(new ParamHistory<uint32_t>(&m_Value, next_value, m_Prev), true);
    }
#else
    ASDX_UNUSED(tag);
#endif
}



///////////////////////////////////////////////////////////////////////////////
// EditTexture class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
EditTexture2D::EditTexture2D(const std::string& value)
: m_Path        (value)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
EditTexture2D::~EditTexture2D()
{ Term(); }

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void EditTexture2D::Term()
{ m_Texture.Release(); }

//-----------------------------------------------------------------------------
//      パスを設定します.
//-----------------------------------------------------------------------------
void EditTexture2D::SetPath(const std::string& value, bool history)
{
    if (!history)
    {
        m_Path = value;
        return;
    }

    if (m_Path == value)
    { return; }

    AppHistoryMgr::GetInstance().Add(CreateHistory(value));
    m_Path = value;
}

//-----------------------------------------------------------------------------
//      パスを取得します.
//-----------------------------------------------------------------------------
const std::string& EditTexture2D::GetPath() const
{ return m_Path; }

//-----------------------------------------------------------------------------
//      グループヒストリー用のヒストリーを作成します.
//-----------------------------------------------------------------------------
IHistory* EditTexture2D::CreateHistory(const std::string& next)
{ return new Texture2DHistory(&m_Texture, next, m_Path); }

//-----------------------------------------------------------------------------
//      コントールを描画します.
//-----------------------------------------------------------------------------
void EditTexture2D::DrawControl
(
    const char* label,
    const char* defaultPath,
    uint32_t    width,
    uint32_t    height
)
{
#if ASDX_ENABLE_IMGUI
    ImGui::PushID(label);
    {
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, float(width));
        auto descriptor = m_Texture.GetSRV();
        if (descriptor != nullptr)
        {
            ImTextureID texture = (void*)descriptor;
            ImGui::Image(texture, ImVec2(float(width), float(height)));

            if (ImGui::IsItemHovered())
            { ImGui::SetTooltip("%s", m_Path.c_str()); }
        }
        else
        { ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), kTagNoTexture.c_str()); }

        ImGui::NextColumn();
        ImGui::Text(label);
        auto resource = m_Texture.GetTexture();
        if (resource != nullptr)
        {
            D3D11_TEXTURE2D_DESC desc;
            resource->GetDesc(&desc);
            ImGui::BulletText(u8"%u×%u", desc.Width, desc.Height);
            ImGui::BulletText(u8"%s", asdx::GetString(desc.Format));
            ImGui::BulletText(u8"%u mips", desc.MipLevels);
        }
        if (ImGui::Button(kTagLoad.c_str()))
        {
            std::string path;
            if (OpenFileDlg(
                "Texture(*.bmp, *.jpg, *.png, *.gif, *.tga, *.dds)\0*.bmp;*.jpg;*.png;*.gif;*.tga;*.dds\0\0",
                path, defaultPath))
            { SetPath(path.c_str(), true); }
        }

        if (descriptor != nullptr)
        {
            ImGui::SameLine();
            if (ImGui::Button(kTagDelete.c_str()))
            { SetPath("", true); }
        }

        ImGui::Columns(1);
    }
    ImGui::PopID();
#else
    ASDX_UNUSED(label);
    ASDX_UNUSED(defaultPath);
    ASDX_UNUSED(width);
    ASDX_UNUSED(height);
#endif
}

//-----------------------------------------------------------------------------
//      リソースを取得します.
//-----------------------------------------------------------------------------
ID3D11Texture2D* EditTexture2D::GetResource() const
{ return m_Texture.GetTexture(); }

//-----------------------------------------------------------------------------
//      ディスクリプタを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderResourceView* EditTexture2D::GetSRV() const
{ return m_Texture.GetSRV(); }

} // namespace asdx



#ifdef ASDX_ENABLE_TINYXML2
namespace asdx {

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditBool& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("value", value);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditInt& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("value", value);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditFloat& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("value", value);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditFloat2& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("x", value.x);
    element->SetAttribute("y", value.y);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditFloat3& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("x", value.x);
    element->SetAttribute("y", value.y);
    element->SetAttribute("z", value.z);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditFloat4& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("x", value.x);
    element->SetAttribute("y", value.y);
    element->SetAttribute("z", value.z);
    element->SetAttribute("w", value.w);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditColor3& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("r", value.x);
    element->SetAttribute("g", value.y);
    element->SetAttribute("b", value.z);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditColor4& control)
{
    auto value = control.GetValue();
    auto element = doc->NewElement(tag);
    element->SetAttribute("r", value.x);
    element->SetAttribute("g", value.y);
    element->SetAttribute("b", value.z);
    element->SetAttribute("a", value.w);
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditBit32& control)
{
    auto element = doc->NewElement(tag);
    element->SetAttribute("value", control.GetValue());
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを生成します.
//-----------------------------------------------------------------------------
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditTexture2D& control)
{
    auto element = doc->NewElement(tag);
    element->SetAttribute("path", control.GetPath().c_str());
    return element;
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditBool& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    auto value = e->BoolAttribute("value");
    control = EditBool(value);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditInt& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    auto value = e->IntAttribute("value");
    control = EditInt(value);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditFloat& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    auto value = e->FloatAttribute("value");
    control = EditFloat(value);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditFloat2& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    asdx::Vector2 value;
    value.x = e->FloatAttribute("x");
    value.y = e->FloatAttribute("y");
    control = EditFloat2(value.x, value.y);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditFloat3& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    asdx::Vector3 value;
    value.x = e->FloatAttribute("x");
    value.y = e->FloatAttribute("y");
    value.z = e->FloatAttribute("z");
    control = EditFloat3(value.x, value.y, value.z);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditFloat4& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    asdx::Vector4 value;
    value.x = e->FloatAttribute("x");
    value.y = e->FloatAttribute("y");
    value.z = e->FloatAttribute("z");
    value.w = e->FloatAttribute("w");
    control = EditFloat4(value.x, value.y, value.z, value.w);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditColor3& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    asdx::Vector3 value;
    value.x = e->FloatAttribute("r");
    value.y = e->FloatAttribute("g");
    value.z = e->FloatAttribute("b");
    control = EditColor3(value.x, value.y, value.z);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditColor4& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    asdx::Vector4 value;
    value.x = e->FloatAttribute("r");
    value.y = e->FloatAttribute("g");
    value.z = e->FloatAttribute("b");
    value.w = e->FloatAttribute("a");
    control = EditColor4(value.x, value.y, value.z, value.w);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditBit32& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    auto value = e->UnsignedAttribute("value");
    control = EditBit32(value);
}

//-----------------------------------------------------------------------------
//      XMLエレメントを解析します.
//-----------------------------------------------------------------------------
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditTexture2D& control)
{
    auto e = element->FirstChildElement(tag);
    if (e == nullptr)
    { return; }

    auto path = e->Attribute("path");
    control.SetPath(path);
}

} // namespace asdx
#endif//ASDX_ENABLE_TINYXML2
