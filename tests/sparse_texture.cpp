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

        void glTexturePageCommitmentARB(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit) const;
        void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

        static SPTGLExtensions* getExtensions( unsigned int contextID,bool createIfNotInitalized );
    protected:

        typedef void ( GL_APIENTRY *TexturePageCommitmentARB ) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);
        typedef void ( GL_APIENTRY *GetInternalformativ)     (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);
        TexturePageCommitmentARB _glTexturePageCommitmentARB;
        GetInternalformativ      _glGetInternalformativ;
    };

    SPTGLExtensions::SPTGLExtensions( unsigned int contextID )
    {
        setupGLExtensions( contextID );
    }

    SPTGLExtensions::SPTGLExtensions( const SPTGLExtensions &rhs )
        : Referenced()
    {
        _glTexturePageCommitmentARB           = rhs._glTexturePageCommitmentARB;
        _glGetInternalformativ                = rhs._glGetInternalformativ;
    }


    void SPTGLExtensions::lowestCommonDenominator( const SPTGLExtensions &rhs )
    {
        if ( !rhs._glTexturePageCommitmentARB )
        {
            _glTexturePageCommitmentARB = rhs._glTexturePageCommitmentARB;
        }

        if ( !rhs._glGetInternalformativ )
        {
            _glGetInternalformativ = rhs._glGetInternalformativ;
        }
    }

    void SPTGLExtensions::setupGLExtensions( unsigned int contextID )
    {
        _glTexturePageCommitmentARB = 0;
        osg::setGLExtensionFuncPtr( _glTexturePageCommitmentARB, "glTexturePageCommitmentARB", "glTexturePageCommitmentEXT" );

        _glGetInternalformativ = 0;
        osg::setGLExtensionFuncPtr( _glGetInternalformativ, "glGetInternalformativ" );
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

    void SPTGLExtensions::glTexturePageCommitmentARB(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit) const
    {

        if ( _glTexturePageCommitmentARB )
        {
            _glTexturePageCommitmentARB( texture, level, xoffset, yoffset, zoffset, width, height, depth, commit );
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
    
    class SparseTexture  : public osg::Texture2DArray
    {



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

#if 0
            const Texture2DArray::Extensions* extensions = Texture2DArray::getExtensions(contextID,true);
#endif
			    
			const osg::GLExtensions* extensions = osg::GLExtensions::Get(contextID,true);



            //osg::Texture2DArray::apply(state);

			osg::Texture::TextureObjectManager* tom = Texture::getTextureObjectManager(contextID).get();
			osg::ElapsedTime elapsedTime(&(tom->getApplyTime()));
			tom->getNumberApplied()++;


			// if not supported, then return
			if (!extensions->isTexture2DArraySupported || !extensions->isTexture3DSupported)
			{
				OSG_WARN<<"Warning: Texture2DArray::apply(..) failed, 2D texture arrays are not support by OpenGL driver."<<std::endl;
				return;
			}

			// get the texture object for the current contextID.
			TextureObject* textureObject = getTextureObject(contextID);

			GLsizei textureDepth = computeTextureDepth();

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
					for(Images::const_iterator itr = _images.begin();
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
				}

			}

			// there is no texture object, but exists a subload callback, so use it to upload images
			else if (_subloadCallback.valid())
			{
				// generate texture (i.e. glGenTexture) and apply parameters
				textureObject = generateAndAssignTextureObject(contextID, GL_TEXTURE_2D_ARRAY_EXT);
				textureObject->bind();
				applyTexParameters(GL_TEXTURE_2D_ARRAY_EXT, state);
				_subloadCallback->load(*this,state);
			}

			// nothing before, but we have valid images, so do manual upload and create texture object manually
			// TODO: we assume _images[0] is valid, however this may not be always the case
			//       some kind of checking for the first valid image is required (Art, may 2008)
			else if (imagesValid())
			{
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
				for(Images::const_iterator itr = _images.begin();
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

				const osg::GLExtensions* extensions = state.get<osg::GLExtensions>();
				// source images have no mipmamps but we could generate them...
				if( _min_filter != LINEAR && _min_filter != NEAREST && !_images[0]->isMipmap() &&
					_useHardwareMipMapGeneration && extensions->isGenerateMipMapSupported )
				{
					_numMipmapLevels = osg::Image::computeNumberOfMipmapLevels( _textureWidth, _textureHeight );
					generateMipmap( state );
				}

				textureObject->setAllocated(_numMipmapLevels, _internalFormat, _textureWidth, _textureHeight, textureDepth,0);

				// unref image data?
				if (isSafeToUnrefImageData(state))
				{
					SparseTexture* non_const_this = const_cast<SparseTexture*>(this);
					for(Images::iterator itr = non_const_this->_images.begin();
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

			}

			// No images present, but dimensions are set. So create empty texture
			else if ( (_textureWidth > 0) && (_textureHeight > 0) && (_textureDepth > 0) && (_internalFormat!=0) )
			{
				auto spte = getExtensions(contextID,true);

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				
				// generate texture
				textureObject = generateAndAssignTextureObject(
					contextID, GL_TEXTURE_2D_ARRAY_EXT,_numMipmapLevels,_internalFormat, _textureWidth, _textureHeight, _textureDepth,0);

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

				GLsizei  width =  _textureWidth;
				GLsizei  height =  _textureHeight;
				for (int i = 0; i < MaxLevels; i++) {
					extensions->glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, i, _internalFormat, width, height, /*depth*/_textureDepth, 0, _sourceFormat, _sourceType, NULL);
					width = std::max(1, (width / 2));
					height = std::max(1, (height / 2));
				}

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
    
    private:
        void _init( /*const Texture2DArray::Extensions**/const osg::GLExtensions* t2da_extensions, Extensions* extensions) const
        {
            if(!_binit)
            {

                
                _binit = true;
            }
        }

    private:
        mutable bool _binit;
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

    ss->setTextureAttribute(0, new SparseTexture());
    ss->addUniform( new osg::Uniform("Diffuse", 0) );
    ss->setAttributeAndModes(program);	


    osgViewer::Viewer viewer(arguments);

    viewer.getCamera()->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    //viewer.getCamera()->setProjectionMatrix(osg::Matrixd::identity());
    //viewer.getCamera()->setViewMatrix(osg::Matrixd::scale(1,1,-1)); 


    viewer.getCamera()->setCullingMode( osg::CullSettings::NO_CULLING );
	viewer.apply(new osgViewer::SingleScreen(1));

    viewer.setSceneData( root.get() );
    return viewer.run();
}

 AUTO_REG(main_sparse_texture)