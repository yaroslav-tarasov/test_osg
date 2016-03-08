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

	class  InstancedAnimationManager
	{
	public:
		typedef std::vector<osg::Matrixd> InstancedDataType;

	public:

		InstancedAnimationManager   (osg::Node* base_model);
		
		osg::Geode*                 createGeode();
	    bool                        loadAnimationData(std::string const&  filename);

		inline void                 addMatrix(const osg::Matrixd& matrix) { instancesData_.push_back(matrix); }
		inline osg::Matrixd         getMatrix(size_t index) const         { return instancesData_[index]; }
		inline void                 clearMatrices()                       { instancesData_.clear(); }

		osg::TextureRectangle*      createAnimationTexture( image_data& idata);
		osg::TextureRectangle*      createTextureHardwareInstancedGeode(osg::Geometry* geometry);
		void                        setInstanceData(size_t idx, const osg::Matrixf& matrix);
		const InstancedDataType &   getInstancedData() const { return instancesData_; }
		const image_data&           getImageData()     const { return idata_; }   

	private:

		osgAnimation::BoneMap getBoneMap(osg::Node* base_model);

	private:
		avAnimation::AnimationDataType      anim_data_;
		osgAnimation::BoneMap               bm_;
		image_data                          idata_;
		InstancedDataType                   instancesData_;
		osg::ref_ptr<osg::TextureRectangle> texture_;
		osg::ref_ptr<osg::Node>             src_model_;
		
		osg::ref_ptr<osg::TextureRectangle> animTexture_;
		osg::ref_ptr<osg::TextureRectangle> instTexture_;

	};

}