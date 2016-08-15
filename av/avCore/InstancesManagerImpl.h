#pragma once

#include "utils/cpp_utils/id_generator.h"
#include "InstancesManager.h"

//
// Local includes
//

#if (((OSG_VERSION_MAJOR>=3) && (OSG_VERSION_MINOR>3)) )
#include <osg/BufferTemplate>
#else
#include "osg/BufferTemplate_3_4"
#endif

namespace avCore
{
    class  InstancesManagerImpl  : public InstancesManager
    {
        struct InstancedNodeType
        {
            InstancedNodeType()
                : parented(false)
            {}

            InstancedNodeType(osg::Node* first, osg::Node* second )
                : parented(false)
                , first   (first)
                , second  (second)
            {}

            osg::ref_ptr<osg::Node> first;
            osg::ref_ptr<osg::Node> second;
            bool                    parented;
        };

        typedef std::vector< InstancedNodeType > InstancedNodesVectorType;
    public:
        typedef std::vector<osg::Matrixf>               InstancedDataType;
    public:

        META_Object(avCore,InstancesManagerImpl);

        InstancesManagerImpl ();

        InstancesManagerImpl (const InstancesManagerImpl& im,const osg::CopyOp& =osg::CopyOp::SHALLOW_COPY);

        InstancesManagerImpl (osg::Node* prototype, const std::string& anim_file, size_t  const  max_instances = 4096u - 1);

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
        osg::TextureBuffer*         _createTextureInstancesData();
        bool                        _loadAnimationData(std::string const&  filename);
        bool                        _initSkinning(osg::Geometry& geom, const image_data& id );
        osg::Geode*                 _createGeode();
        osgAnimation::BoneMap       _getBoneMap(osg::Node* base_model);
        void                        _initData();

    private:
        osg::observer_ptr<osg::Node>             srcModel_;
        osg::Quat                                srcQuat_;
        // Instanced staff   
        osg::ref_ptr<osg::Geode>                 instGeode_;

        image_data                               imageData_;
        InstancedDataType                        instancesData_;
        InstancedNodesVectorType                 instancesNodes_; 
        size_t                                   instNum_;
        size_t      							 maxInstances_;

        bool								     animDataLoaded_;
        osg::ref_ptr<osg::TextureRectangle>      animTextureBuffer_;
        osg::ref_ptr<osg::TextureBuffer>         instTextureBuffer_;
    private:
        utils::fixed_id_generator<unsigned>      instIdGen_;

    private:
        OpenThreads::Mutex                       mutex_;
    private: 
        static size_t  const  texture_row_data_size =   4096u;
    };

}