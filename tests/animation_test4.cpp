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

#include "InstancedData.h"

namespace
{
    class LogFileHandler : public osg::NotifyHandler
    {
    public:
        LogFileHandler( const std::string& file )
        { _log.open( file.c_str() ); }
        virtual ~LogFileHandler() { _log.close(); }
        virtual void notify(osg::NotifySeverity severity,
            const char* msg)
        { 
            static std::string str_severity[] =                           
            {
                "ALWAYS",
                "FATAL",
                "WARN",
                "NOTICE",
                "INFO",
                "DEBUG_INFO",
                "DEBUG_FP"
            };

            _log << str_severity[severity] << ": " << msg;

        }
    protected:
        std::ofstream _log;
    };
 

class ComputeTextureBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
public:
    ComputeTextureBoundingBoxCallback(std::vector<osg::Matrixd> instanceMatrices)
        : m_instanceMatrices(instanceMatrices)
    {
    }

    virtual osg::BoundingBox computeBound(const osg::Drawable& drawable) const;
private:
    std::vector<osg::Matrixd> m_instanceMatrices;
};


osg::BoundingBox ComputeTextureBoundingBoxCallback::computeBound(const osg::Drawable& drawable) const
{
    osg::BoundingBox bounds;
    const osg::Geometry* geometry = dynamic_cast<const osg::Geometry*>(&drawable);

    if (!geometry)
        return bounds;

    const osg::Vec3Array* vertices = dynamic_cast<const osg::Vec3Array*>(geometry->getVertexArray());

    for (unsigned int i = 0; i < m_instanceMatrices.size(); ++i)
    {
        for (auto it = vertices->begin(); it != vertices->end(); ++it)
        {
            bounds.expandBy(*it * m_instanceMatrices[i]);
        }
    }

    return bounds;
}

}

namespace  {

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

	avAnimation::AnimationDataType anim_data_;
	osgAnimation::BoneMap          bm_;
    image_data                     idata_;
    std::vector<osg::Matrixd>      matrices_;
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

    inline void addMatrix(const osg::Matrixd& matrix) { matrices_.push_back(matrix); }
    inline osg::Matrixd getMatrix(size_t index) const { return matrices_[index]; }
    inline void clearMatrices() { matrices_.clear(); }

    osg::TextureRectangle* createAnimationTexture( image_data& idata)
	{
		size_t something_num = idata.data_num ;
		
		osg::ref_ptr<osg::Image> image = new osg::Image;
		image->setImage(idata.s, idata.t, idata.r, idata.internalFormat, idata.pixelFormat, idata.type, &idata.data[0], osg::Image::NO_DELETE);

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

    osg::TextureRectangle* createTextureHardwareInstancedGeode(osg::Geometry* geometry) const
    {
        const unsigned int start = 0;
        const unsigned int end = matrices_.size();

        // first turn on hardware instancing for every primitive set
        for (unsigned int i = 0; i < geometry->getNumPrimitiveSets(); ++i)
        {
            geometry->getPrimitiveSet(i)->setNumInstances(matrices_.size());
        }

        // we need to turn off display lists for instancing to work
        geometry->setUseDisplayList(false);
        geometry->setUseVertexBufferObjects(true);

        // create texture to encode all matrices
        unsigned int height = ((/*end-start*/matrices_.size()) / 4096u) + 1u;
        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->allocateImage(16384, height, 1, GL_RGBA, GL_FLOAT);
        image->setInternalTextureFormat(GL_RGBA32F_ARB);

        for (unsigned int i = /*start*/0, j = 0; i < /*end*/matrices_.size(); ++i, ++j)
        {
            osg::Matrixf matrix = matrices_[i];
            float * data = (float*)image->data((j % 4096u) *4u, j / 4096u);
            memcpy(data, matrix.ptr(), 16 * sizeof(float));
        }

        osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle(image);
        texture->setInternalFormat(GL_RGBA32F_ARB);
        texture->setSourceFormat(GL_RGBA);
        texture->setSourceType(GL_FLOAT);
        texture->setTextureSize(4, end-start);
        texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
        texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);

        // copy part of matrix list and create bounding box callback
        std::vector<osg::Matrixd> matrices;
        matrices.insert(matrices.begin(), matrices_.begin(), matrices_.end());
        geometry->setComputeBoundingBoxCallback(new ComputeTextureBoundingBoxCallback(matrices));
        
        return texture.release();
    }

    const image_data& getImageData() const                
    {
        return idata_;
    }
};

}

namespace {

    struct MyRigTransformHardware : public osgAnimation::RigTransformHardware
    {
        avAnimation::AnimationChannelMatricesType       anim_mat_data_;
        osg::ref_ptr<osg::Geometry>        inst_geom_;

        void operator()(osgAnimation::RigGeometry& geom);
        bool init(osgAnimation::RigGeometry& geom);
        bool init(osg::Geometry& geom);
        void computeMatrixPaletteUniform(const osg::Matrix& transformFromSkeletonToGeometry, const osg::Matrix& invTransformFromSkeletonToGeometry);
        void setInstancedGeometry(osg::Geometry* geom) {inst_geom_ = geom;}
    };

    void MyRigTransformHardware::operator()(osgAnimation::RigGeometry& geom)
    {
        if (_needInit)
            if (!init(geom))
                return;
            else if (!init(*inst_geom_.get()))
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
            if (!_uniformMatrixPalette->setElement(i, result))
                OSG_WARN << "RigTransformHardware::computeUniformMatrixPalette can't set uniform at " << i << " elements" << std::endl;
        }
    }

    bool MyRigTransformHardware::init(osgAnimation::RigGeometry& geom)
    {
        return osgAnimation::RigTransformHardware::init(geom);
    }

    bool MyRigTransformHardware::init(osg::Geometry& geom)
    {
        osg::Geometry& source = geom;
        osg::Vec3Array* positionSrc = dynamic_cast<osg::Vec3Array*>(source.getVertexArray());
        if (!positionSrc)
        {
            OSG_WARN << "RigTransformHardware no vertex array in the geometry " << geom.getName() << std::endl;
            return false;
        }

        osg::ref_ptr<osg::Program> cSkinningProg = creators::createProgram("skininst").program; 
        cSkinningProg->setName("SkinningShader");

        int attribIndex = 11;
        int nbAttribs = getNumVertexAttrib();
        for (int i = 0; i < nbAttribs; i++)
        {
            std::stringstream ss;
            ss << "boneWeight" << i;
            cSkinningProg->addBindAttribLocation(ss.str(), attribIndex + i);
            geom.setVertexAttribArray(attribIndex + i, getVertexAttrib(i));
            OSG_INFO << "set vertex attrib " << ss.str() << std::endl;
        }

        osg::ref_ptr<osg::StateSet> ss = geom.getOrCreateStateSet();
        ss->addUniform(getMatrixPaletteUniform());
        ss->addUniform(new osg::Uniform("nbBonesPerVertex", getNumBonesPerVertex()));
        ss->setAttributeAndModes(cSkinningProg.get());

        return true;
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



int main_anim_test4( int argc, char** argv )
{  
   osg::ArgumentParser arguments(&argc,argv);

   osg::DisplaySettings::instance()->setNumMultiSamples( 8 );
   
   osg::setNotifyLevel( osg::INFO );   
   osg::setNotifyHandler( new LogFileHandler("animtestlog.txt") );

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
   
   osg::Vec3 scale(2.0f, 2.0f, 2.0f);

   // create some matrices
   srand(time(NULL));
   for (unsigned int i = 0; i < 64; ++i)
   {
       for (unsigned int j = 0; j < 64; ++j)
       {
           // get random angle and random scale
           double angle = (rand() % 360) / 180.0 * cg::pi;
           double scale = (rand() % 2)  + 1.0;

           // calculate position
           osg::Vec3 position(i * 70, j * 70, 0.0f);
           osg::Vec3 jittering((rand() % 100) * 2.f, (rand() % 100) * 2.f, (rand() % 100) * 2.f);
           position += jittering;
           position.z() *= 2.0f;
           position.x() *= 2.0f;
           position.y() *= 2.0f;

           osg::Matrixd modelMatrix = osg::Matrixd::scale(scale, scale, scale) 
                                    * osg::Matrixd::rotate(angle, osg::Vec3d(0.0, 0.0, 1.0), angle, osg::Vec3d(0.0, 1.0, 0.0), angle, osg::Vec3d(1.0, 0.0, 0.0)) 
                                    * osg::Matrixd::translate(position);
           im.addMatrix(modelMatrix);
       }
   }

   std::string filename = "data.row";

   image_data rd;

   std::ifstream image_data_file(filename, std::ios_base::binary);
   
   osg::ref_ptr<osg::TextureRectangle> animTex;
   osg::ref_ptr<osg::TextureRectangle> instTex;
   
   if (image_data_file.good())
   {
	   binary::bytes_t data;
	   data.resize(boost::filesystem::file_size(filename));
	   image_data_file.read(data.data(), data.size());
	   binary::unwrap(data, rd);
	   animTex = im.createAnimationTexture(rd);
   }
   
   osg::ref_ptr<osg::Image> image = osgDB::readImageFile("crow/crow_tex.dds");
   osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image);
   texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
   texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
   texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
   texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
   texture->setUseHardwareMipMapGeneration(true);


#if 1
   ////////////////////////////////////////////////////////////////////////////

   findNodeByType< osg::Geode> geode_finder;  
   geode_finder.apply(*object_file);

   osg::Geode*    gnode = dynamic_cast<osg::Geode*>(geode_finder.getLast()); // cNodeFinder.FindChildByName_nocase( "CrowMesh" ));
   osg::Geometry* mesh = gnode->getDrawable(0)->asGeometry();

   osg::ref_ptr<osg::MatrixTransform> ph_ctrl = new osg::MatrixTransform;

   osg::ref_ptr<osg::Geode>	   geode = new osg::Geode;
   osg::StateSet* pSS = geode->getOrCreateStateSet();
   osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry(*mesh, osg::CopyOp::DEEP_COPY_ALL
       & ~osg::CopyOp::DEEP_COPY_CALLBACKS
       );
   
   osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();
   geometry->setComputeBoundingBoxCallback(pDummyBBCompute);
   
   instTex = im.createTextureHardwareInstancedGeode(geometry);

   geode->addDrawable(geometry);
   osg::Matrix trMatrix;
   trMatrix.setTrans(osg::Vec3f( -20, -20, 20));
   trMatrix.setRotate(osg::Quat(osg::inDegrees(0.0)  ,osg::Z_AXIS));
   ph_ctrl->setMatrix(trMatrix);


   pSS->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
   pSS->addUniform(new osg::Uniform("colorTex", 0));
   //pSS->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.8f), osg::StateAttribute::ON);

   pSS->setTextureAttributeAndModes(6, animTex.get(), osg::StateAttribute::ON);
   pSS->addUniform(new osg::Uniform("animationTex", 6));

   pSS->setTextureAttributeAndModes(1, instTex.get(), osg::StateAttribute::ON);
   pSS->addUniform(new osg::Uniform("instanceMatrixTexture", 1));

   ph_ctrl->addChild(geode);
   root->addChild(ph_ctrl);



   //////////////////////////////////////////////////////////////////////////////
#endif   

   
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

   switcher.my_ptr->setInstancedGeometry(geometry.get());

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
       // mc.setDurationRatio(10.);
       mc.next();
       mc.play();

   } else {
       osg::notify(osg::WARN) << "no osgAnimation::AnimationManagerBase found in the subgraph, no animations available" << std::endl;
   }


   viewer.addEventHandler( new osgViewer::StatsHandler );
   viewer.setSceneData(root);

   viewer.run();


   return 0;
}

AUTO_REG(main_anim_test4)