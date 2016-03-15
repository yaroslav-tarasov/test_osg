#pragma once

#include "utils/cpp_utils/id_generator.h"
#include "InstancedData.h"

namespace avCore
{
	class  InstancedAnimationManager  : public osg::Object       
	{
	public:
		typedef std::vector<osg::Matrixf>              InstancedDataType;
        typedef std::vector< std::pair<osg::ref_ptr<osg::Node>,osg::ref_ptr<osg::Node>> > InstancedNodesVectorType;
	public:
        
        META_Object(avCore,InstancedAnimationManager);
        
        InstancedAnimationManager();

        InstancedAnimationManager(const InstancedAnimationManager& im,const osg::CopyOp& =osg::CopyOp::SHALLOW_COPY);

		InstancedAnimationManager   (osg::Node* base_model, const std::string anim_file);

        void                        commitInstancePositions();

		inline void                 addMatrix(const osg::Matrixf& matrix) { instancesData_.push_back(matrix); }
		inline osg::Matrixf         getMatrix(size_t index) const         { return instancesData_[index]; }
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
		osg::observer_ptr<osg::Node>             src_model_;

     // Instanced staff   
        osg::ref_ptr<osg::Geode>                 instGeode_;

        image_data                               image_data_;
        InstancedDataType                        instancesData_;
        InstancedNodesVectorType                 inst_nodes_; 
        size_t                                   inst_num_;
		osg::ref_ptr<osg::TextureRectangle>      animTexture_;
		osg::ref_ptr<osg::TextureRectangle>      instTexture_;
    private:
        utils::fixed_id_generator<unsigned>      inst_id_gen_;

    private:
        OpenThreads::Mutex                       mutex_;
    private: 

        static size_t  const  texture_row_data_size =   4096u;
	};

}