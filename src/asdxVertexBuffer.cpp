//-------------------------------------------------------------------------------------------------
// File : asdxVertexBuffer.cpp
// Desc : Vertex Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxVertexBuffer.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// VertexBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
VertexBuffer::VertexBuffer()
: m_Buffer  ()
, m_SRV     ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool VertexBuffer::Init
(
    ID3D11Device*   pDevice,
    size_t          size,
    size_t          stride,
    const void*     pInitData,
    bool            isDynamic,
    bool            isSRV
)
{
    if ( pDevice == nullptr || size == 0 )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    desc.Usage          = (isDynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = (isDynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
    desc.ByteWidth      = UINT(size);
    desc.MiscFlags      = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    if ( pInitData != nullptr )
    {
        D3D11_SUBRESOURCE_DATA res;
        ZeroMemory( &res, sizeof(res) ); 
        res.pSysMem = pInitData;

        auto hr = pDevice->CreateBuffer( &desc, &res, m_Buffer.GetAddress() );
        if ( FAILED(hr) )
        {
            ELOG( "Error : ID3D11Device::CreateBuffer() Failed." );
            return false;
        }
    }
    else
    {
        auto hr = pDevice->CreateBuffer( &desc, nullptr, m_Buffer.GetAddress() );
        if ( FAILED(hr) )
        {
            ELOG( "Error : ID3D11Device::CreateBuffer() Failed." );
            return false;
        }
    }

    if (isSRV)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory( &srvDesc, sizeof(srvDesc) );
        srvDesc.ViewDimension           = D3D11_SRV_DIMENSION_BUFFEREX;
        srvDesc.Format                  = DXGI_FORMAT_R32_TYPELESS;
        srvDesc.BufferEx.Flags          = D3D11_BUFFEREX_SRV_FLAG_RAW;
        srvDesc.BufferEx.FirstElement   = 0;
        srvDesc.BufferEx.NumElements    = desc.ByteWidth / 4;

        auto hr = pDevice->CreateShaderResourceView( m_Buffer.GetPtr(), &srvDesc, m_SRV.GetAddress() );
        if ( FAILED(hr) )
        {
            ELOG( "Error : ID3D11Device::CreateShaderResourceView() Failed." );
            return false;
        }
    }

    m_Stride = uint32_t(stride);

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void VertexBuffer::Term()
{
    m_SRV.Reset();
    m_Buffer.Reset();
}

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Buffer* const VertexBuffer::GetBuffer() const
{ return m_Buffer.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* const VertexBuffer::GetShaderResource() const
{ return m_SRV.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      1頂点あたりのデータサイズを取得します.
//-------------------------------------------------------------------------------------------------
uint32_t VertexBuffer::GetStride() const
{ return m_Stride; }

//-------------------------------------------------------------------------------------------------
//      アロー演算子です.
//-------------------------------------------------------------------------------------------------
ID3D11Buffer* const VertexBuffer::operator -> () const
{ return m_Buffer.GetPtr(); }


} // namespace asdx
