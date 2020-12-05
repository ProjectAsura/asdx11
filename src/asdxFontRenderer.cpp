//-----------------------------------------------------------------------------
// File : asdxFontRenderer
// Desc : Font Renderer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cstdio>
#include <vector>
#include <asdxFontRenderer.h>
#include <asdxLogger.h>


namespace {

//-----------------------------------------------------------------------------
// Constant Valules.
//-----------------------------------------------------------------------------
static const uint32_t kBinaryVersion1    = 0x3;
static const size_t   kVertexPerSprite   = 4;
static const size_t   kIndexPerSprite    = 6;


///////////////////////////////////////////////////////////////////////////////
// DRAW_OPTION enum
///////////////////////////////////////////////////////////////////////////////
enum DRAW_OPTION
{
    DRAW_OPTION_NONE        = 0,
    DRAW_OPTION_OUTLINE     = 0x1 << 0,
    DRAW_OPTION_DROP_SHADOW = 0x1 << 1,
    DRAW_OPTION_OUTER_GLOW  = 0x1 << 2,
};

///////////////////////////////////////////////////////////////////////////////
// FontHeader structure
///////////////////////////////////////////////////////////////////////////////
struct FontHeader
{
    uint8_t     Magic[4];       // マジック "FNT\0";
    uint32_t    Version;        // バージョン.
    int         Ascent;         // ベースラインからの上方向の高さ.
    int         Descent;        // ベースラインからの下方向の高さ.
    int         LineGap;        // 行間.
    float       HeightScale;    // フォントスケール.
    uint32_t    GlyphCount;     // グリフ数.
    uint32_t    Width;          // テクスチャ横幅.
    uint32_t    Height;         // テクスチャ縦幅.
    uint64_t    PixelSize;      // ピクセルデータのサイズ(bytes).
    // 後続データは
    // * ResGlyphの配列.
    // * テクスチャデータ.
    // の順番.
};

///////////////////////////////////////////////////////////////////////////////
// ResGlyph structure
///////////////////////////////////////////////////////////////////////////////
struct ResGlyph
{
    uint32_t        Code;       // 文字コード.
    asdx::Glyph     Glyph;      // グリフ.
};

//-----------------------------------------------------------------------------
//      フラグを設定します.
//-----------------------------------------------------------------------------
inline void SetFlag(uint8_t& flag, uint8_t bit, bool enable)
{
    if (enable)
    { flag |= bit; }
    else
    { flag &= ~(bit); }
}

//-------------------------------------------------------------------------------------------------
//! @brief      2次元テクスチャを生成します.
//!
//! @param [in]     pDevice         デバイスです.
//! @param [in]     pDeviceContext  デバイスコンテキストです.
//! @param [in]     width           テクスチャの横幅です.
//! @param [in]     height          テクスチャの縦幅です.
//! @param [in]     mipCount        ミップマップ数です.
//! @param [in]     surfaceCount    サーフェイス数です.
//! @param [in]     isCubeMap       キューブマップかどうか.
//! @param [in]     format          DXGIフォーマットです.
//! @param [in]     usage           使用方法.
//! @param [in]     bindFlags       バインドフラグです.
//! @param [in]     cpuAccessFlags  CPUAアクセスフラグです.
//! @param [in]     miscFlags       その他のオプション.
//! @param [in]     pInitData       サブリソースです.
//! @param [out]    ppTexture       テクスチャです.
//! @param [out]    ppSRV           シェーダリソースビューです.
//! @retval true    生成に成功.
//! @retval false   生成に失敗.
//-------------------------------------------------------------------------------------------------
bool CreateTexture2D
(
    ID3D11Device*               pDevice,
    ID3D11DeviceContext*        pDeviceContext,
    UINT                        width,
    UINT                        height,
    UINT                        mipCount,
    UINT                        surfaceCount,
    bool                        isCubeMap,
    DXGI_FORMAT                 format,
    D3D11_USAGE                 usage,
    UINT                        bindFlags,
    UINT                        cpuAccessFlags,
    UINT                        miscFlags,
    D3D11_SUBRESOURCE_DATA*     pInitData,
    ID3D11Texture2D**           ppTexture,
    ID3D11ShaderResourceView**  ppSRV
)
{
    assert( pInitData != nullptr );
    if ( mipCount == 0 )
    { mipCount = 1; }

    bool autogen = false;
    uint32_t fmtSupport = 0;
    HRESULT hr = pDevice->CheckFormatSupport( format, &fmtSupport );
    if ( mipCount == 0 
      && SUCCEEDED( hr ) 
      && ( fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN ) )
    {
        autogen = true;
    }

    // テクスチャ2Dの設定
    D3D11_TEXTURE2D_DESC td;

    td.Width              = width;
    td.Height             = height;
    td.MipLevels          = ( autogen ) ? 0 : mipCount;
    td.ArraySize          = surfaceCount;
    td.Format             = format;
    td.SampleDesc.Count   = 1;
    td.SampleDesc.Quality = 0;
    td.Usage              = usage;
    td.BindFlags          = bindFlags;
    td.CPUAccessFlags     = cpuAccessFlags;
    if ( isCubeMap )
    { td.MiscFlags = miscFlags | D3D11_RESOURCE_MISC_TEXTURECUBE; }
    else
    { td.MiscFlags = miscFlags & ~D3D11_RESOURCE_MISC_TEXTURECUBE; }

    if ( autogen )
    {
        td.BindFlags |= D3D11_BIND_RENDER_TARGET;
        td.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    // テクスチャを生成.
    ID3D11Texture2D* pTexture = nullptr;
    hr = pDevice->CreateTexture2D( &td, (autogen) ? nullptr : pInitData, &pTexture );

    // チェック.
    if ( SUCCEEDED( hr ) && ( pTexture != nullptr ) )
    {
        // シェーダリソースビューが指定されている場合.
        if ( ppSRV != nullptr )
        {
            // シェーダリソースビューの設定.
            D3D11_SHADER_RESOURCE_VIEW_DESC sd;

            // ゼロクリア.
            memset( &sd, 0, sizeof( sd ) );

            // 画像フォーマットの設定.
            sd.Format = format;

            // キューブマップの場合.
            if ( isCubeMap )
            {
                sd.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
                sd.TextureCube.MipLevels = ( autogen ) ? -1 : mipCount;
            }
            // 普通の2Dテクスチャの場合.
            else
            {
                sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                sd.TextureCube.MipLevels = ( autogen ) ? -1 : mipCount;
            }

            // シェーダリソースビューを生成.
            hr = pDevice->CreateShaderResourceView( pTexture, &sd, ppSRV );

            // チェック.
            if ( FAILED( hr ) )
            {
                // テクスチャを解放.
                pTexture->Release();

                // 異常終了.
                return false;
            }

            if ( autogen )
            {
                if ( surfaceCount <= 1 )
                {
                    pDeviceContext->UpdateSubresource( pTexture, 0, nullptr, pInitData[0].pSysMem, pInitData[0].SysMemPitch, pInitData[0].SysMemSlicePitch );
                }
                else
                {
                    D3D11_TEXTURE2D_DESC desc;
                    pTexture->GetDesc( &desc );

                    for( uint32_t i=0; i<surfaceCount; ++i )
                    {
                        auto res = D3D11CalcSubresource( 0, i, desc.MipLevels );
                        pDeviceContext->UpdateSubresource( pTexture, res, nullptr, pInitData[i].pSysMem, pInitData[i].SysMemPitch, pInitData[i].SysMemSlicePitch );
                    }
                }
                pDeviceContext->GenerateMips( (*ppSRV) );
            }
        }

        // テクスチャが指定されている場合.
        if ( ppTexture != nullptr )
        {
            // テクスチャを設定.
            (*ppTexture) = pTexture;
        }
        else
        {
            // テクスチャを解放.
            pTexture->Release();
        }

        // 正常終了.
        return true;
    }

    // 異常終了.
    return false;
}


} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Font class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Font::Font()
: m_Ascent      (0)
, m_Descent     (0)
, m_LineGap     (0)
, m_HeightScale (0.0f)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Font::~Font()
{ Term(); }

//-----------------------------------------------------------------------------
//      バイナリをロードします.
//-----------------------------------------------------------------------------
bool Font::Load
(
    ID3D11Device*           pDevice,
    ID3D11DeviceContext*    pContext,
    const char*             path
)
{
    FILE* pFile = nullptr;
    auto err = fopen_s(&pFile, path, "rb");
    if (err != 0)
    {
        ELOGA("Error : File Open Failed. path = %s", path);
        return false;
    }

    FontHeader header = {};
    fread(&header, sizeof(header), 1, pFile);

    if (header.Magic[0] != 'F' ||
        header.Magic[1] != 'N' ||
        header.Magic[2] != 'T' ||
        header.Magic[3] != '\0')
    {
        ELOGA("Error : Invalid File Magic. path = %s", path);
        fclose(pFile);
        return false;
    }

    if (header.Version != kBinaryVersion1)
    {
        ELOGA("Error : Invalid File Verision. verison = %u", header.Version);
        fclose(pFile);
        return false;
    }

    m_Ascent        = header.Ascent;
    m_Descent       = header.Descent;
    m_LineGap       = header.LineGap;
    m_HeightScale   = header.HeightScale;

    // グリフ読み込み.
    for(auto i=0u; i<header.GlyphCount; ++i)
    {
        ResGlyph res = {};
        fread(&res, sizeof(res), 1, pFile);
        m_Glyphs[res.Code] = res.Glyph;
    }

    // ピクセルデータ読み込み.
    std::vector<uint8_t> pixels;
    pixels.resize(header.PixelSize);
    fread(pixels.data(), header.PixelSize, 1, pFile);

    // ファイルを閉じる.
    fclose(pFile);

    // テクスチャ生成.
    {
        D3D11_SUBRESOURCE_DATA res = {};
        res.pSysMem          = pixels.data();
        res.SysMemPitch      = header.Width * 4;
        res.SysMemSlicePitch = header.Width * header.Height * 4;
        
        auto ret = CreateTexture2D(
            pDevice,
            pContext,
            header.Width,
            header.Height,
            1,
            1,
            false,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            D3D11_USAGE_DEFAULT,
            D3D11_BIND_SHADER_RESOURCE,
            0,
            0,
            &res,
            m_Texture.GetAddress(),
            m_SRV.GetAddress());

        pixels.clear();

        if (!ret)
        {
            ELOGA("Error : CreateTexture2D() Failed.");
            Term();
            return false;
        }
    }

    // 正常終了.
    return true;
}

//-----------------------------------------------------------------------------
//      破棄処理を行います.
//-----------------------------------------------------------------------------
void Font::Term()
{
    m_Ascent        = 0;
    m_Descent       = 0;
    m_LineGap       = 0;
    m_HeightScale   = 0.0f;

    m_Glyphs .clear();
    m_Texture.Reset();
    m_SRV    .Reset();
}

//-----------------------------------------------------------------------------
//      ベースラインから上方向の高さを取得します.
//-----------------------------------------------------------------------------
int Font::GetAscent() const
{ return m_Ascent; }

//-----------------------------------------------------------------------------
//      ベースラインから下方向のたかあを取得します.
//-----------------------------------------------------------------------------
int Font::GetDescent() const
{ return m_Descent; }

//-----------------------------------------------------------------------------
//      行間を取得します.
//-----------------------------------------------------------------------------
int Font::GetLineGap() const
{ return m_LineGap; }

//-----------------------------------------------------------------------------
//      フォントスケールを取得します.
//-----------------------------------------------------------------------------
float Font::GetHeightScale() const
{ return m_HeightScale; }

//-----------------------------------------------------------------------------
//      一行あたりの高さを取得します.
//-----------------------------------------------------------------------------
int Font::GetHeight() const
{ return (m_Ascent - m_Descent) + m_LineGap; }

//-----------------------------------------------------------------------------
//      グリフを持つかどうかチェックします.
//-----------------------------------------------------------------------------
bool Font::HasGlyph(uint32_t code) const
{ return m_Glyphs.find(code) != m_Glyphs.end(); }

//-----------------------------------------------------------------------------
//      グリフを取得します.
//-----------------------------------------------------------------------------
Glyph Font::GetGlyph(uint32_t code) const
{ return m_Glyphs.at(code); }

//-----------------------------------------------------------------------------
//      テクスチャを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderResourceView* Font::GetSRV() const
{ return m_SRV.GetPtr(); }


///////////////////////////////////////////////////////////////////////////////
// FontRenderer
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
FontRenderer::FontRenderer()
: m_Font            (nullptr)
, m_Scale           (1.0f)
, m_Color           (1.0f, 1.0f, 1.0f, 1.0f)
, m_MaxSpriteCount  (0)
, m_SpriteCount     (0)
, m_ScreenSize      (1.0f, 1.0f)
, m_Flags           (0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
FontRenderer::~FontRenderer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool FontRenderer::Init
(
    ID3D11Device*   pDevice,
    uint32_t        maxSpriteCount,
    int             w,
    int             h
)
{
    // 頂点シェーダ生成.

    // ピクセルシェーダ生成.

    // 頂点バッファ生成.

    // 定数バッファ生成.

    m_MaxSpriteCount    = maxSpriteCount;
    m_ScreenSize.x      = float(w);
    m_ScreenSize.y      = float(h);
    m_SpriteCount       = 0;
    m_Vertices.resize(m_MaxSpriteCount * kVertexPerSprite);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void FontRenderer::Term()
{
    // 初期値に戻す.
    m_Font              = nullptr;
    m_Scale             = 1.0f;
    m_Color             = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    m_MaxSpriteCount    = 0;
    m_SpriteCount       = 0;
    m_Flags             = 0;

    // メモリ解放.
    m_Vertices.clear();
    m_VS.Reset();
    m_PS.Reset();
    m_VB.Reset();
    m_IB.Reset();
    m_CB.Reset();
    m_IL.Reset();
}

//-----------------------------------------------------------------------------
//      フォントを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetFont(const Font* font)
{ m_Font = font; }

//-----------------------------------------------------------------------------
//      フォントを取得します.
//-----------------------------------------------------------------------------
const Font* FontRenderer::GetFont() const
{ return m_Font; }

//-----------------------------------------------------------------------------
//      カラーを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetColor(float r, float g, float b, float a)
{
    m_Color.x = r;
    m_Color.y = g;
    m_Color.z = b;
    m_Color.w = a;
}

//-----------------------------------------------------------------------------
//      カラーを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetColor(const Vector4& color)
{ m_Color = color; }

//-----------------------------------------------------------------------------
//      カラーを取得します.
//-----------------------------------------------------------------------------
Vector4 FontRenderer::GetColor() const
{ return m_Color; }

//-----------------------------------------------------------------------------
//      スケールを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetScale(float value)
{ m_Scale = value; }

//-----------------------------------------------------------------------------
//      スケールを取得します.
//-----------------------------------------------------------------------------
float FontRenderer::GetScale() const
{ return m_Scale; }

//-----------------------------------------------------------------------------
//      アウトラインフラグを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetOutline(bool value)
{ SetFlag(m_Flags, DRAW_OPTION_OUTLINE, value); }

//-----------------------------------------------------------------------------
//      アウトラインフラグを取得します.
//-----------------------------------------------------------------------------
bool FontRenderer::GetOutline() const
{ return !!(m_Flags & DRAW_OPTION_OUTLINE); }

//-----------------------------------------------------------------------------
//      アウターグローフラグを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetOuterGlow(bool value)
{ SetFlag(m_Flags, DRAW_OPTION_OUTER_GLOW, value); }

//-----------------------------------------------------------------------------
//      アウターグローフラグを取得します.
//-----------------------------------------------------------------------------
bool FontRenderer::GetOuterGlow() const
{ return !!(m_Flags & DRAW_OPTION_OUTER_GLOW); }

//-----------------------------------------------------------------------------
//      ドロップシャドウフラグを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetDropShadow(bool value)
{ SetFlag(m_Flags, DRAW_OPTION_DROP_SHADOW, value); }

//-----------------------------------------------------------------------------
//      ドロップシャドウフラグを取得します.
//-----------------------------------------------------------------------------
bool FontRenderer::GetDropShadow() const
{ return !!(m_Flags & DRAW_OPTION_DROP_SHADOW); }

//-----------------------------------------------------------------------------
//      スクリーンサイズを設定します.
//-----------------------------------------------------------------------------
void FontRenderer::SetScreenSize(int w, int h)
{
    m_ScreenSize.x = float(w);
    m_ScreenSize.y = float(h);
}

//-----------------------------------------------------------------------------
//      スクリーンサイズを取得します.
//-----------------------------------------------------------------------------
Vector2 FontRenderer::GetScreenSize() const
{ return m_ScreenSize; }

//-----------------------------------------------------------------------------
//      描画を開始します.
//-----------------------------------------------------------------------------
void FontRenderer::Begin(ID3D11DeviceContext* pContext)
{
}

//-----------------------------------------------------------------------------
//      文字列を描画します.
//-----------------------------------------------------------------------------
void FontRenderer::DrawString(int x, int y, const char* text)
{ DrawString(x, y, 0, text); }

//-----------------------------------------------------------------------------
//      文字列を描画します.
//-----------------------------------------------------------------------------
void FontRenderer::DrawString(int x, int y, int z, const char* text)
{
    if (m_Font == nullptr)
    { return; }

    auto count = strlen(text);
    if (count == 0)
    { return; }

    auto px = x;
    auto py = y;

    for(size_t i=0; i<count; ++i)
    {
        // 改行コード.
        if (uint32_t(text[i]) == 0x0a)
        {
            px = x;
            py += m_Font->GetHeight();
        }

        auto code = text[i];
        assert(m_Font->HasGlyph(code));

        // グリフ取得.
        auto glyph = m_Font->GetGlyph(code);

        // 頂点バッファ組み立て.
        AddSprite(px, py, glyph);

        // 横方向に移動.
        px += glyph.Advance;
    }
}

//-----------------------------------------------------------------------------
//      フォーマットを指定して文字列を描画します.
//-----------------------------------------------------------------------------
void FontRenderer::DrawFormat(int x, int y, const char* format, ...)
{
    char buffer[2048] = {};

    va_list arg;
    va_start(arg, format);
    vsprintf_s(buffer, format, arg);
    va_end(arg);

    DrawString(x, y, buffer);
}

//-----------------------------------------------------------------------------
//      フォーマットを指定して文字列を描画します.
//-----------------------------------------------------------------------------
void FontRenderer::DrawFormat(int x, int y, int z, const char* format, ...)
{
    char buffer[2048] = {};

    va_list arg;
    va_start(arg, format);
    vsprintf_s(buffer, format, arg);
    va_end(arg);

    DrawString(x, y, z, buffer);
}

//-----------------------------------------------------------------------------
//      描画を終了し，ドローコールを発行します.
//-----------------------------------------------------------------------------
void FontRenderer::End(ID3D11DeviceContext* pContext)
{
    // 描画キック.
}

//-----------------------------------------------------------------------------
//      スプライトを追加します.
//-----------------------------------------------------------------------------
void FontRenderer::AddSprite(int x, int y, const Glyph& glyph)
{
    // 登録できるかチェック.
    if ((m_SpriteCount + 1) > (m_MaxSpriteCount - 1))
    { return; }

    // 頂点データのポインタ取得.
    auto pVertices = &m_Vertices[ m_SpriteCount * kVertexPerSprite ];

    // データ設定.


    // スプライト数をカウントアップ.
}

} // namespace asdx
