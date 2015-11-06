#pragma once

#include "Animation.h"
#include "AnimationPrototype.h"

namespace avAnimation 
{

class AnimationObjectImpl : public AnimationObject
{
public:
    AnimationObjectImpl ();
    ~AnimationObjectImpl ( );
    void Update ( float SimulationTime );
    void UpdateAfterCull ( float SimulationTime );
    bool SetModel ( const char *Model, ResourceLoaderPtr ); 

private:
    // AnimationObject Interface implementation
    //
    void AddAnimationElement ( const std::string& AnimationName, float StartTime, float Duration, int RepeatTimes = 1, float BlendToNextTime = 0.0f,
                                                                                bool ClearTheQueue = false );
    void AddRoutedAnimationElement ( const std::string& AnimationName, float StartTime, float Duration, const std::vector<osg::Vec3f>& route, 
                                                                                float Speed, int RepeatTimes = 1, float BlendToNextTime = 0.0f, const osg::Vec3f *ForwardDirection = 0 );
    void AddAnimationSubElement ( const std::string& AnimationName, float PauseLength, float Duration, float _BlendBorderLength = 0.0f );
    void RemoveAnimationElements ( );
    void SetAnimationElementsRepeat ( bool Repeat );
    void AddNameToCull ( const char* Name );
    void RemoveNameToCull ( const char* Name );
    bool IsNameCulled ( const char* Name ) const;
    void SetScale ( const osg::Vec3& Scale );

    osg::MatrixTransform * AddTrackableBone( const char * szBoneName );
    //

    AnimationPrototype*                     m_pAnimationPrototype;
    osg::ref_ptr<osg::Uniform>              m_UniformBoneMatrices;
    osg::ref_ptr<osg::Uniform>              m_UniformScale;

    std::list<AnimationElement>             m_AnimationElementQueue;
    std::list<AnimationElement>::iterator   m_CurAnimationElement;
    std::set<std::string>                   m_CulledNames;
    bool                                    m_bAnimationElementsRepeat;

    ExternalBonesMap                        m_trackableNodesMap;
};

}

