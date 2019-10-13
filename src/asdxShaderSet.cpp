//-----------------------------------------------------------------------------
// File : asdxShaderSet.h
// Desc : Shader Set Module.
// Copyright(c) Project Asura. ALl right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <asdxShaderSet.h>
#include <asdxLogger.h>


namespace asdx {

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