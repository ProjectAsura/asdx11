//-------------------------------------------------------------------------------------------------
// File : asdxSound.cpp
// Desc : Sound Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxSound.h>
#include <asdxMisc.h>
#include <asdxLogger.h>
#include <Windows.h>


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
//      エラーを表示します.
//-------------------------------------------------------------------------------------------------
void ShowError( uint32_t ret )
{
    if ( ret == 0 )
        return;

    char buf[ 512 ];
    mciGetErrorStringA( ret, buf, sizeof(buf) );
    ELOGA( "Error : %s", buf );
}

} // namespace /* anonymous */


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SndMgr class
///////////////////////////////////////////////////////////////////////////////////////////////////
SndMgr SndMgr::s_Instance;

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
SndMgr::SndMgr()
: m_Status()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
SndMgr::~SndMgr()
{
    for( auto itr = m_Status.begin(); itr != m_Status.end(); )
    {
        mciSendCommandW( itr->first, MCI_CLOSE, 0, 0 );
        m_UserIds.erase(itr->second.DeviceId);
        m_Status.erase(itr++);
    }
}

//-------------------------------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-------------------------------------------------------------------------------------------------
SndMgr& SndMgr::GetInstance() 
{ return s_Instance; }

//-------------------------------------------------------------------------------------------------
//      サウンドデータを登録します.
//-------------------------------------------------------------------------------------------------
bool SndMgr::Open( uint32_t id, const char* filename )
{
    if ( filename == nullptr )
    { return false; }

    if ( m_Status.find(id) != m_Status.end() )
    { return false; }

    MCIERROR ret;
    MCI_OPEN_PARMSA param;
    param.lpstrElementName = filename;

    auto ext = GetExtA( filename );
    if ( ext == "wave" || ext == "wav" )
    {
        param.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_WAVEFORM_AUDIO;
        ret = mciSendCommandA( 0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT, (DWORD_PTR)&param );
    }
    else if ( ext == "midi" || ext == "mid" )
    {
        param.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_SEQUENCER;
        ret = mciSendCommandA( 0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT, (DWORD_PTR)&param );
    }
    else if ( ext == "mp3" )
    {
        param.lpstrDeviceType = "MPEGVideo";
        ret = mciSendCommandA( 0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD_PTR)&param );
    }
    else 
    { return false; }

    if ( ret != 0 )
    {
        ShowError( ret );
        return false;
    }

    Status status;
    status.CurLoopCount = 0;
    status.MaxLoopCount = 0;
    status.State        = SndState::Stop;
    status.DeviceId     = param.wDeviceID;

    m_Status[id] = status;
    m_UserIds[status.DeviceId] = id;

    return true;
}

//-------------------------------------------------------------------------------------------------
//      サウンドデータの登録を解除します.
//-------------------------------------------------------------------------------------------------
void SndMgr::Close( uint32_t id )
{
    if ( m_Status.find(id) == m_Status.end() )
    { return; }

    auto ret = mciSendCommandW( m_Status[id].DeviceId, MCI_CLOSE, 0, 0 );
    m_UserIds.erase(m_Status[id].DeviceId);
    m_Status.erase(id);

    ShowError( ret );
}

//-------------------------------------------------------------------------------------------------
//      サウンドデータを再生します.
//-------------------------------------------------------------------------------------------------
void SndMgr::Play( uint32_t id, int loopCount )
{
    if ( m_Status.find(id) == m_Status.end() )
    { return; }

    MCI_PLAY_PARMS param;
    param.dwCallback = reinterpret_cast<DWORD_PTR>(m_Handle);

    auto ret = mciSendCommandW( m_Status[id].DeviceId, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&param );
    if ( ret == 0 )
    {
        m_Status[id].State = SndState::Play;
        m_Status[id].MaxLoopCount = loopCount;
        m_Status[id].CurLoopCount = 0;
    }

    ShowError( ret );
}

//-------------------------------------------------------------------------------------------------
//      サウンドデータを停止します.
//-------------------------------------------------------------------------------------------------
void SndMgr::Stop( uint32_t id )
{
    if ( m_Status.find(id) == m_Status.end() )
    { return; }

    auto ret = mciSendCommandW( m_Status[id].DeviceId, MCI_STOP, 0, 0 );
    if ( ret == 0 )
    { m_Status[id].State = SndState::Stop; }

    if ( ret != 0 )
    { ShowError( ret ); }

    ret = mciSendCommandW( m_Status[id].DeviceId, MCI_SEEK, MCI_SEEK_TO_START, 0 );
    ShowError( ret );
}

//-------------------------------------------------------------------------------------------------
//      サウンドデータを一時停止します.
//-------------------------------------------------------------------------------------------------
void SndMgr::Pause( uint32_t id )
{
    if ( m_Status.find(id) == m_Status.end() )
    { return; }

    auto ret = mciSendCommandW( m_Status[id].DeviceId, MCI_PAUSE, 0, 0 );
    if ( ret == 0 )
    { m_Status[id].State = SndState::Pause; }

    ShowError( ret );
}

//-------------------------------------------------------------------------------------------------
//      一時停止したサウンドデータの再生を再開します.
//-------------------------------------------------------------------------------------------------
void SndMgr::Resume( uint32_t id )
{
    if ( m_Status.find(id) == m_Status.end() )
    { return; }

    auto ret = mciSendCommandW( m_Status[id].DeviceId, MCI_RESUME, 0, 0 );
    if ( ret == 0 )
    { m_Status[id].State = SndState::Play; }

    ShowError( ret );
}

//-------------------------------------------------------------------------------------------------
//      状態を取得します.
//-------------------------------------------------------------------------------------------------
SndState SndMgr::GetState( uint32_t id )
{
    if ( m_Status.find(id) == m_Status.end() )
    { return SndState::Error; }

    return (SndState)m_Status[id].State;
}

//-------------------------------------------------------------------------------------------------
//      コールバックハンドルを設定します.
//-------------------------------------------------------------------------------------------------
void SndMgr::SetHandle( HWND handle )
{ m_Handle = handle; }

//-------------------------------------------------------------------------------------------------
//      コールバック関数です.
//-------------------------------------------------------------------------------------------------
void SndMgr::OnNofity( uint32_t id, uint32_t param )
{
    if ( m_UserIds.find(id) == m_UserIds.end() )
    { return; }

    auto key = m_UserIds[id];

    if ( param == MCI_NOTIFY_SUCCESSFUL )
    {
        mciSendCommandW( m_Status[key].DeviceId, MCI_SEEK, MCI_SEEK_TO_START, 0 );
        m_Status[key].CurLoopCount++;

        if ( m_Status[key].CurLoopCount < m_Status[key].MaxLoopCount || m_Status[key].MaxLoopCount == -1 )
        {
            MCI_PLAY_PARMS param;
            param.dwCallback = reinterpret_cast<DWORD_PTR>(m_Handle);

            auto ret = mciSendCommandW( m_Status[key].DeviceId, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&param );
            if ( ret == 0 )
            { m_Status[key].State = SndState::Play; }
            ShowError( ret );
        }
        else
        {
            m_Status[key].State = SndState::Done;
        }
    }
    else if ( param == MCI_NOTIFY_FAILURE )
    { m_Status[key].State = SndState::Error; }

}


} // namespace asdx
