#include "stdafx.h"
#include "av/precompiled.h"

#include <osgAnimation/RigGeometry>
#include <osgAnimation/RigTransformHardware>
#include <osgAnimation/BoneMapVisitor>

#include "InstancedAnimationManager.h"

#include "utils/visitors/find_node_visitor.h"
#include "utils/materials.h"

namespace
{
	class ComputeTextureBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
	{
	public:
		ComputeTextureBoundingBoxCallback(const std::vector<osg::Matrixd>& instanceMatrices)
			: m_instanceMatrices(instanceMatrices)
		{
		}

		virtual osg::BoundingBox computeBound(const osg::Drawable& drawable) const;
	private:
		const std::vector<osg::Matrixd>& m_instanceMatrices;
	};


	osg::BoundingBox ComputeTextureBoundingBoxCallback::computeBound(const osg::Drawable& drawable) const
	{
		osg::BoundingBox bounds;
		const osg::Geometry* geometry = dynamic_cast<const osg::Geometry*>(&drawable);

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

void RigTransformHardware::operator()(osgAnimation::RigGeometry& geom)
{
	if (_needInit)
		if (!init(geom))
			return;
		else if (!init(*inst_geom_.get()))
			return;

	avCore::RigTransformHardware::computeMatrixPaletteUniform(geom.getMatrixFromSkeletonToGeometry(), geom.getInvMatrixFromSkeletonToGeometry());
}

void RigTransformHardware::computeMatrixPaletteUniform(const osg::Matrix& transformFromSkeletonToGeometry, const osg::Matrix& invTransformFromSkeletonToGeometry)
{
	const size_t palSize = _bonePalette.size();
	for (size_t i = 0; i < palSize; i++)
	{
		osg::ref_ptr<osgAnimation::Bone> bone = _bonePalette[i].get();
		const osg::Matrix& invBindMatrix = bone->getInvBindMatrixInSkeletonSpace();
		const osg::Matrix& boneMatrix = bone->getMatrixInSkeletonSpace();
		osg::Matrix resultBoneMatrix = invBindMatrix * boneMatrix;
		osg::Matrix result =  transformFromSkeletonToGeometry * resultBoneMatrix * invTransformFromSkeletonToGeometry;
		if (!_uniformMatrixPalette->setElement(i, result))
			OSG_WARN << "RigTransformHardware::computeUniformMatrixPalette can't set uniform at " << i << " elements" << std::endl;
	}
}

bool RigTransformHardware::init(osgAnimation::RigGeometry& geom)
{
	return osgAnimation::RigTransformHardware::init(geom);
}

bool RigTransformHardware::init(osg::Geometry& geom)
{
	osg::Geometry& source = geom;
	osg::Vec3Array* positionSrc = dynamic_cast<osg::Vec3Array*>(source.getVertexArray());
	if (!positionSrc)
	{
		OSG_WARN << "RigTransformHardware no vertex array in the geometry " << geom.getName() << std::endl;
		return false;
	}

	osg::ref_ptr<osg::Program> cSkinningProg = creators::createProgram("skininst").program; 
	cSkinningProg->setName("SkinningShader");

	int attribIndex = 11;
	int nbAttribs = getNumVertexAttrib();
	for (int i = 0; i < nbAttribs; i++)
	{
		std::stringstream ss;
		ss << "boneWeight" << i;
		cSkinningProg->addBindAttribLocation(ss.str(), attribIndex + i);
		geom.setVertexAttribArray(attribIndex + i, getVertexAttrib(i));
		OSG_INFO << "set vertex attrib " << ss.str() << std::endl;
	}

	osg::ref_ptr<osg::StateSet> ss = geom.getOrCreateStateSet();
	ss->addUniform(getMatrixPaletteUniform());
	ss->addUniform(new osg::Uniform("nbBonesPerVertex", getNumBonesPerVertex()));
	ss->setAttributeAndModes(cSkinningProg.get());

	return true;
}

}


namespace avCore
{
    InstancedAnimationManager::InstancedAnimationManager() 
    {

    }

    InstancedAnimationManager::InstancedAnimationManager(const InstancedAnimationManager& object,const osg::CopyOp& copyop)
        : osg::Object(object,copyop)
    {
    
    }

	InstancedAnimationManager::InstancedAnimationManager(osg::Node* base_model, const std::string anim_file_name)
		: bm_(_getBoneMap(base_model))
		, src_model_    (base_model)
	{ 

        _initData();
        _loadAnimationData(anim_file_name);

        instGeode_  = _createGeode();
    }


	osg::TextureRectangle* InstancedAnimationManager::createAnimationTexture( image_data& idata)
	{
		size_t something_num = idata.data_len ;

		osg::ref_ptr<osg::Image> image = new osg::Image;
		image->setImage(idata.s, idata.t, idata.r, idata.internalFormat, idata.pixelFormat, idata.type, &idata.data[0], osg::Image::NO_DELETE);

		osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle(image);
		texture->setInternalFormat(GL_RGBA32F_ARB);
		texture->setSourceFormat(GL_RGBA);
		texture->setSourceType(GL_FLOAT);
		texture->setTextureSize(4, something_num);
		texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
		texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);

		idata_.data.resize(texture->getImage()->getTotalSizeInBytes());
		unsigned char* dest_ptr = &idata_.data[0];
		for(osg::Image::DataIterator itr(texture->getImage()); itr.valid(); ++itr)
		{
			memcpy(dest_ptr, itr.data(), itr.size());
			dest_ptr += itr.size();
		}

		return texture.release();
	}



	osg::TextureRectangle* InstancedAnimationManager::_createTextureHardwareInstancedGeode(osg::Geometry* geometry) 
	{
		const unsigned int start = 0;
		const unsigned int end = instancesData_.size();

		// first turn on hardware instancing for every primitive set
		for (unsigned int i = 0; i < geometry->getNumPrimitiveSets(); ++i)
		{
			geometry->getPrimitiveSet(i)->setNumInstances(instancesData_.size());
		}

		// we need to turn off display lists for instancing to work
		geometry->setUseDisplayList(false);
		geometry->setUseVertexBufferObjects(true);

		// create texture to encode all matrices
		unsigned int height = ((/*end-start*/instancesData_.size()) / 4096u) + 1u;

		osg::ref_ptr<osg::Image>       image = new osg::Image; 
		image->allocateImage(16384, height, 1, GL_RGBA, GL_FLOAT);
		image->setInternalTextureFormat(GL_RGBA32F_ARB);


		for (unsigned int i = /*start*/0, j = 0; i < /*end*/instancesData_.size(); ++i, ++j)
		{
			const osg::Matrixf& matrix = instancesData_[i];
			float * data = (float*)image->data((j % 4096u) *4u, j / 4096u);
			memcpy(data, matrix.ptr(), 16 * sizeof(float));
		}

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
		geometry->setComputeBoundingBoxCallback(new ComputeTextureBoundingBoxCallback(instancesData_));

		return instTexture_.get();
	}
	
	osgAnimation::BoneMap InstancedAnimationManager::_getBoneMap(osg::Node* base_model)
	{
		findNodeByType< osgAnimation::Skeleton> s_finder;  
		s_finder.apply(*base_model);

		auto* skel  = dynamic_cast<osgAnimation::Skeleton*>(s_finder.getLast());
		osgAnimation::BoneMapVisitor mapVisitor;
		skel->accept(mapVisitor);
		return mapVisitor.getBoneMap();
	}

	void InstancedAnimationManager::setInstanceData(size_t idx, const osg::Matrixf& matrix)
	{
		instancesData_[idx] = matrix;
		float * data = (float*)instTexture_->getImage(0)->data((idx % 4096u) *4u, idx / 4096u);
		memcpy(data, matrix.ptr(), 16 * sizeof(float));
		instTexture_->dirtyTextureObject();
	}


	osg::Geode* InstancedAnimationManager::_createGeode()
	{
		findNodeByType< osg::Geode> geode_finder;  
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

		/*instTexture_ =*/  _createTextureHardwareInstancedGeode(geometry);

		geode->addDrawable(geometry);
        instGeometry_  = geometry;
#if 0
		pSS->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
		pSS->addUniform(new osg::Uniform("colorTex", 0));
		//pSS->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.8f), osg::StateAttribute::ON);
#endif

		pSS->setTextureAttributeAndModes(6, animTexture_.get(), osg::StateAttribute::ON);
		pSS->addUniform(new osg::Uniform("animationTex", 6));

		pSS->setTextureAttributeAndModes(1, instTexture_.get(), osg::StateAttribute::ON);
		pSS->addUniform(new osg::Uniform("instanceMatrixTexture", 1));

		return geode.release();
	}

    void  InstancedAnimationManager::_initData()
    {
        const size_t x_num = 64; 
        const size_t y_num = 64;

        // create some matrices
        srand(time(NULL));
        for (unsigned int i = 0; i < x_num; ++i)
        {
            for (unsigned int j = 0; j < y_num; ++j)
            {
                // get random angle and random scale
                double angle = (rand() % 360) / 180.0 * cg::pi;
                double scale = static_cast<double>(rand() % 10) / 10.0 ;

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


	bool  InstancedAnimationManager::_loadAnimationData(std::string const&  filename)
	{
		image_data rd;

		std::ifstream image_data_file(filename, std::ios_base::binary);

		if (image_data_file.good())
		{
			binary::bytes_t data;
			data.resize(boost::filesystem::file_size(filename));
			image_data_file.read(data.data(), data.size());
			binary::unwrap(data, rd);
			animTexture_ = createAnimationTexture(rd);
			return true;
		}

		return false;
	}


 }