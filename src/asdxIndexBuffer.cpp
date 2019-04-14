//-------------------------------------------------------------------------------------------------
// File : asdxIndexBuffer.cpp
// Desc : Index Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxIndexBuffer.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// IndexBuffer class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
IndexBuffer::IndexBuffer()
: m_Buffer  ()
, m_SRV     ()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
IndexBuffer::~IndexBuffer()
{ Term(); }

//-------------------------------------------------------------------------------------------------
//      初期化処理を行います.
//-------------------------------------------------------------------------------------------------
bool IndexBuffer::Init(ID3D11Device* pDevice, size_t size, const uint32_t* pInitData, bool isDynamic)
{
    if ( pDevice == nullptr || size == 0 )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags      = D3D11_BIND_INDEX_BUFFER | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    desc.ByteWidth      = UINT(size);
    desc.Usage          = (isDynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = (isDynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
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

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory( &srvDesc, sizeof(srvDesc) );
    srvDesc.ViewDimension           = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.Format                  = DXGI_FORMAT_R32_TYPELESS;
    srvDesc.BufferEx.FirstElement   = 0;
    srvDesc.BufferEx.Flags          = D3D11_BUFFEREX_SRV_FLAG_RAW;
    srvDesc.BufferEx.NumElements    = desc.ByteWidth / 4;

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
void IndexBuffer::Term()
{
    m_SRV.Reset();
    m_Buffer.Reset();
}

//-------------------------------------------------------------------------------------------------
//      バッファを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11Buffer* const IndexBuffer::GetBuffer() const
{ return m_Buffer.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* const IndexBuffer::GetShaderResource() const
{ return m_SRV.GetPtr(); }

//-------------------------------------------------------------------------------------------------
//      アロー演算子です.
//-------------------------------------------------------------------------------------------------
ID3D11Buffer* const IndexBuffer::operator -> () const
{ return m_Buffer.GetPtr(); }


} // namespace asdx
