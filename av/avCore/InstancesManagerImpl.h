#pragma once

#include "utils/cpp_utils/id_generator.h"
#include "InstancesManager.h"

namespace avCore
{
	class  InstancesManagerImpl  : public InstancesManager
	{
		struct instanced_nodes_vector_t
		{
			instanced_nodes_vector_t()
				: parented(false)
			{}

			instanced_nodes_vector_t(osg::Node* first, osg::Node* second )
				: parented(false)
				, first   (first)
				, second  (second)
			{}

			osg::ref_ptr<osg::Node> first;
		    osg::ref_ptr<osg::Node> second;
			bool                    parented;
		};

		typedef std::vector< instanced_nodes_vector_t > InstancedNodesVectorType;
	public:
		typedef std::vector<osg::Matrixf>              InstancedDataType;
	public:
        
		META_Object(avCore,InstancesManagerImpl);
        
        InstancesManagerImpl  ();

        InstancesManagerImpl  (const InstancesManagerImpl& im,const osg::CopyOp& =osg::CopyOp::SHALLOW_COPY);

		InstancesManagerImpl  (osg::Node* prototype, const std::string& anim_file);

        void                        commitInstancesPositions();

		inline void                 addMatrix(const osg::Matrixf& matrix) { instancesData_.push_back(matrix); }
		inline osg::Matrixf         getMatrix(size_t index) const         { return instancesData_[index]; }
		inline void                 clearMatrices()                       { instancesData_.clear(); }
        inline osg::Geode*          getMainNode()                         { return instGeode_.get(); }
        osg::Node *                 getObjectInstance();                    


#if 0
		void                        setInstanceData(size_t idx, const osg::Matrixf& matrix);
		const InstancedDataType &   getInstancedData() const { return instancesData_; }
		const image_data&           getImageData()     const { return image_data_; } 
#endif  

	private:
		osg::TextureRectangle*      _createAnimationTexture( image_data& idata);
        osg::TextureRectangle*      _createTextureInstancedData();
        bool                        _loadAnimationData(std::string const&  filename);
        bool                        _initSkinning(osg::Geometry& geom, const image_data& id );
        osg::Geode*                 _createGeode();
		osgAnimation::BoneMap       _getBoneMap(osg::Node* base_model);
        void                        _initData();

	private:
		osg::observer_ptr<osg::Node>             src_model_;
		osg::Quat                                src_quat_;
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