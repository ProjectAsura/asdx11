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
#include "../external/imgui/imstb_truetype.h"   // For TTF Font.
#include "../external/imgui/imstb_rectpack.h"   // For Texture Atlas.


namespace {

//-----------------------------------------------------------------------------
// Constant Valules.
//-----------------------------------------------------------------------------
static const uint32_t kBinaryVersion1    = 0x3;
static const size_t   kVertexPerSprite   = 4;
static const size_t   kIndexPerSprite    = 6;

static const uint32_t kGlyphJapanese[] = {
    #include "GlyphJapanese.h"
};

//-----------------------------------------------------------------------------
//      任意の数以上の最小の2のべき乗を求めます.
//-----------------------------------------------------------------------------
int NearPowerOf2(int n)
{
    if (n <= 0)
    { return 0; }

    if ((n & (n -1)) == 0)
    { return n; }

    int ret = 1;
    while(n > 0)
    {
        ret <<= 1;
        n >>= 1;
    }

    return ret;
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
//      ファイルをロードします.
//-----------------------------------------------------------------------------
bool LoadFile(const char* path, std::vector<uint8_t>& binary)
{
    FILE* pFile = nullptr;
    auto err = fopen_s(&pFile, path, "rb");
    if (err != 0)
    {
        return false;
    }

    auto pos = ftell(pFile);
    fseek(pFile, SEEK_END, 0);
    auto end = ftell(pFile);
    fseek(pFile, SEEK_SET, 0);
    auto size = end - pos;

    binary.resize(size);
    fread(binary.data(), size, 1, pFile);
    fclose(pFile);

    return true;
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
: m_Ascent  (0)
, m_Descent (0)
, m_LineGap (0)
, m_Scale   (0.0f)
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
    const char*             path,
    float                   fontSize
)
{
    // テクスチャピクセル.
    std::vector<uint8_t> pixels;

    // テクスチャサイズ.
    int textureWidth  = 0;
    int textureHeight = 0;

    // フォント処理.
    {
        std::vector<uint8_t> binary;
        if (!LoadFile(path, binary))
        {
            ELOGA("Error : LoadFile() Failed. path = %s", path);
            return false;
        }

        auto offset = stbtt_GetFontOffsetForIndex(binary.data(), 0);

        stbtt_fontinfo font;
        if (!stbtt_InitFont(&font, binary.data(), offset))
        {
            ELOGA("Error : stbtt_InitFont() Failed.");
            return false;
        }

        const auto fontScale = stbtt_ScaleForPixelHeight(&font, fontSize);

        int ascent  = 0;
        int descent = 0;
        int lineGap = 0;
        stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);

        m_Ascent    = float(ascent);
        m_Descent   = float(descent);
        m_Scale     = fontScale;
        m_FontSize  = fontSize;
        m_LineGap   = float(lineGap);

        auto padding = 1;
        auto totalSurface = 0;

        auto count = _countof(kGlyphJapanese);

        std::vector<stbrp_rect> rects;
        std::vector<int>        codes;
        rects.reserve(count);
        codes.reserve(count);

        for(auto i=0u; i<count; ++i)
        {
            auto code = int(kGlyphJapanese[i]);
            auto index = stbtt_FindGlyphIndex(&font, code);
            if (index == 0)
                continue;

            int x0 = 0;
            int y0 = 0;
            int x1 = 0;
            int y1 = 0;
            stbtt_GetGlyphBitmapBoxSubpixel(
                &font, index, m_Scale, m_Scale, 0.0f, 0.0f,
                &x0, &y0,
                &x1, &y1);

            auto w = (x1 - x0) + padding;
            auto h = (y1 - y0) + padding;

            stbrp_rect rect = {};
            rect.w  = stbrp_coord(w);
            rect.h  = stbrp_coord(h);
            rects.push_back(rect);
            codes.push_back(int(code));

            totalSurface += (w * h);
        }

        // テクスチャ横幅を計算.
        const auto surfaceSqrt = sqrt(float(totalSurface)) + 1;

        const int kTexHeight = 1024 * 32;
        textureWidth = 512;
        if (surfaceSqrt >= 4096 * 0.7f)
        { textureWidth = 4096; }
        else if (surfaceSqrt >= 2048 * 0.7f)
        { textureWidth = 2048; }
        else if (surfaceSqrt >= 1024 * 0.7f)
        { textureWidth = 1024; }

        // パック開始.
        stbtt_pack_context spc = {};
        stbtt_PackBegin(&spc, nullptr, textureWidth, kTexHeight, 0, padding, nullptr);

        stbrp_pack_rects(reinterpret_cast<stbrp_context*>(&spc.pack_info), rects.data(), rects.size());

        for(size_t i=0; i<rects.size(); ++i)
        {
            if (!rects[i].was_packed)
                continue;

            textureHeight = asdx::Max<int>(textureHeight, rects[i].y + rects[i].h);
        }

        // 2のべき乗にする.
        textureHeight = NearPowerOf2(textureHeight);

        // テクスチャメモリを確保.
        size_t size  = textureWidth;
               size *= textureHeight;
        pixels.resize(size);

        std::vector<stbtt_packedchar> packedChars;
        packedChars.resize(codes.size());

        stbtt_pack_range packRange = {};
        packRange.font_size                         = fontSize;
        packRange.first_unicode_codepoint_in_range  = 0;
        packRange.array_of_unicode_codepoints       = codes.data();
        packRange.num_chars                         = codes.size();
        packRange.chardata_for_range                = packedChars.data();
        packRange.h_oversample                      = 3;
        packRange.v_oversample                      = 1;

        spc.height = textureHeight;
        spc.pixels = pixels.data();

        // ラスタライズ.
        stbtt_PackFontRangesRenderIntoRects(&spc, &font, &packRange, 1, rects.data());

        // パック終了.
        stbtt_PackEnd(&spc);

        rects.clear();

        auto offsetY = floor(m_Ascent + 0.5f);

        // ランタイム用のグリフデータを構築.
        for(size_t i=0; i<codes.size(); ++i)
        {
            const auto code = uint32_t(codes[i]);
            const auto& pc = packedChars[i];

            auto posX = 0.0f;   // ダミー.
            auto posY = 0.0f;   // ダミー.
            stbtt_aligned_quad quad = {};
            stbtt_GetPackedQuad(packedChars.data(), textureWidth, textureHeight, i, &posX, &posY, &quad, 0);

            // グリフを設定.
            Glyph glyph = {};
            glyph.P0.x      = quad.x0;
            glyph.P0.y      = quad.y0 + offsetY;
            glyph.P1.x      = quad.x1;
            glyph.P1.y      = quad.y1 + offsetY;
            glyph.UV0.x     = quad.s0;
            glyph.UV0.y     = quad.t0;
            glyph.UV1.x     = quad.s1;
            glyph.UV1.y     = quad.t1;
            glyph.Advance   = pc.xadvance;

            // グリフ追加.
            m_Glyphs[code] = glyph;
        }
    }

    // D3D11テクスチャを生成.
    {
        D3D11_SUBRESOURCE_DATA res = {};
        res.pSysMem             = pixels.data();
        res.SysMemPitch         = UINT(textureWidth);
        res.SysMemSlicePitch    = UINT(textureWidth * textureHeight);

        auto ret = CreateTexture2D(pDevice, pContext,
            uint32_t(textureWidth),
            uint32_t(textureHeight),
            1,
            1,
            false,
            DXGI_FORMAT_R8_UNORM,
            D3D11_USAGE_DEFAULT,
            D3D11_BIND_SHADER_RESOURCE,
            0,
            0,
            &res, 
            m_Texture.GetAddress(),
            m_SRV.GetAddress());

        if (!ret)
        {
            ELOGA("Error : CreateTexture2D() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      破棄処理を行います.
//-----------------------------------------------------------------------------
void Font::Term()
{
    m_Ascent    = 0;
    m_Descent   = 0;
    m_LineGap   = 0;
    m_Scale     = 0.0f;

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
float Font::GetScale() const
{ return m_Scale; }

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
        if (uint16_t(text[i]) == 0x0a)
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
