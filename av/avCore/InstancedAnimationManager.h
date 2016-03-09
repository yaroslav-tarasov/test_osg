#pragma once

#include "InstancedData.h"

namespace avCore
{
	struct RigTransformHardware : public osgAnimation::RigTransformHardware
	{
		avAnimation::AnimationChannelMatricesType       anim_mat_data_;
		osg::ref_ptr<osg::Geometry>                         inst_geom_;

		void operator()(osgAnimation::RigGeometry& geom);
		bool init(osgAnimation::RigGeometry& geom);
		bool init(osg::Geometry& geom);
		void computeMatrixPaletteUniform(const osg::Matrix& transformFromSkeletonToGeometry, const osg::Matrix& invTransformFromSkeletonToGeometry);
		void setInstancedGeometry(osg::Geometry* geom) {inst_geom_ = geom;}
	};

	class  InstancedAnimationManager  : public osg::Object       
	{
	public:
		typedef std::vector<osg::Matrixd> InstancedDataType;

	public:
        
        META_Object(avCore,InstancedAnimationManager);
        
        InstancedAnimationManager();

        InstancedAnimationManager(const InstancedAnimationManager& im,const osg::CopyOp& =osg::CopyOp::SHALLOW_COPY);

		InstancedAnimationManager   (osg::Node* base_model, const std::string anim_file);

		inline void                 addMatrix(const osg::Matrixd& matrix) { instancesData_.push_back(matrix); }
		inline osg::Matrixd         getMatrix(size_t index) const         { return instancesData_[index]; }
		inline void                 clearMatrices()                       { instancesData_.clear(); }
        inline osg::Geode*          getInstGeode()                        { return instGeode_.get(); }
        inline osg::Geometry*       getInstGeometry()                     { return instGeometry_.get(); }

		osg::TextureRectangle*      createAnimationTexture( image_data& idata);

		void                        setInstanceData(size_t idx, const osg::Matrixf& matrix);
		const InstancedDataType &   getInstancedData() const { return instancesData_; }
		const image_data&           getImageData()     const { return idata_; }   

	private:
        osg::TextureRectangle*      _createTextureHardwareInstancedGeode(osg::Geometry* geometry);
        bool                        _loadAnimationData(std::string const&  filename);
        osg::Geode*                 _createGeode();
		osgAnimation::BoneMap       _getBoneMap(osg::Node* base_model);
        void                        _initData();
	private:
		avAnimation::AnimationDataType      anim_data_;
		osgAnimation::BoneMap               bm_;

		osg::observer_ptr<osg::Node>        src_model_;

     // Instanced staff   
        osg::ref_ptr<osg::Geode>            instGeode_;
        osg::observer_ptr<osg::Geometry>    instGeometry_;

        image_data                          idata_;
        InstancedDataType                   instancesData_;
		osg::ref_ptr<osg::TextureRectangle> animTexture_;
		osg::ref_ptr<osg::TextureRectangle> instTexture_;

	};

}