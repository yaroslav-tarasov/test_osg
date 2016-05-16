#pragma once

#include "avCore/avCore.h"
#include "avDynamicObject/DynamicObjectManager.h"

namespace avAnimation 
{

    class AnimationManager : public avCore::EventHandlerMap<avCore::ObjectStatic>
    {
    public:

        static AnimationManager*    GetInstance();
        static void                 Create();
        static void                 Release();

        static avDynamicObject::VisualDynamicObject* CreateAnimation( unsigned char ucObjectClassEnum, uint32_t nID, avCore::XML::XmlNode pInfoXMLRootElement, const std::string& FolderFileName );

    private:
        AnimationManager();
        ~AnimationManager();

        static void EventHandler_ManState ( avCore::MessageManager::MessageStream& mStream ); 
        static void EventHandler_ManOutfitState ( avCore::MessageManager::MessageStream& mStream ); 
        static void EventHandler_ManTools ( avCore::MessageManager::MessageStream& mStream ); 

    private:
        static AnimationManager*    _pAnimationManager;
        static avDynamicObject::RegisterVisualDynamicObjectFactoryHelper AnimationsFactory;
    };

#define GetAnimationManager() AnimationManager::GetInstance()

}