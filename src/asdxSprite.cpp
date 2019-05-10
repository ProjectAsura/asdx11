//-------------------------------------------------------------------------------------------------
// File : asdxSprite.cpp
// Desc : Sprite Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxSprite.h>
#include <asdxMisc.h>
#include <asdxLogger.h>
#include <vector>


namespace {

#include "../res/shaders/Compiled/SpriteVS.inc"
#include "../res/shaders/Compiled/SpritePS_Color.inc"
#include "../res/shaders/Compiled/SpritePS_Tex2D.inc"
#include "../res/shaders/Compiled/SpritePS_Tex2DArray.inc"
#include "../res/shaders/Compiled/SpritePS_Tex3D.inc"
#include "../res/shaders/Compiled/SpritePS_TexCube.inc"

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sprite class
///////////////////////////////////////////////////////////////////////////////////////////////////

// 入力要素です.
const D3D11_INPUT_ELEMENT_DESC Sprite::InputElements[ InputElementCount ] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "VTXCOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
Sprite::Sprite()
: m_pVS          ( nullptr )
, m_pPSTex2D     ( nullptr )
, m_pPSTex2DArray( nullptr )
, m_pPSTexCube   ( nullptr )
, m_pPSTex3D     ( nullptr )
, m_pVB          ( nullptr )
, m_pIB          ( nullptr )
, m_pCB          ( nullptr )
, m_pIL          ( nullptr )
, m_pSRV         ( nullptr )
, m_pSmp         ( nullptr )
, m_SpriteCount  ( 0 )
, m_SurfaceIndex ( 0 )
, m_ScreenSize   ( 1.0f, 1.0f )
, m_Color        ( 1.0f, 1.0f, 1.0f, 1.0f )
, m_Transform    ( Matrix::CreateIdentity() )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
Sprite::~Sprite()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool Sprite::Init
(
    ID3D11Device*   pDevice,
    float           screenWidth,
    float           screenHeight
)
{
    HRESULT hr = S_OK;

    // 頂点バッファの生成.
    {
        // 頂点バッファの設定.
        D3D11_BUFFER_DESC desc;
        ZeroMemory( &desc, sizeof( D3D11_BUFFER_DESC ) );
        desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth      = sizeof( Sprite::Vertex ) * NUM_SPRITES * NUM_VERTEX_PER_SPRITE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        // 頂点バッファの生成.
        hr = pDevice->CreateBuffer( &desc, nullptr, m_pVB.GetAddress() );
        if ( FAILED( hr ) )
        {
            // エラーログ出力.
            ELOG( "Error : ID3D11Device::CreateBuffer() Failed." );
            return false;
        }
        //SetDebugObjectName( m_pVB.GetPtr(), "asdx::Sprite::m_pVB" );
    }

    // インデックスバッファの生成.
    {
        // インデックスバッファの設定.
        D3D11_BUFFER_DESC desc;
        ZeroMemory( &desc, sizeof( D3D11_BUFFER_DESC ) );
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.Usage     = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof( uint16_t ) * NUM_SPRITES * NUM_INDEX_PER_SPRITE;

        // インデックスバッファのメモリを確保.
        std::vector< uint16_t > indices;
        indices.reserve( NUM_SPRITES * NUM_INDEX_PER_SPRITE );

        // インデックス設定.
        for( uint16_t i=0; i<NUM_SPRITES * NUM_INDEX_PER_SPRITE; i+=NUM_VERTEX_PER_SPRITE )
        {
            indices.push_back( i + 0 );
            indices.push_back( i + 1 );
            indices.push_back( i + 2 );

            indices.push_back( i + 1 );
            indices.push_back( i + 3 );
            indices.push_back( i + 2 );
        }

        // サブリソースデータの設定.
        D3D11_SUBRESOURCE_DATA res;
        ZeroMemory( &res, sizeof( D3D11_SUBRESOURCE_DATA ) );
        res.pSysMem = &indices.front();

        // インデックスバッファ生成.
        hr = pDevice->CreateBuffer( &desc, &res, m_pIB.GetAddress() );
        if ( FAILED( hr ) )
        {
            // エラーログ出力.
            ELOG( "Error : ID3D11Device::CreateBuffer() Failed." );

            // インデックスをクリア.
            indices.clear();

            // 異常終了.
            return false;
        }
        // デバッグオブジェクト名.
        //SetDebugObjectName( m_pIB.GetPtr(), "asdx::Sprite" );

        // インデックスをクリア.
        indices.clear();
    }

    // 頂点シェーダの生成.
    {
        // 頂点シェーダ生成.
        hr = pDevice->CreateVertexShader( SpriteVS, sizeof( SpriteVS ), nullptr, m_pVS.GetAddress() );
        if ( FAILED( hr ) )
        {
            // エラーログ出力.
            ELOG( "Error : ID3D11Device::CreateVertexShader() Failed." );

            // 異常終了.
            return false;
        }

        // デバッグオブジェクト名を設定.
        //SetDebugObjectName( m_pVS.GetPtr(), "asdx::Sprite::m_pVS" );

        // 入力レイアウトを生成.
        hr = pDevice->CreateInputLayout( Sprite::InputElements, Sprite::InputElementCount, SpriteVS, sizeof( SpriteVS ), m_pIL.GetAddress() );
        if ( FAILED( hr ) )
        {
            // エラーログ出力.
            ELOG( "Error : ID3D11Device::CreateInputLayout() Failed. ");

            // 異常終了.
            return false;
        }
        // デバッグオブジェクト名を設定.
        //SetDebugObjectName( m_pIL.GetPtr(), "asdx::Font::m_pIL" );
    }

    {
        // ピクセルシェーダ生成.
        hr = pDevice->CreatePixelShader( SpritePS_Color, sizeof( SpritePS_Color ), nullptr, m_pPSColor.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreatePixelShader() Failed." );

            return false;
        }

        // デバッグオブジェクト名を設定.
        //SetDebugObjectName( m_pPSColor.GetPtr(), "asdx::Sprite::m_pPSColor" );
    }

    {
        // ピクセルシェーダ生成.
        hr = pDevice->CreatePixelShader( SpritePS_Tex2D, sizeof( SpritePS_Tex2D ), nullptr, m_pPSTex2D.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreatePixelShader() Failed." );

            return false;
        }

        // デバッグオブジェクト名を設定.
        //SetDebugObjectName( m_pPSTex2D.GetPtr(), "asdx::Sprite::m_pPSTex2D" );
    }

    {
        // ピクセルシェーダ生成.
        hr = pDevice->CreatePixelShader( SpritePS_Tex2DArray, sizeof( SpritePS_Tex2DArray ), nullptr, m_pPSTex2DArray.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreatePixelShader() Failed." );

            return false;
        }

        // デバッグオブジェクト名を設定.
        //SetDebugObjectName( m_pPSTex2DArray.GetPtr(), "asdx::Sprite::m_pPSTex2DArray" );
    }

    {
        // ピクセルシェーダ生成.
        hr = pDevice->CreatePixelShader( SpritePS_TexCube, sizeof( SpritePS_TexCube ), nullptr, m_pPSTexCube.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreatePixelShader() Failed." );

            return false;
        }

        // デバッグオブジェクト名を設定.
        //SetDebugObjectName( m_pPSTexCube.GetPtr(), "asdx::Sprite::m_pPSTexCube" );
    }

    {
        // ピクセルシェーダ生成.
        hr = pDevice->CreatePixelShader( SpritePS_Tex3D, sizeof( SpritePS_Tex3D ), nullptr, m_pPSTex3D.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreatePixelShader() Failed." );

            return false;
        }

        // デバッグオブジェクト名を設定.
        //SetDebugObjectName( m_pPSTex3D.GetPtr(), "asdx::Sprite::m_pPSTex3D" );
    }

    // 定数バッファの生成.
    {
        // 定数バッファの設定.
        D3D11_BUFFER_DESC desc;
        ZeroMemory( &desc, sizeof( D3D11_BUFFER_DESC ) );
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.ByteWidth = sizeof( asdx::Matrix );
        desc.Usage     = D3D11_USAGE_DEFAULT;

        // 定数バッファ生成.
        hr = pDevice->CreateBuffer( &desc, nullptr, m_pCB.GetAddress() );
        if ( FAILED( hr ) )
        {
            // エラーログ出力.
            ELOG( "Error : ID3D11Device::CreateBuffer() Failed." );

            // 異常終了.
            return false;
        }
        // デバッグオブジェクト名を設定.
        //SetDebugObjectName( m_pCB.GetPtr(), "asdx::Sprite::m_pCB" );
    }

    SetScreenSize( screenWidth, screenHeight );

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です
//-------------------------------------------------------------------------------------------------
void Sprite::Term()
{
    m_pVS           .Reset();
    m_pPSColor      .Reset();
    m_pPSTex2D      .Reset();
    m_pPSTex2DArray .Reset();
    m_pPSTexCube    .Reset();
    m_pPSTex3D      .Reset();

    m_pVB.Reset();
    m_pIB.Reset();
    m_pCB.Reset();
    m_pIL.Reset();

    m_pSRV = nullptr;
    m_pSmp = nullptr;

    m_ScreenSize.x  = 0.0f;
    m_ScreenSize.y  = 0.0f;
    m_Color         = asdx::Vector4( 1.0f, 1.0f, 1.0f, 1.0f );
    m_SpriteCount   = 0;
    m_SurfaceIndex  = 0;
}

//-------------------------------------------------------------------------------------------------
//      スクリーンサイズを設定します.
//-------------------------------------------------------------------------------------------------
void Sprite::SetScreenSize( float width, float height )
{
    m_ScreenSize.x = width;
    m_ScreenSize.y = height;

    float xScale = ( m_ScreenSize.x > 0.0f ) ? 2.0f / m_ScreenSize.x : 0.0f;
    float yScale = ( m_ScreenSize.y > 0.0f ) ? 2.0f / m_ScreenSize.y : 0.0f;

    m_Transform = Matrix(
        xScale,     0.0f,   0.0f,   0.0f,
         0.0f,   -yScale,   0.0f,   0.0f,
         0.0f,      0.0f,   1.0f,   0.0f,
        -1.0f,      1.0f,   0.0f,   1.0f );
}

//-------------------------------------------------------------------------------------------------
//      頂点カラーを設定します.
//-------------------------------------------------------------------------------------------------
void Sprite::SetColor( float r, float g, float b, float a )
{
    m_Color.x = r;
    m_Color.y = g;
    m_Color.z = b;
    m_Color.w = a;
}

//-------------------------------------------------------------------------------------------------
//      サーフェイスインデックスを設定します.
//-------------------------------------------------------------------------------------------------
void Sprite::SetSurfaceIndex( int index )
{ m_SurfaceIndex = index; }

//-------------------------------------------------------------------------------------------------
//      テクスチャを設定します.
//-------------------------------------------------------------------------------------------------
void Sprite::SetTexture( ID3D11ShaderResourceView* pSRV, ID3D11SamplerState* pSmp )
{
    m_pSRV = pSRV;
    m_pSmp = pSmp;
}

//-------------------------------------------------------------------------------------------------
//      スクリーンサイズを取得します.
//-------------------------------------------------------------------------------------------------
asdx::Vector2 Sprite::GetScreenSize() const
{ return m_ScreenSize; }

//-------------------------------------------------------------------------------------------------
//      頂点カラーを取得します.
//-------------------------------------------------------------------------------------------------
asdx::Vector4 Sprite::GetColor() const
{ return m_Color; }

//-------------------------------------------------------------------------------------------------
//      サーフェイスインデックスを取得します.
//-------------------------------------------------------------------------------------------------
int Sprite::GetSurfaceIndex() const
{ return m_SurfaceIndex; }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* Sprite::GetSRV() const
{ return m_pSRV; }

//-------------------------------------------------------------------------------------------------
//      サンプラーステートを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11SamplerState* Sprite::GetSmp() const
{ return m_pSmp; }

//-------------------------------------------------------------------------------------------------
//      描画開始処理です.
//-------------------------------------------------------------------------------------------------
void Sprite::Begin( ID3D11DeviceContext* pDeviceContext, Sprite::SHADER_TYPE type )
{
    // スプライト数をリセット.
    m_SpriteCount = 0;

    // プリミティブトポロジーを設定します.
    pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // 入力レイアウトを設定します.
    pDeviceContext->IASetInputLayout( m_pIL.GetPtr() );

    ID3D11VertexShader* pVS = ( type == SHADER_TYPE_NONE ) ? nullptr : m_pVS.GetPtr();
    ID3D11PixelShader*  pPS = nullptr;
    auto textureMap = true;

    switch( type )
    {
    case SHADER_TYPE_NONE:
        { textureMap = false; }
        break;

    case SHADER_TYPE_COLOR:
        { 
            pPS = m_pPSColor.GetPtr();
            textureMap = false;
        }
        break;

    case SHADER_TYPE_TEXTURE2D:
        { pPS = m_pPSTex2D.GetPtr(); }
        break;

    case SHADER_TYPE_TEXTURE2DARRAY:
        { pPS = m_pPSTex2DArray.GetPtr(); }
        break;

    case SHADER_TYPE_TEXTURECUBE:
        { pPS = m_pPSTexCube.GetPtr(); }
        break;

    case SHADER_TYPE_TEXTURE3D:
        { pPS = m_pPSTex3D.GetPtr(); }
        break;
    }

    // シェーダを設定します.
    pDeviceContext->VSSetShader( pVS,     nullptr, 0 );
    pDeviceContext->PSSetShader( pPS,     nullptr, 0 );
    pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
    pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
    pDeviceContext->DSSetShader( nullptr, nullptr, 0 );

    uint32_t stride = sizeof( Sprite::Vertex );
    uint32_t offset = 0;

    // 頂点バッファとインデックスバッファを設定します.
    pDeviceContext->IASetVertexBuffers( 0, 1, m_pVB.GetAddress(), &stride, &offset );
    pDeviceContext->IASetIndexBuffer( m_pIB.GetPtr(), DXGI_FORMAT_R16_UINT, 0 );

    // 定数バッファを更新し，設定します.
    pDeviceContext->UpdateSubresource( m_pCB.GetPtr(), 0, nullptr, &m_Transform, 0, 0 );
    pDeviceContext->VSSetConstantBuffers( 0, 1, m_pCB.GetAddress() );

    if ( textureMap && m_pSRV != nullptr && m_pSmp != nullptr )
    {
        pDeviceContext->PSSetSamplers( 0, 1, &m_pSmp );
        pDeviceContext->PSSetShaderResources( 0, 1, &m_pSRV );
    }
}

//-------------------------------------------------------------------------------------------------
//      頂点バッファを更新します.
//-------------------------------------------------------------------------------------------------
void Sprite::Draw( const int x, const int y, const int w, const int h )
{ Draw( x, y, w, h, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), 0 ); }

//-------------------------------------------------------------------------------------------------
//      頂点バッファを更新します.
//-------------------------------------------------------------------------------------------------
void Sprite::Draw( const int x, const int y, const int w, const int h, const Vector2& uv0, const Vector2& uv1 )
{ Draw( x, y, w, h, uv0, uv1, 0 ); }

//-------------------------------------------------------------------------------------------------
//      頂点バッファを更新します.
//-------------------------------------------------------------------------------------------------
void Sprite::Draw( const int x, const int y, const int w, const int h, const int layerDepth )
{ Draw( x, y, w, h, Vector2( 0.0f, 0.0f ), Vector2( 1.0f, 1.0f ), layerDepth ); }

//-------------------------------------------------------------------------------------------------
//      頂点バッファを更新します.
//-------------------------------------------------------------------------------------------------
void Sprite::Draw( const int x, const int y, const int w, const int h, const Vector2& uv0, const Vector2& uv1, const int layerDepth )
{
    // 最大スプライト数を超えないかチェック.
    if ( m_SpriteCount + 1 > NUM_SPRITES )
    { return; }

    float posX = static_cast<float>( x );
    float posY = static_cast<float>( y );

    float width   = static_cast<float>( w );
    float height  = static_cast<float>( h );
    float depth   = static_cast<float>( layerDepth );
    float surface = static_cast<float>( m_SurfaceIndex );

    // 頂点データのポインタ取得.
    Sprite::Vertex* pVertices = &m_Vertices[ m_SpriteCount * NUM_VERTEX_PER_SPRITE ];

    // テクスチャ座標
    float u0 = uv0.x;
    float u1 = uv1.x;
    float v0 = uv0.y;
    float v1 = uv1.y;

    // 位置座標.
    float x0 = posX;
    float x1 = posX + width;
    float y0 = posY;
    float y1 = posY + height;

    // Vertex : 0
    pVertices[ 0 ].Position.x = x0;
    pVertices[ 0 ].Position.y = y0;
    pVertices[ 0 ].Position.z = depth;
    pVertices[ 0 ].Color      = m_Color;
    pVertices[ 0 ].TexCoord.x = u0;
    pVertices[ 0 ].TexCoord.y = v1;
    pVertices[ 0 ].TexCoord.z = surface;

    // Vertex : 1
    pVertices[ 1 ].Position.x = x1;
    pVertices[ 1 ].Position.y = y0;
    pVertices[ 1 ].Position.z = depth;
    pVertices[ 1 ].Color      = m_Color;
    pVertices[ 1 ].TexCoord.x = u1;
    pVertices[ 1 ].TexCoord.y = v1;
    pVertices[ 1 ].TexCoord.z = surface;

    // Vertex : 2
    pVertices[ 2 ].Position.x = x0;
    pVertices[ 2 ].Position.y = y1;
    pVertices[ 2 ].Position.z = depth;
    pVertices[ 2 ].Color      = m_Color;
    pVertices[ 2 ].TexCoord.x = u0;
    pVertices[ 2 ].TexCoord.y = v0;
    pVertices[ 2 ].TexCoord.z = surface;

    // Vertex : 3
    pVertices[ 3 ].Position.x = x1;
    pVertices[ 3 ].Position.y = y1;
    pVertices[ 3 ].Position.z = depth;
    pVertices[ 3 ].Color      = m_Color;
    pVertices[ 3 ].TexCoord.x = u1;
    pVertices[ 3 ].TexCoord.y = v0;
    pVertices[ 3 ].TexCoord.z = surface;

    // スプライト数をカウントアップします.
    m_SpriteCount++;
}

//-------------------------------------------------------------------------------------------------
//      描画終了処理です.
//-------------------------------------------------------------------------------------------------
void Sprite::End( ID3D11DeviceContext* pDeviceContext )
{
    D3D11_MAPPED_SUBRESOURCE mappedBuffer;

    // マップします.
    HRESULT hr = pDeviceContext->Map( m_pVB.GetPtr(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer );
    if ( FAILED( hr ) )
    { return; }

    // 頂点データのポインタ取得.
    Sprite::Vertex* pVertices = (Sprite::Vertex*)mappedBuffer.pData;

    // がばっとコピる.
    memcpy( pVertices, m_Vertices, sizeof( Sprite::Vertex ) * m_SpriteCount * NUM_VERTEX_PER_SPRITE );

    // アンマップします.
    pDeviceContext->Unmap( m_pVB.GetPtr(), 0 );

    // インデックス数.
    uint32_t indexCount = m_SpriteCount * NUM_INDEX_PER_SPRITE;

    // スプライトを描画.
    pDeviceContext->DrawIndexed( indexCount, 0, 0 );

    // シェーダをアンバイド.
    pDeviceContext->VSSetShader( nullptr, nullptr, 0 );
    pDeviceContext->PSSetShader( nullptr, nullptr, 0 );
    pDeviceContext->GSSetShader( nullptr, nullptr, 0 );
    pDeviceContext->HSSetShader( nullptr, nullptr, 0 );
    pDeviceContext->DSSetShader( nullptr, nullptr, 0 );
 
    ID3D11ShaderResourceView* pNullSRV[ 1 ] = { nullptr };
    ID3D11SamplerState*       pNullSmp[ 1 ] = { nullptr };
    pDeviceContext->PSSetShaderResources( 0, 1, pNullSRV );
    pDeviceContext->PSSetSamplers( 0, 1, pNullSmp );
}


} // namespace asdx