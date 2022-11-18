//-----------------------------------------------------------------------------
// File : asdx.cpp
// Desc : Sky Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxSky.h>
#include <asdxLogger.h>
#include <vector>


namespace {

#include "../res/shaders/Compiled/SkyBoxVS.inc"
#include "../res/shaders/Compiled/SkyBoxPS.inc"
#include "../res/shaders/Compiled/SkySphereVS.inc"
#include "../res/shaders/Compiled/SkySpherePS.inc"
#include "../res/shaders/Compiled/SkySphereFlowPS.inc"

///////////////////////////////////////////////////////////////////////////////
// CbSkyBox structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(16) CbSkyBox
{
    asdx::Matrix    World;          //!< ワールド行列.
    asdx::Matrix    View;           //!< ビュー行列.
    asdx::Matrix    Proj;           //!< 射影行列.
    asdx::Vector3   CameraPos;      //!< カメラ位置.
    float           BoxSize;        //!< スカイボックスサイズ.
};

///////////////////////////////////////////////////////////////////////////////
// CbSkySphere structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(16) CbSkySphere
{
    asdx::Matrix    World;          //!< ワールド行列.
    asdx::Matrix    View;           //!< ビュー行列.
    asdx::Matrix    Proj;           //!< 射影行列.
    float           SphereSize;     //!< スカイスフィアサイズ.
    float           Padding[3];
};

///////////////////////////////////////////////////////////////////////////////
// CbSkySphereFlow structure
///////////////////////////////////////////////////////////////////////////////
struct alignas(16) CbSkySphereFlow
{
    asdx::Vector3   WindDirection;  //!< 風の方向.
    float           Padding0;
    asdx::Vector2   Offset;         //!< オフセット.
};

///////////////////////////////////////////////////////////////////////////////
// Vertex structure
///////////////////////////////////////////////////////////////////////////////
struct Vertex
{
    asdx::Vector3   Position;       //!< 位置座標です.
    asdx::Vector2   Texcoord;       //!< テクスチャ座標です.
    asdx::Vector3   Normal;         //!< 法線ベクトルです.
    asdx::Vector3   Tangent;        //!< 接線ベクトルです.
};

} // namespace

namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// SkyBox class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SkyBox::SkyBox()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SkyBox::~SkyBox()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SkyBox::Init(ID3D11Device* pDevice)
{
    HRESULT hr = S_OK;

    // 頂点シェーダ・入力レイアウトの生成.
    {
        hr = pDevice->CreateVertexShader( SkyBoxVS, sizeof(SkyBoxVS), nullptr, m_pVS.GetAddress() );
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateVertexShader() Failed.");
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA }
        };

        hr = pDevice->CreateInputLayout( elements, 1, SkyBoxVS, sizeof(SkyBoxVS), m_pIL.GetAddress() );
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateInputLayout() Failed.");
            return false;
        }
    }

    // ピクセルシェーダの生成.
    {
        hr = pDevice->CreatePixelShader( SkyBoxPS, sizeof(SkyBoxPS), nullptr, m_pPS.GetAddress() );
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreatePixelShader() Failed.");
            return false;
        }
    }

    // 定数バッファの生成.
    {
        D3D11_BUFFER_DESC desc = {};
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.ByteWidth = sizeof(CbSkyBox);
        desc.Usage     = D3D11_USAGE_DEFAULT;

        hr = pDevice->CreateBuffer( &desc, nullptr, m_pCB.GetAddress() );
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateBuffer() Failed.");
            return false;
        }
    }

    // 頂点バッファの生成.
    {
        Vector3 vertices[] = {
            asdx::Vector3(-1.0f,  1.0f, -1.0f),
            asdx::Vector3(-1.0f, -1.0f, -1.0f),
            asdx::Vector3( 1.0f, -1.0f, -1.0f),

            asdx::Vector3(-1.0f,  1.0f, -1.0f),
            asdx::Vector3( 1.0f, -1.0f, -1.0f),
            asdx::Vector3( 1.0f,  1.0f, -1.0f),

            asdx::Vector3(1.0f,  1.0f, -1.0f),
            asdx::Vector3(1.0f, -1.0f, -1.0f),
            asdx::Vector3(1.0f, -1.0f,  1.0f),

            asdx::Vector3(1.0f,  1.0f, -1.0f),
            asdx::Vector3(1.0f, -1.0f,  1.0f),
            asdx::Vector3(1.0f,  1.0f,  1.0f),

            asdx::Vector3( 1.0f,  1.0f, 1.0f),
            asdx::Vector3( 1.0f, -1.0f, 1.0f),
            asdx::Vector3(-1.0f, -1.0f, 1.0f),

            asdx::Vector3( 1.0f,  1.0f, 1.0f),
            asdx::Vector3(-1.0f, -1.0f, 1.0f),
            asdx::Vector3(-1.0f,  1.0f, 1.0f),

            asdx::Vector3(-1.0f,  1.0f,  1.0f),
            asdx::Vector3(-1.0f, -1.0f,  1.0f),
            asdx::Vector3(-1.0f, -1.0f, -1.0f),

            asdx::Vector3(-1.0f,  1.0f,  1.0f),
            asdx::Vector3(-1.0f, -1.0f, -1.0f),
            asdx::Vector3(-1.0f,  1.0f, -1.0f),

            asdx::Vector3(-1.0f, 1.0f,  1.0f),
            asdx::Vector3(-1.0f, 1.0f, -1.0f),
            asdx::Vector3( 1.0f, 1.0f, -1.0f),

            asdx::Vector3(-1.0f, 1.0f,  1.0f),
            asdx::Vector3( 1.0f, 1.0f, -1.0f),
            asdx::Vector3( 1.0f, 1.0f,  1.0f),

            asdx::Vector3(-1.0f, -1.0f, -1.0f),
            asdx::Vector3(-1.0f, -1.0f,  1.0f),
            asdx::Vector3( 1.0f, -1.0f,  1.0f),

            asdx::Vector3(-1.0f, -1.0f, -1.0f),
            asdx::Vector3( 1.0f, -1.0f,  1.0f),
            asdx::Vector3( 1.0f, -1.0f, -1.0f),
        };

        auto vertexCount = uint32_t(sizeof(vertices) / sizeof(vertices[0]));
 
        D3D11_BUFFER_DESC desc = {};
        desc.Usage      = D3D11_USAGE_DEFAULT;
        desc.ByteWidth  = sizeof( Vector3 ) * vertexCount;
        desc.BindFlags  = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA res = {};
        res.pSysMem = &vertices[0];

        hr = pDevice->CreateBuffer( &desc, &res, m_pVB.GetAddress() );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateBuffer() Failed." );
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SkyBox::Term()
{
    m_pVB.Reset();
    m_pCB.Reset();
    m_pIL.Reset();
    m_pVS.Reset();
    m_pPS.Reset();
}

//-----------------------------------------------------------------------------
//      描画時の処理です.
//-----------------------------------------------------------------------------
void SkyBox::Draw
(
    ID3D11DeviceContext*        pContext,
    ID3D11ShaderResourceView*   pSRV,
    ID3D11SamplerState*         pSmp,
    float                       boxSize,
    const Vector3&              cameraPos,
    const Matrix&               view,
    const Matrix&               proj
)
{
    auto stride = uint32_t(sizeof(Vector3));
    auto offset = 0u;

    pContext->VSSetShader(m_pVS.GetPtr(), nullptr, 0);
    pContext->PSSetShader(m_pPS.GetPtr(), nullptr, 0);
    pContext->GSSetShader(nullptr, nullptr, 0);
    pContext->HSSetShader(nullptr, nullptr, 0);
    pContext->DSSetShader(nullptr, nullptr, 0);

    auto pCB = m_pCB.GetPtr();
    {
        CbSkyBox buf = {};
        buf.World = Matrix::CreateTranslation(cameraPos);
        buf.View  = view;
        buf.Proj  = proj;
        buf.CameraPos = cameraPos;
        buf.BoxSize   = boxSize;

        pContext->UpdateSubresource(pCB, 0, nullptr, &buf, 0, 0);
    }

    pContext->VSSetConstantBuffers(0, 1, &pCB);
    pContext->PSSetShaderResources(0, 1, &pSRV);
    pContext->PSSetSamplers(0, 1, &pSmp);

    pContext->IASetInputLayout(m_pIL.GetPtr());
    pContext->IASetVertexBuffers(0, 1, m_pVB.GetAddress(), &stride, &offset);
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pContext->Draw(36, 0);

    ID3D11ShaderResourceView* pNullSRV[1] = { nullptr };
    ID3D11SamplerState*       pNullSmp[1] = { nullptr };
    pContext->PSSetShaderResources(0, 1, pNullSRV);
    pContext->PSSetSamplers(0, 1, pNullSmp);
    pContext->VSSetShader(nullptr, nullptr, 0);
    pContext->PSSetShader(nullptr, nullptr, 0);
}


///////////////////////////////////////////////////////////////////////////////
// SkySphere class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
SkySphere::SkySphere()
{ 
    ResetFlow();
}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
SkySphere::~SkySphere()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool SkySphere::Init(ID3D11Device* pDevice, int tessellation)
{
    HRESULT hr = S_OK;

    // 頂点シェーダ・入力レイアウトの生成.
    {
        hr = pDevice->CreateVertexShader(SkySphereVS, sizeof(SkySphereVS), nullptr, m_pVS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateVertexShader() Failed.");
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA },
            { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA }
        };

        hr = pDevice->CreateInputLayout(elements, 4, SkySphereVS, sizeof(SkySphereVS), m_pIL.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateInputLayout() Failed.");
            return false;
        }
    }

    // ピクセルシェーダの生成.
    {
        hr = pDevice->CreatePixelShader(SkySpherePS, sizeof(SkySpherePS), nullptr, m_pPS.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreatePixelShader() Failed.");
            return false;
        }
    }

    // フロー用ピクセルシェーダの生成.
    {
        hr = pDevice->CreatePixelShader(SkySphereFlowPS, sizeof(SkySphereFlowPS), nullptr, m_pPSFlow.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreatePixelShader() Failed.");
            return false;
        }
    }

    // 定数バッファの生成.
    {
        D3D11_BUFFER_DESC desc = {};
        desc.BindFlags  = D3D11_BIND_CONSTANT_BUFFER;
        desc.ByteWidth  = sizeof(CbSkySphere);
        desc.Usage      = D3D11_USAGE_DEFAULT;

        hr = pDevice->CreateBuffer(&desc, nullptr, m_pCB.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateBuffer() Failed.");
            return false;
        }
    }

    // 定数バッファの生成.
    {
        D3D11_BUFFER_DESC desc = {};
        desc.BindFlags  = D3D11_BIND_CONSTANT_BUFFER;
        desc.ByteWidth  = sizeof(CbSkySphereFlow);
        desc.Usage      = D3D11_USAGE_DEFAULT;

        hr = pDevice->CreateBuffer(&desc, nullptr, m_pCBFlow.GetAddress());
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateBuffer() Failed.");
            return false;
        }
    }

    // 頂点バッファとインデックスバッファの生成.
    {
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;

        uint32_t verticalSegments   = tessellation;
        uint32_t horizontalSegments = tessellation * 2;

        float radius = 1.0f;

        // Create rings of vertices at progressively higher latitudes.
        for (size_t i = 0; i <= verticalSegments; i++)
        {
            float v = 1 - (float)i / verticalSegments;

            float theta = (i * F_PI / verticalSegments) - F_PIDIV2;

            auto st = sin(theta);
            auto ct = cos(theta);

            // Create a single ring of vertices at this latitude.
            for (size_t j = 0; j <= horizontalSegments; j++)
            {
                float u = (float)j / horizontalSegments;

                float phi = j * F_2PI / horizontalSegments;

                auto sp = sin(phi);
                auto cp = cos(phi);

                auto normal = Vector3(sp * ct, st, cp * ct);
                auto uv = Vector2(u, v);

                auto l = normal.x * normal.x + normal.z * normal.z;
                Vector3 tangent;
                tangent.x = st * cp;
                tangent.y = st * sp;
                tangent.z = ct;

                Vertex vert;
                vert.Position = normal * radius;
                vert.Texcoord = uv;
                vert.Normal   = normal;
                vert.Tangent  = tangent;

                vertices.push_back(vert);
            }
        }

        // Fill the index buffer with triangles joining each pair of latitude rings.
        uint32_t stride = horizontalSegments + 1;

        for (auto i = 0u; i < verticalSegments; i++)
        {
            for (auto j = 0u; j <= horizontalSegments; j++)
            {
                auto nextI = i + 1;
                auto nextJ = (j + 1) % stride;

                indices.push_back(i * stride + j);
                indices.push_back(nextI * stride + j);
                indices.push_back(i * stride + nextJ);

                indices.push_back(i * stride + nextJ);
                indices.push_back(nextI * stride + j);
                indices.push_back(nextI * stride + nextJ);
            }
        }

        // 頂点バッファの生成.
        {
            auto vertexCount = uint32_t(vertices.size());

            D3D11_BUFFER_DESC desc = {};
            desc.Usage     = D3D11_USAGE_DEFAULT;
            desc.ByteWidth = sizeof(Vertex) * vertexCount;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA res = {};
            res.pSysMem = &vertices[0];

            hr = pDevice->CreateBuffer(&desc, &res, m_pVB.GetAddress());
            if (FAILED(hr))
            {
                ELOG("Error : ID3D11Device::CreateBuffer() Failed.");
                return false;
            }
        }

        // インデックスバッファの生成.
        {
            m_IndexCount = uint32_t(indices.size());

            D3D11_BUFFER_DESC desc = {};
            desc.Usage     = D3D11_USAGE_DEFAULT;
            desc.ByteWidth = sizeof(uint32_t) * m_IndexCount;
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA res = {};
            res.pSysMem = &indices[0];

            hr = pDevice->CreateBuffer(&desc, &res, m_pIB.GetAddress());
            if (FAILED(hr))
            {
                ELOG("Error : ID3D11Device::CreateBuffer() Failed.");
                return false;
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void SkySphere::Term()
{
    m_pVB.Reset();
    m_pIB.Reset();
    m_pCB.Reset();
    m_pIL.Reset();
    m_pVS.Reset();
    m_pPS.Reset();
    m_pPSFlow.Reset();
    m_pCBFlow.Reset();
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void SkySphere::Draw
(
    ID3D11DeviceContext*        pContext,
    ID3D11ShaderResourceView*   pSRV,
    ID3D11SamplerState*         pSmp,
    float                       sphereSize,
    const Vector3&              cameraPos,
    const Matrix&               view,
    const Matrix&               proj
)
{
    auto stride = uint32_t(sizeof(Vertex));
    auto offset = 0u;

    pContext->VSSetShader(m_pVS.GetPtr(), nullptr, 0);
    pContext->PSSetShader(m_pPS.GetPtr(), nullptr, 0);
    pContext->GSSetShader(nullptr, nullptr, 0);
    pContext->HSSetShader(nullptr, nullptr, 0);
    pContext->DSSetShader(nullptr, nullptr, 0);

    auto pCB = m_pCB.GetPtr();
    {
        CbSkySphere buf = {};
        buf.World       = Matrix::CreateTranslation(cameraPos);
        buf.View        = view;
        buf.Proj        = proj;
        buf.SphereSize  = sphereSize;

        pContext->UpdateSubresource(pCB, 0, nullptr, &buf, 0, 0);
    }

    pContext->VSSetConstantBuffers(0, 1, &pCB);
    pContext->PSSetShaderResources(0, 1, &pSRV);
    pContext->PSSetSamplers(0, 1, &pSmp);

    pContext->IASetInputLayout(m_pIL.GetPtr());
    pContext->IASetIndexBuffer(m_pIB.GetPtr(), DXGI_FORMAT_R32_UINT, 0);
    pContext->IASetVertexBuffers(0, 1, m_pVB.GetAddress(), &stride, &offset);
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    pContext->DrawIndexed(m_IndexCount, 0, 0);

    ID3D11ShaderResourceView* pNullSRV[1] = { nullptr };
    ID3D11SamplerState*       pNullSmp[1] = { nullptr };
    pContext->PSSetShaderResources(0, 1, pNullSRV);
    pContext->PSSetSamplers(0, 1, pNullSmp);
    pContext->VSSetShader(nullptr, nullptr, 0);
    pContext->PSSetShader(nullptr, nullptr, 0);
}


//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
void SkySphere::DrawFlow
(
    ID3D11DeviceContext*        pContext,
    ID3D11ShaderResourceView*   pSRV,
    ID3D11SamplerState*         pSmp,
    float                       sphereSize,
    const Vector3&              cameraPos,
    const Matrix&               view,
    const Matrix&               proj,
    const Vector3&              flowDir,
    float                       flowStep
)
{
    auto stride = uint32_t(sizeof(Vertex));
    auto offset = 0u;

    pContext->VSSetShader(m_pVS.GetPtr(), nullptr, 0);
    pContext->PSSetShader(m_pPSFlow.GetPtr(), nullptr, 0);
    pContext->GSSetShader(nullptr, nullptr, 0);
    pContext->HSSetShader(nullptr, nullptr, 0);
    pContext->DSSetShader(nullptr, nullptr, 0);

    auto pCB = m_pCB.GetPtr();
    {
        CbSkySphere buf = {};
        buf.World = Matrix::CreateRotationX(F_PIDIV2) * Matrix::CreateTranslation(cameraPos);
        buf.View = view;
        buf.Proj = proj;
        buf.SphereSize = sphereSize;

        pContext->UpdateSubresource(pCB, 0, nullptr, &buf, 0, 0);
    }

    auto pCBFlow = m_pCBFlow.GetPtr();
    {
        CbSkySphereFlow buf = {};
        buf.WindDirection = flowDir;
        buf.Offset.x      = m_FlowOffset[0];
        buf.Offset.y      = m_FlowOffset[1];

        m_FlowOffset[0] += flowStep;
        m_FlowOffset[1] += flowStep;

        if (m_FlowOffset[0] > 1.0f)
        { m_FlowOffset[0] -= 1.0f; }

        if (m_FlowOffset[1] > 1.0f)
        { m_FlowOffset[1] -= 1.0f; }

        pContext->UpdateSubresource(pCBFlow, 0, nullptr, &buf, 0, 0);
    }

    pContext->VSSetConstantBuffers(0, 1, &pCB);
    pContext->PSSetConstantBuffers(1, 1, &pCBFlow);
    pContext->PSSetShaderResources(0, 1, &pSRV);
    pContext->PSSetSamplers(0, 1, &pSmp);

    pContext->IASetInputLayout(m_pIL.GetPtr());
    pContext->IASetIndexBuffer(m_pIB.GetPtr(), DXGI_FORMAT_R32_UINT, 0);
    pContext->IASetVertexBuffers(0, 1, m_pVB.GetAddress(), &stride, &offset);
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    pContext->DrawIndexed(m_IndexCount, 0, 0);

    ID3D11ShaderResourceView* pNullSRV[1] = { nullptr };
    ID3D11SamplerState*       pNullSmp[1] = { nullptr };
    pContext->PSSetShaderResources(0, 1, pNullSRV);
    pContext->PSSetSamplers(0, 1, pNullSmp);
    pContext->VSSetShader(nullptr, nullptr, 0);
    pContext->PSSetShader(nullptr, nullptr, 0);
}

//-----------------------------------------------------------------------------
//      フローオフセットをリセットします.
//-----------------------------------------------------------------------------
void SkySphere::ResetFlow()
{
    m_FlowOffset[0] = 0.0f;
    m_FlowOffset[1] = 0.5f;
}

} // namespace asdx
