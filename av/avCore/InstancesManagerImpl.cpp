#include "stdafx.h"
#include "av/precompiled.h"

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>

#include "InstancesManagerImpl.h"

#include "utils/visitors/find_node_visitor.h"
#include "utils/materials.h"


using namespace avCore;

namespace
{
	class ComputeTextureBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
	{
	public:
		ComputeTextureBoundingBoxCallback(const InstancesManagerImpl::InstancedDataType& instanceMatrices)
			: m_instanceMatrices(instanceMatrices)
		{
		}

		virtual osg::BoundingBox computeBound(const osg::Drawable& drawable) const override;
	private:
		const InstancesManagerImpl::InstancedDataType& m_instanceMatrices;
	};


	osg::BoundingBox ComputeTextureBoundingBoxCallback::computeBound(const osg::Drawable& drawable) const
	{
		osg::BoundingBox bounds;
		const osg::Geometry* geometry = drawable.asGeometry();

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


namespace avCore
{


    InstancesManagerImpl::InstancesManagerImpl() 
        : instNum_ (0)
        , animDataLoaded_(false)
    {

    }

    InstancesManagerImpl::InstancesManagerImpl(const InstancesManagerImpl& object,const osg::CopyOp& copyop)
        : InstancesManager(object,copyop)
    {

    }

    InstancesManagerImpl::InstancesManagerImpl(osg::Node* prototype, const std::string& anim_file_name, size_t  const  max_instances)
        : srcModel_     (prototype)
        , instNum_ (0)
        , animDataLoaded_(false)
        , maxInstances_(max_instances)
    { 

        _initData();
        animDataLoaded_ = _loadAnimationData(anim_file_name);

        instGeode_  = _createGeode();

        auto tr = srcModel_->asTransform();
        if(tr->asPositionAttitudeTransform())
        {
            srcQuat_ = tr->asPositionAttitudeTransform()->getAttitude();
        }
        else
            if(tr->asMatrixTransform())
            {
                srcQuat_ = tr->asMatrixTransform()->getMatrix().getRotate();
            }
    }


    osg::TextureRectangle* InstancesManagerImpl::_createAnimationTexture( image_data& idata)
    {
        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->setImage(idata.s, idata.t, idata.r, idata.internalFormat, idata.pixelFormat, idata.type, &idata.data[0], osg::Image::NO_DELETE);

        osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle(image);
        texture->setInternalFormat(GL_RGBA32F_ARB);
        texture->setSourceFormat(GL_RGBA);
        texture->setSourceType(GL_FLOAT);
        texture->setTextureSize(4, idata.data_len);
        texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
        texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);

        imageData_.data.resize(texture->getImage()->getTotalSizeInBytes());
        unsigned char* dest_ptr = &imageData_.data[0];
        for(osg::Image::DataIterator itr(texture->getImage()); itr.valid(); ++itr)
        {
            memcpy(dest_ptr, itr.data(), itr.size());
            dest_ptr += itr.size();
        }

        return texture.release();
    }



    osg::TextureBuffer* InstancesManagerImpl::_createTextureInstancesData()
    {
        // create texture to encode all matrices

        osg::ref_ptr<osg::Image>       image = new osg::Image; 
        image->allocateImage(maxInstances_ * 4, 1, 1, GL_RGBA, GL_FLOAT);
        image->setInternalTextureFormat(GL_RGBA32F_ARB);

        instTextureBuffer_ = new osg::TextureBuffer(image);
        instTextureBuffer_->setInternalFormat(GL_RGBA32F_ARB);
        //instTextureBuffer_->setSourceFormat(GL_RGBA);
        //instTextureBuffer_->setSourceType(GL_FLOAT);


        // copy part of matrix list and create bounding box callback
        //std::vector<osg::Matrixd> matrices;
        //matrices.insert(matrices.begin(), instancesData_.begin(), instancesData_.end());
        // Этим тоже можно fps убить 
        //geometry->setComputeBoundingBoxCallback(new ComputeTextureBoundingBoxCallback(instancesData_));

        return instTextureBuffer_.get();
    }

    osgAnimation::BoneMap InstancesManagerImpl::_getBoneMap(osg::Node* base_model)
    {
        FindNodeByType< osgAnimation::Skeleton> s_finder;  
        s_finder.apply(*base_model);

        auto* skel  = dynamic_cast<osgAnimation::Skeleton*>(s_finder.getLast());
        osgAnimation::BoneMapVisitor mapVisitor;
        skel->accept(mapVisitor);
        return mapVisitor.getBoneMap();
    }


    osg::Geode* InstancesManagerImpl::_createGeode()
    {
        FindNodeByType< osg::Geode> geode_finder;  
        geode_finder.apply(*srcModel_);

        osg::Geode*    gnode = dynamic_cast<osg::Geode*>(geode_finder.getLast()); 

        osg::ref_ptr<osg::Geode>	   geode = new osg::Geode;
        osg::StateSet* pSS = geode->getOrCreateStateSet();

        for(size_t drb = 0; drb < gnode->getNumDrawables(); ++drb )
        {
            osg::Geometry* mesh = gnode->getDrawable(drb)->asGeometry();
            osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry(*mesh, osg::CopyOp::DEEP_COPY_ALL
                & ~osg::CopyOp::DEEP_COPY_CALLBACKS
                );

            osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();
            geometry->setComputeBoundingBoxCallback(pDummyBBCompute);

            // we need to turn off display lists for instancing to work
            geometry->setUseDisplayList(false);
            geometry->setUseVertexBufferObjects(true);

            if(animDataLoaded_)
                _initSkinning(*geometry.get(), imageData_ );

            geode->addDrawable(geometry);
        }

        _createTextureInstancesData();

#if 0
        pSS->setTextureAttributeAndModes(BASE_COLOR_TEXTURE_UNIT, texture, osg::StateAttribute::ON);
        pSS->addUniform(new osg::Uniform("colorTex", BASE_COLOR_TEXTURE_UNIT));
        //pSS->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.8f), osg::StateAttribute::ON);
#endif

        pSS->setTextureAttributeAndModes(BASE_HW_INST_TEXTURE_UNIT, instTextureBuffer_.get(), osg::StateAttribute::ON);
        pSS->addUniform(new osg::Uniform("instanceMatrixTexture", BASE_HW_INST_TEXTURE_UNIT));

        if(animDataLoaded_)
        {
            pSS->setTextureAttributeAndModes(BASE_HW_ANIMATION_TEXTURE_UNIT, animTextureBuffer_.get(), osg::StateAttribute::ON);
            pSS->addUniform(new osg::Uniform("animationTex", BASE_HW_ANIMATION_TEXTURE_UNIT));
        }


        return geode.release();
    }

    void  InstancesManagerImpl::_initData()
    {
        const size_t x_num = 16; 
        const size_t y_num = 16;

        instancesData_.reserve( x_num * y_num);
#if 0
        // create some matrices
        srand(time(NULL));

        for (unsigned int i = 0; i < x_num; ++i)
        {
            for (unsigned int j = 0; j < y_num; ++j)
            {
                // get random angle and random scale
                double angle = (rand() % 360) / 180.0 * cg::pi;
                double scale = 0.02 + (static_cast<double>(rand() % 10) / 1000.0 - 0.005);

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
                addMatrix(modelMatrix);
            }
        }
#endif

    }


    bool  InstancesManagerImpl::_loadAnimationData(std::string const&  filename)
    {
        std::ifstream image_data_file(filename, std::ios_base::binary);

        if (image_data_file.good())
        {
            binary::bytes_t data;
            data.resize(boost::filesystem::file_size(filename));
            image_data_file.read(data.data(), data.size());
            binary::unwrap(data, imageData_);
            animTextureBuffer_ = _createAnimationTexture(imageData_);
            return true;
        }

        return false;
    }

    bool InstancesManagerImpl::_initSkinning(osg::Geometry& geom, const image_data& id )
    {
        osg::Geometry& source = geom;
        osg::Vec3Array* positionSrc = dynamic_cast<osg::Vec3Array*>(source.getVertexArray());
        if (!positionSrc)
        {
            OSG_WARN << "InstancedAnimationManager no vertex array in the geometry " << geom.getName() << std::endl;
            return false;
        }

        osg::ref_ptr<osg::Program> cSkinningProg = creators::createProgram("skininst2").program; 
        cSkinningProg->setName("SkinningShader");

        const int attribIndex = cAttribSkinningBaseIndex;
        int nbAttribs = id.bonesWeights.size();
        for (int i = 0; i < nbAttribs; i++)
        {
            osg::ref_ptr<osg::Vec4Array> array = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
            const image_data::weights_t&  w = id.bonesWeights[i];
            for (unsigned j = 0; j < w.size(); j+=id.divisor)
            {
                array->push_back(osg::Vec4(w.at(j),w.at(j+1),w.at(j+2),w.at(j+3)));
            }		

            std::stringstream ss;
            ss << "boneWeight" << i;
            cSkinningProg->addBindAttribLocation(ss.str(), attribIndex + i);
            geom.setVertexAttribArray(attribIndex + i, array);
            OSG_INFO << "set vertex attrib " << ss.str() << std::endl;
        }

        osg::ref_ptr<osg::StateSet> ss = geom.getOrCreateStateSet();
        ss->addUniform(new osg::Uniform("nbBonesPerVertex", id.bonesPerVertex));
        ss->setAttributeAndModes(cSkinningProg.get());

        return true;
    }

    osg::Node *  InstancesManagerImpl::getObjectInstance()
    {  
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex_);

        osg::Group* root = new osg::Group;
        const unsigned  inst_id = instIdGen_.create();
        root->setUserValue("inst_id", inst_id);

        instancesNodes_.push_back(InstancedNodeType(root,nullptr));
        return root; 
    }

    void InstancesManagerImpl::commitInstancesPositions()
    {
        bool bCommit = false;
        size_t instCounter=0;

        if(animDataLoaded_)
        {
            instancesNodes_.erase(std::remove_if(instancesNodes_.begin(), instancesNodes_.end(),
                [](const InstancedNodesVectorType::value_type& v){return v.second && (v.second->getNumParents()==0) && v.parented;})
                , instancesNodes_.end());

            for ( size_t idx = 0; idx < instancesNodes_.size(); ++idx )
            {
                auto & nd = instancesNodes_[idx];
                auto & inst_data = instancesData_[idx];
                if(!nd.second)
                {
                    nd.second = findFirstNode(nd.first,"phys_ctrl",FindNodeVisitor::not_exact,osg::NodeVisitor::TRAVERSE_PARENTS);
                }

                if(nd.second && nd.second->getNumParents()>0)
                {
                    nd.parented  = true;
                    osg::Matrixf matrix = nd.second->asTransform()->asMatrixTransform()->getMatrix();
                    osg::Matrixf modelMatrix = osg::Matrixf::scale(instancesData_[idx].getScale()) 
                        * osg::Matrix::rotate(srcQuat_ * matrix.getRotate())
                        * osg::Matrixf::translate(matrix.getTrans());

                    instancesData_[idx] = modelMatrix;

                    float * data = (float*)instTextureBuffer_->getImage(0)->data( idx  *4u);
                    memcpy(data, instancesData_[idx].ptr(), 16 * sizeof(float));
                    bCommit = true;
                    instCounter++;
                }

            } 
        }
        else
        {
            instCounter = 	instancesData_.size();
            float * data = (float*)instTextureBuffer_->getImage(0)->data(0);

            if (instNum_!=instCounter)
                memcpy(data, instancesData_[0].ptr(), 16 * sizeof(float) * instCounter );
        }



        if (instNum_!=instCounter)
            bCommit = true;

        if(bCommit)
        {
            instTextureBuffer_->getImage(0)->dirty();

            instGeode_->setNodeMask(instCounter>0?REFLECTION_MASK:0);

            for(unsigned i=0;i<instGeode_->getNumDrawables(); ++i)
            { 
                instGeode_->getDrawable(i)->dirtyBound();

                if (instNum_!=instCounter)
                {
                    auto geometry = instGeode_->getDrawable(i)->asGeometry();
                    // first turn on hardware instancing for every primitive set
                    for (unsigned int j = 0; j < geometry->getNumPrimitiveSets(); ++j)
                    {
                        geometry->getPrimitiveSet(j)->setNumInstances(instCounter);
                    }
                }

            }
        }

        instNum_ = instCounter;
    }

}