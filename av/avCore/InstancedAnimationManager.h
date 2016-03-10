#pragma once

#include "utils/cpp_utils/id_generator.h"
#include "InstancedData.h"

namespace avCore
{
	class  InstancedAnimationManager  : public osg::Object       
	{
	public:
		typedef std::vector<osg::Matrixd>              InstancedDataType;
        typedef std::vector< std::pair<osg::ref_ptr<osg::Node>,osg::ref_ptr<osg::Node>> > InstancedNodesVectorType;
	public:
        
        META_Object(avCore,InstancedAnimationManager);
        
        InstancedAnimationManager();

        InstancedAnimationManager(const InstancedAnimationManager& im,const osg::CopyOp& =osg::CopyOp::SHALLOW_COPY);

		InstancedAnimationManager   (osg::Node* base_model, const std::string anim_file);

        bool                        commit();

		inline void                 addMatrix(const osg::Matrixd& matrix) { instancesData_.push_back(matrix); }
		inline osg::Matrixd         getMatrix(size_t index) const         { return instancesData_[index]; }
		inline void                 clearMatrices()                       { instancesData_.clear(); }
        inline osg::Geode*          getInstGeode()                        { return instGeode_.get(); }
        osg::Node *                 getInstancedNode();                    

        osg::TextureRectangle*      createAnimationTexture( image_data& idata);

		void                        setInstanceData(size_t idx, const osg::Matrixf& matrix);
		const InstancedDataType &   getInstancedData() const { return instancesData_; }
		const image_data&           getImageData()     const { return image_data_; }   

	private:
        osg::TextureRectangle*      _createTextureHardwareInstancedGeode(osg::Geometry* geometry);
        bool                        _loadAnimationData(std::string const&  filename);
        bool                        _initSkinning(osg::Geometry& geom, const image_data& id );
        osg::Geode*                 _createGeode();
		osgAnimation::BoneMap       _getBoneMap(osg::Node* base_model);
        void                        _initData();
	private:
		osgAnimation::BoneMap                    bm_;

		osg::observer_ptr<osg::Node>             src_model_;

     // Instanced staff   
        osg::ref_ptr<osg::Geode>                 instGeode_;
        osg::observer_ptr<osg::Geometry>         instGeometry_;

        image_data                               image_data_;
        InstancedDataType                        instancesData_;
		osg::ref_ptr<osg::TextureRectangle>      animTexture_;
		osg::ref_ptr<osg::TextureRectangle>      instTexture_;
    private:
        utils::fixed_id_generator<unsigned>      inst_id_gen_;
        InstancedNodesVectorType                 inst_nodes_; 
	};

}