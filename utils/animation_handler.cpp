#include "stdafx.h"
#include "animation_handler.h"



AnimationHandler::AnimationHandler(osg::Node* model,const std::string animationName
                                                   ,on_effect_f on_fire
                                                   ,on_effect2_f on_test_effect
                                                   ,on_effect2_f on_lod_effect)
    : model_         (model)
    , animationName_ (animationName)
    , manager_       (nullptr)
    , on_fire_       (on_fire)
    , on_test_effect_(on_test_effect)
    , on_lod_effect_ (on_lod_effect)
{

    manager_ =  dynamic_cast<osgAnimation::BasicAnimationManager*> ( model_->getUpdateCallback() );

    if ( manager_ )
    {   
        const osgAnimation::AnimationList& animations =
            manager_->getAnimationList();

        std::cout << "**** Animations ****" << std::endl;

        for ( unsigned int i=0; i<animations.size(); ++i )
        {
            const std::string& name = animations[i]-> getName();
            std::cout << "Animation name: " << name << std::endl;
        }

        std::cout << "********************" << std::endl;
    }

}


bool AnimationHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
    if (!viewer) return false;

    switch(ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F1/*'1'*/)
            {
                osg::notify(osg::NOTICE)<<"Play first animation"<<std::endl;
                AnimateIt(osgAnimation::Animation::ONCE);
            }                
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F2/*'2'*/)
            {
                osg::notify(osg::NOTICE)<<"Play second animation"<<std::endl;
                AnimateIt(osgAnimation::Animation::ONCE_BACKWARDS);
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F3)
            {
                
                osg::notify(osg::NOTICE)<<"Play third animation"<<std::endl;
                AnimateIt(osgAnimation::Animation::STAY);
                //osg::notify(osg::NOTICE)<<"Fire !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
                //if(on_fire_) 
                //    on_fire_();
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F7)
            {
                static bool off = true;
                osg::notify(osg::NOTICE)<<"Test !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
                off = !off;
                if(on_test_effect_)
                    on_test_effect_(off);
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F5)
            {
                static bool low = false;
                osg::notify(osg::NOTICE)<<"Test !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
                low = !low;
                if(on_lod_effect_)
                    on_lod_effect_(low);
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F11)
            {
                osg::notify(osg::NOTICE)<<"KEY_F11 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
                
                if ( manager_ )
                {   

                    const osgAnimation::AnimationList& animations =
                        manager_->getAnimationList();
                   
                    for ( unsigned int i=0; i<animations.size(); ++i )
                    {
                        const std::string& name = animations[i]->getName();
                        animations[i]->computeDuration();
                        double d = animations[i]->getDuration();
                        if ( name==animationName_ )
                        {
                            // animations[i]->setStartTime(d/2.f);
                            animations[i]->update(0);
                            // animations[i]->setPlayMode(/*pm*/osgAnimation::Animation::ONCE);                   
                            // manager_->playAnimation( animations[i].get());
                        }

                    }

                    manager_->update(0);
                }
            } 
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F12)
            {
                osg::notify(osg::NOTICE)<<"KEY_F12 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;

                if ( manager_ )
                {   

                    const osgAnimation::AnimationList& animations =
                        manager_->getAnimationList();
                    
                    double d = 0.0;
                    
                    for ( unsigned int i=0; i<animations.size(); ++i )
                    {
                        const std::string& name = animations[i]->getName();
                        animations[i]->computeDuration();
                        d = animations[i]->getDuration();
                        if ( name==animationName_ )
                        {
                            // animations[i]->setStartTime(d/2.f);
                            animations[i]->update(d/2.f);
                            // animations[i]->setPlayMode(/*pm*/osgAnimation::Animation::ONCE);                   
                            // manager_->playAnimation( animations[i].get());
                        }

                    }

                    manager_->update(d/2.f);
                }
            } 

            return true;
        }

    default:
        return false;
    }
}

void AnimationHandler::AnimateIt(osgAnimation::Animation::PlayMode pm)
{
    if ( manager_ )
    {   

        const osgAnimation::AnimationList& animations =
            manager_->getAnimationList();

        for ( unsigned int i=0; i<animations.size(); ++i )
        {
            const std::string& name = animations[i]-> getName();
            if ( name==animationName_ )
            {
                animations[i]->setPlayMode(pm);                   
                manager_->playAnimation( animations[i].get()/*,2,2.0*/ );
            }

        }
    }
}  
