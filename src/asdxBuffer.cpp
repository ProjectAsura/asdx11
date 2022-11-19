//-----------------------------------------------------------------------------
// File : asdxBuffer.cpp
// Desc : Buffer Module.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxBuffer.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// VertexBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
VertexBuffer::VertexBuffer()
: m_Buffer  ()
, m_SRV     ()
, m_Stride  (0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
VertexBuffer::~VertexBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void VertexBuffer::Term()
{
    m_SRV   .Reset();
    m_Buffer.Reset();
    m_Stride = 0;
}

//-----------------------------------------------------------------------------
//      バッファを取得します.
//-----------------------------------------------------------------------------
ID3D11Buffer* const VertexBuffer::GetBuffer() const
{ return m_Buffer.GetPtr(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderResourceView* const VertexBuffer::GetShaderResource() const
{ return m_SRV.GetPtr(); }

//-----------------------------------------------------------------------------
//      1頂点あたりのデータサイズを取得します.
//-----------------------------------------------------------------------------
uint32_t VertexBuffer::GetStride() const
{ return m_Stride; }

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D11Buffer* const VertexBuffer::operator -> () const
{ return m_Buffer.GetPtr(); }


///////////////////////////////////////////////////////////////////////////////
// IndexBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
IndexBuffer::IndexBuffer()
: m_Buffer  ()
, m_SRV     ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
IndexBuffer::~IndexBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void IndexBuffer::Term()
{
    m_SRV.Reset();
    m_Buffer.Reset();
}

//-----------------------------------------------------------------------------
//      バッファを取得します.
//-----------------------------------------------------------------------------
ID3D11Buffer* const IndexBuffer::GetBuffer() const
{ return m_Buffer.GetPtr(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderResourceView* const IndexBuffer::GetShaderResource() const
{ return m_SRV.GetPtr(); }

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D11Buffer* const IndexBuffer::operator -> () const
{ return m_Buffer.GetPtr(); }


///////////////////////////////////////////////////////////////////////////////
// ConstantBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ConstantBuffer::ConstantBuffer()
: m_Buffer()
{ /* DO_NOTHING */}

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ConstantBuffer::~ConstantBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ConstantBuffer::Init(ID3D11Device* pDevice, size_t size, bool mappable)
{
    if ( pDevice == nullptr || size == 0 )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.Usage          = (mappable) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    desc.ByteWidth      = UINT(size);
    desc.CPUAccessFlags = (mappable) ? D3D11_CPU_ACCESS_WRITE : 0;

    auto hr = pDevice->CreateBuffer( &desc, nullptr, m_Buffer.GetAddress() );
    if ( FAILED(hr) )
    {
        ELOG( "Error : ID3D11Device::CreateBuffer() Failed." );
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ConstantBuffer::Term()
{ m_Buffer.Reset(); }

//-----------------------------------------------------------------------------
//      バッファを取得します.
//-----------------------------------------------------------------------------
ID3D11Buffer* const ConstantBuffer::GetBuffer() const
{ return m_Buffer.GetPtr(); }

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D11Buffer* const ConstantBuffer::operator -> () const
{ return m_Buffer.GetPtr(); }


///////////////////////////////////////////////////////////////////////////////
// StructuredBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
StructuredBuffer::StructuredBuffer()
: m_Buffer  ()
, m_SRV     ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
StructuredBuffer::~StructuredBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool StructuredBuffer::Init(ID3D11Device* pDevice, uint32_t size, uint32_t stride, const void* pInitData, bool uav)
{
    if ( pDevice == nullptr || size == 0 )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    uint32_t bindFlags = D3D11_BIND_SHADER_RESOURCE;
    if (uav)
    { bindFlags |= D3D11_BIND_UNORDERED_ACCESS; }

    D3D11_BUFFER_DESC desc = {};
    desc.BindFlags           = bindFlags;
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

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
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

    if (uav)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format              = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements  = desc.ByteWidth / desc.StructureByteStride;

        hr = pDevice->CreateUnorderedAccessView( m_Buffer.GetPtr(), &uavDesc, m_UAV.GetAddress() );
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateUnorderedAccessView() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void StructuredBuffer::Term()
{
    m_SRV.Reset();
    m_UAV.Reset();
    m_Buffer.Reset();
}

//-----------------------------------------------------------------------------
//      バッファを取得します.
//-----------------------------------------------------------------------------
ID3D11Buffer* const StructuredBuffer::GetBuffer() const
{ return m_Buffer.GetPtr(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderResourceView* const StructuredBuffer::GetSRV() const
{ return m_SRV.GetPtr(); }

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューを取得します.
//-----------------------------------------------------------------------------
ID3D11UnorderedAccessView* const StructuredBuffer::GetUAV() const
{ return m_UAV.GetPtr(); }

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D11Buffer* const StructuredBuffer::operator -> () const
{ return m_Buffer.GetPtr(); }


///////////////////////////////////////////////////////////////////////////////
// RawBuffer class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
RawBuffer::RawBuffer()
: m_Buffer  ()
, m_SRV     ()
, m_UAV     ()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
RawBuffer::~RawBuffer()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool RawBuffer::Init(ID3D11Device* pDevice, uint32_t size, const void* pInitData, bool uav)
{
    if ( pDevice == nullptr || size == 0 )
    {
        ELOG( "Error : Invalid Argument." );
        return false;
    }

    uint32_t bindFlags = D3D11_BIND_SHADER_RESOURCE;
    if (uav)
    { bindFlags |= D3D11_BIND_UNORDERED_ACCESS; }

    D3D11_BUFFER_DESC desc = {};
    desc.BindFlags           = bindFlags;
    desc.ByteWidth           = size;

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

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension           = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.Format                  = DXGI_FORMAT_R32_TYPELESS;
    srvDesc.BufferEx.FirstElement   = 0;
    srvDesc.BufferEx.NumElements    = size / 4;
    srvDesc.BufferEx.Flags          = D3D11_BUFFEREX_SRV_FLAG_RAW;

    auto hr = pDevice->CreateShaderResourceView( m_Buffer.GetPtr(), &srvDesc, m_SRV.GetAddress() );
    if ( FAILED(hr) )
    {
        ELOG( "Error : ID3D11Device::CreateShaderResourceView() Failed." );
        return false;
    }

    if (uav)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format              = DXGI_FORMAT_R32_TYPELESS;
        uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements  = size / 4;
        uavDesc.Buffer.Flags        = D3D11_BUFFER_UAV_FLAG_RAW;

        hr = pDevice->CreateUnorderedAccessView( m_Buffer.GetPtr(), &uavDesc, m_UAV.GetAddress() );
        if (FAILED(hr))
        {
            ELOG("Error : ID3D11Device::CreateUnorderedAccessView() Failed.");
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void RawBuffer::Term()
{
    m_SRV.Reset();
    m_UAV.Reset();
    m_Buffer.Reset();
}

//-----------------------------------------------------------------------------
//      バッファを取得します.
//-----------------------------------------------------------------------------
ID3D11Buffer* const RawBuffer::GetBuffer() const
{ return m_Buffer.GetPtr(); }

//-----------------------------------------------------------------------------
//      シェーダリソースビューを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderResourceView* const RawBuffer::GetSRV() const
{ return m_SRV.GetPtr(); }

//-----------------------------------------------------------------------------
//      アンオーダードアクセスビューを取得します.
//-----------------------------------------------------------------------------
ID3D11UnorderedAccessView* const RawBuffer::GetUAV() const
{ return m_UAV.GetPtr(); }

//-----------------------------------------------------------------------------
//      アロー演算子です.
//-----------------------------------------------------------------------------
ID3D11Buffer* const RawBuffer::operator -> () const
{ return m_Buffer.GetPtr(); }

} // namespace asdx
