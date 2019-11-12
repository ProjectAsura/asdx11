//-----------------------------------------------------------------------------
// File : asdxShader.h
// Desc : Shader Set Module.
// Copyright(c) Project Asura. ALl right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxShader.h>
#include <asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ShaderCBV class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ShaderCBV::ShaderCBV()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ShaderCBV::~ShaderCBV()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ShaderCBV::Init
(
    ID3D11Device*                           pDevice,
    ID3D11ShaderReflectionConstantBuffer*   pReflection
)
{
    if (pDevice == nullptr || pReflection == nullptr)
    {
        ELOG("Error : Invalid Argument.");
        return false;
    }

    D3D11_SHADER_BUFFER_DESC buf_desc;
    auto hr = pReflection->GetDesc(&buf_desc);
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11ShaderReflectionConstantBuffer::GetDesc() Failed. recode = 0x%x", hr);
        return false;
    }

    auto size = buf_desc.Size;

    m_Memory.resize(size);

    D3D11_BUFFER_DESC cb_desc = {};
    cb_desc.Usage           = D3D11_USAGE_DEFAULT;
    cb_desc.BindFlags       = D3D11_BIND_CONSTANT_BUFFER;
    cb_desc.ByteWidth       = size;
    cb_desc.CPUAccessFlags  = 0;

    hr = pDevice->CreateBuffer(&cb_desc, nullptr, m_CB.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateBuffer() Failed");
        return false;
    }

    for(auto i=0u; i<buf_desc.Variables; ++i)
    {
        auto pVar = pReflection->GetVariableByIndex(i);
        if (pVar == nullptr)
        { continue; }

        D3D11_SHADER_VARIABLE_DESC var_desc;
        hr = pVar->GetDesc(&var_desc);
        if (FAILED(hr))
        { continue; }

        BufferParam param;
        param.Offset = var_desc.StartOffset;
        param.Size   = var_desc.Size;

        if (!Contain(var_desc.Name))
        { m_ParamMap[var_desc.Name] = param; }

        auto head = m_Memory.data();
        memcpy(head + var_desc.StartOffset, var_desc.DefaultValue, var_desc.Size);
    }

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ShaderCBV::Term()
{
    m_CB.Reset();

    m_ParamMap.clear();
    m_Memory  .clear();
}

//-----------------------------------------------------------------------------
//      パラメータを設定します.
//-----------------------------------------------------------------------------
bool ShaderCBV::SetParam(const char* name, const void* ptr, size_t size)
{
    if (!Contain(name))
    { return false; }

    auto param = m_ParamMap[name];
    if (param.Size != size)
    { return false; }

    auto head = m_Memory.data();
    memcpy(head + param.Offset, ptr, param.Size);
    return true;
}

//-----------------------------------------------------------------------------
//      パラメータを取得します.
//-----------------------------------------------------------------------------
bool ShaderCBV::GetParam(const char* name, void* ptr, size_t size) const
{
    if (!Contain(name))
    { return false; }

    auto param = m_ParamMap.at(name);
    if (param.Size != size)
    { return false; }

    auto head = m_Memory.data();
    memcpy(ptr, head + param.Offset, param.Size);
    return true;
}

//-----------------------------------------------------------------------------
//      指定されたパラメータ名が含まれるかチェックします.
//-----------------------------------------------------------------------------
bool ShaderCBV::Contain(const char* name) const
{ return m_ParamMap.find(name) != m_ParamMap.end(); }

//-----------------------------------------------------------------------------
//      サブリソースを更新します.
//-----------------------------------------------------------------------------
void ShaderCBV::UpdateSubresource(ID3D11DeviceContext* pContext)
{ pContext->UpdateSubresource(m_CB.GetPtr(), 0, nullptr, m_Memory.data(), 0, 0); }

//-----------------------------------------------------------------------------
//      バッファを返却します
//-----------------------------------------------------------------------------
ID3D11Buffer* ShaderCBV::GetPtr() const
{ return m_CB.GetPtr(); }



///////////////////////////////////////////////////////////////////////////////
// VertexShader class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
VertexShader::VertexShader()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
VertexShader::~VertexShader()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool VertexShader::Init
(
    ID3D11Device*                   pDevice,
    const uint8_t*                  pBinary,
    size_t                          binarySize,
    uint32_t                        elementCount,
    const D3D11_INPUT_ELEMENT_DESC* pElements
)
{
    auto hr = pDevice->CreateVertexShader(pBinary, binarySize, nullptr, m_VS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateVertexShader() Failed.");
        return false;
    }

    hr = pDevice->CreateInputLayout(pElements, elementCount, pBinary, binarySize, m_IL.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateInputLayout() Failed.");
        return false;
    }

    hr = D3DReflect(pBinary, binarySize, IID_PPV_ARGS(m_Reflection.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : D3DReflect() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool VertexShader::Init
(
    ID3D11Device*                   pDevice,
    const wchar_t*                  path,
    const char*                     entryPoint,
    const char*                     shaderModel,
    uint32_t                        elementCount,
    const D3D11_INPUT_ELEMENT_DESC* pElements
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompileFromFile(
        path,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }

    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize(),
        elementCount,
        pElements);
}


//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool VertexShader::Init
(
    ID3D11Device*                   pDevice,
    const char*                     sourceCode,
    size_t                          sourceCodeSize,
    const char*                     entryPoint,
    const char*                     shaderModel,
    uint32_t                        elementCount,
    const D3D11_INPUT_ELEMENT_DESC* pElements
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompile(
        sourceCode,
        sourceCodeSize,
        "vertex_shader",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }

    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize(),
        elementCount,
        pElements);
}


//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void VertexShader::Term()
{
    m_VS.Reset();
    m_IL.Reset();
    m_Reflection.Reset();
}

//-----------------------------------------------------------------------------
//      シェーダを設定します.
//-----------------------------------------------------------------------------
void VertexShader::Bind(ID3D11DeviceContext* pContext)
{
    pContext->IASetInputLayout(m_IL.GetPtr());
    pContext->VSSetShader(m_VS.GetPtr(), nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダの設定を解除します.
//-----------------------------------------------------------------------------
void VertexShader::UnBind(ID3D11DeviceContext* pContext)
{
    pContext->IASetInputLayout(nullptr);
    pContext->VSSetShader(nullptr, nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダリフレクションを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderReflection* VertexShader::GetReflection() const
{
    return m_Reflection.GetPtr();
}


///////////////////////////////////////////////////////////////////////////////
// PixelShader class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
PixelShader::PixelShader()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
PixelShader::~PixelShader()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool PixelShader::Init(ID3D11Device* pDevice, const uint8_t* pBinary, size_t binarySize)
{
    auto hr = pDevice->CreatePixelShader(pBinary, binarySize, nullptr, m_PS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreatePixelShader() Failed.");
        return false;
    }

    hr = D3DReflect(pBinary, binarySize, IID_PPV_ARGS(m_Reflection.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : D3DReflect() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool PixelShader::Init
(
    ID3D11Device*   pDevice,
    const wchar_t*  path,
    const char*     entryPoint,
    const char*     shaderModel
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompileFromFile(
        path,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }
 
    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize());
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool PixelShader::Init
(
    ID3D11Device*   pDevice,
    const char*     sourceCode,
    size_t          sourceCodeSize,
    const char*     entryPoint,
    const char*     shaderModel
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompile(
        sourceCode,
        sourceCodeSize,
        "pixel_shader",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }
 
    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize());
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void PixelShader::Term()
{
    m_PS.Reset();
    m_Reflection.Reset();
}

//-----------------------------------------------------------------------------
//      シェーダを設定します.
//-----------------------------------------------------------------------------
void PixelShader::Bind(ID3D11DeviceContext* pContext)
{
    pContext->PSSetShader(m_PS.GetPtr(), nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダの設定を解除します.
//-----------------------------------------------------------------------------
void PixelShader::UnBind(ID3D11DeviceContext* pContext)
{
    pContext->PSSetShader(nullptr, nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダリフレクションを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderReflection* PixelShader::GetReflection() const
{
    return m_Reflection.GetPtr();
}


///////////////////////////////////////////////////////////////////////////////
// GeometryShader class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
GeometryShader::GeometryShader()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
GeometryShader::~GeometryShader()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool GeometryShader::Init(ID3D11Device* pDevice, const uint8_t* pBinary, size_t binarySize)
{
    auto hr = pDevice->CreateGeometryShader(pBinary, binarySize, nullptr, m_GS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateGeometryShader() Failed.");
        return false;
    }

    hr = D3DReflect(pBinary, binarySize, IID_PPV_ARGS(m_Reflection.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : D3DReflect() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool GeometryShader::Init
(
    ID3D11Device*   pDevice,
    const wchar_t*  path,
    const char*     entryPoint,
    const char*     shaderModel
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompileFromFile(
        path,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }
 
    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize());
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool GeometryShader::Init
(
    ID3D11Device*   pDevice,
    const char*     sourceCode,
    size_t          sourceCodeSize,
    const char*     entryPoint,
    const char*     shaderModel
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompile(
        sourceCode,
        sourceCodeSize,
        "geometry_shader",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }
 
    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize());
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void GeometryShader::Term()
{
    m_GS.Reset();
    m_Reflection.Reset();
}

//-----------------------------------------------------------------------------
//      シェーダを設定します.
//-----------------------------------------------------------------------------
void GeometryShader::Bind(ID3D11DeviceContext* pContext)
{
    pContext->GSSetShader(m_GS.GetPtr(), nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダの設定を解除します.
//-----------------------------------------------------------------------------
void GeometryShader::UnBind(ID3D11DeviceContext* pContext)
{
    pContext->GSSetShader(nullptr, nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダリフレクションを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderReflection* GeometryShader::GetReflection() const
{
    return m_Reflection.GetPtr();
}

///////////////////////////////////////////////////////////////////////////////
// HullShader class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
HullShader::HullShader()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
HullShader::~HullShader()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool HullShader::Init(ID3D11Device* pDevice, const uint8_t* pBinary, size_t binarySize)
{
    auto hr = pDevice->CreateHullShader(pBinary, binarySize, nullptr, m_HS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreatePixelShader() Failed.");
        return false;
    }

    hr = D3DReflect(pBinary, binarySize, IID_PPV_ARGS(m_Reflection.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : D3DReflect() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool HullShader::Init
(
    ID3D11Device*   pDevice,
    const wchar_t*  path,
    const char*     entryPoint,
    const char*     shaderModel
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompileFromFile(
        path,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }
 
    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize());
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool HullShader::Init
(
    ID3D11Device*   pDevice,
    const char*     sourceCode,
    size_t          sourceCodeSize,
    const char*     entryPoint,
    const char*     shaderModel
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompile(
        sourceCode,
        sourceCodeSize,
        "hull_shader",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }
 
    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize());
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void HullShader::Term()
{
    m_HS.Reset();
    m_Reflection.Reset();
}

//-----------------------------------------------------------------------------
//      シェーダを設定します.
//-----------------------------------------------------------------------------
void HullShader::Bind(ID3D11DeviceContext* pContext)
{
    pContext->HSSetShader(m_HS.GetPtr(), nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダの設定を解除します.
//-----------------------------------------------------------------------------
void HullShader::UnBind(ID3D11DeviceContext* pContext)
{
    pContext->HSSetShader(nullptr, nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダリフレクションを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderReflection* HullShader::GetReflection() const
{
    return m_Reflection.GetPtr();
}


///////////////////////////////////////////////////////////////////////////////
// DomainShader class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
DomainShader::DomainShader()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
DomainShader::~DomainShader()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool DomainShader::Init(ID3D11Device* pDevice, const uint8_t* pBinary, size_t binarySize)
{
    auto hr = pDevice->CreateDomainShader(pBinary, binarySize, nullptr, m_DS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreateDomainShader() Failed.");
        return false;
    }

    hr = D3DReflect(pBinary, binarySize, IID_PPV_ARGS(m_Reflection.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : D3DReflect() Failed.");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool DomainShader::Init
(
    ID3D11Device*   pDevice,
    const wchar_t*  path,
    const char*     entryPoint,
    const char*     shaderModel
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompileFromFile(
        path,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }
 
    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize());
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool DomainShader::Init
(
    ID3D11Device*   pDevice,
    const char*     sourceCode,
    size_t          sourceCodeSize,
    const char*     entryPoint,
    const char*     shaderModel
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompile(
        sourceCode,
        sourceCodeSize,
        "domain_shader",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }
 
    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize());
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void DomainShader::Term()
{
    m_DS.Reset();
    m_Reflection.Reset();
}

//-----------------------------------------------------------------------------
//      シェーダを設定します.
//-----------------------------------------------------------------------------
void DomainShader::Bind(ID3D11DeviceContext* pContext)
{
    pContext->DSSetShader(m_DS.GetPtr(), nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダの設定を解除します.
//-----------------------------------------------------------------------------
void DomainShader::UnBind(ID3D11DeviceContext* pContext)
{
    pContext->DSSetShader(nullptr, nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダリフレクションを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderReflection* DomainShader::GetReflection() const
{
    return m_Reflection.GetPtr();
}


///////////////////////////////////////////////////////////////////////////////
// ComputeShader class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ComputeShader::ComputeShader()
: m_ThreadX(0)
, m_ThreadY(0)
, m_ThreadZ(0)
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ComputeShader::~ComputeShader()
{ Term(); }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ComputeShader::Init(ID3D11Device* pDevice, const uint8_t* pBinary, size_t binarySize)
{
    auto hr = pDevice->CreateComputeShader(pBinary, binarySize, nullptr, m_CS.GetAddress());
    if (FAILED(hr))
    {
        ELOG("Error : ID3D11Device::CreatePixelShader() Failed.");
        return false;
    }

    hr = D3DReflect(pBinary, binarySize, IID_PPV_ARGS(m_Reflection.GetAddress()));
    if (FAILED(hr))
    {
        ELOG("Error : D3DReflect() Failed.");
        return false;
    }

    m_Reflection->GetThreadGroupSize(&m_ThreadX, &m_ThreadY, &m_ThreadZ);

    return true;
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool ComputeShader::Init
(
    ID3D11Device*   pDevice,
    const wchar_t*  path,
    const char*     entryPoint,
    const char*     shaderModel
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompileFromFile(
        path,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }

    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize());
}

//-----------------------------------------------------------------------------
//      ソースコードから初期化処理を行います.
//-----------------------------------------------------------------------------
bool ComputeShader::Init
(
    ID3D11Device*   pDevice,
    const char*     sourceCode,
    size_t          sourceCodeSize,
    const char*     entryPoint,
    const char*     shaderModel
)
{
    DWORD flag = D3DCOMPILE_ENABLE_STRICTNESS;

    #if defined(DEBUG) || defined(_DEBUG)
        flag |= D3DCOMPILE_DEBUG;
    #else
        flag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    RefPtr<ID3DBlob> pBlob;
    RefPtr<ID3DBlob> pErrorBlob;
    auto hr = D3DCompile(
        sourceCode,
        sourceCodeSize,
        "compute_shader",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        shaderModel,
        flag,
        0,
        pBlob.GetAddress(),
        pErrorBlob.GetAddress());

    if (FAILED(hr))
    {
        if (pErrorBlob.GetPtr() != nullptr)
        { ELOGA("Error : D3DCompileFromFile() Failed. msg = %s", pErrorBlob->GetBufferPointer()); }

        ELOGA("Error : D3DCompileFromFile() errcode = 0x%x", hr);
        return false;
    }

    return Init(
        pDevice,
        reinterpret_cast<uint8_t*>(pBlob->GetBufferPointer()),
        pBlob->GetBufferSize());
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ComputeShader::Term()
{
    m_CS.Reset();
    m_Reflection.Reset();
    m_ThreadX = m_ThreadY = m_ThreadZ = 0;
}

//-----------------------------------------------------------------------------
//      シェーダを設定します.
//-----------------------------------------------------------------------------
void ComputeShader::Bind(ID3D11DeviceContext* pContext)
{
    pContext->CSSetShader(m_CS.GetPtr(), nullptr, 0);
}

//-----------------------------------------------------------------------------
//      シェーダの設定を解除します.
//-----------------------------------------------------------------------------
void ComputeShader::UnBind(ID3D11DeviceContext* pContext)
{
    pContext->CSSetShader(nullptr, nullptr, 0);
}

//-----------------------------------------------------------------------------
//      ディスパッチします.
//-----------------------------------------------------------------------------
void ComputeShader::Dispatch(ID3D11DeviceContext* pContext)
{
    pContext->Dispatch(m_ThreadX, m_ThreadY, m_ThreadZ);
}

//-----------------------------------------------------------------------------
//      シェーダリフレクションを取得します.
//-----------------------------------------------------------------------------
ID3D11ShaderReflection* ComputeShader::GetReflection() const
{
    return m_Reflection.GetPtr();
}

} // namespace asdx