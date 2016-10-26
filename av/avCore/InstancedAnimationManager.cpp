#include "av/precompiled.h"

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>

#include "InstancedAnimationManager.h"

#include "av/avUtils/visitors/find_node_visitor.h"
#include "av/avUtils/materials.h"

#include "av/avCore/Callbacks.h"
#include "av/avScene/Scene.h"

#include "InstancesNum.h"

using namespace avCore;

namespace
{
	class ComputeTextureBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
	{
	public:
		ComputeTextureBoundingBoxCallback(const InstancedAnimationManager::InstancedDataType& instanceMatrices)
			: m_instanceMatrices(instanceMatrices)
		{
		}

		virtual osg::BoundingBox computeBound(const osg::Drawable& drawable) const override;
	private:
		const InstancedAnimationManager::InstancedDataType& m_instanceMatrices;
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


    InstancedAnimationManager::InstancedAnimationManager() 
        : instNum_       (0)
		, animDataLoaded_(false)
        , instGeodeParent_ (new osg::Group)
    {

    }

    InstancedAnimationManager::InstancedAnimationManager(const InstancedAnimationManager& object,const osg::CopyOp& copyop)
        : InstancesManager(object,copyop)
        , animDataLoaded_ (object.animDataLoaded_)
        , instNum_        (object.instNum_)
        , maxInstances_   (object.maxInstances_)
    {

    }
    
	InstancedAnimationManager::InstancedAnimationManager(osg::Node* prototype, const std::string& anim_file_name, size_t  const  max_instances)
        : srcModel_        (prototype)
        , instNum_         (0)
		, animDataLoaded_  (false)
		, maxInstances_    (max_instances)
        , instGeodeParent_ (new osg::Group)
	{ 

        _initData();
        animDataLoaded_ = _loadAnimationData(anim_file_name);

        instGeode_  = _createGeode();
        instGeodeParent_->addChild(instGeode_);

		auto tr = srcModel_->asTransform();
	    if(tr->asPositionAttitudeTransform())
		{
			srcQuat_ = tr->asPositionAttitudeTransform()->getAttitude();
            srcScale_  = tr->asPositionAttitudeTransform()->getScale();
		}
		else
		if(tr->asMatrixTransform())
		{
			srcQuat_ = tr->asMatrixTransform()->getMatrix().getRotate();
		}


        if(!animDataLoaded_)
        {
            instGeodeParent_->setCullCallback(Utils::makeNodeCallback(instGeodeParent_.get(), this, &InstancedAnimationManager::cull));
            cullStatePack_ = new CullStatePacks(instGeode_.get(), 3, max_instances);
        }
        
    }


	osg::TextureRectangle* InstancedAnimationManager::_createAnimationTexture( image_data& idata)
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



    osg::TextureRectangle* InstancedAnimationManager::_createTextureInstancesData()
    {
		const unsigned int start = 0;
		const unsigned int end = maxInstances_; // instancesData_.size();

		// create texture to encode all matrices

		unsigned int height = ((/*end-start*//*instancesData_.size()*/maxInstances_) / texture_row_data_size) + 1u;

		osg::ref_ptr<osg::Image>       image = new osg::Image; 
		image->allocateImage(maxInstances_ * 4, height, 1, GL_RGBA, GL_FLOAT);
		image->setInternalTextureFormat(GL_RGBA32F_ARB);

		instTextureBuffer_ = new osg::TextureRectangle(image);
		instTextureBuffer_->setInternalFormat(GL_RGBA32F_ARB);
		instTextureBuffer_->setSourceFormat(GL_RGBA);
		instTextureBuffer_->setSourceType(GL_FLOAT);
		instTextureBuffer_->setTextureSize(4, end-start);
		instTextureBuffer_->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
		instTextureBuffer_->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
		instTextureBuffer_->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
		instTextureBuffer_->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);

		// copy part of matrix list and create bounding box callback
		//std::vector<osg::Matrixd> matrices;
		//matrices.insert(matrices.begin(), instancesData_.begin(), instancesData_.end());
		// Этим тоже можно fps убить 
        //geometry->setComputeBoundingBoxCallback(new ComputeTextureBoundingBoxCallback(instancesData_));

		return instTextureBuffer_.get();
	}
	
	osgAnimation::BoneMap InstancedAnimationManager::_getBoneMap(osg::Node* base_model)
	{
		FindNodeByType< osgAnimation::Skeleton> s_finder;  
		s_finder.apply(*base_model);

		auto* skel  = dynamic_cast<osgAnimation::Skeleton*>(s_finder.getLast());
		osgAnimation::BoneMapVisitor mapVisitor;
		skel->accept(mapVisitor);
		return mapVisitor.getBoneMap();
	}


	osg::Geode* InstancedAnimationManager::_createGeode()
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

    void  InstancedAnimationManager::_initData()
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


	bool  InstancedAnimationManager::_loadAnimationData(std::string const&  filename)
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

    bool InstancedAnimationManager::_initSkinning(osg::Geometry& geom, const image_data& id )
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
             
    osg::Node *  InstancedAnimationManager::getObjectInstance()
    {  
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex_);

        osg::Group* root = new osg::Group;
        const unsigned  inst_id = instIdGen_.create();
        root->setUserValue("inst_id", inst_id);

        instancesNodes_.push_back(InstancedNodeType(root,nullptr));
        return root; 
    }

    void InstancedAnimationManager::commitInstancesPositions()
    {
        bool bCommit = false;
		size_t instCounter=0;
 		counter_ = 0;

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
				  osg::Matrixf modelMatrix = osg::Matrixf::scale(/*instancesData_[idx].getScale()*/srcScale_) 
										   * osg::Matrix::rotate(srcQuat_ * matrix.getRotate())
										   * osg::Matrixf::translate(matrix.getTrans());

				  instancesData_[idx] = modelMatrix;

				  float * data = (float*)instTextureBuffer_->getImage()->data((idx % texture_row_data_size) *4u, idx / texture_row_data_size);
				  memcpy(data, instancesData_[idx].ptr(), 16 * sizeof(float));
				  bCommit = true;
				  instCounter++;
				}
            
			} 

            instGeode_->setNodeMask(instCounter>0?REFLECTION_MASK:0);

		}
        else
        {
            instCounter =  instancesData_.size();
            if (instNum_!=instCounter)
            {
                float * data = (float*)instTextureBuffer_->getImage()->data(0);
                memcpy(data, instancesData_[0].ptr(), 16 * sizeof(float) * instCounter );
                
            }
        }
        
        if (instNum_!=instCounter || bCommit)
        {
            instTextureBuffer_->getImage()->dirty();
            _commit(instCounter);
        }

        instNum_ = instCounter;
    }

	void InstancedAnimationManager::_commit( size_t instCounter )
	{
		// instGeode_->setNodeMask(instCounter>0?REFLECTION_MASK:0);

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

    //
    // OSG callbacks
    //

#if 0
    // update pass
    void InstancedAnimationManager::update( osg::NodeVisitor * nv )
    {
    }
#endif
    
    inline double distance(const osg::Vec3& coord,const osg::Matrix& matrix)
    {
        return -((double)coord[0]*(double)matrix(0,2)+(double)coord[1]*(double)matrix(1,2)+(double)coord[2]*(double)matrix(2,2)+matrix(3,2));
    }

    // cull pass
    osg::StateSet* InstancedAnimationManager::cull(osg::NodeVisitor * nv)
    {
        // get cull visitor
        osgUtil::CullVisitor * pCV = static_cast<osgUtil::CullVisitor *>(nv);
        assert(pCV);

        if(animDataLoaded_)
            return nullptr;

        auto const & bs = srcModel_->getBound();

        processedIndexes_.resize(0);
        osg::Polytope& fr = pCV->getCurrentCullingSet().getFrustum();

        for (unsigned i = 0; i < instancesData_.size(); ++i)
        {
            const auto & inst_data = instancesData_[i];
            const osg::Vec3& vWorldPos = inst_data.getTrans();

            if (pCV->isCulled(osg::BoundingSphere(vWorldPos, double(inst_data.getScale().x() * bs.radius()))))
            // if (!fr.contains(osg::BoundingSphere(vWorldPos, double(inst_data.getScale().x() * bs.radius()))))
            {
                // osg::RefMatrix& matrix = *pCV->getModelViewMatrix();
                // if(distance(vWorldPos,matrix)>50000.0)
                    continue;
            }

            processedIndexes_.push_back(i);
        }


        auto & currentPack = cullStatePack_->getOrCreatePack(counter_);
        cullStatePack_->commit(counter_, processedIndexes_);
  
        
        counter_++;
        return currentPack.ss.get();
    }


												
    InstancedAnimationManager::CullStatePacks::Pack::Pack()
        : ss         (new osg::StateSet)
        , baseInstance (new osg::Uniform("baseInstance", int( 0 )))
    {
        ss->addUniform(baseInstance);
    }
   


    InstancedAnimationManager::CullStatePacks::CullStatePacks(osg::Geode* ig, uint8_t max_stages, uint32_t max_elements_on_stage )
        : instanced_g (ig)
    {
        states.reserve(max_stages);

        osg::Image* cullImage = new osg::Image;
        cullImage->allocateImage(  max_elements_on_stage * max_stages , 1, 1, GL_RED, GL_UNSIGNED_INT);
        cullTextureBuffer = new osg::TextureBuffer(cullImage);
        cullTextureBuffer->setInternalFormat( GL_R32I );
        cullTextureBuffer->setUsageHint(GL_DYNAMIC_DRAW);
        cullTextureBuffer->bindToImageUnit(BASE_CULL_TEXTURE_UNIT, osg::Texture::READ_WRITE);
        cullTextureBuffer->setUnRefImageDataAfterApply(false);

#if 0
		instanced_g->getParent(0)->getOrCreateStateSet()->addUniform(new osg::Uniform("cullTex", BASE_CULL_TEXTURE_UNIT));		
		instanced_g->getParent(0)->getOrCreateStateSet()->setTextureAttribute(BASE_CULL_TEXTURE_UNIT, cullTextureBuffer.get(), osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);
#endif

    }

    const InstancedAnimationManager::CullStatePacks::Pack& InstancedAnimationManager::CullStatePacks::getOrCreatePack(uint8_t num)
    {
        if(states.size() <= num)
        {
            states.push_back(new Pack);
			auto pStateSet = states.back()->ss;

#if 1
			pStateSet->setTextureAttribute(BASE_CULL_TEXTURE_UNIT, cullTextureBuffer.get(), osg::StateAttribute::ON);
			pStateSet->addUniform(new osg::Uniform("cullTex", BASE_CULL_TEXTURE_UNIT));
#endif

			// setup inst
			osg::InstancesNum * pINFunc = new osg::InstancesNum(instanced_g, 0);
			pStateSet->setAttributeAndModes(pINFunc, osg::StateAttribute::ON);
        }

        return *states[num].get();
    }
    
    size_t   InstancedAnimationManager::CullStatePacks::packs_len(uint8_t num)
    {
        size_t s = 0;
        for(int i = 0; i < std::min(states.size(), size_t(num)); ++i )
             s += states[i].get()->last_len;

        return s;
    }

    void InstancedAnimationManager::CullStatePacks::commit (uint8_t num, const CullIndexes& ci)
    {
        auto & currentPack = *states[num].get();
        currentPack.last_len = ci.size();
		size_t baseInstance = packs_len(num);
        uint32_t * data = (uint32_t*)cullTextureBuffer->getImage()->data(baseInstance);
        memcpy(data, &ci[0], sizeof(uint32_t) * ci.size() );
        cullTextureBuffer->getImage()->dirty();
#if 0
		force_log fl;       
		LOG_ODS_MSG( " CullStatePacks::commit num = " << uint32_t(num) << "  baseInstance = " << baseInstance<<  "\n");
#endif
        auto ia = static_cast<osg::InstancesNum*>(currentPack.ss->getAttribute(static_cast<osg::StateAttribute::Type>(INSTANCES_NUM_OBJECT),0));
        if(ia)
            ia->setNum(ci.size());

		currentPack.baseInstance->set(int(baseInstance));
    }


}