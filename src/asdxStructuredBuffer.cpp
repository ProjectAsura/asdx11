//-------------------------------------------------------------------------------------------------
// File : asdxStructuredBuffer.cpp
// Desc : Structured Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxStructuredBuffer.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// StructuredBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
StructuredBuffer::StructuredBuffer()
: m_Buffer  ()
, m_SRV     ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
StructuredBuffer::~StructuredBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool StructuredBuffer::Init(ID3D11Device* pDevice, uint32_t size, uint32_t stride, const void* pInitData)
{
    if ( pDevice == nullptr || size == 0 )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags           = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.ByteWidth           = size;
    desc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = stride;

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

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory( &srvDesc, sizeof(srvDesc) );
    srvDesc.ViewDimension           = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.Format                  = DXGI_FORMAT_UNKNOWN;
    srvDesc.BufferEx.FirstElement   = 0;
    srvDesc.BufferEx.NumElements    = desc.ByteWidth / desc.StructureByteStride;

    auto hr = pDevice->CreateShaderResourceView( m_Buffer.GetPtr(), &srvDesc, m_SRV.GetAddress() );
    if ( FAILED(hr) )
    {
        ELOG( "Error : ID3D11Device::CreateShaderResourceView() Failed." );
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理を行います.
//-------------------------------------------------------------------------------------------------
void StructuredBuffer::Term()
{
    m_SRV.Reset();
    m_Buffer.Reset();
}

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Buffer* const StructuredBuffer::GetBuffer() const
{ return m_Buffer.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* const StructuredBuffer::GetShaderResource() const
{ return m_SRV.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      アロー演算子です.
//-------------------------------------------------------------------------------------------------
ID3D11Buffer* const StructuredBuffer::operator -> () const
{ return m_Buffer.GetPtr(); }

} // namespace asdx
