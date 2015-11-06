#pragma once

#include <osg/Geode>
#include "AnimationLoader.h"
#include "Bone.h"

namespace avAnimation 
{

struct AnimationKey : public std::vector<FrameBonePosition> // All positions of the bone for some given frames
{
    Bone*   m_pBone;
};

struct AnimationSet : public std::vector<AnimationKey>
{
    int     m_LengthInFrames;
};

typedef std::map<std::string,AnimationSet> AnimationCollection;

struct AnimationSubElement
{
    const AnimationSet*         m_pAnimationSet;
    float                       m_fPauseLength;
    float                       m_fBlendBorderLength;
    float                       m_fDuration;
};

struct AnimationElement
{
    const AnimationSet*                 m_pAnimationSet;
    std::vector<osg::Vec3f>             m_Route;
    std::vector<float>                  m_RouteTiming;
    float                               m_fRouteStartTime;
    float                               m_fRoutePeriod;
    int                                 m_nRepeatTimes;
    float                               m_fSpeed;
    float                               m_fDuration;
    float                               m_fBlendToNextTime;
    std::vector<AnimationSubElement>    m_SubElements;
    osg::Vec3f                          m_ForwardDirection; 
    bool                                m_bClearTheQueue;
};

typedef std::map<std::string, osg::MatrixTransform *> ExternalBonesMap;

class AnimationPrototype : public osg::Geode, public AnimationLoader
{
public:
    AnimationPrototype ( ResourceLoaderPtr TextureLoader );
    ~AnimationPrototype ( );
    
    void                        Update ( float TimeFromAnimationStart, const AnimationElement* MainAE, osg::Uniform* Table, ExternalBonesMap & extMatrixMap,
                                            const AnimationElement* NextAE, float ToNextAnimationBlendFactor );
    const AnimationSet*         GetAnimationSet ( const std::string& AnimationName ) const;
    static AnimationPrototype*  CreateAnimationPrototype ( const char* Model, ResourceLoaderPtr );

    Bone*       GetBone ( const std::string& Name );

private:
    Bone*       CreateBone ( const std::string& Name );
    void        ResetBones ( Bone* bone );
    void        UpdateBones ( float TimeFromAnimationStart, const AnimationElement& AE, float CurNextInfluenceBlendFactor );
    void        RecalcBones ( Bone* bone, const osg::Matrixf& Accumulated, osg::Uniform* Table, ExternalBonesMap & m_extMatrixMap );
    bool        RemoveRedundantBones ( Bone* bone, Bone* FatherBone );
    void        OptimizeBonesTree ( );
    // Just for input data consistency test
    void        RecalcMTBM ( Bone* bone, const osg::Matrixf& Accumulated );
    
    // AnimationLoader interface implementation 
    // For loading Bones tree
    void        AddBone ( const std::string& Name );  
    void        SetBoneInitialMatrix ( const osg::Matrixf& M );
    void        MoveLevelDown ( );
    void        MoveLevelUp ( );

    // For loading additional Bones and setting ModelToBoneMatrix
    void        SetIndexAndModelToBoneMatrix ( const std::string& Name, const osg::Matrixf& Mat );
    // For loading Drawables
    void        AddDrawable( const std::string& Name, std::vector<osg::Vec3f>& Vertices, std::vector<osg::Vec3f>& Normals,
                             std::vector<osg::Vec2f>& TxtCoord, std::vector<int>& Indices,
                             std::map<std::string, std::vector<std::pair<int,float>>>& BonesMapOfVerticesWeghtsTables,
                             const std::string& TextureName
                            );

    // For building Bones tree
    void        BeginAnimationSet ( const std::string& AnimationSetName );
    void        BeginAnimation ( const std::string& BoneName, int FramesNum );
    void        AddFrame ( int Frame, const osg::Matrixf& osgM );
    void        EndAnimation ( );
    void        EndAnimationSet ( );
    // End of AnimationLoader interface implementation 

    typedef std::map<std::string, Bone*> PrototypeBoneMap;

    std::string                                         m_Name;
    osg::ref_ptr<Bone>                                  m_RootBone;
    PrototypeBoneMap                                    m_BoneMap;
    AnimationCollection                                 m_Animations;
    bool                                                m_bTextureDefined;
    int                                                 m_nMatrixIndex;

    static std::map<std::string, AnimationPrototype*>   m_Cache;

    // Data needed during loading time
    AnimationCollection::iterator                       m_CurAnimationSetForLoad;  
    osg::Matrixf                                        m_InitialMatrix;
    bool                                                m_bThisMatrixIsNotUsed;
    bool                                                m_bAllEqualToInitial;
    std::vector<Bone*>                                  m_BonesLoadingStack;             
    Bone*                                               m_pLastBoneAdded;
    bool                                                m_bNeedToOptimize;
    
    ResourceLoaderPtr m_ResourceLoader ;
};

const int MaxBlendingMatricesPerModel = 60;

}

