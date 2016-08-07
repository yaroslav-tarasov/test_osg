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
        : inst_num_ (0)
    {

    }

    InstancesManagerImpl::InstancesManagerImpl(const InstancesManagerImpl& object,const osg::CopyOp& copyop)
        : InstancesManager(object,copyop)
    {
    
    }

	InstancesManagerImpl::InstancesManagerImpl(osg::Node* prototype, const std::string& anim_file_name)
        : src_model_     (prototype)
        , inst_num_ (0)
	{ 

        _initData();
        _loadAnimationData(anim_file_name);

        instGeode_  = _createGeode();

		auto tr = src_model_->asTransform();
	    if(tr->asPositionAttitudeTransform())
		{
			src_quat_ = tr->asPositionAttitudeTransform()->getAttitude();
		}
		
		if(tr->asMatrixTransform())
		{
			src_quat_ = tr->asMatrixTransform()->getMatrix().getRotate();
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

		image_data_.data.resize(texture->getImage()->getTotalSizeInBytes());
		unsigned char* dest_ptr = &image_data_.data[0];
		for(osg::Image::DataIterator itr(texture->getImage()); itr.valid(); ++itr)
		{
			memcpy(dest_ptr, itr.data(), itr.size());
			dest_ptr += itr.size();
		}

		return texture.release();
	}



    osg::TextureRectangle* InstancesManagerImpl::_createTextureInstancedData()
    {
		const unsigned int start = 0;
		const unsigned int end = instancesData_.size();

		// create texture to encode all matrices
        const size_t fixed_data_size =   4096u;

		unsigned int height = ((/*end-start*//*instancesData_.size()*/fixed_data_size) / texture_row_data_size) + 1u;

		osg::ref_ptr<osg::Image>       image = new osg::Image; 
		image->allocateImage(fixed_data_size * 4, height, 1, GL_RGBA, GL_FLOAT);
		image->setInternalTextureFormat(GL_RGBA32F_ARB);

		instTexture_ = new osg::TextureRectangle(image);
		instTexture_->setInternalFormat(GL_RGBA32F_ARB);
		instTexture_->setSourceFormat(GL_RGBA);
		instTexture_->setSourceType(GL_FLOAT);
		instTexture_->setTextureSize(4, end-start);
		instTexture_->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
		instTexture_->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
		instTexture_->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
		instTexture_->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);

		// copy part of matrix list and create bounding box callback
		//std::vector<osg::Matrixd> matrices;
		//matrices.insert(matrices.begin(), instancesData_.begin(), instancesData_.end());
		// Этим тоже можно fps убить 
        //geometry->setComputeBoundingBoxCallback(new ComputeTextureBoundingBoxCallback(instancesData_));

		return instTexture_.get();
	}


	osg::TextureBuffer* InstancesManagerImpl::_createTextureBuffer() 
	{
		const size_t sizeoffarray = 1024;
		bufferMatrices_ = new BufferInstancesT;
		bufferMatrices_->getData().reserve(sizeoffarray);


		const osg::Matrixf matrix;

		for (unsigned int j = 0; j < sizeoffarray; ++j)
		{
			bufferMatrices_->getData().push_back( StaticInstance(0, j, matrix));
		}


		osg::Image* instancesImage = new osg::Image;
		instancesImage->setImage( bufferMatrices_->getTotalDataSize() / sizeof(osg::Vec4f), 1, 1, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT, (unsigned char*)bufferMatrices_->getDataPointer(), osg::Image::NO_DELETE );
		osg::TextureBuffer* instancesTextureBuffer = new osg::TextureBuffer(instancesImage);
		instancesTextureBuffer->setUsageHint(GL_STATIC_DRAW);
		instancesTextureBuffer->setUnRefImageDataAfterApply(false);
		
		return instancesTextureBuffer;
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
		geode_finder.apply(*src_model_);

		osg::Geode*    gnode = dynamic_cast<osg::Geode*>(geode_finder.getLast()); 
		osg::Geometry* mesh = gnode->getDrawable(0)->asGeometry();

		osg::ref_ptr<osg::Geode>	   geode = new osg::Geode;
		osg::StateSet* pSS = geode->getOrCreateStateSet();
		osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry(*mesh, osg::CopyOp::DEEP_COPY_ALL
			& ~osg::CopyOp::DEEP_COPY_CALLBACKS
			);

		osg::Drawable::ComputeBoundingBoxCallback * pDummyBBCompute = new osg::Drawable::ComputeBoundingBoxCallback();
		geometry->setComputeBoundingBoxCallback(pDummyBBCompute);

        // we need to turn off display lists for instancing to work
        geometry->setUseDisplayList(false);
        geometry->setUseVertexBufferObjects(true);

		_createTextureInstancedData();
        
        _initSkinning(*geometry.get(), image_data_ );

		geode->addDrawable(geometry);
#if 0
		pSS->setTextureAttributeAndModes(BASE_COLOR_TEXTURE_UNIT, texture, osg::StateAttribute::ON);
		pSS->addUniform(new osg::Uniform("colorTex", BASE_COLOR_TEXTURE_UNIT));
		//pSS->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.8f), osg::StateAttribute::ON);
#endif
        
        pSS->setTextureAttributeAndModes(BASE_HW_INST_TEXTURE_UNIT, instTexture_.get(), osg::StateAttribute::ON);
        pSS->addUniform(new osg::Uniform("instanceMatrixTexture", BASE_HW_INST_TEXTURE_UNIT));

		pSS->setTextureAttributeAndModes(BASE_HW_ANIMATION_TEXTURE_UNIT, animTexture_.get(), osg::StateAttribute::ON);
		pSS->addUniform(new osg::Uniform("animationTex", BASE_HW_ANIMATION_TEXTURE_UNIT));



		return geode.release();
	}

    void  InstancesManagerImpl::_initData()
    {
        const size_t x_num = 16; 
        const size_t y_num = 16;
        
        instancesData_.reserve( x_num * y_num);

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
    }


	bool  InstancesManagerImpl::_loadAnimationData(std::string const&  filename)
	{
		std::ifstream image_data_file(filename, std::ios_base::binary);

		if (image_data_file.good())
		{
			binary::bytes_t data;
			data.resize(boost::filesystem::file_size(filename));
			image_data_file.read(data.data(), data.size());
			binary::unwrap(data, image_data_);
			animTexture_ = _createAnimationTexture(image_data_);
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

        osg::ref_ptr<osg::Program> cSkinningProg = creators::createProgram("skininst").program; 
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
        const unsigned  inst_id = inst_id_gen_.create();
        root->setUserValue("inst_id", inst_id);

        inst_nodes_.push_back(instanced_nodes_vector_t(root,nullptr));
        return root; 
    }

    void InstancesManagerImpl::commitInstancesPositions()
    {
        bool bcommit = false;
        size_t inst_counter=0;

        inst_nodes_.erase(std::remove_if(inst_nodes_.begin(), inst_nodes_.end(),
                                        [](const InstancedNodesVectorType::value_type& v){return v.second && (v.second->getNumParents()==0) && v.parented;})
                                        , inst_nodes_.end());

        for ( size_t idx = 0; idx < inst_nodes_.size(); ++idx )
        {
            auto & nd = inst_nodes_[idx];
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
                                       * osg::Matrix::rotate(src_quat_ * matrix.getRotate())
                                       * osg::Matrixf::translate(matrix.getTrans());

			  instancesData_[idx] = modelMatrix;

              float * data = (float*)instTexture_->getImage(0)->data((idx % texture_row_data_size) *4u, idx / texture_row_data_size);
              memcpy(data, instancesData_[idx].ptr(), 16 * sizeof(float));
			  bcommit = true;
              inst_counter++;
			}
            
        } 

        if (inst_num_!=inst_counter)
             bcommit = true;
		
        if(bcommit)
        {
            instTexture_->getImage(0)->dirty();

            instGeode_->setNodeMask(inst_counter>0?REFLECTION_MASK:0);

            for(unsigned i=0;i<instGeode_->getNumDrawables(); ++i)
            { 
                instGeode_->getDrawable(i)->dirtyBound();
                        
                //if(inst_counter != inst_nodes_.size())
                if (inst_num_!=inst_counter)
                {
                    auto geometry = instGeode_->getDrawable(i)->asGeometry();
                    // first turn on hardware instancing for every primitive set
                    for (unsigned j = 0; j < geometry->getNumPrimitiveSets(); ++j)
                    {
                        geometry->getPrimitiveSet(j)->setNumInstances(inst_counter);
                    }
                }

            }
        }
        
        inst_num_ = inst_counter;
    }
 
}