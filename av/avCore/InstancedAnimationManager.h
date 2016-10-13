#pragma once

#include "utils/cpp_utils/id_generator.h"
#include "InstancesManager.h"

namespace avCore
{
	class  InstancedAnimationManager  : public InstancesManager
	{

	public:
       		
		META_Object(avCore,InstancedAnimationManager);

        InstancedAnimationManager ();

        InstancedAnimationManager (const InstancedAnimationManager& im,const osg::CopyOp& =osg::CopyOp::SHALLOW_COPY);

		InstancedAnimationManager (osg::Node* prototype, const std::string& anim_file, size_t  const  max_instances = 4096u - 1);

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
        osg::TextureRectangle*      _createTextureInstancesData();
        bool                        _loadAnimationData(std::string const&  filename);
        bool                        _initSkinning(osg::Geometry& geom, const image_data& id );
        osg::Geode*                 _createGeode();
		osgAnimation::BoneMap       _getBoneMap(osg::Node* base_model);
        void                        _initData();
        void                        _commit( size_t instCounter );

    private:
        // update callback
        // void update( osg::NodeVisitor * nv );
        // cull callback
        void cull( osg::NodeVisitor * nv );

	private:
		osg::observer_ptr<osg::Node>             srcModel_;
        osg::Quat                                srcQuat_;
		osg::Vec3                                srcScale_;
     // Instanced staff   
        osg::ref_ptr<osg::Geode>                 instGeode_;

        image_data                               imageData_;
        InstancedDataType                        instancesData_;
        InstancedDataType                        processedInstancesData_;

        InstancedNodesVectorType                 instancesNodes_; 
        size_t                                   instNum_;
		size_t      							 maxInstances_;

		bool								     animDataLoaded_;
		osg::ref_ptr<osg::TextureRectangle>      animTextureBuffer_;
		osg::ref_ptr<osg::TextureRectangle>      instTextureBuffer_;
    private:
        utils::fixed_id_generator<unsigned>      instIdGen_;

    private:
        OpenThreads::Mutex                       mutex_;
    private: 
        static size_t  const  texture_row_data_size =   4096u;
	};

}