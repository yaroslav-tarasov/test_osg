#include "stdafx.h"
#include "avAnimation/AnimationManager.h"
#include "avAnimation/Man.h"

using namespace avAnimation;

AnimationManager* AnimationManager::_pAnimationManager = NULL;
avDynamicObject::RegisterVisualDynamicObjectFactoryHelper AnimationManager::AnimationsFactory ( "Man", AnimationManager::CreateAnimation );

AnimationManager::AnimationManager()
{
    AddHandler ( "manState", EventHandler_ManState );
    AddHandler ( "manOutfitState", EventHandler_ManOutfitState);
    AddHandler ( "manTools", EventHandler_ManTools);

    RegisterEventListeners();
}

//////////////////////////////////////////////////////////////////////////
AnimationManager::~AnimationManager()
{
    UnregisterEventListeners();
}

AnimationManager* AnimationManager::GetInstance()
{
    avAssert( _pAnimationManager );
    return _pAnimationManager;
}

void AnimationManager::Create()
{
    avAssert( _pAnimationManager == NULL );
    _pAnimationManager = new AnimationManager;
}

void AnimationManager::Release()
{
    avAssert( _pAnimationManager );
    svReleaseMem( _pAnimationManager );
}

// static method of ship creating
avDynamicObject::VisualDynamicObject * AnimationManager::CreateAnimation( unsigned char ucObjectClassEnum, uint32_t nID, avCore::XML::XmlNode pInfoXMLRootElement, const std::string& FolderFileName )
{
    return CreateMan ( nID, pInfoXMLRootElement );
}

void AnimationManager::EventHandler_ManState( avCore::MessageManager::MessageStream& mStream )
{
    // Get data from the event
    uint32_t objectID = mStream.get<uint32_t>();
    unsigned count;
    const avCore::uint8* animation = mStream.get_vector<avCore::uint8> ( count );

    // Process event
    avAnimation::Man* man = avCore::ObjectManager::GetInstance()->GetObjectByID<avAnimation::Man>( avCore::ObjectBase::OBJECT_DYNAMIC, objectID );

    if ( man != NULL && count != 0 )
    {
        man->setAnimation ( animation, count );
    }
}

void AnimationManager::EventHandler_ManOutfitState ( avCore::MessageManager::MessageStream& mStream )
{
    // Get data from the event
    uint32_t objectID = mStream.get<uint32_t>();
    const char*	SubobjectName = mStream.get<const char*>();
    bool CullingIsOn = mStream.get<bool>();

    // Process event
    avAnimation::Man* man = avCore::ObjectManager::GetInstance()->GetObjectByID<avAnimation::Man>( avCore::ObjectBase::OBJECT_DYNAMIC, objectID );

    if ( man != NULL )
    {
        man->setSubobjectCulling( SubobjectName, CullingIsOn );
    }
}

void AnimationManager::EventHandler_ManTools ( avCore::MessageManager::MessageStream& mStream )
{
    // Get data from the event
    uint32_t objectID = mStream.get<uint32_t>();
    unsigned count;
    const uint32_t * pTools = mStream.get_vector<uint32_t>(count);

    // Process event
    avAnimation::Man * pMan = avCore::ObjectManager::GetInstance()->GetObjectByID<avAnimation::Man>(avCore::ObjectBase::OBJECT_DYNAMIC, objectID);
    if (pMan)
    {
        pMan->clearAllTools();
        for (unsigned i = 0; i < count; ++i)
            pMan->placeTool(i, pTools[i]);
    }
}
