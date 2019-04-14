﻿//-------------------------------------------------------------------------------------------------
// File : asdxRef.h
// Desc : Reference Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <cstdint>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// IReference interface
///////////////////////////////////////////////////////////////////////////////////////////////////
struct IReference
{
    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    virtual ~IReference()
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを1つ増やします.
    //---------------------------------------------------------------------------------------------
    virtual void AddRef() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを1つ減らします.
    //---------------------------------------------------------------------------------------------
    virtual void Release() = 0;

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを取得します.
    //!
    //! @return     参照カウントを返却します.
    //---------------------------------------------------------------------------------------------
    virtual uint32_t GetCount() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// RefPtr class
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class RefPtr
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    template<typename U> friend class RefPtr;

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      コンストラクタです.
    //---------------------------------------------------------------------------------------------
    RefPtr() noexcept
    : m_pPtr( nullptr )
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //---------------------------------------------------------------------------------------------
    explicit RefPtr( std::nullptr_t ) noexcept
    : m_pPtr( nullptr )
    { /* DO_NOTHING */ }

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //---------------------------------------------------------------------------------------------
    explicit RefPtr( T* pInterface ) noexcept
    : m_pPtr( pInterface )
    { AddRef(); }

    //---------------------------------------------------------------------------------------------
    //! @brief      コピーコンストラクタです.
    //---------------------------------------------------------------------------------------------
    RefPtr( const RefPtr& value ) noexcept
    : m_pPtr( value.m_pPtr )
    { AddRef(); }

    //---------------------------------------------------------------------------------------------
    //! @brief      ムーブコンストラクタです.
    //---------------------------------------------------------------------------------------------
    RefPtr( RefPtr&& value ) noexcept
    : m_pPtr( nullptr )
    {
        if ( this != reinterpret_cast<RefPtr*>(&reinterpret_cast<uint8_t&>(value)) )
        { Swap(value); }
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      引数付きコンストラクタです.
    //---------------------------------------------------------------------------------------------
    template<typename U>
    explicit RefPtr( U* value ) noexcept
    : m_pPtr( value )
    { AddRef(); }

    //---------------------------------------------------------------------------------------------
    //! @brief      デストラクタです.
    //---------------------------------------------------------------------------------------------
    ~RefPtr() noexcept
    { Release(); }

    //---------------------------------------------------------------------------------------------
    //! @brief      代入演算子です.
    //---------------------------------------------------------------------------------------------
    RefPtr& operator = ( T* value ) noexcept
    {
        if ( m_pPtr != value )
        { RefPtr(value).Swap(*this); }
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      代入演算子です.
    //---------------------------------------------------------------------------------------------
    RefPtr& operator = ( const RefPtr& value ) noexcept
    {
        if ( m_pPtr != value.m_pPtr )
        { RefPtr(value).Swap(*this); }
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ムーブ代入演算子です.
    //---------------------------------------------------------------------------------------------
    RefPtr& operator = ( RefPtr&& value ) noexcept
    {
        RefPtr(static_cast<RefPtr&&>(value)).Swap(*this);
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      代入演算子です.
    //---------------------------------------------------------------------------------------------
    template<typename U>
    RefPtr& operator = ( U* value ) noexcept
    {
        RefPtr(value).Swap(*this);
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      代入演算子です.
    //---------------------------------------------------------------------------------------------
    template<typename U>
    RefPtr& operator = ( const RefPtr<U>& value ) noexcept
    {
        RefPtr(value).Swap(*this);
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ムーブ代入演算子です.
    //---------------------------------------------------------------------------------------------
    template<typename U>
    RefPtr& operator = ( RefPtr<U>&& value ) noexcept
    {
        RefPtr(static_cast<RefPtr&&>(value)).Swap(*this);
        return *this;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      代入演算子です.
    //---------------------------------------------------------------------------------------------
    RefPtr& operator = ( std::nullptr_t ) noexcept
    {
        Release();
        return (*this);
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      アタッチします.
    //---------------------------------------------------------------------------------------------
    void Attach( T* value ) noexcept
    {
        if ( m_pPtr != nullptr )
        { m_pPtr->Release(); }
        m_pPtr = value;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      デタッチします.
    //---------------------------------------------------------------------------------------------
    T* Detach() noexcept
    {
        T* ptr = m_pPtr;
        m_pPtr = nullptr;
        return ptr;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      値を入れ替えます.
    //---------------------------------------------------------------------------------------------
    void Swap( RefPtr& value ) noexcept
    {
        T* temp = m_pPtr;
        m_pPtr  = value.m_pPtr;
        value.m_pPtr = temp;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      値を入れ替えます.
    //---------------------------------------------------------------------------------------------
    void Swap( RefPtr&& value ) noexcept
    {
        T* temp = m_pPtr;
        m_pPtr  = value.m_pPtr;
        value.m_pPtr = temp;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      ポインタを取得します.
    //---------------------------------------------------------------------------------------------
    T* GetPtr() const noexcept
    { return m_pPtr; }

    //---------------------------------------------------------------------------------------------
    //! @brief      アドレスを取得します.
    //---------------------------------------------------------------------------------------------
    T* const* GetAddress() const noexcept
    { return &m_pPtr; }

    //---------------------------------------------------------------------------------------------
    //! @brief      アドレスを取得します.
    //---------------------------------------------------------------------------------------------
    T** GetAddress() noexcept
    { return &m_pPtr; }

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを減らしてからアドレスを取得します.
    //---------------------------------------------------------------------------------------------
    T** ReleaseAndGetAddress() noexcept
    {
        Release();
        return &m_pPtr;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      リセットします.
    //---------------------------------------------------------------------------------------------
    void Reset() noexcept
    { Release(); }

    //---------------------------------------------------------------------------------------------
    //! @brief      指定された値にコピーを行います.
    //---------------------------------------------------------------------------------------------
    void CopyTo( T** ptr ) noexcept
    {
        AddRef();
        (*ptr) = m_pPtr;
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      アロー演算子です.
    //---------------------------------------------------------------------------------------------
    T* operator -> () const noexcept
    { return m_pPtr; }

    //---------------------------------------------------------------------------------------------
    //! @brief      bool型へのキャストです.
    //---------------------------------------------------------------------------------------------
    operator bool () const noexcept
    { return m_pPtr != nullptr; }

protected:
    //=============================================================================================
    // protected variables.
    //=============================================================================================
    T*  m_pPtr;     //!< 参照カウントインタフェースを実装するオブジェクトのポインタです.

    //=============================================================================================
    // protected methods.
    //=============================================================================================

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを増やします.
    //---------------------------------------------------------------------------------------------
    void AddRef()
    {
        if ( m_pPtr != nullptr )
        { m_pPtr->AddRef(); }
    }

    //---------------------------------------------------------------------------------------------
    //! @brief      参照カウントを減らします.
    //---------------------------------------------------------------------------------------------
    void Release()
    {
        T* ptr = m_pPtr;
        if ( ptr != nullptr )
        {
            m_pPtr = nullptr;
            ptr->Release();
        }
    }

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};

template<typename T> inline
bool operator == ( const RefPtr<T>& value, const T* ptr )
{ return ( value.GetPtr() == ptr ); }

template<typename T> inline
bool operator == ( const T* ptr, const RefPtr<T>& value )
{ return ( value.GetPtr() == ptr ); }

template<typename T> inline
bool operator == ( const RefPtr<T>& a, const RefPtr<T>& b )
{ return a.GetPtr() == b.GetPtr(); }

template<typename T, typename U> inline
bool operator == ( const RefPtr<T>& a, const RefPtr<U>& b )
{ return a.GetPtr() == b.GetPtr(); }

template<typename T> inline
bool operator != ( const RefPtr<T>& value, const T* ptr )
{ return ( value.GetPtr() != ptr ); }

template<typename T> inline
bool operator != ( const T* ptr, const RefPtr<T>& value )
{ return ( value.GetPtr() != ptr ); }

template<typename T> inline
bool operator != ( const RefPtr<T>& a, const RefPtr<T>& b )
{ return a.GetPtr() != b.GetPtr(); }

template<typename T, typename U> inline
bool operator != ( const RefPtr<T>& a, const RefPtr<U>& b )
{ return a.GetPtr() != b.GetPtr(); }

template<typename T> inline
bool operator == ( const RefPtr<T>& value, std::nullptr_t )
{ return ( value.GetPtr() == nullptr ); }

template<typename T> inline
bool operator == ( std::nullptr_t, const RefPtr<T>& value )
{ return ( value.GetPtr() == nullptr ); }

template<typename T> inline
bool operator != ( const RefPtr<T>& value, std::nullptr_t )
{ return ( value.GetPtr() != nullptr ); }

template<typename T> inline
bool operator != ( std::nullptr_t, const RefPtr<T>& value )
{ return ( value.GetPtr() != nullptr ); }


} // namespace asura
