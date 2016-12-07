#include "stdafx.h"

#include <osg/buffered_value>
#include <osg/ref_ptr>
#include <osg/GLExtensions>
#include <GL/glut.h>
#include <osg/Version>
#include <osg/Texture2DArray>

//#include <GL/gl3w.h>
#include "glm/glm.hpp"

namespace gli
{
	//template <typename T, precision P, template <typename, precision> class vecType>
	//inline T levels(vecType<T, P> const& Extent)
	//{
	//	return glm::log2(compMax(Extent)) + static_cast<T>(1);
	//}

	template <typename T>
	inline T levels(T Extent)
	{
		return static_cast<T>(glm::log2(Extent) + static_cast<size_t>(1));
	}
}//namespace gli


#ifndef GL_ARB_sparse_texture
#define GL_ARB_sparse_texture 1
#define GL_TEXTURE_SPARSE_ARB             0x91A6
#define GL_VIRTUAL_PAGE_SIZE_INDEX_ARB    0x91A7
#define GL_NUM_SPARSE_LEVELS_ARB          0x91AA
#define GL_NUM_VIRTUAL_PAGE_SIZES_ARB     0x91A8
#define GL_VIRTUAL_PAGE_SIZE_X_ARB        0x9195
#define GL_VIRTUAL_PAGE_SIZE_Y_ARB        0x9196
#define GL_VIRTUAL_PAGE_SIZE_Z_ARB        0x9197
#define GL_MAX_SPARSE_TEXTURE_SIZE_ARB    0x9198
#define GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB 0x9199
#define GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB 0x919A
#define GL_SPARSE_TEXTURE_FULL_ARRAY_CUBE_MIPMAPS_ARB 0x91A9
#if 0
typedef void (APIENTRYP PFNGLTEXPAGECOMMITMENTARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void APIENTRY glTexPageCommitmentARB (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);
#endif
#endif
#endif /* GL_ARB_sparse_texture */

#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_TEXTURE_2D_ARRAY	 GL_TEXTURE_2D_ARRAY_EXT

namespace  {
    
    class SPTGLExtensions;


    class SPTGLExtensions : public osg::Referenced
    {
    public:
        SPTGLExtensions( unsigned int contextID );
        SPTGLExtensions( const SPTGLExtensions &rhs );
        void lowestCommonDenominator( const SPTGLExtensions &rhs );
        void setupGLExtensions( unsigned int contextID );

        void glTexturePageCommitmentARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit) const;
        void glGetInternalformativ     (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);
        void glTexStorage3D            (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

        static SPTGLExtensions* getExtensions( unsigned int contextID,bool createIfNotInitalized );
    protected:

        typedef void ( GL_APIENTRY *TexturePageCommitmentARB ) ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);
        typedef void ( GL_APIENTRY *GetInternalformativ)       ( GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);
        typedef void ( GL_APIENTRY * TexStorage3D)             ( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
        TexturePageCommitmentARB _glTexPageCommitmentARB;
        GetInternalformativ      _glGetInternalformativ;
        TexStorage3D             _glTexStorage3D;
    };

    SPTGLExtensions::SPTGLExtensions( unsigned int contextID )
    {
        setupGLExtensions( contextID );
    }

    SPTGLExtensions::SPTGLExtensions( const SPTGLExtensions &rhs )
        : Referenced()
    {
        _glTexPageCommitmentARB           = rhs._glTexPageCommitmentARB;
        _glGetInternalformativ                = rhs._glGetInternalformativ;
        _glTexStorage3D                = rhs._glTexStorage3D;
    }


    void SPTGLExtensions::lowestCommonDenominator( const SPTGLExtensions &rhs )
    {
        if ( !rhs._glTexPageCommitmentARB )
        {
            _glTexPageCommitmentARB = rhs._glTexPageCommitmentARB;
        }

        if ( !rhs._glGetInternalformativ )
        {
            _glGetInternalformativ = rhs._glGetInternalformativ;
        }

        if ( !rhs._glTexStorage3D )
        {
            _glTexStorage3D = rhs._glTexStorage3D;
        }
    }

    void SPTGLExtensions::setupGLExtensions( unsigned int contextID )
    {
        _glTexPageCommitmentARB = 0;
        osg::setGLExtensionFuncPtr( _glTexPageCommitmentARB, "glTexPageCommitmentARB"/*, "glTexturePageCommitmentEXT"*/ );

        _glGetInternalformativ = 0;
        osg::setGLExtensionFuncPtr( _glGetInternalformativ, "glGetInternalformativ" );

        _glTexStorage3D = 0;
        osg::setGLExtensionFuncPtr( _glTexStorage3D, "glTexStorage3D" );
    }

    typedef osg::buffered_value< osg::ref_ptr<SPTGLExtensions> > BufferedSPTGLExtensions;
    static BufferedSPTGLExtensions bsptExtensions;

    SPTGLExtensions* SPTGLExtensions::getExtensions( unsigned int contextID,bool createIfNotInitalized )
    {
        if ( !bsptExtensions[contextID] && createIfNotInitalized )
        {
            bsptExtensions[contextID] = new SPTGLExtensions( contextID );
        }
        return bsptExtensions[contextID].get();
    }

    void SPTGLExtensions::glTexturePageCommitmentARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit) const
    {

        if ( _glTexPageCommitmentARB )
        {
            _glTexPageCommitmentARB( target, level, xoffset, yoffset, zoffset, width, height, depth, commit );
        }
        else
        {
            OSG_WARN<<"Error: glTexturePageCommitmentARB not supported by OpenGL driver"<<std::endl;
        }
    }

    void SPTGLExtensions::glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
    {

        if ( _glGetInternalformativ )
        {
            _glGetInternalformativ(target, internalformat, pname, bufSize, params);
        }
        else
        {
            OSG_WARN<<"Error: glGetInternalformativ not supported by OpenGL driver"<<std::endl;
        }
    }
    
    
    void SPTGLExtensions::glTexStorage3D( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
    {

        if ( _glTexStorage3D )
        {
            _glTexStorage3D( target, levels, internalformat, width, height, depth);
        }
        else
        {
            OSG_WARN<<"Error: glGetInternalformativ not supported by OpenGL driver"<<std::endl;
        }
    }

    class SparseTexture  : public osg::Texture2DArray
    {
        struct page_t
        {
            page_t()
                : level  (0)
                , xoffset(0)
                , yoffset(0)
                , zoffset(0)
                , width  (0)
                , height (0)
                , depth  (0)
            {}

            page_t( GLint   level, GLint   xoffset,  GLint   yoffset,  GLint   zoffset,  GLsizei width, GLsizei height,  GLsizei depth)
                : level  (level)
                , xoffset(xoffset)
                , yoffset(yoffset)
                , zoffset(zoffset)
                , width  (width)
                , height (height)
                , depth  (depth)
            {}

            GLint   level;
            GLint   xoffset;
            GLint   yoffset;
            GLint   zoffset;
            GLsizei width;
            GLsizei height;
            GLsizei depth;
        };

        typedef std::vector<page_t> pages_t; 

    public:

        typedef  SPTGLExtensions Extensions;

    public:

        SparseTexture()
            : _binit(false)
        {
            setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
            setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR_MIPMAP_NEAREST);
			 GLsizei const Size(16384);
			
			_textureWidth   =  GLsizei(Size);
	        _textureHeight  =  GLsizei(Size);
			_textureDepth   =  1;
			_internalFormat =  GL_RGBA8;
			_sourceFormat   =  GL_RGBA;
			_sourceType     =  GL_UNSIGNED_BYTE;
        }


         /** Bind the texture if already compiled. Otherwise recompile.*/
        virtual void apply(osg::State& state) const
        {
            // get the contextID (user defined ID of 0 upwards) for the
            // current OpenGL context.
            const unsigned int contextID = state.getContextID();

#if (((OSG_VERSION_MAJOR>=3) && (OSG_VERSION_MINOR>3)) )
			const osg::GLExtensions* extensions = osg::GLExtensions::Get(contextID,true);
#else
            const Texture2DArray::Extensions* extensions = Texture2DArray::getExtensions(contextID,true);
#endif
            auto spte = getExtensions(contextID,true);
            
            //osg::Texture2DArray::apply(state);

			osg::Texture::TextureObjectManager* tom = Texture::getTextureObjectManager(contextID).get();
			osg::ElapsedTime elapsedTime(&(tom->getApplyTime()));
			tom->getNumberApplied()++;


			// if not supported, then return
#if (((OSG_VERSION_MAJOR>=3) && (OSG_VERSION_MINOR>3)) )
			if (!extensions->isTexture2DArraySupported || !extensions->isTexture3DSupported)
#else
			if (!extensions->isTexture2DArraySupported() || !extensions->isTexture3DSupported())
#endif
			{
				OSG_WARN<<"Warning: Texture2DArray::apply(..) failed, 2D texture arrays are not support by OpenGL driver."<<std::endl;
				return;
			}

			// get the texture object for the current contextID.
			TextureObject* textureObject = getTextureObject(contextID);

#if (((OSG_VERSION_MAJOR>=3) && (OSG_VERSION_MINOR>3)) )
			const GLsizei textureDepth = computeTextureDepth();
#else
            const GLsizei textureDepth = _textureDepth;
#endif

			if (textureObject && textureDepth>0)
			{
				const osg::Image* image = (_images.size()>0) ? _images[0].get() : 0;
				if (image && getModifiedCount(0, contextID) != image->getModifiedCount())
				{
					// compute the internal texture format, this set the _internalFormat to an appropriate value.
					computeInternalFormat();

					GLsizei new_width, new_height, new_numMipmapLevels;

					// compute the dimensions of the texture.
					computeRequiredTextureDimensions(state, *image, new_width, new_height, new_numMipmapLevels);

					if (!textureObject->match(GL_TEXTURE_2D_ARRAY_EXT, new_numMipmapLevels, _internalFormat, new_width, new_height, textureDepth, _borderWidth))
					{
						Texture::releaseTextureObject(contextID, _textureObjectBuffer[contextID].get());
						_textureObjectBuffer[contextID] = 0;
						textureObject = 0;
					}
				}
			}

			// if we already have an texture object, then
			if (textureObject)
			{
				// bind texture object
				textureObject->bind();

				// if texture parameters changed, then reset them
				if (getTextureParameterDirty(state.getContextID())) applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT,state);

				// if subload is specified, then use it to subload the images to GPU memory
				if (_subloadCallback.valid())
				{
					_subloadCallback->subload(*this,state);
				}
				else
				{
					GLsizei n = 0;
					for(auto itr = _images.begin();
						itr != _images.end();
						++itr)
					{
						osg::Image* image = itr->get();
						if (image)
						{
							if (getModifiedCount(n,contextID) != image->getModifiedCount())
							{
#if (((OSG_VERSION_MAJOR>=3) && (OSG_VERSION_MINOR>3)) )
								applyTexImage2DArray_subload(state, image, n, _textureWidth, _textureHeight, image->r(), _internalFormat, _numMipmapLevels);
#else
                                applyTexImage2DArray_subload(state, image, _textureWidth, _textureHeight, n, _internalFormat, _numMipmapLevels);
#endif
								getModifiedCount(n,contextID) = image->getModifiedCount();
							}
							n += image->r();
						}
					}
				}

                if(_to_uncommit_pages.size()>0)
                {
#if 1
                    const page_t& p = _to_uncommit_pages.back();
                    
                    spte->glTexturePageCommitmentARB(GL_TEXTURE_2D_ARRAY, p.level,
                        p.xoffset, p.yoffset, p.zoffset,
                        p.width, p.height, p.depth,
                        GL_FALSE);
#endif
                     _to_uncommit_pages.pop_back();
                }
			}

			// there is no texture object, but exists a subload callback, so use it to upload images
			else if (_subloadCallback.valid())
			{
#if 0
				// generate texture (i.e. glGenTexture) and apply parameters
				textureObject = generateAndAssignTextureObject(contextID, GL_TEXTURE_2D_ARRAY_EXT);
				textureObject->bind();
				applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT, state);
				_subloadCallback->load(*this,state);
#endif
			}

			// nothing before, but we have valid images, so do manual upload and create texture object manually
			// TODO: we assume _images[0] is valid, however this may not be always the case
			//       some kind of checking for the first valid image is required (Art, may 2008)
			else if (!_images.empty() && imagesValid())
			{
#if 0
				// compute the internal texture format, this set the _internalFormat to an appropriate value.
				computeInternalFormat();

				// compute the dimensions of the texture.
				computeRequiredTextureDimensions(state,*_images[0],_textureWidth, _textureHeight, _numMipmapLevels);

				// create texture object
				textureObject = generateAndAssignTextureObject(
					contextID, GL_TEXTURE_2D_ARRAY_EXT,_numMipmapLevels, _internalFormat, _textureWidth, _textureHeight, textureDepth,0);

				// bind texture
				textureObject->bind();
				applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT, state);

				// First we need to allocate the texture memory
				int sourceFormat = _sourceFormat ? _sourceFormat : _internalFormat;

				if( isCompressedInternalFormat( sourceFormat ) &&
					sourceFormat == _internalFormat &&
					extensions->isCompressedTexImage3DSupported() )
				{
					extensions->glCompressedTexImage3D( GL_TEXTURE_2D_ARRAY_EXT, 0, _internalFormat,
						_textureWidth, _textureHeight, textureDepth, _borderWidth,
						_images[0]->getImageSizeInBytes() * textureDepth,
						0);
				}
				else
				{
					// Override compressed source format with safe GL_RGBA value which not generate error
					// We can safely do this as source format is not important when source data is NULL
					if( isCompressedInternalFormat( sourceFormat ) )
						sourceFormat = GL_RGBA;

					extensions->glTexImage3D( GL_TEXTURE_2D_ARRAY_EXT, 0, _internalFormat,
						_textureWidth, _textureHeight, textureDepth, _borderWidth,
						sourceFormat, _sourceType ? _sourceType : GL_UNSIGNED_BYTE,
						0);
				}

				// For certain we have to manually allocate memory for mipmaps if images are compressed
				// if not allocated OpenGL will produce errors on mipmap upload.
				// I have not tested if this is necessary for plain texture formats but
				// common sense suggests its required as well.
				if( _min_filter != LINEAR && _min_filter != NEAREST && _images[0]->isMipmap() )
				{
					allocateMipmap( state );
				}

				GLsizei n = 0;
				for(auto itr = _images.begin();
					itr != _images.end();
					++itr)
				{
					osg::Image* image = itr->get();
					if (image)
					{
						if (getModifiedCount(n,contextID) != image->getModifiedCount())
						{
							applyTexImage2DArray_subload(state, image, n, _textureWidth, _textureHeight, image->r(), _internalFormat, _numMipmapLevels);
							getModifiedCount(n,contextID) = image->getModifiedCount();
						}
						n += image->r();
					}
				}

                bool isGenerateMipMapSupported; 
#if (((OSG_VERSION_MAJOR>=3) && (OSG_VERSION_MINOR>3)) )
				const osg::GLExtensions* extensions = state.get<osg::GLExtensions>();
                isGenerateMipMapSupported = extensions->isGenerateMipMapSupported;
#else
                const Texture::Extensions* extensions = Texture::getExtensions(contextID,true);
                isGenerateMipMapSupported = extensions->isGenerateMipMapSupported();
#endif
				// source images have no mipmamps but we could generate them...
				if( _min_filter != LINEAR && _min_filter != NEAREST && !_images[0]->isMipmap() &&
					_useHardwareMipMapGeneration && isGenerateMipMapSupported )
				{
					_numMipmapLevels = osg::Image::computeNumberOfMipmapLevels( _textureWidth, _textureHeight );
					generateMipmap( state );
				}

				textureObject->setAllocated(_numMipmapLevels, _internalFormat, _textureWidth, _textureHeight, textureDepth,0);

				// unref image data?
				if (isSafeToUnrefImageData(state))
				{
					SparseTexture* non_const_this = const_cast<SparseTexture*>(this);
					for(auto itr = non_const_this->_images.begin();
						itr != non_const_this->_images.end();
						++itr)
					{
						osg::Image* image = itr->get();
						if (image && image->getDataVariance()==STATIC)
						{
							*itr = NULL;
						}
					}
				}
#endif
			}

			// No images present, but dimensions are set. So create empty texture
			else if ( (_textureWidth > 0) && (_textureHeight > 0) && (_textureDepth > 0) && (_internalFormat!=0) )
			{


				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#if (((OSG_VERSION_MAJOR>=3) && (OSG_VERSION_MINOR>3)) )				
				// generate texture
				textureObject = generateAndAssignTextureObject(
					contextID, GL_TEXTURE_2D_ARRAY_EXT,_numMipmapLevels,_internalFormat, _textureWidth, _textureHeight, _textureDepth,0);
#else
                // generate texture
                _textureObjectBuffer[contextID] = textureObject = generateTextureObject(
                    this, contextID, GL_TEXTURE_2D_ARRAY_EXT,_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,_textureDepth,0);
#endif

				textureObject->bind();
				applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT,state);

				GLsizei const Size(16384);
				std::size_t const Levels = gli::levels(static_cast<double>(Size));
				std::size_t const MaxLevels = 4;

				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, MaxLevels - 1);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SPARSE_ARB, GL_TRUE);

				osg::Vec3i PageSize;
				spte->glGetInternalformativ(GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &PageSize._v[0]);
				spte->glGetInternalformativ(GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &PageSize._v[1]);
				spte->glGetInternalformativ(GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_Z_ARB, 1, &PageSize._v[2]);

				std::vector<glm::u8vec4> Page;
				Page.resize(static_cast<std::size_t>(PageSize.x() * PageSize.y() * PageSize.z()));

				osg::Vec3i Page3DSize;
				spte->glGetInternalformativ(GL_TEXTURE_3D, GL_RGBA32F, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &Page3DSize._v[0]);
				spte->glGetInternalformativ(GL_TEXTURE_3D, GL_RGBA32F, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &Page3DSize._v[1]);
				spte->glGetInternalformativ(GL_TEXTURE_3D, GL_RGBA32F, GL_VIRTUAL_PAGE_SIZE_Z_ARB, 1, &Page3DSize._v[2]); 

#if 0
				extensions->glTexImage3D( GL_TEXTURE_2D_ARRAY_EXT, 0, _internalFormat,
					_textureWidth, _textureHeight, _textureDepth,
					_borderWidth,
					_sourceFormat ? _sourceFormat : _internalFormat,
					_sourceType ? _sourceType : GL_UNSIGNED_BYTE,
					0);
#endif

#if 0
				GLsizei  width =  _textureWidth;
				GLsizei  height =  _textureHeight;
				for (int i = 0; i < Levels; i++) {
					extensions->glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, i, _internalFormat, width, height, /*depth*/_textureDepth, 0, _sourceFormat, _sourceType, NULL);
					width = std::max(1, (width / 2));
					height = std::max(1, (height / 2));
				}
#endif
                spte->glTexStorage3D(GL_TEXTURE_2D_ARRAY, Levels, GL_RGBA8, GLsizei(Size), GLsizei(Size), 1);

				for(std::size_t Level = 0; Level < MaxLevels; ++Level)
				{
					GLsizei LevelSize = (Size >> Level);
					GLsizei TileCountY = LevelSize / PageSize.y();
					GLsizei TileCountX = LevelSize / PageSize.x();

					for(GLsizei j = 0; j < TileCountY; ++j)
						for(GLsizei i = 0; i < TileCountX; ++i)
						{
							if(glm::abs(glm::length(glm::vec2(i, j) / glm::vec2(TileCountX, TileCountY) * 2.0f - 1.0f)) > 1.0f)
								continue;

							std::fill(Page.begin(), Page.end(), glm::u8vec4(
								static_cast<unsigned char>(float(i) / float(LevelSize / PageSize.x()) * 255),
								static_cast<unsigned char>(float(j) / float(LevelSize / PageSize.y()) * 255),
								static_cast<unsigned char>(float(Level) / float(MaxLevels) * 255), 255));

							spte->glTexturePageCommitmentARB(GL_TEXTURE_2D_ARRAY, static_cast<GLint>(Level),
								static_cast<GLsizei>(PageSize.x()) * i, static_cast<GLsizei>(PageSize.y()) * j, 0,
								static_cast<GLsizei>(PageSize.x()), static_cast<GLsizei>(PageSize.y()), 1,
								GL_TRUE);

							extensions->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, static_cast<GLint>(Level),
								static_cast<GLsizei>(PageSize.x()) * i, static_cast<GLsizei>(PageSize.y()) * j, 0,
								static_cast<GLsizei>(PageSize.x()), static_cast<GLsizei>(PageSize.y()), 1,
								GL_RGBA, GL_UNSIGNED_BYTE,
								&Page[0][0]);


                            _pages.push_back(page_t(static_cast<GLint>(Level),
                                static_cast<GLsizei>(PageSize.x() * i) , static_cast<GLsizei>(PageSize.y() * j) , 0,
                                static_cast<GLsizei>(PageSize.x()), static_cast<GLsizei>(PageSize.y()), 1) );


						}
				}


				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			}

			// nothing before, so just unbind the texture target
			else
			{
				glBindTexture( GL_TEXTURE_2D_ARRAY_EXT, 0 );
			}

			// if texture object is now valid and we have to allocate mipmap levels, then
			if (textureObject != 0 && _texMipmapGenerationDirtyList[contextID])
			{
				generateMipmap(state);
			}


        }

        //static void setExtensions(unsigned int contextID,Extensions* extensions);
        static Extensions* getExtensions(unsigned int contextID,bool createIfNotInitalized);

        osg::Image* getImage(unsigned int layer) override
        {
            return (layer<static_cast<unsigned int>(_images.size())) ? _images[layer].get() : 0;
        }

        const osg::Image* getImage(unsigned int layer) const  override
        {
            return (layer<static_cast<unsigned int>(_images.size())) ? _images[layer].get() : 0;
        }

        void uncommitOne()
        {
            if(_pages.size()>0)  
            {
                _to_uncommit_pages.push_back(_pages.back());
               _pages.pop_back();
            }
        }

    private:
        void _init( 
#if (((OSG_VERSION_MAJOR>=3) && (OSG_VERSION_MINOR>3)) )
            const osg::GLExtensions* t2da_extensions
#else
            const Texture2DArray::Extensions* t2da_extensions
#endif
            , Extensions* extensions) const
        {
            if(!_binit)
            {

                
                _binit = true;
            }
        }

    private:
        mutable bool _binit;
        mutable pages_t      _pages;
        mutable pages_t      _to_uncommit_pages;
 
   };

    SparseTexture::Extensions* SparseTexture::getExtensions(unsigned int contextID,bool createIfNotInitalized)
    {
        return Extensions::getExtensions(contextID, createIfNotInitalized);
    }

    //void SparseTexture::setExtensions(unsigned int contextID,Extensions* extensions)
    //{
    //    Extensions::set = extensions;
    //}


    class TeapotDrawable : public osg::Drawable
    {
    public:
        TeapotDrawable( float size=1.0f ) : _size(size) {}
        TeapotDrawable( const TeapotDrawable& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY )
            : osg::Drawable(copy, copyop), _size(copy._size) {}

        META_Object( osg, TeapotDrawable );

#if OSG_MIN_VERSION_REQUIRED(3,3,2)
		virtual osg::BoundingBox computeBoundingBox() const
#else
		virtual osg::BoundingBox computeBound() const
#endif
        {
            osg::Vec3 _min(-_size,-_size,-_size), _max(_size, _size, _size);
            return osg::BoundingBox(_min, _max);
        }

        virtual void drawImplementation( osg::RenderInfo& renderInfo ) const
        {      
            
            glFrontFace( GL_CW );
            glutSolidTeapot( _size );
            glFrontFace( GL_CCW );
        }

    protected:
        float _size;
    };


        char vertexShaderSource[] = 
            "#version 430 compatibility \n"
            "layout (location = 0) in vec4 pos; \n"
            "layout (location = 8) in vec2 uv; \n"
            "out block \n"
            "{ \n"
            " vec2 Texcoord; \n"
            "} Out;  \n"
            " \n"
            "void main(void) \n"
            "{ \n"
            "    Out.Texcoord = uv;\n"
            "    gl_Position = gl_ModelViewProjectionMatrix  * gl_Vertex ;\n"
            "}\n";

        // * instanceModelMatrix

        char fragmentShaderSource[] =  
            "#version 430 compatibility \n"
            "#define DIFFUSE			0 \n"
            "#define FRAG_COLOR		0 \n"
            "precision highp float; \n"
            "precision highp int; \n"
            
            
            "layout(std140, column_major) uniform; \n"
            "layout(binding = DIFFUSE) uniform sampler2DArray Diffuse; \n"
            "in block \n"
            "{ \n"
            "vec2 Texcoord;   \n"
            "} In;\n"
            "layout(location = FRAG_COLOR, index = 0) out vec4 Color;\n"
            "void main(void) \n"
            "{ \n"
            "\n"
			"    Color = texture(Diffuse, vec3(In.Texcoord.st, 0.0)); \n"
            "}\n";


        osg::Program* createProgram( const std::string& name, const std::string& vertexSource, const std::string& fragmentSource  )
        {
            osg::ref_ptr<osg::Program> program = new osg::Program;
            program->setName( name );

            osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader(osg::Shader::VERTEX, vertexSource);
            vertexShader->setName( name + "_vertex" );
            program->addShader(vertexShader.get());

            osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader(osg::Shader::FRAGMENT, fragmentSource);
            fragmentShader->setName( name + "_fragment" );
            program->addShader(fragmentShader.get());

            return program.release();
        }


osg::Geometry*    CreateGeometry()
{
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array;
    float depth = 0;
    vertices->push_back(osg::Vec3(-1.0,1.0,depth));
    vertices->push_back(osg::Vec3(-1.0,-1.0,depth));
    vertices->push_back(osg::Vec3(1.0,-1.0,depth));
    vertices->push_back(osg::Vec3(1.0,1.0,depth));
    geom->setVertexArray(vertices);

    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,0.0,0.2f,0.2f));
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);

    osg::Vec2Array* uv = new osg::Vec2Array;
    uv->push_back(osg::Vec2(0.0,0.0));
    uv->push_back(osg::Vec2(0.0,1.0));
    uv->push_back(osg::Vec2(1.0,1.0));
    uv->push_back(osg::Vec2(1.0,0.0));
    uv->setNormalize(true);
    geom->setTexCoordArray(0, uv, osg::Array::BIND_PER_VERTEX);


    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

    return geom;
}

class DeleteCallback: public osg::NodeCallback
{
public:
      DeleteCallback( SparseTexture* spt, double rate = 1.0 )
        : _rate(rate) 
        , _spt(spt)
        , _prev_time(0.0)
      {}

      virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)         
      {
          double   curr_time = nv->getFrameStamp()->getSimulationTime();
          if(curr_time -_prev_time > _rate)
          {
              _spt->uncommitOne();
              _prev_time = curr_time;
          }
      }

private:
    const double     _rate;
    double   _prev_time;
    SparseTexture* _spt;
};

}



int main_sparse_texture( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    osg::setNotifyLevel( osg::INFO ); 

	osg::ref_ptr<osg::Group> root= new osg::Group;
	osg::ref_ptr<osg::Geode> teapot = new osg::Geode;
    // teapot->addDrawable( new TeapotDrawable(1.0f) );
	teapot->addDrawable( CreateGeometry() );
	
    root->addChild(teapot);
    
    osg::StateSet* ss = teapot->getOrCreateStateSet();
    
    auto program = createProgram("sparse_texture",vertexShaderSource,fragmentShaderSource);

    auto spt =  new SparseTexture();
    ss->setTextureAttribute(0, spt );
    ss->addUniform( new osg::Uniform("Diffuse", 0) );
    ss->setAttributeAndModes(program);	
    teapot->setUpdateCallback(new DeleteCallback(spt, 1));

    osgViewer::Viewer viewer(arguments);




    viewer.getCamera()->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    //viewer.getCamera()->setProjectionMatrix(osg::Matrixd::identity());
    //viewer.getCamera()->setViewMatrix(osg::Matrixd::scale(1,1,-1)); 


    viewer.getCamera()->setCullingMode( osg::CullSettings::NO_CULLING );
	viewer.apply(new osgViewer::SingleScreen(1));
       viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.setSceneData( root.get() );

    osgViewer::Viewer::Windows windows;
    viewer.getWindows(windows);
    for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
        itr != windows.end();
        ++itr)
    {
        osg::State *s=(*itr)->getState();
        s->setCheckForGLErrors(osg::State::ONCE_PER_ATTRIBUTE);
    }

    return viewer.run();
}

 AUTO_REG(main_sparse_texture)