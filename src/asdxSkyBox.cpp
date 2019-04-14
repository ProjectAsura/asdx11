//-----------------------------------------------------------------------------
// File : asdxSkyBox.cpp
// Desc : Sky Box Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxSkyBox.h>
#include <asdxLogger.h>


namespace {

#include "../res/shaders/Compiled/SkyBoxVS.inc"
#include "../res/shaders/Compiled/SkyBoxPS.inc"

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

} // namespace asdx
