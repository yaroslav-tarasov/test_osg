#pragma once

namespace Utils
{
    struct MyRigTransformHardware : public osgAnimation::RigTransformHardware
    {

        avAnimation::AnimationChannelMatricesType       anim_mat_data_;
        image_data                                      image_data_;

        void operator()(osgAnimation::RigGeometry& geom);
        bool init(osgAnimation::RigGeometry& geom);
        void computeMatrixPaletteUniform(const osg::Matrix& transformFromSkeletonToGeometry, const osg::Matrix& invTransformFromSkeletonToGeometry);
    };
}

namespace avAnimation {


    
    class  InstancedAnimationManager
    {
        AnimationDataType                anim_data_;
        osgAnimation::BoneMap                   bm_;
        image_data                           idata_;
        osg::ref_ptr<Utils::MyRigTransformHardware>  mrth_;
    
    private:

        inline osgAnimation::BoneMap getBoneMap(osg::Node* base_model);

    public:

        InstancedAnimationManager(osg::Node* base_model);
        osg::TextureRectangle* createAnimationData();
        
        void setupRigGeometry(osg::Node* node, bool hardware = true);

        const image_data& getImageData() const;
    };

}

