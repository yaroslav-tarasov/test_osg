#include "stdafx.h"

#include "empty_scene.h"
#include "animutils.h"
#include "av/precompiled.h"
#include "PickHandler.h"

struct UpdateNode: public osg::NodeCallback
{
    UpdateNode(osg::Transform * tr)
        : tr_ (tr)
    {

    }

    void operator()(osg::Node* node, osg::NodeVisitor* nv) {
        
        if(tr_->asTransform())
        {
            auto mat = tr_->asTransform()->asPositionAttitudeTransform();
            //const osg::Vec3d& pos = mat->getMatrix().getTrans();
            //osg::Matrix trMatrix;            
            //trMatrix.setTrans(pos + osg::Vec3d(0.001,0.001,0));
            // mat->setMatrix(trMatrix);
            const osg::Vec3d& pos = mat->getPosition();
            //mat->setPosition(pos + osg::Vec3d(0.1,0.1,0));

            auto& mc   = avAnimation::AnimtkViewerModelController::instance();
            if(!mc.playing())
            {
                mat->setPosition(pos + osg::Vec3d(0.0,-170.0,0));
                mc.play();
            }
        }
        traverse(node,nv);
    }

    osg::ref_ptr<osg::Transform> tr_;
};

struct UpdateNode2: public osg::NodeCallback
{
    UpdateNode2(osg::Node* node,  const std::string& name)
        : flag_delayed(false)
    {
          _body =  findFirstNode(node,name,findNodeVisitor::not_exact);
          _body =  _body.valid()?_body->getParent(0):nullptr;
          _pos  =  _body.valid()? _body->asTransform()->asMatrixTransform()->getMatrix().getTrans(): osg::Vec3d();

          osg::notify(osg::WARN) << "Initial values " << 
              _body->computeBound().center() <<  "    " <<
              _body->computeBound().radius()
              << std::endl;
    }

    UpdateNode2()
        : flag_delayed(false)
        , _body(nullptr)
    {
    }

    void operator()(osg::Node* node, osg::NodeVisitor* nv) {
        
        if(node->asTransform())
        {
            auto mat = node->asTransform()->asPositionAttitudeTransform();
            const osg::Vec3d& pos = mat->getPosition();
            auto& mc   = avAnimation::AnimtkViewerModelController::instance();
            
            if(flag_delayed)
            {
                if(_body.valid())
                {
                    osg::notify(osg::WARN) << "Diff position " << 
                        _pos  - _body->asTransform()->asMatrixTransform()->getMatrix().getTrans() <<  "    " <<
                        _body->computeBound().center() <<  "    " <<
                        _body->computeBound().radius()
                        << std::endl;
                }


                mat->setPosition(pos + osg::Vec3d(0.0,-300.0,0));     // -170
                flag_delayed = false;
            }

            if(!mc.playing())
            {
                mc.play();
                flag_delayed = true;
            }

        }

        traverse(node,nv);
    }

private:
    bool                     flag_delayed;
    osg::ref_ptr<osg::Node>         _body;
     osg::Vec3d                      _pos;
};

int main_anim_test( int argc, char** argv )
{  
   osg::ArgumentParser arguments(&argc,argv);

   osg::DisplaySettings::instance()->setNumMultiSamples( 8 );

   osgViewer::Viewer viewer(arguments);
   //arguments.reportRemainingOptionsAsUnrecognized();
   viewer.apply(new osgViewer::SingleScreen(1));

   osg::ref_ptr<osg::Group> root = new osg::Group;
   osg::ref_ptr<osg::Group> mt = new osg::Group;
   
   auto anim_file = osgDB::readNodeFile("running/running.fbx");
   
   osgAnimation::AnimationManagerBase* animationManager = dynamic_cast<osgAnimation::AnimationManagerBase*>(anim_file->getUpdateCallback());
   
   if(!animationManager) 
   {
       osg::notify(osg::FATAL) << "Did not find AnimationManagerBase updateCallback needed to animate elements" << std::endl;
       return 1;
   }

   auto object_file = osgDB::readNodeFile("running/Remy.fbx");

   auto pat = new osg::PositionAttitudeTransform; 
   pat->addChild(object_file);
   pat->setAttitude(osg::Quat(osg::inDegrees(90.0),osg::X_AXIS));
   pat->asTransform()->asPositionAttitudeTransform()->setScale(osg::Vec3(0.5,0.5,0.5));

   //root->setUpdateCallback(new UpdateNode(pat));
   mt->addChild(creators::createBase(osg::Vec3(0,0,0),1000));        
   root->addChild(mt);
  

   osg::ref_ptr<osg::MatrixTransform> ph_ctrl = new osg::MatrixTransform;
   ph_ctrl->setName("phys_ctrl");
   ph_ctrl->setUserValue("id",6666);
   ph_ctrl->addChild( pat );

   root->addChild(ph_ctrl);
   
   pat->addUpdateCallback(new UpdateNode2(pat,"Body"));

   using namespace avAnimation;
   AnimationManagerFinder finder;
   /*pat*/anim_file->accept(finder);
   if (finder._am.valid()) {
       pat->addUpdateCallback(finder._am.get());
       AnimtkViewerModelController::setModel(finder._am.get());

       // We're safe at this point, so begin processing.
       AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
       mc.setPlayMode(osgAnimation::Animation::ONCE);
       mc.setDurationRatio(10.);
       mc.play();

   } else {
       osg::notify(osg::WARN) << "no osgAnimation::AnimationManagerBase found in the subgraph, no animations available" << std::endl;
   }
   


   osg::ref_ptr<PickHandler> picker = new PickHandler;
   root->addChild( picker->getOrCreateSelectionBox() );

   viewer.addEventHandler( picker.get() );
   viewer.setSceneData(root);

   viewer.run();
   
   return 0;
}

AUTO_REG(main_anim_test)