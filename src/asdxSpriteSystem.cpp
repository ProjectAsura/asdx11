//-----------------------------------------------------------------------------
// File : SpriteSystem.cpp
// Desc : Sprite Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxMisc.h>
#include <asdxLogger.h>
#include <asdxSpriteSystem.h>


namespace {

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
#include "../res/shaders/Compiled/SpriteVS.inc"
#include "../res/shaders/Compiled/SpritePS.inc"

constexpr uint32_t kSpriteCount             = 1024;
constexpr uint32_t kBatchCount              = 512;
constexpr uint32_t kVertexCountPerSprite    = 4;
constexpr uint32_t kIndexCountPerSprite     = 6;

// 入力要素です.
static const D3D11_INPUT_ELEMENT_DESC kInputElements[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

} // namespace


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SpriteSystem class
///////////////////////////////////////////////////////////////////////////////
SpriteSystem SpriteSystem::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SpriteSystem::SpriteSystem()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SpriteSystem::~SpriteSystem()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
SpriteSystem& SpriteSystem::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理です.
//-----------------------------------------------------------------------------
bool SpriteSystem::Init(ID3D11Device* pDevice, float w, float h)
{
    HRESULT hr = S_OK;

    // 頂点バッファの生成.
    {
        auto count = kSpriteCount * kVertexCountPerSprite;
        m_Vertices.resize(count);

        D3D11_BUFFER_DESC desc = {};
        desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth      = sizeof(Vertex) * count;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        hr = pDevice->CreateBuffer(&desc, nullptr, m_pVB.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateBuffer() Failed.");
            return false;
        }
    }

    // インデックスバッファの生成.
    {
        auto count = kSpriteCount * kIndexCountPerSprite;

        D3D11_BUFFER_DESC desc = {};
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.Usage     = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(uint16_t) * count;

        std::vector<uint16_t> indices;
        indices.reserve(count);

        // インデックス設定.
        for(uint16_t i=0; i<count; i+=kVertexCountPerSprite)
        {
            indices.push_back( i + 0 );
            indices.push_back( i + 1 );
            indices.push_back( i + 2 );

            indices.push_back( i + 1 );
            indices.push_back( i + 3 );
            indices.push_back( i + 2 );
        }

        D3D11_SUBRESOURCE_DATA res = {};
        res.pSysMem = &indices.front();

        hr = pDevice->CreateBuffer(&desc, &res, m_pIB.GetAddress());
        if (FAILED(hr))
        {
            ELOG( "Error : ID3D11Device::CreateBuffer() Failed." );
            indices.clear();
            return false;
        }

        // インデックスをクリア.
        indices.clear();
    }

    // 定数バッファの生成.
    {
        // 定数バッファの設定.
        D3D11_BUFFER_DESC desc = {};
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.ByteWidth = sizeof(asdx::Matrix);
        desc.Usage     = D3D11_USAGE_DEFAULT;

        // 定数バッファ生成.
        hr = pDevice->CreateBuffer( &desc, nullptr, m_pCB.GetAddress() );
        if (FAILED(hr))
        {
            ELOG( "Error : ID3D11Device::CreateBuffer() Failed." );
            return false;
        }
    }

    // グラフィックスパイプラインステート生成.
    {
        asdx::GraphicsPipelineStateDesc desc;

        desc.VS                 = { SpriteVS, sizeof(SpriteVS) };
        desc.PS                 = { SpritePS, sizeof(SpritePS) };
        desc.BlendState         = asdx::GetBlendDesc(asdx::AlphaBlend);
        desc.RasterizerState    = asdx::GetRasterizerDesc(asdx::CullBack);
        desc.DepthStencilState  = asdx::GetDepthStencilDesc(asdx::Default);
        desc.InputLayout        = { kInputElements, _countof(kInputElements) };
        desc.PrimitiveTopology  = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        if (!m_PSO.Init(pDevice, desc))
        {
            ELOG("Error : PipelineState::Init() Failed.");
            return false;
        }
    }

    // サンプラーステート生成.
    {
        auto desc = asdx::GetSamplerDesc(asdx::PointClamp);
        hr = pDevice->CreateSamplerState(&desc, m_pSmp.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateSamplerState() Failed.");
            return false;
        }
    }

    // バッチメモリ確保.
    {
        m_BatchCount = 0;
        m_Batches.resize(kBatchCount);
    }

    SetScreenSize(w, h);

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理です
//-----------------------------------------------------------------------------
void SpriteSystem::Term()
{
    m_pVB .Reset();
    m_pIB .Reset();
    m_pCB .Reset();
    m_pSmp.Reset();
    m_PSO .Term();

    m_ScreenSize.x  = 0.0f;
    m_ScreenSize.y  = 0.0f;
    m_Color         = {255, 255, 255, 255};
    m_SpriteCount   = 0;
    m_BatchCount    = 0;

    m_Vertices.clear();
    m_Batches .clear();
}

//-----------------------------------------------------------------------------
//      シェーダリソースビューを設定します.
//-----------------------------------------------------------------------------
void SpriteSystem::SetSRV(ID3D11ShaderResourceView* pSRV)
{
    if (pSRV == nullptr)
    { return; }

    if (pSRV == m_pSRV)
    { return; }

    auto index = m_BatchCount;
    m_BatchCount++;
    m_Batches[index].IndexCount  = 0;
    m_Batches[index].IndexOffset = m_SpriteCount * kIndexCountPerSprite;
    m_Batches[index].pSRV        = pSRV;

    m_pSRV = pSRV;
}

//-----------------------------------------------------------------------------
//      スクリーンサイズを設定します.
//-----------------------------------------------------------------------------
void SpriteSystem::SetScreenSize( float width, float height )
{
    m_ScreenSize.x = width;
    m_ScreenSize.y = height;

    float xScale = ( m_ScreenSize.x > 0.0f ) ? 2.0f / m_ScreenSize.x : 0.0f;
    float yScale = ( m_ScreenSize.y > 0.0f ) ? 2.0f / m_ScreenSize.y : 0.0f;

    m_Transform = asdx::Matrix(
        xScale,     0.0f,   0.0f,   0.0f,
         0.0f,   -yScale,   0.0f,   0.0f,
         0.0f,      0.0f,   1.0f,   0.0f,
        -1.0f,      1.0f,   0.0f,   1.0f );
}

//-----------------------------------------------------------------------------
//      頂点カラーを設定します.
//-----------------------------------------------------------------------------
void SpriteSystem::SetColor( float r, float g, float b, float a )
{
    m_Color.r = asdx::Clamp<uint8_t>(uint8_t(r * 255.0f), 0, 255);
    m_Color.g = asdx::Clamp<uint8_t>(uint8_t(g * 255.0f), 0, 255);
    m_Color.b = asdx::Clamp<uint8_t>(uint8_t(b * 255.0f), 0, 255);
    m_Color.a = asdx::Clamp<uint8_t>(uint8_t(a * 255.0f), 0, 255);
}

//-----------------------------------------------------------------------------
//      頂点カラーを設定します.
//-----------------------------------------------------------------------------
void SpriteSystem::SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    m_Color.r = r;
    m_Color.g = g;
    m_Color.b = b;
    m_Color.a = a;
}

//-----------------------------------------------------------------------------
//      スクリーンサイズを取得します.
//-----------------------------------------------------------------------------
const asdx::Vector2& SpriteSystem::GetScreenSize() const
{ return m_ScreenSize; }

//-----------------------------------------------------------------------------
//      頂点カラーを取得します.
//-----------------------------------------------------------------------------
asdx::Vector4 SpriteSystem::GetColor() const
{
    return asdx::Vector4(
        asdx::Saturate(m_Color.r / 255.0f),
        asdx::Saturate(m_Color.g / 255.0f),
        asdx::Saturate(m_Color.b / 255.0f),
        asdx::Saturate(m_Color.a / 255.0f));
}

//-----------------------------------------------------------------------------
//      描画開始処理です.
//-----------------------------------------------------------------------------
void SpriteSystem::Reset()
{
    m_SpriteCount = 0;
    m_BatchCount  = 0;
    m_SubmitCount = 0;

    m_Batches[0].IndexCount  = 0;
    m_Batches[0].IndexOffset = 0;
    m_Batches[0].pSRV        = nullptr;

    m_pSRV  = nullptr;
    m_Color = {255, 255, 255, 255};
}

//-------------------------------------------------------------------------------------------------
//      頂点バッファを更新します.
//-------------------------------------------------------------------------------------------------
void SpriteSystem::Draw(int x, int y, int w, int h, const asdx::Vector2& uv0, const asdx::Vector2& uv1, int layer)
{
    // 最大スプライト数を超えないかチェック.
    if (m_SpriteCount + 1 > kSpriteCount)
    { return; }

    // テクスチャ設定済みかどうかチェック.
    if (m_BatchCount == 0)
    { return; }

    auto& batch = m_Batches[m_BatchCount - 1];
    batch.IndexCount += kIndexCountPerSprite;

    // 頂点データのポインタ取得.
    auto pVertices = &m_Vertices[m_SpriteCount * kVertexCountPerSprite];

    // テクスチャ座標
    float u0 = uv0.x;
    float u1 = uv1.x;
    float v0 = uv0.y;
    float v1 = uv1.y;

    float d = float(layer);

    // 位置座標.
    float x0 = float(x);
    float x1 = float(x + w);
    float y0 = float(y);
    float y1 = float(y + h);

    // Vertex : 0
    pVertices[ 0 ].Position.x = x0;
    pVertices[ 0 ].Position.y = y0;
    pVertices[ 0 ].Position.z = d;
    pVertices[ 0 ].Color      = m_Color;
    pVertices[ 0 ].TexCoord.x = u0;
    pVertices[ 0 ].TexCoord.y = v1;

    // Vertex : 1
    pVertices[ 1 ].Position.x = x1;
    pVertices[ 1 ].Position.y = y0;
    pVertices[ 1 ].Position.z = d;
    pVertices[ 1 ].Color      = m_Color;
    pVertices[ 1 ].TexCoord.x = u1;
    pVertices[ 1 ].TexCoord.y = v1;

    // Vertex : 2
    pVertices[ 2 ].Position.x = x0;
    pVertices[ 2 ].Position.y = y1;
    pVertices[ 2 ].Position.z = d;
    pVertices[ 2 ].Color      = m_Color;
    pVertices[ 2 ].TexCoord.x = u0;
    pVertices[ 2 ].TexCoord.y = v0;

    // Vertex : 3
    pVertices[ 3 ].Position.x = x1;
    pVertices[ 3 ].Position.y = y1;
    pVertices[ 3 ].Position.z = d;
    pVertices[ 3 ].Color      = m_Color;
    pVertices[ 3 ].TexCoord.x = u1;
    pVertices[ 3 ].TexCoord.y = v0;

    // スプライト数をカウントアップします.
    m_SpriteCount++;
}

//-------------------------------------------------------------------------------------------------
//      描画終了処理です.
//-------------------------------------------------------------------------------------------------
void SpriteSystem::Submit(ID3D11DeviceContext* pContext)
{
    D3D11_MAPPED_SUBRESOURCE res = {};

    // 頂点バッファ更新.
    auto hr = pContext->Map(m_pVB.GetPtr(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    if (SUCCEEDED(hr))
    { 
        auto pVertices = reinterpret_cast<Vertex*>(res.pData);
        memcpy(pVertices, m_Vertices.data(), sizeof(Vertex) * m_SpriteCount * kVertexCountPerSprite);
    }
    pContext->Unmap( m_pVB.GetPtr(), 0 );

    uint32_t stride = sizeof(Vertex);
    uint32_t offset = 0;

    // 頂点バッファとインデックスバッファを設定します.
    pContext->IASetVertexBuffers(0, 1, m_pVB.GetAddress(), &stride, &offset);
    pContext->IASetIndexBuffer(m_pIB.GetPtr(), DXGI_FORMAT_R16_UINT, 0);

    // 定数バッファを更新し，設定します.
    pContext->UpdateSubresource(m_pCB.GetPtr(), 0, nullptr, &m_Transform, 0, 0);
    pContext->VSSetConstantBuffers(0, 1, m_pCB.GetAddress());

    pContext->PSSetSamplers(0, 1, m_pSmp.GetAddress());

    m_PSO.Bind(pContext);
    for(auto i=m_SubmitCount; i<m_BatchCount; ++i)
    {
        auto& batch = m_Batches[i];
        pContext->PSSetShaderResources(0, 1, &batch.pSRV);
        pContext->DrawIndexed(batch.IndexCount, batch.IndexOffset, 0);
        m_SubmitCount++;
    }
    m_PSO.Unbind(pContext);
}

} // namespace asdx
