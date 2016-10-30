#include "stdafx.h"

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>
#include <osg/TextureRectangle>

#include "av/avUtils/visitors/find_node_visitor.h"
#include "av/avUtils/osg_helpers.h"
#include "av/avUtils/empty_scene.h"
//#include "av/avUtils/animutils.h"


#include "av/avCore/InstancedData.h"
#include "core_anim.h"

namespace Utils
{

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



}


namespace avAnimation {


        inline osgAnimation::BoneMap InstancedAnimationManager::getBoneMap(osg::Node* base_model)
        {
            FindNodeByType< osgAnimation::Skeleton> s_finder;  
            s_finder.apply(*base_model);

            auto* skel  = dynamic_cast<osgAnimation::Skeleton*>(s_finder.getLast());
            osgAnimation::BoneMapVisitor mapVisitor;
            skel->accept(mapVisitor);
            return mapVisitor.getBoneMap();
        }

        InstancedAnimationManager::InstancedAnimationManager(osg::Node* base_model)
            :bm_(getBoneMap(base_model))
        {}


        osg::TextureRectangle* InstancedAnimationManager::createAnimationData()
        {
            const Utils::MyRigTransformHardware& rth = *mrth_;
            
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

        const image_data& InstancedAnimationManager::getImageData() const                
        {
            return idata_;
        }

        void InstancedAnimationManager::setupRigGeometry(osg::Node* node, bool hardware)
        {
            Utils::SetupRigGeometry switcher(true);
            node->accept(switcher);
            mrth_ = switcher.my_ptr;
        }
}