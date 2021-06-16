//-------------------------------------------------------------------------------------------------
// File : asdxHash.h
// Desc : Hash Key Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <xxhash.h>

namespace asdx {

//-----------------------------------------------------------------------------
//! @brief      xxhash 32bit によるハッシュ値を計算します.
//!
//! @param[in]      buffer      バッファ.
//! @param[in]      size        バッファサイズ.
//! @return     ハッシュ値を返却します.
//-----------------------------------------------------------------------------
inline uint32_t CalcHash32(const uint8_t* buffer, uint32_t size)
{ return XXH32(buffer, size, 0); }

    //-----------------------------------------------------------------------------
//! @brief      xxhash 64bit によるハッシュ値を計算します.
//!
//! @param[in]      buffer      バッファ.
//! @param[in]      size        バッファサイズ.
//! @return     ハッシュ値を返却します.
//-----------------------------------------------------------------------------
inline uint64_t CalcHash64(const uint8_t* buffer, uint64_t size)
{ return XXH64(buffer, size, 0); }

//-----------------------------------------------------------------------------
//! @brief      FNV1 32bitによるハッシュ値を計算します.
//! 
//! @param[in]      buffer      バッファ.
//! @param[in]      size        バッファサイズ.
//! @return     ハッシュ値を返却します.
//-----------------------------------------------------------------------------
inline uint32_t CalcFnv1(const uint8_t* buffer, uint32_t size)
{
    const uint32_t kOffset  = 0x811c9dc5;
    const uint32_t kPrime   = 0x01000193;

    auto hash = kOffset;
    for(auto i=0u; i<size; ++i)
    { hash = (kPrime * hash) ^ buffer[i]; }

    return hash;
}

    //-----------------------------------------------------------------------------
//! @brief      FNV1 64bitによるハッシュ値を計算します.
//! 
//! @param[in]      buffer      バッファ.
//! @param[in]      size        バッファサイズ.
//! @return     ハッシュ値を返却します.
//-----------------------------------------------------------------------------
inline uint64_t CalcFnv1(const uint8_t* buffer, uint64_t size)
{
    const uint64_t kOffset  = 0xcbf29ce484222325;
    const uint64_t kPrime   = 0x100000001b3;

    auto hash = kOffset;
    for(uint64_t i=0; i<size; ++i)
    { hash = (kPrime * hash) ^ buffer[i]; }

    return hash;
}



} // asdx