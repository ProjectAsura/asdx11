﻿//-----------------------------------------------------------------------------
// File : asdxEditParam.h
// Desc : Edit Parameter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxMath.h>
#include <asdxTexture.h>


namespace asdx {

//-----------------------------------------------------------------------------
// Forward Declarations.
//-----------------------------------------------------------------------------
struct IHistory;


///////////////////////////////////////////////////////////////////////////////
// EditBool class
///////////////////////////////////////////////////////////////////////////////
class EditBool
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
    //!
    //! @param[in]      tag     データ識別用タグ名です.
    //! @param[in]      value   初期値です.
    //-------------------------------------------------------------------------
    EditBool(bool value = false);

    //-------------------------------------------------------------------------
    //! @brief      値を設定します.
    //!
    //! @param[in]      value       設定する値です.
    //! @param[in]      history     ヒストリー登録する場合は true を指定.
    //-------------------------------------------------------------------------
    void SetValue(bool value, bool history = false);

    //-------------------------------------------------------------------------
    //! @brief      値を取得します.
    //!
    //! @return     値を返却します.
    //-------------------------------------------------------------------------
    bool GetValue() const;

    //-------------------------------------------------------------------------
    //! @brief      グループヒストリー用履歴を作成します.
    //!
    //! @param[in]      nexe        次に設定する値.
    //! @return     生成した履歴を返却します.
    //-------------------------------------------------------------------------
    IHistory* CreateHistory(bool next);

    //-------------------------------------------------------------------------
    //! @brief      描画します.
    //!
    //! @param[in]      label       表示ラベル名.
    //-------------------------------------------------------------------------
    void DrawCheckbox(const char* label);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    bool        m_Value;    //!< 値です.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// EditInt class
///////////////////////////////////////////////////////////////////////////////
class EditInt
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
    //!
    //! @param[in]      value   初期値です.
    //-------------------------------------------------------------------------
    EditInt(int value = 0);

    //-------------------------------------------------------------------------
    //! @brief      値を設定します.
    //!
    //! @param[in]      value       設定する値です.
    //! @param[in]      history     ヒストリー登録する場合は true を指定.
    //-------------------------------------------------------------------------
    void SetValue(int value, bool history = false);

    //-------------------------------------------------------------------------
    //! @brief      値を取得します.
    //!
    //! @return     値を返却します.
    //-------------------------------------------------------------------------
    int GetValue() const;

    //-------------------------------------------------------------------------
    //! @brief      グループヒストリー用履歴を作成します.
    //!
    //! @param[in]      nexe        次に設定する値.
    //! @return     生成した履歴を返却します.
    //-------------------------------------------------------------------------
    IHistory* CreateHistory(int next);

    //-------------------------------------------------------------------------
    //! @brief      スライダーとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawSlider(const char* label, int step = 1, int mini = 0, int maxi = 0);

    //-------------------------------------------------------------------------
    //! @brief      入力可能なエディットボックスとして描画します.
    //-------------------------------------------------------------------------
    void DrawEditBox(const char* label);

    //-------------------------------------------------------------------------
    //! @brief      コンボボックスを描画します.
    //!
    //! @param[in]      label       表示名.
    //! @param[in]      count       アイテム数.
    //! @param[in]      items       アイテム名.
    //-------------------------------------------------------------------------
    void DrawCombo(const char* label, int count, const char** items);

    //-------------------------------------------------------------------------
    //! @brief      コンボボックスを描画します.
    //!
    //! @param[in]      label           表示名.
    //! @param[in]      items_getter    アイテム取得関数.
    //! @param[in]      count           アイテム数.
    //------------------------------------------------------------------------
    void DrawCombo(const char* label, bool (*items_getter)(void* data, int idx, const char** out_text), int count);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    int         m_Value;    //!< 値です.
    int         m_Prev;     //!< 前の値.
    bool        m_Dragged;  //!< ドラッグ中かどうか?

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// EditFloat class
///////////////////////////////////////////////////////////////////////////////
class EditFloat
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
    //!
    //! @param[in]      value   初期値です.
    //-------------------------------------------------------------------------
    EditFloat(float value = 0);

    //-------------------------------------------------------------------------
    //! @brief      値を設定します.
    //!
    //! @param[in]      value       設定する値です.
    //! @param[in]      history     ヒストリー登録する場合は true を指定.
    //-------------------------------------------------------------------------
    void SetValue(float value, bool history = false);

    //-------------------------------------------------------------------------
    //! @brief      値を取得します.
    //!
    //! @return     値を返却します.
    //-------------------------------------------------------------------------
    float GetValue() const;

    //-------------------------------------------------------------------------
    //! @brief      グループヒストリー用履歴を作成します.
    //!
    //! @param[in]      nexe        次に設定する値.
    //! @return     生成した履歴を返却します.
    //-------------------------------------------------------------------------
    IHistory* CreateHistory(float next);

    //-------------------------------------------------------------------------
    //! @brief      スライダーとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawSlider(const char* label, float step = 0.1f, float mini = 0.0f, float maxi = 0.0f);

    //-------------------------------------------------------------------------
    //! @brief      入力可能なエディットボックスとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawEditBox(const char* label);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    float       m_Value;    //!< 値です.
    float       m_Prev;     //!< 前の値.
    bool        m_Dragged;  //!< ドラッグ中かどうか?

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// EditFloat2 class
///////////////////////////////////////////////////////////////////////////////
class EditFloat2
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
    //!
    //! @param[in]      value   初期値です.
    //-------------------------------------------------------------------------
    EditFloat2(float x = 0.0f, float y = 0.0f);

    //-------------------------------------------------------------------------
    //! @brief      値を設定します.
    //!
    //! @param[in]      value       設定する値です.
    //! @param[in]      history     ヒストリー登録する場合は true を指定.
    //-------------------------------------------------------------------------
    void SetValue(const Vector2& value, bool history = false);

    //-------------------------------------------------------------------------
    //! @brief      値を取得します.
    //!
    //! @return     値を返却します.
    //-------------------------------------------------------------------------
    const Vector2& GetValue() const;

    //-------------------------------------------------------------------------
    //! @brief      グループヒストリー用履歴を作成します.
    //!
    //! @param[in]      nexe        次に設定する値.
    //! @return     生成した履歴を返却します.
    //-------------------------------------------------------------------------
    IHistory* CreateHistory(const Vector2& next);

    //-------------------------------------------------------------------------
    //! @brief      スライダーとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawSlider(const char* label, float step = 0.1f, float mini = 0.0f, float maxi = 0.0f);

    //-------------------------------------------------------------------------
    //! @brief      入力可能なエディットボックスとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawEditBox(const char* label);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    Vector2     m_Value;    //!< 値です.
    Vector2     m_Prev;     //!< 前の値.
    bool        m_Dragged;  //!< ドラッグ中かどうか?

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// EditFloat3 class
///////////////////////////////////////////////////////////////////////////////
class EditFloat3
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
    //!
    //! @param[in]      value   初期値です.
    //-------------------------------------------------------------------------
    EditFloat3(float x = 0.0f, float y = 0.0f, float z = 0.0f);

    //-------------------------------------------------------------------------
    //! @brief      値を設定します.
    //!
    //! @param[in]      value       設定する値です.
    //! @param[in]      history     ヒストリー登録する場合は true を指定.
    //-------------------------------------------------------------------------
    void SetValue(const Vector3& value, bool history = false);

    //-------------------------------------------------------------------------
    //! @brief      値を取得します.
    //!
    //! @return     値を返却します.
    //-------------------------------------------------------------------------
    const Vector3& GetValue() const;

    //-------------------------------------------------------------------------
    //! @brief      グループヒストリー用履歴を作成します.
    //!
    //! @param[in]      nexe        次に設定する値.
    //! @return     生成した履歴を返却します.
    //-------------------------------------------------------------------------
    IHistory* CreateHistory(const Vector3& next);

    //-------------------------------------------------------------------------
    //! @brief      スライダーとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawSlider(const char* label, float step = 0.1f, float mini = 0.0f, float maxi = 0.0f);

    //-------------------------------------------------------------------------
    //! @brief      入力可能なエディットボックスとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawEditBox(const char* label);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    Vector3     m_Value;    //!< 値です.
    Vector3     m_Prev;     //!< 前の値.
    bool        m_Dragged;  //!< ドラッグ中かどうか?

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

///////////////////////////////////////////////////////////////////////////////
// EditFloat4 class
///////////////////////////////////////////////////////////////////////////////
class EditFloat4
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
    //!
    //! @param[in]      value   初期値です.
    //-------------------------------------------------------------------------
    EditFloat4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f);

    //-------------------------------------------------------------------------
    //! @brief      値を設定します.
    //!
    //! @param[in]      value       設定する値です.
    //! @param[in]      history     ヒストリー登録する場合は true を指定.
    //-------------------------------------------------------------------------
    void SetValue(const Vector4& value, bool history = false);

    //-------------------------------------------------------------------------
    //! @brief      値を取得します.
    //!
    //! @return     値を返却します.
    //-------------------------------------------------------------------------
    const Vector4& GetValue() const;

    //-------------------------------------------------------------------------
    //! @brief      グループヒストリー用履歴を作成します.
    //!
    //! @param[in]      nexe        次に設定する値.
    //! @return     生成した履歴を返却します.
    //-------------------------------------------------------------------------
    IHistory* CreateHistory(const Vector4& next);

    //-------------------------------------------------------------------------
    //! @brief      スライダーとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawSlider(const char* label, float step = 0.1f, float mini = 0.0f, float maxi = 0.0f);

    //-------------------------------------------------------------------------
    //! @brief      入力可能なエディットぼっくとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawEditBox(const char* label);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    Vector4     m_Value;    //!< 値です.
    Vector4     m_Prev;     //!< 前の値.
    bool        m_Dragged;  //!< ドラッグ中かどうか?

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// EditColor3 class
///////////////////////////////////////////////////////////////////////////////
class EditColor3
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
    //!
    //! @param[in]      value   初期値です.
    //-------------------------------------------------------------------------
    EditColor3(float r = 0.0f, float g = 0.0f, float b = 0.0f);

    //-------------------------------------------------------------------------
    //! @brief      値を設定します.
    //!
    //! @param[in]      value       設定する値です.
    //! @param[in]      history     ヒストリー登録する場合は true を指定.
    //-------------------------------------------------------------------------
    void SetValue(const Vector3& value, bool history = false);

    //-------------------------------------------------------------------------
    //! @brief      値を取得します.
    //!
    //! @return     値を返却します.
    //-------------------------------------------------------------------------
    const Vector3& GetValue() const;

    //-------------------------------------------------------------------------
    //! @brief      グループヒストリー用履歴を作成します.
    //!
    //! @param[in]      nexe        次に設定する値.
    //! @return     生成した履歴を返却します.
    //-------------------------------------------------------------------------
    IHistory* CreateHistory(const Vector3& next);

    //-------------------------------------------------------------------------
    //! @brief      カラーピッカーとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawPicker(const char* label);

    //-------------------------------------------------------------------------
    //! @brief      カラーホイールとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawWheel(const char* label);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    const char* m_Tag;      //!< 識別用タグです.
    Vector3     m_Value;    //!< 値です.
    Vector3     m_Prev;     //!< 前の値.
    bool        m_Dragged;  //!< ドラッグ中かどうか?

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// EditColor4 class
///////////////////////////////////////////////////////////////////////////////
class EditColor4
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
    //!
    //! @param[in]      value   初期値です.
    //-------------------------------------------------------------------------
    EditColor4(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);

    //-------------------------------------------------------------------------
    //! @brief      値を設定します.
    //!
    //! @param[in]      value       設定する値です.
    //! @param[in]      history     ヒストリー登録する場合は true を指定.
    //-------------------------------------------------------------------------
    void SetValue(const Vector4& value, bool history = false);

    //-------------------------------------------------------------------------
    //! @brief      値を取得します.
    //!
    //! @return     値を返却します.
    //-------------------------------------------------------------------------
    const Vector4& GetValue() const;

    //-------------------------------------------------------------------------
    //! @brief      グループヒストリー用履歴を作成します.
    //!
    //! @param[in]      nexe        次に設定する値.
    //! @return     生成した履歴を返却します.
    //-------------------------------------------------------------------------
    IHistory* CreateHistory(const Vector4& next);

    //-------------------------------------------------------------------------
    //! @brief      描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawPicker(const char* label);

    //-------------------------------------------------------------------------
    //! @brief      カラーホイールとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawWheel(const char* label);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    Vector4     m_Value;    //!< 値です.
    Vector4     m_Prev;     //!< 前の値.
    bool        m_Dragged;  //!< ドラッグ中かどうか?

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// EditBit32 class
///////////////////////////////////////////////////////////////////////////////
class EditBit32
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
    //!
    //! @param[in]      value   初期値です.
    //-------------------------------------------------------------------------
    EditBit32(uint32_t value = 0);

    //-------------------------------------------------------------------------
    //! @brief      値を設定します.
    //!
    //! @param[in]      value       設定する値です.
    //! @param[in]      history     ヒストリー登録する場合は true を指定.
    //-------------------------------------------------------------------------
    void SetValue(uint32_t value, bool history = false);

    //-------------------------------------------------------------------------
    //! @brief      値を取得します.
    //!
    //! @return     値を返却します.
    //-------------------------------------------------------------------------
    uint32_t GetValue() const;

    //-------------------------------------------------------------------------
    //! @brief      グループヒストリー用履歴を作成します.
    //!
    //! @param[in]      nexe        次に設定する値.
    //! @return     生成した履歴を返却します.
    //-------------------------------------------------------------------------
    IHistory* CreateHistory(uint32_t next);

    //-------------------------------------------------------------------------
    //! @brief      フラグを取得します.
    //!
    //! @param
    //-------------------------------------------------------------------------
    bool GetFlag(uint32_t bit);

    //-------------------------------------------------------------------------
    //! @brief      入力可能なテキストボックスとして描画します.
    //!
    //! @param[in]      label       表示名.
    //-------------------------------------------------------------------------
    void DrawCheckBox(const char* label);

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    uint32_t    m_Value;    //!< 値です.
    uint32_t    m_Prev;     //!< 前の値.
    bool        m_Dragged;  //!< ドラッグ中かどうか?

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////
// EditTexture2D class
///////////////////////////////////////////////////////////////////////////////
class EditTexture2D
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
    EditTexture2D(const std::string& value = "");

    //-------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //-------------------------------------------------------------------------
    ~EditTexture2D();

    //-------------------------------------------------------------------------
    //! @brief      終了処理を行います.
    //-------------------------------------------------------------------------
    void Term();

    //-------------------------------------------------------------------------
    //! @brief      ファイルパスを設定します.
    //!
    //! @param[in]      value       設定するファイルパスです.
    //! @param[in]      history     ヒストリー登録する場合は true を指定.
    //-------------------------------------------------------------------------
    void SetPath(const std::string& value, bool history = false);

    //-------------------------------------------------------------------------
    //! @brief      ファイルパスを取得します.
    //!
    //! @return     ファイルパスを返却します.
    //-------------------------------------------------------------------------
    const std::string& GetPath() const;

    //-------------------------------------------------------------------------
    //! @brief      グループヒストリー用履歴を作成します.
    //!
    //! @param[in]      next        次に設定する値.
    //! @return     生成した履歴を返却します.
    //-------------------------------------------------------------------------
    IHistory* CreateHistory(const std::string& next);

    //-------------------------------------------------------------------------
    //! @brief      コントロールを描画します.
    //!
    //! @param[in]      label           表示名です.
    //! @param[in]      defaultPath     デフォルトパスです.
    //! @param[in]      width           表示する横幅.
    //! @param[in]      height          表示する縦幅
    //-------------------------------------------------------------------------
    void DrawControl(
        const char* label,
        const char* defaultPath,
        uint32_t width  = 64,
        uint32_t height = 64,
        bool     jp     = true);

    //-------------------------------------------------------------------------
    //! @brief      リソースを取得します.
    //!
    //! @return     リソースを返却します.
    //-------------------------------------------------------------------------
    ID3D11Texture2D* GetResource() const;

    //-------------------------------------------------------------------------
    //! @brief      シェーダリソースビューを取得します.
    //!
    //! @return     シェーダリソースビューを返却します.
    //-------------------------------------------------------------------------
    ID3D11ShaderResourceView* GetSRV() const;

private:
    //=========================================================================
    // private variables.
    //=========================================================================
    Texture2D       m_Texture;      //!< テクスチャです.
    std::string     m_Path;         //!< ファイルパスです.

    //=========================================================================
    // private methods.
    //=========================================================================
    /* NOTHING */
};

} // namespace asdx


#ifdef ASDX_ENABLE_TINYXML2
#include <tinyxml2.h>

namespace asdx {

tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditBool&        control);
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditInt&         control);
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditFloat&       control);
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditFloat2&      control);
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditFloat3&      control);
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditFloat4&      control);
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditColor3&      control);
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditColor4&      control);
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditBit32&       control);
tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument* doc, const char* tag, const EditTexture2D&   control);

void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditBool&      control);
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditInt&       control);
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditFloat&     control);
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditFloat2&    control);
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditFloat3&    control);
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditFloat4&    control);
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditColor3&    control);
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditColor4&    control);
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditBit32&     control);
void Deserialize(tinyxml2::XMLElement* element, const char* tag, EditTexture2D& control);

} // namespace asdx
#endif//ASDX_ENABLE_TINYXML2