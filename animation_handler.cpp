#include "stdafx.h"
#include "animation_handler.h"



AnimationHandler::AnimationHandler(osg::Node* model,const std::string animationName,on_fire_f on_fire)
    : model_         (model)
    , animationName_ (animationName)
    , manager_       (nullptr)
    , on_fire_       (on_fire)
{

    //auto p = model_->getUpdateCallback();
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
                osg::notify(osg::NOTICE)<<"Fire !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
                if(on_fire_) on_fire_();
            }

            return false;
        }
        //case(osgGA::GUIEventAdapter::PUSH):
        //case(osgGA::GUIEventAdapter::MOVE):
        //    {
        //        _mx = ea.getX();
        //        _my = ea.getY();
        //        return false;
        //    }
        //case(osgGA::GUIEventAdapter::RELEASE):
        //    {
        //        if (_mx == ea.getX() && _my == ea.getY())
        //        {
        //            // only do a pick if the mouse hasn't moved
        //            pick(ea,viewer);
        //        }
        //        return true;
        //    }    

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
                auto anim = (osg::clone(animations[i].get(), "Animation_clone", osg::CopyOp::DEEP_COPY_ALL)); 
                // manager->unregisterAnimation(animations[i].get());
                // manager->registerAnimation  (anim/*.get()*/);

                animations[i]->setPlayMode(pm);                   
                manager_->playAnimation( /*anim*/ animations[i].get(),2,2.0 );

            }

        }
    }
}  
