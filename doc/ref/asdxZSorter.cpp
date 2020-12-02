//-------------------------------------------------------------------------------------------------
// File : asdxZSorter.cpp
// Desc : Z-Sorter Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asdxZSorter.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// ZSorter class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
ZSorter::ZSorter( const Vector3& cameraPosition, const Vector3& cameraDir )
: m_CameraDir( cameraDir )
, m_CameraPosition( cameraPosition )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
ZSorter::~ZSorter()
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      Z比較を行います.
//-------------------------------------------------------------------------------------------------
bool ZSorter::operator () ( const Mesh& a, const Mesh& b ) const
{
    BoundingBox boxA = a.GetBoundingBox();
    BoundingBox boxB = b.GetBoundingBox();

    Vector3 vecA = ( boxA.maxi + boxA.mini ) * 0.5f - m_CameraPosition;
    Vector3 vecB = ( boxB.maxi + boxB.mini ) * 0.5f - m_CameraPosition;

    f32 distA = asdx::Vector3::Dot( m_CameraDir, vecA );
    f32 distB = asdx::Vector3::Dot( m_CameraDir, vecB );

    if ( distA < distB )
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      Z比較を行います.
//-------------------------------------------------------------------------------------------------
bool ZSorter::operator () ( const Mesh* pA, const Mesh* pB ) const
{
    BoundingBox boxA = pA->GetBoundingBox();
    BoundingBox boxB = pB->GetBoundingBox();

    Vector3 vecA = ( boxA.maxi + boxA.mini ) * 0.5f - m_CameraPosition;
    Vector3 vecB = ( boxB.maxi + boxB.mini ) * 0.5f - m_CameraPosition;

    f32 distA = asdx::Vector3::Dot( m_CameraDir, vecA );
    f32 distB = asdx::Vector3::Dot( m_CameraDir, vecB );

    if ( distA < distB )
    { return true; }

    return false;
}

} // namespace asdx
