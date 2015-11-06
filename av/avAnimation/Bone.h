#pragma once

#include <string>
#include <vector>

namespace avAnimation 
{
struct FrameBonePosition                                        // Position of the bone for a given frame
{
    int             _Frame;
    osg::Quat       _quaternion;
    osg::Vec3f      _position;
    FrameBonePosition ( int Frame, const osg::Quat &quaternion, const osg::Vec3f &position ) :
     _Frame ( Frame ),
     _quaternion ( quaternion ),
     _position ( position )
    {
    }
};

class Bone : public osg::Referenced
{
public:
    Bone ( const std::string& Name );
    void                                SetInitialMatrix ( const osg::Matrixf& M );
    void                                SetIndexAndModelToBoneMatrix ( int& Index, const osg::Matrixf& Mat );
    void                                Animate ( float CurPosInFrames, float BlendIncomingFactor, const std::vector<FrameBonePosition> &ak );
    void                                Reset();
    osg::Matrixf                        GetCurrentMatrix() const;
    const std::string&                  GetName() const;
    int                                 GetMatrixIndex() const { return m_nResultIndex; }
    const osg::Matrixf                  GetInitialMatrix() const { return m_InitialMatrix; }
    const osg::Matrixf                  GetModelToBoneMatrix() const { return m_ModelToBoneMatrix; }
    std::vector<osg::ref_ptr<Bone>>&    GetChildren() { return m_Children; }

private:
    osg::Quat                       m_InitialQuaternion; 
    osg::Vec3f                      m_InitialPosition;   
    osg::Quat                       m_AccumQuaternion;
    osg::Vec3f                      m_AccumPosition;
    bool                            m_bWasAnimated;
    std::string                     m_Name;
    int                             m_nResultIndex;
    osg::Matrixf                    m_InitialMatrix, m_ModelToBoneMatrix;
    std::vector<osg::ref_ptr<Bone>> m_Children;


};

}
