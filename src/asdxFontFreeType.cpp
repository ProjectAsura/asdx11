//-----------------------------------------------------------------------------
// File : asdxFont.cpp
// Desc : Font Rasterizer.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <asdxFont.h>
#include <asdxLogger.h>

#include <ft2build.h>
#include FT_FREETYPE_H



namespace {

//-------------------------------------------------------------------------------------------------
//! @brief      ビットマップファイルヘッダーです.
//-------------------------------------------------------------------------------------------------
#pragma pack( push, 1 )
struct BMP_FILE_HEADER
{
    unsigned short      Type;           // ファイルタイプ 'BM'
    unsigned int        Size;           // ファイルサイズ.
    unsigned short      Reserved1;      // 予約領域 (0固定).
    unsigned short      Reserved2;      // 予約領域 (0固定).
    unsigned int        OffBits;        // ファイル先頭から画像データまでのオフセット.
};
#pragma pack( pop )

//-------------------------------------------------------------------------------------------------
//! @brief      ビットマップの圧縮タイプです.
//-------------------------------------------------------------------------------------------------
enum BMP_COMPRESSION_TYPE
{
    BMP_COMPRESSION_RGB       = 0,      // 無圧縮.
    BMP_COMPRESSION_RLE8      = 1,      // RLE圧縮 8 bits/pixel.
    BMP_COMPRESSION_RLE4      = 2,      // RLE圧縮 4 bits/pixel.
    BMP_COMPRESSION_BITFIELDS = 3,      // ビットフィールド.
};

//-------------------------------------------------------------------------------------------------
//! @brief      ビットマップ情報ヘッダーです.
//-------------------------------------------------------------------------------------------------
#pragma pack( push, 1 )
struct BMP_INFO_HEADER
{
    unsigned int        Size;           // ヘッダサイズ (40固定).
    long                Width;          // 画像の横幅.
    long                Height;         // 画像の縦幅.
    unsigned short      Planes;         // プレーン数 (1固定).
    unsigned short      BitCount;       // 1ピクセルあたりのビット数.
    unsigned int        Compression;    // 圧縮形式.
    unsigned int        SizeImage;      // 画像データ部のサイズ.
    long                XPelsPerMeter;  // 横方向の解像度.
    long                YPelsPerMeter;  // 縦方向の解像度.
    unsigned int        ClrUsed;        // 格納されているパレット数.
    unsigned int        ClrImportant;   // 重要なパレットのインデックス.
};
#pragma pack( pop )



//-------------------------------------------------------------------------------------------------
//      BMPファイルヘッダを書き込みます.
//-------------------------------------------------------------------------------------------------
void WriteBmpFileHeader( BMP_FILE_HEADER& header, FILE* pFile )
{
    fwrite( &header.Type,       sizeof(unsigned short), 1, pFile );
    fwrite( &header.Size,       sizeof(unsigned int),   1, pFile );
    fwrite( &header.Reserved1,  sizeof(unsigned short), 1, pFile );
    fwrite( &header.Reserved2,  sizeof(unsigned short), 1, pFile );
    fwrite( &header.OffBits,    sizeof(unsigned int),   1, pFile );
}

//-------------------------------------------------------------------------------------------------
//      BMP情報ヘッダを書き込みます.
//-------------------------------------------------------------------------------------------------
void WriteBmpInfoHeader( BMP_INFO_HEADER& header, FILE* pFile )
{
    fwrite( &header.Size,           sizeof(unsigned int),   1, pFile );
    fwrite( &header.Width,          sizeof(long),           1, pFile );
    fwrite( &header.Height,         sizeof(long),           1, pFile );
    fwrite( &header.Planes,         sizeof(unsigned short), 1, pFile );
    fwrite( &header.BitCount,       sizeof(unsigned short), 1, pFile );
    fwrite( &header.Compression,    sizeof(unsigned int),   1, pFile );
    fwrite( &header.SizeImage,      sizeof(unsigned int),   1, pFile );
    fwrite( &header.XPelsPerMeter,  sizeof(long),           1, pFile );
    fwrite( &header.YPelsPerMeter,  sizeof(long),           1, pFile );
    fwrite( &header.ClrUsed,        sizeof(unsigned int),   1, pFile );
    fwrite( &header.ClrImportant,   sizeof(unsigned int),   1, pFile );
}

//-------------------------------------------------------------------------------------------------
//      BMPファイルに書き出します.
//-------------------------------------------------------------------------------------------------
void WriteBmp( FILE* pFile, const int width, const int height, const uint8_t* pPixel )
{
    BMP_FILE_HEADER fileHeader;
    BMP_INFO_HEADER infoHeader;

    fileHeader.Type      = 'MB';
    fileHeader.Size      = sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER) + ( width * height * 3 );
    fileHeader.Reserved1 = 0;
    fileHeader.Reserved2 = 0;
    fileHeader.OffBits   = sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER);

    infoHeader.Size          = 40;
    infoHeader.Width         = width;
    infoHeader.Height        = height;
    infoHeader.Planes        = 1;
    infoHeader.BitCount      = 24;
    infoHeader.Compression   = BMP_COMPRESSION_RGB;
    infoHeader.SizeImage     = 0;
    infoHeader.XPelsPerMeter = 0;
    infoHeader.YPelsPerMeter = 0;
    infoHeader.ClrUsed       = 0;
    infoHeader.ClrImportant  = 0;

    WriteBmpFileHeader( fileHeader, pFile );
    WriteBmpInfoHeader( infoHeader, pFile );

    for ( int i=0; i<height; ++i )
    {
        for( int j=0; j<width; ++j )
        {
            auto index = ( i * width ) + ( j );

            auto v = pPixel[index];

            {
                fwrite( &v, sizeof(v), 1, pFile );
                fwrite( &v, sizeof(v), 1, pFile );
                fwrite( &v, sizeof(v), 1, pFile );
            }
        }
    }
}

int CalcBaseLine(FT_Face face, uint32_t fontSize)
{
    auto maxY = face->bbox.yMax;
    auto minY = face->bbox.yMin;
    return fontSize * maxY / (maxY - minY);
}

void CopyTo(int x, int y, int baseLine, FT_GlyphSlot glyph, asdx::Font::Bitmap& bitmap)
{
    auto x0 = x + glyph->bitmap_left;
    auto y0 = y + baseLine - glyph->bitmap_top;
    auto x1 = x0 + glyph->bitmap.width;
    auto y1 = y0 + glyph->bitmap.rows;

    if (x0 < 0) { x0 = 0; }
    if (y0 < 0) { y0 = 0; }
    if (x1 > bitmap.Width)  { x1 = bitmap.Width; }
    if (y1 > bitmap.Height) 
    { y1 = bitmap.Height; }

    for(size_t i=y0, r=0; i<y1; ++i, ++r)
    {
        auto w = (x1 - x0);
        auto dst = &bitmap.Pixels[size_t(x0) + i * bitmap.Width];
        auto src = &glyph->bitmap.buffer[r * glyph->bitmap.width];

        memcpy(dst, src, w);
    }
}



} // namespace

namespace asdx {

bool Save(const char* path, const asdx::Font::Bitmap& bitmap)
{
    FILE* pFile;
    auto err = fopen_s(&pFile, path, "wb");
    if (err != 0)
    { return false; }

    WriteBmp(pFile, bitmap.Width, bitmap.Height, bitmap.Pixels.data());

    fclose(pFile);

    return true;
}

void Font::Bitmap::Resize()
{ Pixels.resize(size_t(Width) * size_t(Height)); }

void Font::Bitmap::Clear()
{
    Pixels.clear();
    Pixels.shrink_to_fit();
}


///////////////////////////////////////////////////////////////////////////////
// Font class
///////////////////////////////////////////////////////////////////////////////
struct Font::Body
{
    uint32_t                FontSize;
    FT_Library              Library;
    FT_Face                 Face;
};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
Font::Font()
: m_pBody(nullptr)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
Font::~Font()
{
    // Term()ちゃんと明示的に呼んでね!
    assert(m_pBody == nullptr);
}

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool Font::Init(const char* path, uint32_t fontSize)
{
    m_pBody = new Font::Body();

    {
        auto err = FT_Init_FreeType(&m_pBody->Library);
        if (err != 0)
        {
            ELOGA("Error : FT_Init_FreeType() Failed. errcode = 0x%x", err);
            Term();
            return false;
        }

        err = FT_New_Face(m_pBody->Library, path, 0, &m_pBody->Face);
        if (err != 0)
        {
            ELOGA("Error : FT_New_Face() Failed. errcode = 0x%x", err);
            Term();
            return false;
        }

        FT_Size_RequestRec desc = {};
        desc.type           = FT_SIZE_REQUEST_TYPE_BBOX;
        desc.width          = 0;
        desc.height         = (fontSize << 6);
        desc.horiResolution = 0;
        desc.vertResolution = 0;

        err = FT_Request_Size(m_pBody->Face, &desc);
        if (err != 0)
        {
            ELOGA("Error : FT_Set_Pixel_Sizes() Failed. errcode = 0x%x", err);
            Term();
            return false;
        }

        m_pBody->FontSize = fontSize;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void Font::Term()
{
    if (m_pBody != nullptr)
    {
        delete m_pBody;
        m_pBody = nullptr;
    }
}

//-----------------------------------------------------------------------------
//      フォントサイズを設定します.
//-----------------------------------------------------------------------------
void Font::SetSize(uint32_t fontSize)
{
    FT_Size_RequestRec desc = {};
    desc.type           = FT_SIZE_REQUEST_TYPE_BBOX;
    desc.width          = 0;
    desc.height         = (fontSize << 6);
    desc.horiResolution = 0;
    desc.vertResolution = 0;

    auto err = FT_Request_Size(m_pBody->Face, &desc);
    if (err != 0)
    { return; }

    m_pBody->FontSize = fontSize;
}

//-----------------------------------------------------------------------------
//      ラスタライズに必要なサイズをもめます.
//-----------------------------------------------------------------------------
void Font::CalcSize(const wchar_t* text, uint32_t& w, uint32_t& h) const
{
    w = 0;
    h = 0;
    auto count = wcslen(text);
    if (count == 0)
    { return; }

    auto x = 0u;
    auto y = (m_pBody->Face->size->metrics.height >> 6);
    for(auto i=0; i<count; ++i)
    {
        auto code = int(text[i]);
        if (code == '\n')
        {
            y += (m_pBody->Face->size->metrics.height >> 6);
            w = max(w, x);
            x = 0;
            continue;
        }

        auto err = FT_Load_Char(m_pBody->Face, text[i], FT_LOAD_NO_BITMAP);
        if (err != 0)
        { continue; }

        x += (m_pBody->Face->glyph->advance.x >> 6);
    }

    w = max(x, w);
    h = y;
}

//-----------------------------------------------------------------------------
//      ラスタライズ処理を行います.
//-----------------------------------------------------------------------------
Font::Bitmap Font::Rasterize(const wchar_t* text) const
{
    assert(m_pBody != nullptr);
    Font::Bitmap result;

    // メモリサイズ計算.
    CalcSize(text, result.Width, result.Height);

    // メモリ確保.
    result.Resize();

    // ラスタライズ.
    Rasterize(result, text);

    return result;
}

//-----------------------------------------------------------------------------
//      フォーマットを指定してラスタライズ処理を行います.
//-----------------------------------------------------------------------------
Font::Bitmap Font::RasterizeFormat(const wchar_t* format, ...) const
{
    wchar_t buffer[2048] = {};

    va_list arg;
    va_start(arg, format);
    vswprintf_s(buffer, format, arg);
    va_end(arg);

    return Rasterize(buffer);
}

//-----------------------------------------------------------------------------
//      指定されたメモリにラスタライズ処理を行います.
//-----------------------------------------------------------------------------
void Font::Rasterize(Bitmap& bitmap, const wchar_t* text) const
{
    auto count = wcslen(text);
    if (count == 0)
    { return; }

    auto x = 0;
    auto y = 0;
    auto baseLine = CalcBaseLine(m_pBody->Face, m_pBody->FontSize);

    for(auto i=0; i<count; ++i)
    {
        auto code = int(text[i]);
        if (code == '\n')
        {
            y += m_pBody->Face->size->metrics.height;
            x = 0;
            continue;
        }

        auto err = FT_Load_Char(m_pBody->Face, text[i], FT_LOAD_RENDER);
        if (err != 0)
        { continue; }

        auto glyph = m_pBody->Face->glyph;

        CopyTo(x, y, baseLine, glyph, bitmap);

        x += (glyph->advance.x >> 6);
    }
}

//-----------------------------------------------------------------------------
//      フォーマットを指定して指定されたメモリにラスタライズ処理を行います.
//-----------------------------------------------------------------------------
void Font::RasterizeFormat(Bitmap& bitmap, const wchar_t* format, ...) const
{
    wchar_t buffer[2048] = {};

    va_list arg;
    va_start(arg, format);
    vswprintf_s(buffer, format, arg);
    va_end(arg);

    Rasterize(bitmap, buffer);
}

} // namespace asdx
