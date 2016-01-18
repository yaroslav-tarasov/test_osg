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

struct image_data
{
    int s;
    int t;
    int r;
    GLenum pixelFormat;
    GLenum type;
    GLint internalFormat;
    vector<unsigned char> data;
    
    image_data ()
    {}
    
    image_data (const osg::Image* image)
    {
        s = image->s();
        t = image->t();
        r = image->r();
        pixelFormat = image->getPixelFormat(); 
        type  = image->getDataType();
        internalFormat = image->getInternalTextureFormat();
    }
};

REFL_STRUCT(image_data)
    REFL_ENTRY(s)
    REFL_ENTRY(t)
    REFL_ENTRY(r)
    REFL_ENTRY(pixelFormat) 
    REFL_ENTRY(internalFormat) 
    REFL_ENTRY(type)
    REFL_ENTRY(data)
REFL_END()

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
	osgAnimation::BoneMap          bm_;
    image_data                  idata_;
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

	osg::TextureRectangle* createAnimationTexture( const AnimationChannelMatricesType& acmt)
	{
        
        size_t something_num = acmt.size() * acmt.begin()->second.size();

		// create texture to encode all matrices
		unsigned int height = ((something_num) / 4096u) + 1u;
		osg::ref_ptr<osg::Image> image = new osg::Image;
		image->allocateImage(16384, height, 1, GL_RGBA, GL_FLOAT);
		image->setInternalTextureFormat(GL_RGBA32F_ARB);
		
        idata_ = image_data(image.get());

        const AnimationChannelMatricesType&   mats = acmt;

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
        
        idata_.data.resize(texture->getImage()->getTotalSizeInBytes());
        unsigned char* dest_ptr = &idata_.data[0];
        for(osg::Image::DataIterator itr(texture->getImage()); itr.valid(); ++itr)
        {
            memcpy(dest_ptr, itr.data(), itr.size());
            dest_ptr += itr.size();
        }

        return texture.release();
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

    const image_data& getImageData() const                
    {
        return idata_;
    }
}; 

namespace {

    struct MyRigTransformHardware : public osgAnimation::RigTransformHardware
    {
        typedef std::map<std::string, std::vector<osg::Matrix>>      AnimationChannelMatricesType;
        AnimationChannelMatricesType       anim_data_;

        void operator()(osgAnimation::RigGeometry& geom);
        bool init(osgAnimation::RigGeometry& geom);
        void computeMatrixPaletteUniform(const osg::Matrix& transformFromSkeletonToGeometry, const osg::Matrix& invTransformFromSkeletonToGeometry);
    };

    void MyRigTransformHardware::operator()(osgAnimation::RigGeometry& geom)
    {
        if (_needInit)
            if (!init(geom))
                return;
        MyRigTransformHardware::computeMatrixPaletteUniform(geom.getMatrixFromSkeletonToGeometry(), geom.getInvMatrixFromSkeletonToGeometry());
    }
    
    void MyRigTransformHardware::computeMatrixPaletteUniform(const osg::Matrix& transformFromSkeletonToGeometry, const osg::Matrix& invTransformFromSkeletonToGeometry)
    {
        for (int i = 0; i < (int)_bonePalette.size(); i++)
        {
            osg::ref_ptr<osgAnimation::Bone> bone = _bonePalette[i].get();
            const osg::Matrix& invBindMatrix = bone->getInvBindMatrixInSkeletonSpace();
            const osg::Matrix& boneMatrix = bone->getMatrixInSkeletonSpace();
            osg::Matrix resultBoneMatrix = invBindMatrix * boneMatrix;
            osg::Matrix result =  transformFromSkeletonToGeometry * resultBoneMatrix * invTransformFromSkeletonToGeometry;
            anim_data_[bone->getName()].push_back(result);
            if (!_uniformMatrixPalette->setElement(i, result))
                OSG_WARN << "RigTransformHardware::computeUniformMatrixPalette can't set uniform at " << i << " elements" << std::endl;
        }
    }

    bool MyRigTransformHardware::init(osgAnimation::RigGeometry& geom)
    {
        return osgAnimation::RigTransformHardware::init(geom);
    }


    struct SetupRigGeometry : public osg::NodeVisitor
    {
        bool _hardware;
        osg::ref_ptr<MyRigTransformHardware> my_ptr;
        
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
                {        
                    my_ptr = new MyRigTransformHardware;
                    rig->setRigTransformImplementation(my_ptr);
                }
            }

#if 0
            if (geom.getName() != std::string("BoundingBox")) // we disable compute of bounding box for all geometry except our bounding box
                geom.setComputeBoundingBoxCallback(new osg::Drawable::ComputeBoundingBoxCallback);
            //            geom.setInitialBound(new osg::Drawable::ComputeBoundingBoxCallback);
#endif
        }
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
  
#if 1
   for (int i =0;i<1/*300*/;i++)
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

   }
#endif

   using namespace avAnimation;

   AnimationManagerFinder finder;
   AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();

   anim_file->accept(finder);
   if (finder._am.valid()) {
       pat->addUpdateCallback(finder._am.get());
       AnimtkViewerModelController::setModel(finder._am.get());
       AnimtkViewerModelController::addAnimation(anim_idle); 
       AnimtkViewerModelController::addAnimation(anim_running); 

       //at = im.stripAnimation(finder._am.get());

	   pat->getOrCreateStateSet()->setTextureAttributeAndModes(6, at.get(), osg::StateAttribute::ON);
	   pat->getOrCreateStateSet()->addUniform(new osg::Uniform("animatiomTexture", 6));

       // We're safe at this point, so begin processing.
       mc.setPlayMode(osgAnimation::Animation::ONCE);
       // mc.setDurationRatio(10.);
       mc.next();
       mc.play();

   } else {
       osg::notify(osg::WARN) << "no osgAnimation::AnimationManagerBase found in the subgraph, no animations available" << std::endl;
   }

   viewer.setSceneData(root);

   // viewer.run();

   const std::string& cn             = mc.getCurrentAnimationName();
   const AnimtkViewerModelController::AnimationDurationMap& ad_map = mc.getDurations();

   for (double t=0.0;t<ad_map.at(cn);t+= ad_map.at(cn)/150.0 )
   {
       viewer.frame(t);
   }

   at = im.createAnimationTexture(switcher.my_ptr->anim_data_);

   std::string filename = "data.row";
   {
       std::ofstream image_data_file(filename, std::ios_base::binary);
       auto bts = binary::wrap(im.getImageData());
       image_data_file.write(binary::raw_ptr(bts),binary::size(bts));
   }

   image_data rd;
   
   {
       std:ifstream image_data_file(filename, std::ios_base::binary);
       
       if (image_data_file.good())
       {
           binary::bytes_t data;
           data.resize(boost::filesystem::file_size(filename));
           image_data_file.read(data.data(), data.size());
           binary::unwrap(data, rd);
       }



   }


   return 0;
}

AUTO_REG(main_anim_test3)