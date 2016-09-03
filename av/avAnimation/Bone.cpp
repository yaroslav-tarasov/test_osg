#include "Bone.h"

namespace avAnimation 
{

Bone::Bone ( const std::string& Name ) :
 m_Name ( Name ),
 m_nResultIndex ( -1 )
{
}

void Bone::SetInitialMatrix ( const osg::Matrixf& M )
{
    m_InitialMatrix = M;
    m_InitialQuaternion = M.getRotate();
    m_InitialPosition = M.getTrans();
}

void Bone::SetIndexAndModelToBoneMatrix ( int& Index, const osg::Matrixf& Mat )
{
    if ( m_nResultIndex == -1 )
    { 
        m_nResultIndex = Index++;
        m_ModelToBoneMatrix = Mat;
    }
}

void Bone::Animate ( float CurPosInFrames, float BlendIncomingFactor, const std::vector<FrameBonePosition> &ak )
{
    osg::Quat q;
    osg::Vec3f v; 

    if ( CurPosInFrames <= ak.front()._Frame )
    {
        q = ak.front()._quaternion;
        v = ak.front()._position;
    }
    else if ( CurPosInFrames >= ak.back()._Frame )
    {
        q = ak.back()._quaternion;
        v = ak.back()._position;
    }
    else
    {   
        unsigned int i = 0;
        for ( ; i < ak.size ( ) - 1; i++ )
            if ( CurPosInFrames < ak [ i + 1 ]._Frame )
                break;
        // if it will be needed could precalculate and store some intermediate values for farther 
        // two lines calculation (especially slerp) speedup
        float factor = ( CurPosInFrames - ak [ i ]._Frame ) / ( ak [ i + 1 ]._Frame - ak [ i ]._Frame );
        q.slerp( factor, ak [ i ]._quaternion, ak [ i + 1 ]._quaternion );
        //q = ak [ i ]._quaternion;
        v = ( ak [ i ]._position * ( 1.0f - factor ) + ak [ i + 1 ]._position * factor );
    }
    //BlendIncomingFactor = 1.0f;
    if ( BlendIncomingFactor == 1.0f )
    {
        m_AccumQuaternion = q; // Try to get rid of sins and acos in simple case
    }
    else
    {
        m_AccumQuaternion.slerp( BlendIncomingFactor, m_AccumQuaternion, q );
    }
    m_AccumPosition = m_AccumPosition * ( 1.0f - BlendIncomingFactor ) + v * BlendIncomingFactor;
    m_bWasAnimated = true;
}

void Bone::Reset()
{
    m_bWasAnimated = false;
    // Some bones could be affected by additional animations without been touched by main animations
    // so we need m_AccumQuaternion and m_AccumPosition to be properly initialized
    m_AccumQuaternion = m_InitialQuaternion; 
    m_AccumPosition = m_InitialPosition;   
}

osg::Matrixf Bone::GetCurrentMatrix() const
{
    //osg::Matrixf CurrentMatrix ( ( ( m_bWasAnimated ) ? 
    //                                              osg::Matrixf::rotate ( m_AccumQuaternion ) * osg::Matrixf::translate ( m_AccumPosition ) :
    //                                              m_InitialMatrix )); 
    // The following equivalent code is faster
    osg::Matrixf CurrentMatrix ( m_InitialMatrix );
    if ( m_bWasAnimated )
    {
        CurrentMatrix.setRotate ( m_AccumQuaternion );
        CurrentMatrix.setTrans ( m_AccumPosition );
    }
    return CurrentMatrix;
}

const std::string& Bone::GetName() const
{
    return m_Name;
}

}