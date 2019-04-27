﻿//-------------------------------------------------------------------------------------------------
// File : TcpConnector.cpp
// Desc : TCP/IP Connector 
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxTcpConnector.h>
#include <WinSock2.h>
#include <ws2tcpip.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// TcpConnector class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
TcpConnector::TcpConnector()
: m_IsConnected ( false )
, m_IsReady     ( false )
, m_SrcSocket   ( INVALID_SOCKET )
, m_DstSocket   ( INVALID_SOCKET )
, m_IsServer    ( false )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
TcpConnector::~TcpConnector()
{ Close(); }

//-------------------------------------------------------------------------------------------------
//      接続処理を行います
//-------------------------------------------------------------------------------------------------
bool TcpConnector::Connect(const TcpConnector::Desc& info)
{
    if (info.Server)
    { return ConnectAsServer(info); } // ビューア側
    else
    { return ConnectAsClient(info); } // エディタ側.
}

//-------------------------------------------------------------------------------------------------
//      サーバーとしての接続処理を行います
//-------------------------------------------------------------------------------------------------
bool TcpConnector::ConnectAsServer( const TcpConnector::Desc& info )
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    sockaddr_in addr;
    sockaddr_in client;
    int len = sizeof(client);
    int ret = 0;

    WSADATA wsaData;

    if ( !m_IsReady )
    {
        ret = WSAStartup( 0x0202, &wsaData );
        if ( ret != 0 )
        {
            //ELOG( "Error : WSAStartup() Failed." );
            return false;
        }

        // TCP通信の設定でソケットを生成.
        m_SrcSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

        addr.sin_family = AF_INET;
        addr.sin_port   = htons( info.Port );
        if (inet_pton(AF_INET, info.Address, &addr.sin_addr) != 1)
        { return false; }

        // サーバーソケットに名前を付けます.
        ret = bind( m_SrcSocket, (sockaddr*)&addr, sizeof(addr) );
        if ( ret != 0 )
        {
            auto errcode = WSAGetLastError();
            if ( errcode != WSAEADDRINUSE )
            {
                //ELOG( "Error : WSAGetLastError() errorCode = %d", errcode );
                return false;
            }

            // 準備済みフラグを立てえる.
            m_IsReady = true;
        }
    }

    // 非ブロッキングモードにする.
    u_long val = 1;
    ioctlsocket( m_SrcSocket, FIOASYNC, &val );

    // ソケットを受信待機モードにして，保留接続キューのサイズを確保します.
    ret = listen( m_SrcSocket, 5 );
    if ( ret != 0 )
    {
        //ELOG( "Error : listen() Failed." );
        return false;
    }

    // タイムアウト時間を10秒に設定.
    timeval timeout;
    timeout.tv_sec  = 10;
    timeout.tv_usec = 0;
    ret = setsockopt(m_SrcSocket,  SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));
    if (ret != 0)
    {
        // ELOG( "Error : setsockopt() Failed." );
        return false;
    }

    // サーバーソケットを接続要求待ち状態にします.
    m_DstSocket = accept( m_SrcSocket, (sockaddr*)&client, &len );
    if ( m_DstSocket == INVALID_SOCKET )
    {
        //ELOG( "Error : accepct() Failed." );
        return false;
    }

    // 接続済みフラグを立てます.
    m_IsConnected = true;
    m_IsServer    = true;

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      クライアントとしての接続処理を行います
//-------------------------------------------------------------------------------------------------
bool TcpConnector::ConnectAsClient( const TcpConnector::Desc& info )
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    sockaddr_in addr;
    int len = sizeof(sockaddr_in);
    int ret = 0;

    WSADATA wsaData;

    if ( !m_IsReady )
    {
        ret = WSAStartup( 0x0202, &wsaData );
        if ( ret != 0 )
        {
            //ELOG( "Error : WSAStartup() Failed." );
            return false;
        }

        // TCP通信の設定でソケットを生成.
        m_DstSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

        addr.sin_family = AF_INET;
        addr.sin_port   = htons( info.Port );
        if (inet_pton(AF_INET, info.Address, &addr.sin_addr) != 1)
        { return false; }

        ret = connect(m_DstSocket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
        if (ret != 0)
        {
            m_IsConnected = false;
            return false;
        }
    }

    // 非ブロッキングモードにする.
    u_long val = 1;
    ioctlsocket( m_DstSocket, FIOASYNC, &val );

    // 接続済みフラグを立てます.
    m_IsConnected = true;
    m_IsServer    = false;

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      切断処理を行います.
//-------------------------------------------------------------------------------------------------
void TcpConnector::Close()
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    if ( !m_IsConnected && !m_IsReady )
    { return; }

    // 切断通知.
    if (m_IsServer)
    { shutdown( m_SrcSocket, SD_BOTH ); }
    shutdown( m_DstSocket, SD_BOTH );

    // ソケットを解放.
    if (m_IsServer)
    { closesocket( m_SrcSocket ); }
    closesocket( m_DstSocket );

    // 終了処理.
    WSACleanup();

    // フラグをクリア.
    m_IsConnected = false;
    m_IsReady     = false;
}

//-------------------------------------------------------------------------------------------------
//      接続しているかどうかチェックします.
//-------------------------------------------------------------------------------------------------
bool TcpConnector::IsConnect()
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    if ( m_DstSocket == INVALID_SOCKET )
    {
        m_IsConnected = false;
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      送信処理を行います.
//-------------------------------------------------------------------------------------------------
bool TcpConnector::Send( const void* pBuffer, int size )
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    // 引数チェック.
    if ( pBuffer == nullptr || size == 0 )
    { return false; }

    // 送信処理.
    int len = send( m_DstSocket, (const char*)pBuffer, size, 0 );
    if ( len == SOCKET_ERROR )
    { return false; }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      受信処理を行います.
//-------------------------------------------------------------------------------------------------
bool TcpConnector::Receive( void* pBuffer, int size )
{
    std::lock_guard<std::mutex> locker(m_Mutex);

    // 受信処理.
    auto status = recv( m_DstSocket, (char*)pBuffer, size, 0 );

    // エラー.
    if ( status == SOCKET_ERROR )
    {
        //ELOG( "Error : recv() Failed." );
        return false;
    }
    // ソケットが閉じられた場合.
    else if ( status == 0 )
    {
        m_IsConnected = false;
        //DLOG( "Error : Socket is already closed." );
        return false;
    }

    // 正常終了.
    return true;
}

} // namespace asdx