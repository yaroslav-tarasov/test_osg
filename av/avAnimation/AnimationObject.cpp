#include <osgDB/FileUtils>

#include "AnimationObject.h"

namespace avAnimation 
{

bool IsEqual ( float delta, const osg::Matrixf& A, const osg::Matrixf& B );

// ****************** AnimationObjectImpl ************************
struct AO_UpdateCallback : public osg::NodeCallback
{
    void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        AnimationObjectImpl* AO = dynamic_cast <AnimationObjectImpl*> ( node );
        if ( AO )
        {
            AO -> Update ( nv->getFrameStamp()->getSimulationTime());
        }
        traverse(node,nv);
    }
};

class AO_CullCallback : public osg::NodeCallback
{
    void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        AnimationObjectImpl* AO = dynamic_cast <AnimationObjectImpl*> ( node );
        if ( AO )
        {
            AO -> UpdateAfterCull ( nv->getFrameStamp()->getSimulationTime());
        }
        traverse(node,nv);
    }
};


AnimationObjectImpl::AnimationObjectImpl () :
 m_pAnimationPrototype ( NULL ),
 m_bAnimationElementsRepeat ( true )
{
    m_CurAnimationElement = m_AnimationElementQueue.end();
    osg::StateSet* stateset = getOrCreateStateSet();
    m_UniformBoneMatrices = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "BoneMatrices", MaxBlendingMatricesPerModel );
    m_UniformScale = new osg::Uniform("Scale", osg::Vec3 ( 1.0f, 1.0f, 1.0f ) );
    stateset->addUniform(m_UniformBoneMatrices.get());
    stateset->addUniform(m_UniformScale.get());
    stateset->setDataVariance ( osg::Object::DYNAMIC );
    setUpdateCallback ( new AO_UpdateCallback ( ) );
    setCullCallback ( new AO_CullCallback ( ) );
}

AnimationObjectImpl::~AnimationObjectImpl ( )
{
}

void AnimationObjectImpl::Update ( float SimulationTime )
{
    bool ThatsTheFirstPass (false);
    if ( m_CurAnimationElement == m_AnimationElementQueue.end() && m_AnimationElementQueue.size() > 0 )
    {
        m_CurAnimationElement = m_AnimationElementQueue.begin();
        // We should get here only once for the first AnimationElement even with repeat
        // the m_fRouteStartTime of the first AnimationElement keeps common m_fStartTimeOffset now
        m_CurAnimationElement -> m_fRouteStartTime += SimulationTime;
        ThatsTheFirstPass = true;
    }
    if ( m_CurAnimationElement == m_AnimationElementQueue.end() )
    {
        return;
    }


    float TimeFromAnimationStart = SimulationTime - m_CurAnimationElement->m_fRouteStartTime;
    if ( TimeFromAnimationStart >= 0.0f )
    {
        // Go through all the AnimationElements trying to understand where we are
        bool FoundProperAnimationElementOrNoMoreElements (false);
        while ( !FoundProperAnimationElementOrNoMoreElements ) 
        {
            const AnimationElement &AE = *m_CurAnimationElement;
            bool ItsTheLastAnimationElement = ++std::list<AnimationElement>::iterator(m_CurAnimationElement) == m_AnimationElementQueue.end();
            bool WeAreInsideThisAnimationElement = TimeFromAnimationStart < AE.m_fRoutePeriod * AE.m_nRepeatTimes;
            if (!WeAreInsideThisAnimationElement && ItsTheLastAnimationElement && !m_bAnimationElementsRepeat && ThatsTheFirstPass)
            {
                // Ok even if animation time has passed a long time ago we still need to take the last AnimationElement 
                // and play a frame from it's end
                WeAreInsideThisAnimationElement = true;
                TimeFromAnimationStart = AE.m_fRoutePeriod * AE.m_nRepeatTimes - 0.001; // Just before end
                m_CurAnimationElement -> m_fRouteStartTime = SimulationTime - TimeFromAnimationStart;
            }
            if ( WeAreInsideThisAnimationElement )
        {
            float TimeFromLoopStart = fmod ( TimeFromAnimationStart, AE.m_fRoutePeriod );
            for ( int i = 0; i < int (AE.m_Route.size()) - 1; i++ )
            {
                osg::Vec3f ToNext ( AE.m_Route [ i + 1 ] - AE.m_Route [ i ] );
                float TurnTime = AE.m_RouteTiming [ i * 2 ];
                if ( TimeFromLoopStart < TurnTime )
                {
                    _ASSERT ( i > 0 );
                    ToNext.normalize();
                    osg::Vec3f FromPrev ( AE.m_Route [ i ] - AE.m_Route [ i - 1 ] );
                    FromPrev.normalize();
                    float factor = TimeFromLoopStart / TurnTime;
                    osg::Vec3f dir ( FromPrev * ( 1.0 - factor ) + ToNext * factor ); 
                    setMatrix ( osg::Matrixf::rotate ( AE.m_ForwardDirection, dir ) *
                                osg::Matrixf::translate ( AE.m_Route [ i ] ) ); 
                    break;
                }
                TimeFromLoopStart -= TurnTime;
                float MoveTime = AE.m_RouteTiming [ i * 2 + 1 ];
                if ( TimeFromLoopStart < MoveTime )
                {   
                    float factor = TimeFromLoopStart / MoveTime;
                    osg::Vec3f v ( AE.m_Route [ i ] * ( 1.0 - factor ) + AE.m_Route [ i + 1 ] * factor ); 
                    setMatrix ( osg::Matrixf::rotate ( AE.m_ForwardDirection, ToNext ) *
                                osg::Matrixf::translate ( v ) ); 
                    break;
                }
                TimeFromLoopStart -= MoveTime;
            }
                FoundProperAnimationElementOrNoMoreElements = true;
        }
        else
        {
                TimeFromAnimationStart -= AE.m_fRoutePeriod * AE.m_nRepeatTimes;
            if ( m_bAnimationElementsRepeat )
            {
                if ( ++m_CurAnimationElement == m_AnimationElementQueue.end() )
                    m_CurAnimationElement = m_AnimationElementQueue.begin();
            }
            else
            {
                _ASSERT ( !m_AnimationElementQueue.empty() );
                _ASSERT ( m_CurAnimationElement == m_AnimationElementQueue.begin() );
                m_CurAnimationElement = m_AnimationElementQueue.erase ( m_CurAnimationElement );
            }
            if ( m_CurAnimationElement != m_AnimationElementQueue.end() ) 
            {
                if ( m_CurAnimationElement->m_bClearTheQueue )
                {
                    m_AnimationElementQueue.erase(m_AnimationElementQueue.begin(),m_CurAnimationElement);
                }
                    m_CurAnimationElement -> m_fRouteStartTime = SimulationTime - TimeFromAnimationStart;
                }
                else
                {
                    FoundProperAnimationElementOrNoMoreElements = true;
                }
            }
        }
    }
}

void AnimationObjectImpl::UpdateAfterCull ( float SimulationTime )
{
    if ( m_CurAnimationElement == m_AnimationElementQueue.end() ) 
    {
        return;
    }

    AnimationElement &NonConstAE = *m_CurAnimationElement;
    float TimeFromAnimationStart = SimulationTime - NonConstAE.m_fRouteStartTime;
    if ( TimeFromAnimationStart >= 0.0f && m_pAnimationPrototype )
    { 
        // Should we blend to the next AnimationElement?
        float ToNextAnimationBlendFactor = 0.0f;
        const AnimationElement *NextAnimation = 0;
        std::list<AnimationElement>::iterator NextAnimationIt = m_CurAnimationElement;
        ++NextAnimationIt;

        if ( NextAnimationIt == m_AnimationElementQueue.end() && m_bAnimationElementsRepeat )
        {
            NextAnimationIt = m_AnimationElementQueue.begin();
        }
        if ( NextAnimationIt != m_AnimationElementQueue.end() )
        {
            NextAnimation = &*NextAnimationIt;
        }

        float BlendToNextAnimationFactor = 0.0f;    
        float TimeToNextAnimationStart = NonConstAE.m_fRoutePeriod * NonConstAE.m_nRepeatTimes - TimeFromAnimationStart;
        bool NeedToBlend = TimeToNextAnimationStart >= 0.0 && TimeToNextAnimationStart < NonConstAE.m_fBlendToNextTime;
        
        if ( NeedToBlend )
        {
            BlendToNextAnimationFactor = 1.0 - TimeToNextAnimationStart / NonConstAE.m_fBlendToNextTime;
        }
        else
        {
            NextAnimation = 0;
        }


        m_pAnimationPrototype->Update ( TimeFromAnimationStart, &NonConstAE, m_UniformBoneMatrices.get(), m_trackableNodesMap, NextAnimation, BlendToNextAnimationFactor );
    }
}

bool AnimationObjectImpl::SetModel ( const char *Model, ResourceLoaderPtr ResourceLoader )
{
    m_CurAnimationElement = m_AnimationElementQueue.end(); // Reset _CurAnimationElement
    removeChildren ( 0, getNumChildren() );
    m_trackableNodesMap.clear();
    m_pAnimationPrototype = AnimationPrototype::CreateAnimationPrototype ( Model, ResourceLoader );
    if ( m_pAnimationPrototype )
    {
        addChild ( m_pAnimationPrototype );
    }
    return m_pAnimationPrototype != 0;
}

void AnimationObjectImpl::AddAnimationElement ( const std::string &AnimationName, float StartTime, float Duration, int RepeatTimes, 
                                                                                   float BlendToNextTime, bool ClearTheQueue )
{
    if ( !m_pAnimationPrototype )
    { 
        return;
    }
    const AnimationSet *AS = m_pAnimationPrototype -> GetAnimationSet ( AnimationName );
    if ( !AS )
    {
        return;
    }

    m_AnimationElementQueue.push_back( AnimationElement() );
    AnimationElement &AE = m_AnimationElementQueue.back ();

    AE.m_pAnimationSet = AS;
    AE.m_Route = std::vector<osg::Vec3f> ( );
    AE.m_fSpeed = 0.0f;
    AE.m_fRouteStartTime = StartTime;
    AE.m_nRepeatTimes = RepeatTimes;
    AE.m_fRoutePeriod = Duration;
    AE.m_fDuration = Duration;
    AE.m_fBlendToNextTime = BlendToNextTime;
    AE.m_bClearTheQueue = ClearTheQueue;
}

void AnimationObjectImpl::AddRoutedAnimationElement ( const std::string &AnimationName, float StartTime, float Duration, 
                                                      const std::vector<osg::Vec3f> &route, float Speed, int RepeatTimes, 
                                                      float BlendToNextTime, const osg::Vec3f *ForwardDirection )
{
    if ( !m_pAnimationPrototype )
    {
        return;
    }
    const AnimationSet *AS = m_pAnimationPrototype -> GetAnimationSet ( AnimationName );
    if ( !AS )
    {
        return;
    }

    m_AnimationElementQueue.push_back( AnimationElement() );
    AnimationElement &AE = m_AnimationElementQueue.back ();

    AE.m_pAnimationSet = AS;
    AE.m_Route = route;
    AE.m_fSpeed = Speed;
    AE.m_fRouteStartTime = StartTime;
    AE.m_nRepeatTimes = RepeatTimes;
    AE.m_fRoutePeriod = 0.0f;
    AE.m_fDuration = Duration;
    AE.m_fBlendToNextTime = BlendToNextTime;
    AE.m_ForwardDirection = ForwardDirection == 0 ? osg::Vec3f ( 0.0f, 1.0f, 0.0f ) : *ForwardDirection;
    AE.m_bClearTheQueue = false;

    // Let's define that turn speed is proportional to linear speed
    float TurnSpeed = Speed * osg::PI; // PI per 1 m/s

    for ( int i = 0; i < int (route.size()) - 1; i++ )
    {   
        osg::Vec3f ToNext ( route [ i + 1 ] - route [ i ] );
        if ( i > 0 )
        {   
            osg::Vec3f FromPrev ( route [ i ] - route [ i - 1 ] );
            float Cos = ( FromPrev * ToNext ) / ( FromPrev.length () * ToNext.length () );
            if ( Cos < -1.0f )
            {
                Cos = -1.0f;                    
            }
            else if ( Cos > 1.0f )
            {
                Cos = 1.0f;                    
            }
            AE.m_RouteTiming.push_back ( acos ( Cos ) / TurnSpeed );
            AE.m_fRoutePeriod += AE.m_RouteTiming.back();
        }
        else
        {
            AE.m_RouteTiming.push_back ( 0.0f ); // insert the fictive rotation time for the first point
        }
        AE.m_RouteTiming.push_back ( ToNext.length () / Speed );
        AE.m_fRoutePeriod += AE.m_RouteTiming.back();
    }
    
    if ( route.size() < 2 && m_pAnimationPrototype )
    {
        AE.m_fRoutePeriod += AE.m_fDuration;
    }
}

void AnimationObjectImpl::AddAnimationSubElement ( const std::string &AnimationName, float PauseLength, float Duration, 
                                                                                float BlendBorderLength )
{
    if ( !m_pAnimationPrototype )
    {
        return;
    }
    const AnimationSet *AS = m_pAnimationPrototype -> GetAnimationSet ( AnimationName );
    if ( !AS )
    {
        return;
    }

    if ( m_AnimationElementQueue.empty() )
    {
        return;
    }

    m_AnimationElementQueue.back().m_SubElements.push_back ( AnimationSubElement ( ) );
    AnimationSubElement &ASE = m_AnimationElementQueue.back ( ).m_SubElements.back ( );
    ASE.m_pAnimationSet = AS;
    ASE.m_fPauseLength = PauseLength;
    ASE.m_fBlendBorderLength = BlendBorderLength;
    ASE.m_fDuration = Duration;
}

void AnimationObjectImpl::RemoveAnimationElements ( )
{
    m_AnimationElementQueue.clear ();
    m_CurAnimationElement = m_AnimationElementQueue.end();
}

void AnimationObjectImpl::SetAnimationElementsRepeat ( bool Repeat )
{
    m_bAnimationElementsRepeat = Repeat;
    // if Repeat is false then clear all the AnimationElement before the current
    //if ( !Repeat )
    //    _AnimationElementQueue.erase ( _AnimationElementQueue.begin (), _CurAnimationElement );
    
}

void AnimationObjectImpl::SetScale ( const osg::Vec3 &Scale )
{
    if ( m_UniformScale.valid() )
    {
        m_UniformScale->set ( Scale );
    }
}

void AnimationObjectImpl::AddNameToCull( const char *Name )
{
    m_CulledNames.insert ( Name );
}

void AnimationObjectImpl::RemoveNameToCull( const char *Name )
{
    m_CulledNames.erase ( Name );
}

bool AnimationObjectImpl::IsNameCulled( const char *Name ) const
{
    return m_CulledNames.find ( Name ) != m_CulledNames.end();
}

osg::MatrixTransform * AnimationObjectImpl::AddTrackableBone( const char * szBoneName )
{
    if (!m_pAnimationPrototype)
        return NULL;

    // first, find in current childrens map
    ExternalBonesMap::iterator boneChildIt = m_trackableNodesMap.find(szBoneName);
    if (boneChildIt != m_trackableNodesMap.end())
        return boneChildIt->second;

    // not found - add new one
    osg::ref_ptr<osg::MatrixTransform> pNewChild = new osg::MatrixTransform();
    addChild(pNewChild.get());
    return m_trackableNodesMap.insert(ExternalBonesMap::value_type(szBoneName, pNewChild)).first->second;
}

AnimationObject *CreateAnimationObject ( const char *Model, ResourceLoaderPtr ResourceLoader )
{
    AnimationObjectImpl *AOI = new AnimationObjectImpl ;
    
    if ( AOI->SetModel ( Model, ResourceLoader ))
        return AOI;

    delete AOI;

    return 0;
}

} // namespace avAnimation 