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

#include "av/avCore/InstancedData.h"

namespace {

    struct MyRigTransformHardware : public osgAnimation::RigTransformHardware
    {

        avAnimation::AnimationChannelMatricesType       anim_mat_data_;
        image_data                                      image_data_;

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
        const size_t palSize = _bonePalette.size();
        for (size_t i = 0; i < palSize; i++)
        {
            osg::ref_ptr<osgAnimation::Bone> bone = _bonePalette[i].get();
            const osg::Matrix& invBindMatrix = bone->getInvBindMatrixInSkeletonSpace();
            const osg::Matrix& boneMatrix = bone->getMatrixInSkeletonSpace();
            osg::Matrix resultBoneMatrix = invBindMatrix * boneMatrix;
            osg::Matrix result =  transformFromSkeletonToGeometry * resultBoneMatrix * invTransformFromSkeletonToGeometry;
            anim_mat_data_[i].first = bone->getName();
            anim_mat_data_[i].second.push_back(result);
            if (!_uniformMatrixPalette->setElement(i, result))
                OSG_WARN << "RigTransformHardware::computeUniformMatrixPalette can't set uniform at " << i << " elements" << std::endl;
        }
    }

    bool MyRigTransformHardware::init(osgAnimation::RigGeometry& geom)
    {
        bool binit = osgAnimation::RigTransformHardware::init(geom);

        if(binit)
            anim_mat_data_.resize(_bonePalette.size());

        int nbAttribs = getNumVertexAttrib();
        image_data_.divisor = 4;
        for (int i = 0; i < nbAttribs; i++)
        {
            osg::Vec4Array* arr = getVertexAttrib(i);
            image_data::weights_t  w;
            w.resize( arr->size() * image_data_.divisor);
            for (int j = 0; j < arr->size(); j++)
            {
                for (int z = 0; z<image_data_.divisor; ++z ) 
                    w[j*image_data_.divisor + z]=(*arr)[j]._v[z];
            } 

            image_data_.bonesWeights.emplace_back(std::move(w));
        }

        return binit;
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
        }
    };


    inline osg::Node* loadAnimation(std::string aname)
    {
        auto anim = osgDB::readNodeFile("crow/" + aname + ".fbx");
        anim->setName(aname);
        return  anim;
    }

}


namespace avAnimation {


class  InstancedAnimationManager
{

#if 0
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
#endif

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


	osg::TextureRectangle* createAnimationData( const MyRigTransformHardware& rth)
	{
        const AnimationChannelMatricesType& acmt = rth.anim_mat_data_;
        int bonesPerVertex = rth.getNumBonesPerVertex();

        size_t total_anim_len = acmt.size() * acmt.begin()->second.size();

		// create texture to encode all matrices
		unsigned int height = ((total_anim_len) / 4096u) + 1u;
		osg::ref_ptr<osg::Image> image = new osg::Image;
		image->allocateImage(16384, height, 1, GL_RGBA, GL_FLOAT);
		image->setInternalTextureFormat(GL_RGBA32F_ARB);
		
        idata_ = image_data(image.get());

        const AnimationChannelMatricesType&   mats = acmt;

#if 0
		unsigned int j = 0;
		for (auto it_a = mats.begin();it_a != mats.end(); ++it_a)
		{
			auto & vm = it_a->second;
			for (auto it_vm = vm.begin();it_vm != vm.end(); ++it_vm,  ++j)
			{
				osg::Matrixf matrix = *it_vm;
				float * data = (float*)image->data((j % 4096u) *4u, j / 4096u);
				memcpy(data, matrix.ptr(), 16 * sizeof(float));
			}
            idata_.bones.push_back(it_a->first);
		}
#endif
		size_t anim_len= mats.begin()->second.size();

		size_t j = 0;

		for (size_t i = 0; i < anim_len; ++i )
		for (auto it_a = mats.begin();it_a != mats.end(); ++it_a, ++j)
		{
			auto & vm = it_a->second;
			osg::Matrixf matrix = vm[i];
			float * data = (float*)image->data((j % 4096u) *4u, j / 4096u);
			memcpy(data, matrix.ptr(), 16 * sizeof(float));
		}


		for (auto it_a = mats.begin();it_a != mats.end(); ++it_a)
		{
			idata_.bones.push_back(it_a->first);
		}


		osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle(image);
		texture->setInternalFormat(GL_RGBA32F_ARB);
		texture->setSourceFormat(GL_RGBA);
		texture->setSourceType(GL_FLOAT);
		texture->setTextureSize(4, total_anim_len);
		texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
		texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
        
		idata_.data_len = total_anim_len;
        idata_.data.resize(texture->getImage()->getTotalSizeInBytes());
        unsigned char* dest_ptr = &idata_.data[0];
        for(osg::Image::DataIterator itr(texture->getImage()); itr.valid(); ++itr)
        {
            memcpy(dest_ptr, itr.data(), itr.size());
            dest_ptr += itr.size();
        }

        idata_.bonesWeights = rth.image_data_.bonesWeights;
        idata_.divisor = rth.image_data_.divisor;
        idata_.bonesPerVertex = bonesPerVertex;

        return texture.release();
	}

#if 0
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
#endif

    const image_data& getImageData() const                
    {
        return idata_;
    }
};

}



int main_anim_test3( int argc, char** argv )
{  
   osg::ArgumentParser arguments(&argc,argv);

   osg::DisplaySettings::instance()->setNumMultiSamples( 8 );
   
   osg::setNotifyLevel( osg::INFO );   
   
   osgViewer::Viewer viewer(arguments);
   //arguments.reportRemainingOptionsAsUnrecognized();
   viewer.apply(new osgViewer::SingleScreen(1));
   viewer.setUpViewInWindow(0, 0, 5, 5);


   osg::ref_ptr<osg::Group> root = new osg::Group;

   auto anim_file = osgDB::readNodeFile("crow/idle.fbx")  ;
   auto anim_idle    = loadAnimation("flap");
   auto anim_running = loadAnimation("soar");


   auto object_file = osgDB::readNodeFile("crow/flap.fbx");
      
   avAnimation::InstancedAnimationManager im(anim_file);   

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

	osg::ref_ptr<osg::MatrixTransform> ph_ctrl = new osg::MatrixTransform;
	ph_ctrl->setName("phys_ctrl");
	ph_ctrl->setUserValue("id",0);
	ph_ctrl->addChild( pat );
	root->addChild(ph_ctrl);

   using namespace avAnimation;

   AnimationManagerFinder finder;
   AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();

   anim_file->accept(finder);
   if (finder._am.valid()) {
       pat->addUpdateCallback(finder._am.get());
       AnimtkViewerModelController::setModel(finder._am.get());
       AnimtkViewerModelController::addAnimation(anim_idle); 
       AnimtkViewerModelController::addAnimation(anim_running); 

       // We're safe at this point, so begin processing.
       mc.setPlayMode(osgAnimation::Animation::ONCE);
       mc.next();
       mc.play();

	   viewer.setSceneData(root);

	   const AnimtkViewerModelController::AnimationDurationMap& ad_map = mc.getDurations();

	   for (auto it = ad_map.begin(); it!=ad_map.end(); ++it)
	   {
		   const std::string& cn       = mc.getCurrentAnimationName();

		   for (double t=0.0;t<ad_map.at(cn);t+= ad_map.at(cn)/150.0 )
		   {
			   viewer.frame(t);
		   }

		   mc.stop();
		   mc.next();
		   mc.play();
	   }


	   osg::ref_ptr<osg::TextureRectangle> tex = im.createAnimationData(*switcher.my_ptr);

	   std::string filename = "crow/data.row";
	   {
		   std::ofstream image_data_file(filename, std::ios_base::binary);
		   auto bts = binary::wrap(im.getImageData());
		   image_data_file.write(binary::raw_ptr(bts),binary::size(bts));
	   }


   } else {
	   osg::notify(osg::WARN) << "no osgAnimation::AnimationManagerBase found in the subgraph, no animations available" << std::endl;
   }

   return 0;
}

AUTO_REG(main_anim_test3)