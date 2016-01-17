#include "stdafx.h"

#include "empty_scene.h"
#include "animutils.h"
#include "av/precompiled.h"
#include "PickHandler.h"

#include "osg_helpers.h"

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>

#include "utils\visitors\find_node_visitor.h"

#if 0
mat4 decodeMatrix(vec4 m1, vec4 m2, vec4 m3)
{
	return mat4( vec4(m1.xyz,m1.w),
				 vec4(m2.xyz,m2.w),
				 vec4(m3.xyz,m3.w),
				 vec4(0,0,0,1));
}
#endif

class  InstancedAnimationManager
{
	struct InstanceDataElement
	{
		osg::Vec4 world1;            // the world transform for this matrix row 1
		osg::Vec4 world2;            // the world transform for this matrix row 2
		osg::Vec4 world3;            // the world transform for this matrix row 3 (row 4 is implicit)
		osg::Vec4 color;

		// $ Technically this is bundled, but there is not class that makes a uint vector, so just keep flat
		uint32_t animationIndex;     // offset in vectors into the whole data stream for the start of the animation playing
		uint32_t frameOffset;        // offset in vectors into the animation stream for the start of the frame playing
		uint32_t attachmentSet;      // the id to determine which geo attachments get set
		uint32_t lerpValue;          // lerp between frames
	};

	struct AnimChannels
	{
		osg::ref_ptr<osgAnimation::Channel> position;
		osg::ref_ptr<osgAnimation::Channel> rotateX;
		osg::ref_ptr<osgAnimation::Channel> rotateY;
		osg::ref_ptr<osgAnimation::Channel> rotateZ;
		osg::ref_ptr<osgAnimation::Channel> scale;
	};

	typedef std::map<std::string, AnimChannels >                 ChannelMapType;
	typedef std::map<std::string, std::vector<osg::Matrix>>      AnimationChannelMatricesType;
	typedef std::map<std::string, AnimationChannelMatricesType>  AnimationDataType;

	AnimationDataType       anim_data_;
	osgAnimation::BoneMap   bm_;

private:
	
	inline osgAnimation::BoneMap getBoneMap(osg::Node* base_model)
	{
		findNodeByType< osgAnimation::Skeleton> s_finder;  
		s_finder.apply(*base_model);

		auto* skel  = dynamic_cast<osgAnimation::Skeleton*>(s_finder.getLast());
		osgAnimation::BoneMapVisitor mapVisitor;
		skel->accept(mapVisitor);
		return mapVisitor.getBoneMap();
	}

public:

	InstancedAnimationManager(osg::Node* base_model)
		:bm_(getBoneMap(base_model))
	{}


	osg::TextureRectangle* stripAnimation(osgAnimation::BasicAnimationManager* model) 
	{
		size_t mat_num = 0;
		for (osgAnimation::AnimationList::const_iterator it = model->getAnimationList().begin(); it != model->getAnimationList().end(); it++)
		{
			osgAnimation::ChannelList& channels_list = (*it)->getChannels();
			AnimationChannelMatricesType   out_data;
			ChannelMapType  cm; 
		
			for(auto it_a = channels_list.begin();it_a != channels_list.end(); ++it_a)
			{
				osg::ref_ptr<osgAnimation::Channel>& chan = *it_a;
			
	#if 0
				if(!targetName.empty() && targetName != chan->getTargetName())
				{
					anim_channels
					cm.insert(chan->getTargetName(),anim_channels)
				}
	#endif

				if(chan->getName() == "rotateX")
					cm[chan->getTargetName()].rotateX  = chan;
				if(chan->getName() == "rotateY")
					cm[chan->getTargetName()].rotateY  = chan;
				if(chan->getName() == "rotateZ")
					cm[chan->getTargetName()].rotateZ  = chan;
				if(chan->getName() == "position")
					cm[chan->getTargetName()].position = chan;
				if(chan->getName() == "scale")
					cm[chan->getTargetName()].scale    = chan;
			}

			for(auto it_a = cm.begin();it_a != cm.end(); ++it_a)
			{
				osg::Matrix matrix;

				auto* fkc_rX = it_a->second.rotateX?dynamic_cast<osgAnimation::FloatCubicBezierKeyframeContainer*>(it_a->second.rotateX->getSampler()->getKeyframeContainer()):nullptr;
				auto* fkc_rY = it_a->second.rotateY?dynamic_cast<osgAnimation::FloatCubicBezierKeyframeContainer*>(it_a->second.rotateY->getSampler()->getKeyframeContainer()):nullptr;
				auto* fkc_rZ = it_a->second.rotateZ?dynamic_cast<osgAnimation::FloatCubicBezierKeyframeContainer*>(it_a->second.rotateZ->getSampler()->getKeyframeContainer()):nullptr;
			
				size_t fkc_size = fkc_rX?fkc_rX->size():(fkc_rY?fkc_rY->size():(fkc_rZ?fkc_rZ->size():(0)));

				for (size_t i=0; i < fkc_size; i++)
				{
					// (*fkc)[i].getTime();
					matrix.setRotate(osg::Quat(fkc_rX?(*fkc_rX)[i].getValue().getPosition():0,osg::X_AXIS,
											   fkc_rY?(*fkc_rY)[i].getValue().getPosition():0,osg::Y_AXIS,
											   fkc_rZ?(*fkc_rZ)[i].getValue().getPosition():0,osg::Z_AXIS
											   )
					);

					out_data[it_a->first].push_back(matrix);
					mat_num++;
				}			
			}

			anim_data_[(*it)->getName() + "_" + boost::lexical_cast<std::string>(std::distance(model->getAnimationList().begin(),it))] = out_data;
		}
	    
		return createAnimationTexture(mat_num);
	}

	osg::TextureRectangle* createAnimationTexture(size_t something_num)
	{
		// create texture to encode all matrices
		unsigned int height = ((something_num) / 4096u) + 1u;
		osg::ref_ptr<osg::Image> image = new osg::Image;
		image->allocateImage(16384, height, 1, GL_RGBA, GL_FLOAT);
		image->setInternalTextureFormat(GL_RGBA32F_ARB);
		
		const AnimationChannelMatricesType&   mats = anim_data_.begin()->second;

		unsigned int j = 0;
		for (auto it_a = mats.begin();it_a != mats.end(); ++it_a)
		{
			auto & vm = it_a->second;
			for (auto it_vm = vm.begin();it_vm != vm.end(); ++it_vm,  ++j)
			{
				//osg::Matrixf matrix = m_matrices[i];
				osg::Matrixf matrix = *it_vm;
				float * data = (float*)image->data((j % 4096u) *4u, j / 4096u);
				memcpy(data, matrix.ptr(), 16 * sizeof(float));
			}
		}


		osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle(image);
		texture->setInternalFormat(GL_RGBA32F_ARB);
		texture->setSourceFormat(GL_RGBA);
		texture->setSourceType(GL_FLOAT);
		texture->setTextureSize(4, something_num);
		texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
		texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);

		return texture.release();
	}

}; 

namespace {

    struct SetupRigGeometry : public osg::NodeVisitor
    {
        bool _hardware;
        SetupRigGeometry( bool hardware = true) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _hardware(hardware) {}

        void apply(osg::Geode& geode)
        {
            for (unsigned int i = 0; i < geode.getNumDrawables(); i++)
                apply(*geode.getDrawable(i));
        }
        void apply(osg::Drawable& geom)
        {
            if (_hardware) {
                osgAnimation::RigGeometry* rig = dynamic_cast<osgAnimation::RigGeometry*>(&geom);
                if (rig)
                    rig->setRigTransformImplementation(new osgAnimation::RigTransformHardware);
            }

#if 0
            if (geom.getName() != std::string("BoundingBox")) // we disable compute of bounding box for all geometry except our bounding box
                geom.setComputeBoundingBoxCallback(new osg::Drawable::ComputeBoundingBoxCallback);
            //            geom.setInitialBound(new osg::Drawable::ComputeBoundingBoxCallback);
#endif
        }
    };

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
                mc.play();
                _timer = ea.getTime();
                return true;
            }
			else if ( ea.getKey()== osgGA::GUIEventAdapter::KEY_P )
			{
				using namespace avAnimation;
				AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
				//mc.stop();
				mc.previous();
                mc.play();
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

osg::ref_ptr<osg::TextureRectangle> at;

int main_anim_test3( int argc, char** argv )
{  
   osg::ArgumentParser arguments(&argc,argv);

   osg::DisplaySettings::instance()->setNumMultiSamples( 8 );
   
   osg::setNotifyLevel( osg::INFO );   
   
   osgViewer::Viewer viewer(arguments);
   //arguments.reportRemainingOptionsAsUnrecognized();
   viewer.apply(new osgViewer::SingleScreen(1));

   osg::ref_ptr<osg::Group> root = new osg::Group;
   osg::ref_ptr<osg::Group> mt = new osg::Group;

#if 1   
   auto anim_file = osgDB::readNodeFile("crow/idle.fbx")  ;

   auto anim_idle    = loadAnimation("flap");
   auto anim_running = loadAnimation("soar");
#endif

   auto object_file = osgDB::readNodeFile("crow/flap.fbx");
   //auto object_file = osgDB::readNodeFile("crow/crow_model.fbx");

   InstancedAnimationManager im(anim_file);   

   osg::ref_ptr<osg::Image> image = osgDB::readImageFile("crow/crow_tex.dds");
   osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image);
   texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
   texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
   texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
   texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
   texture->setUseHardwareMipMapGeneration(true);

   osg::ref_ptr<osg::StateSet> stateSet = object_file->getOrCreateStateSet();
   stateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
   stateSet->addUniform(new osg::Uniform("colorTexture", 0));
   stateSet->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.8f), osg::StateAttribute::ON);

   SetupRigGeometry switcher(true);
   object_file->accept(switcher);

   auto pat = new osg::PositionAttitudeTransform; 
   pat->addChild(object_file);
   pat->setAttitude(
       osg::Quat(osg::inDegrees(0.0)   ,osg::X_AXIS,
                 osg::inDegrees(0.0)   ,osg::Y_AXIS,
                 osg::inDegrees(0.0)   ,osg::Z_AXIS)
       );

   pat->asTransform()->asPositionAttitudeTransform()->setScale(osg::Vec3(0.5,0.5,0.5));

   //root->setUpdateCallback(new UpdateNode(pat));
   mt->addChild(creators::createBase(osg::Vec3(0,0,0),1000));        
   root->addChild(mt);
  
   for (int i =0;i<300;i++)
   {
       osg::ref_ptr<osg::MatrixTransform> ph_ctrl = new osg::MatrixTransform;
       ph_ctrl->setName("phys_ctrl");
       ph_ctrl->setUserValue("id",666 + i);
       ph_ctrl->addChild( pat );

       osg::Matrix trMatrix;
       trMatrix.setTrans(osg::Vec3f(0.0 + i * 20,0.0 + i * 20, 20.0 + i * 1));
       trMatrix.setRotate(osg::Quat(osg::inDegrees(0.0)  ,osg::Z_AXIS));
       ph_ctrl->setMatrix(trMatrix);

       root->addChild(ph_ctrl);
       if (i==0 )
           viewer.addEventHandler( new KeyHandler( ph_ctrl.get() ) );


   }

   using namespace avAnimation;
   AnimationManagerFinder finder;
   anim_file->accept(finder);
   if (finder._am.valid()) {
       pat->addUpdateCallback(finder._am.get());
       AnimtkViewerModelController::setModel(finder._am.get());
       AnimtkViewerModelController::addAnimation(anim_idle); 
       AnimtkViewerModelController::addAnimation(anim_running); 
       
       at = im.stripAnimation(finder._am.get());

	   pat->getOrCreateStateSet()->setTextureAttributeAndModes(6, at.get(), osg::StateAttribute::ON);
	   pat->getOrCreateStateSet()->addUniform(new osg::Uniform("animatiomTexture", 6));

       // We're safe at this point, so begin processing.
       AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
       mc.setPlayMode(osgAnimation::Animation::LOOP);
       // mc.setDurationRatio(10.);
       mc.next();
       mc.play();

   } else {
       osg::notify(osg::WARN) << "no osgAnimation::AnimationManagerBase found in the subgraph, no animations available" << std::endl;
   }


   osg::ref_ptr<PickHandler> picker = new PickHandler;
   root->addChild( picker->getOrCreateSelectionBox() );

   viewer.addEventHandler( new osgViewer::StatsHandler );
   viewer.addEventHandler( picker.get() );
   viewer.setSceneData(root);

   viewer.run();
   
   return 0;
}

AUTO_REG(main_anim_test3)