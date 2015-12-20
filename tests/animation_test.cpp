#include "stdafx.h"

#include "empty_scene.h"
#include "animutils.h"
#include "av/precompiled.h"
#include "PickHandler.h"

#include "osg_helpers.h"

const bool walking = true;

namespace {

class KeyHandler : public osgGA::GUIEventHandler
{

public:
    typedef std::function<void()> do_smthng_f;
public:
    KeyHandler(osg::Node * root) 
        : _root (dynamic_cast<osg::MatrixTransform*>(root))
        , _grad (0.0)
    {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {            
            if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_Right )
            { 
                if ( _root )
                { 
                    _grad += 1;
                    _grad = cg::norm360(_grad);
                    const osg::Vec3d& pos = _root->getMatrix().getTrans();
                    osg::Matrix trMatrix;            
                    trMatrix.setTrans(pos);
                    trMatrix.setRotate(osg::Quat(osg::inDegrees(_grad)  ,osg::Z_AXIS));
                    _root->setMatrix(trMatrix);
                }
                return true;
            }
            else if ( ea.getKey()== osgGA::GUIEventAdapter::KEY_Left )
            {
                if ( _root )
                { 
                    _grad -= 1;
                    _grad = cg::norm360(_grad);
                    const osg::Vec3d& pos = _root->getMatrix().getTrans();
                    osg::Matrix trMatrix;            
                    trMatrix.setTrans(pos);
                    trMatrix.setRotate(osg::Quat(osg::inDegrees(_grad)  ,osg::Z_AXIS));
                    _root->setMatrix(trMatrix);
                }
                return true;
            }
            else if ( ea.getKey()== osgGA::GUIEventAdapter::KEY_N )
            {
                using namespace avAnimation;
                AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
                //mc.stop();
                mc.next();
				_timer = ea.getTime();
                return true;
            }
			else if ( ea.getKey()== osgGA::GUIEventAdapter::KEY_P )
			{
				using namespace avAnimation;
				AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
				//mc.stop();
				mc.previous();
				_timer = ea.getTime();
				return true;
			}			

        }

		if ( ea.getEventType()== osgGA::GUIEventAdapter::FRAME)
		{
			using namespace avAnimation;
			AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
			if(_timer && *_timer + 1.0 <= ea.getTime() )
			{
				mc.stopPrev();
				_timer.reset();
			}
		}

        return false;
    }

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("Keypad +",       "Change texture");
        usage.addKeyboardMouseBinding("Keypad -",       "Change texture");

    }

private:
    osg::ref_ptr<osg::MatrixTransform>    _root;
    double                                _grad;
	boost::optional<double>	     		 _timer;
};

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
                mat->setPosition(pos + osg::Vec3d(0.0,walking?-170.0:-300.0,0));
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
            auto pat = node->asTransform()->asPositionAttitudeTransform();
            auto mat = pat->getParent(0)->asTransform()->asMatrixTransform();

            const osg::Vec3d& pos = mat->getMatrix().getTrans();
            // const osg::Vec3d& pos = mat->getPosition();
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

                //const cg::quaternion orien = from_osg_quat(/*mat->getAttitude()*/mat->getMatrix().getRotate());
                //const cg::cpr&       cpr   = orien.cpr();
                //auto                 posv  = std::polar(-(walking?170.0:300.0),cg::grad2rad() * orien.cpr().course);
                //
               
                //osg::Matrix trMatrix;            
                //trMatrix.setTrans(pos + osg::Vec3d(posv.imag(),posv.real(),0)/*+ osg::Vec3d(0.0,walking?-170.0:-300.0,0)*/);
                //trMatrix.setRotate(mat->getMatrix().getRotate());
                //mat->setMatrix(trMatrix);

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

inline osg::Node* loadAnimation(std::string aname)
{
    auto anim = osgDB::readNodeFile("crow/" + aname + ".fbx");
    anim->setName(aname);
    return  anim;
}

}

int main_anim_test2( int argc, char** argv )
{  
   osg::ArgumentParser arguments(&argc,argv);

   osg::DisplaySettings::instance()->setNumMultiSamples( 8 );

   osgViewer::Viewer viewer(arguments);
   //arguments.reportRemainingOptionsAsUnrecognized();
   viewer.apply(new osgViewer::SingleScreen(1));

   osg::ref_ptr<osg::Group> root = new osg::Group;
   osg::ref_ptr<osg::Group> mt = new osg::Group;
   
   auto anim_file = osgDB::readNodeFile("crow/idle.fbx")  ;
   
   auto anim_idle    = loadAnimation("flap");
   auto anim_running = loadAnimation("soar");

   auto object_file = osgDB::readNodeFile("crow/soar.fbx");

   auto pat = new osg::PositionAttitudeTransform; 
   pat->addChild(object_file);
   pat->setAttitude(
       osg::Quat(osg::inDegrees(90.0)  ,osg::X_AXIS,
                 osg::inDegrees(0.0)   ,osg::Y_AXIS,
                 osg::inDegrees(0.0)   ,osg::Z_AXIS)
       );
   pat->asTransform()->asPositionAttitudeTransform()->setScale(osg::Vec3(0.5,0.5,0.5));

   //root->setUpdateCallback(new UpdateNode(pat));
   mt->addChild(creators::createBase(osg::Vec3(0,0,0),1000));        
   root->addChild(mt);
  

   osg::ref_ptr<osg::MatrixTransform> ph_ctrl = new osg::MatrixTransform;
   ph_ctrl->setName("phys_ctrl");
   ph_ctrl->setUserValue("id",6666);
   ph_ctrl->addChild( pat );

   osg::Matrix trMatrix;
   trMatrix.setTrans(osg::Vec3f(0.0,0.0,20.0));
   trMatrix.setRotate(osg::Quat(osg::inDegrees(0.0)  ,osg::Z_AXIS));
   ph_ctrl->setMatrix(trMatrix);
   
   root->addChild(ph_ctrl);
   
   pat->addUpdateCallback(new UpdateNode2(pat,"CrowMesh"));

   using namespace avAnimation;
   AnimationManagerFinder finder;
   anim_file->accept(finder);
   if (finder._am.valid()) {
       pat->addUpdateCallback(finder._am.get());
       AnimtkViewerModelController::setModel(finder._am.get());
       AnimtkViewerModelController::addAnimation(anim_idle); 
       AnimtkViewerModelController::addAnimation(anim_running); 
       

       // We're safe at this point, so begin processing.
       AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
       mc.setPlayMode(osgAnimation::Animation::ONCE);
       // mc.setDurationRatio(10.);
       mc.play();

   } else {
       osg::notify(osg::WARN) << "no osgAnimation::AnimationManagerBase found in the subgraph, no animations available" << std::endl;
   }

   osg::ref_ptr<PickHandler> picker = new PickHandler;
   root->addChild( picker->getOrCreateSelectionBox() );

   viewer.addEventHandler( new KeyHandler( ph_ctrl.get() ) );
   viewer.addEventHandler( picker.get() );
   viewer.setSceneData(root);

   viewer.run();
   
   return 0;
}

AUTO_REG(main_anim_test2)