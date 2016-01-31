#include <stdafx.h>

#include "av/avScene/Scene.h"
#include "av/avScene/ScreenTexture.h"


//#define _SCREEN_TEXTURE_DEBUG_

namespace avScene
{

ScreenTexture::ScreenTexture( unsigned order, Type type )
    : _texture(new osg::TextureRectangle())
    , _resolveFbo(new osg::FrameBufferObject())
    , _isNeededValidation(false)
{
    setUseDisplayList(false);

    _texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    _texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    _texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    _texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    _texture->setUseHardwareMipMapGeneration(false);

    if (type == DEPTH_TEXTURE)
    {
        _texture->setInternalFormat(GL_DEPTH_COMPONENT);
        _settings = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "DepthTextureSettings");
    }
    else if (type == COLOR_TEXTURE)
        _texture->setInternalFormat(GL_RGBA);
    else
        avAssert(false);

    osg::ref_ptr<osg::StateSet> stateSet = getOrCreateStateSet();
    stateSet->setRenderBinDetails(order, "RenderBin");
    stateSet->setNestRenderBins(false);

    // create FBO for blit operations
    _blitBitMask = 0;
    if (type == DEPTH_TEXTURE)
        _blitBitMask = GL_DEPTH_BUFFER_BIT;
    else if (type == COLOR_TEXTURE)
        _blitBitMask = GL_COLOR_BUFFER_BIT;
    else
        avAssert(false);


#if 1
	_textureNormals= new osg::TextureRectangle();
	_textureNormals->setInternalFormat( GL_RGBA32F_ARB );
	_textureNormals->setSourceFormat(GL_RGBA);
	_textureNormals->setSourceType(GL_FLOAT);
	_textureNormals->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
	_textureNormals->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
    _textureNormals->setUseHardwareMipMapGeneration(false);

	osg::ref_ptr<osg::Vec3Array> quad_coords = new osg::Vec3Array; // vertex coords
	// counter-clockwise
	quad_coords->push_back(osg::Vec3d(0, 0, -1));
	quad_coords->push_back(osg::Vec3d(1, 0, -1));
	quad_coords->push_back(osg::Vec3d(1, 1, -1));
	quad_coords->push_back(osg::Vec3d(0, 1, -1));

#if 0
	osg::ref_ptr<osg::Vec2Array> quad_tcoords = new osg::Vec2Array; // texture coords
	quad_tcoords->push_back(osg::Vec2(0, 0));
	quad_tcoords->push_back(osg::Vec2(tex_width, 0));
	quad_tcoords->push_back(osg::Vec2(tex_width, tex_height));
	quad_tcoords->push_back(osg::Vec2(0, tex_height));
#endif

	osg::Geometry* quad_geom = _pthis = this;
	osg::ref_ptr<osg::DrawArrays> quad_da = new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4);

	osg::ref_ptr<osg::Vec4Array> quad_colors = new osg::Vec4Array;
	quad_colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

	quad_geom->setVertexArray(quad_coords.get());
	// quad_geom->setTexCoordArray(0, quad_tcoords.get());
	quad_geom->addPrimitiveSet(quad_da.get());
	quad_geom->setColorArray(quad_colors.get(), osg::Array::BIND_OVERALL);

	osg::StateSet *stateset = quad_geom->getOrCreateStateSet();
	stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);


	static const char *shaderSource = {
		"uniform int width;\n"
		"void main(void)\n"
		"{\n"
		"    gl_FragData[0] = vec4(1,0,0,1);\n"
		"    gl_FragData[1] = vec4(0,1,0,1);\n"
		"    gl_FragData[2] = vec4(0,0,1,1);\n"
		"    gl_FragData[3] = vec4(0,0,1,1);\n"
		"}\n"
	};

	osg::ref_ptr<osg::Shader> fshader = new osg::Shader( osg::Shader::FRAGMENT , shaderSource);
	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(fshader.get());
	stateset->setAttributeAndModes(program.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED );


#endif


}

ScreenTexture::~ScreenTexture()
{
}

osg::TextureRectangle * ScreenTexture::getTexture(Type type) const
{
   if(type == NORMAL_TEXTURE)
	return _textureNormals.get();

	return _texture.get();
}

osg::Uniform * ScreenTexture::getSettings() const
{
    return _settings.get();
}

void ScreenTexture::validate()
{
    _isNeededValidation = true;
}

void ScreenTexture::drawImplementation( osg::RenderInfo & renderInfo ) const
{
    if (!_isNeededValidation)
        return;

    _isNeededValidation = false;

    osg::State & state = *renderInfo.getState();
    const osg::Viewport * viewport = state.getCurrentViewport();

#if defined(_SCREEN_TEXTURE_DEBUG_)
    const unsigned texunit = state.getActiveTextureUnit();
    state.setActiveTextureUnit(0);
#endif // _SCREEN_TEXTURE_DEBUG_

    // instead of doing copyTexImage we would do FBO blit operation to ensure it correctly handles multisampled FBO

    // get fbo extension which provides us with the glGenerateMipmapEXT function
    osg::FBOExtensions * fbo_ext = osg::FBOExtensions::instance(state.getContextID(), true);

    // get current draw binding
    GLint oldDrawFBO = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &oldDrawFBO);

    // recrreate FBO, is sizes were changed
    if (viewport->width() != _texture->getTextureWidth() || viewport->height() != _texture->getTextureHeight())
    {
        //_resolveFbo = new osg::FrameBufferObject();
        _texture->dirtyTextureObject();
        _texture->setTextureSize(viewport->width(), viewport->height());
        if (_blitBitMask == GL_DEPTH_BUFFER_BIT)
        {
            _resolveFbo->setAttachment(osg::Camera::DEPTH_BUFFER, osg::FrameBufferAttachment(_texture));
			
#if 1
			osg::Geometry* quad_geom = _pthis->asGeometry();
			osg::ref_ptr<osg::Vec2Array> quad_tcoords = new osg::Vec2Array; // texture coords
			quad_tcoords->push_back(osg::Vec2(0, 0));
			quad_tcoords->push_back(osg::Vec2(viewport->width(), 0));
			quad_tcoords->push_back(osg::Vec2(viewport->width(), viewport->height()));
			quad_tcoords->push_back(osg::Vec2(0, viewport->height()));
			quad_geom->setTexCoordArray(0, quad_tcoords.get());			

#define NUM_TEXTURES 1 
			const bool useMultiSample = false;
			
			_textureNormals->dirtyTextureObject();
			_textureNormals->setTextureSize(viewport->width(), viewport->height());			
			for (int i=0; i<NUM_TEXTURES; i++) {
				//if (useMultiSample)
				//	_resolveFbo->setAttachment(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0+i), textureRect[i], 0, 0, false, 4, 4);
				//else
					_resolveFbo->setAttachment(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0+i), osg::FrameBufferAttachment(_textureNormals.get())/*textureRect[i]*/);
			}

		    GLenum status = fbo_ext->glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
			if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
				return;
#else
			_resolveFbo->setAttachment(osg::Camera::COLOR_BUFFER, osg::FrameBufferAttachment(new osg::RenderBuffer(viewport->width(), viewport->height(), GL_RGB)));
#endif

		}
        else if (_blitBitMask == GL_COLOR_BUFFER_BIT)
        {
            _resolveFbo->setAttachment(osg::Camera::COLOR_BUFFER, osg::FrameBufferAttachment(_texture));
        }
    }

    // apply resolve FBO for blit operation
    _resolveFbo->apply(state, osg::FrameBufferObject::DRAW_FRAMEBUFFER);

#if defined(_SCREEN_TEXTURE_DEBUG_)
    GLenum status = fbo_ext->glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
        OutputDebugString("\nCannot create FBO for texture blitting\n");
#endif // _SCREEN_TEXTURE_DEBUG_

    // blit to the resolve framebuffer.
    fbo_ext->glBlitFramebuffer(
        0, 0, static_cast<GLint>(viewport->width()), static_cast<GLint>(viewport->height()),
        0, 0, static_cast<GLint>(viewport->width()), static_cast<GLint>(viewport->height()),
        _blitBitMask, GL_NEAREST);

    // restore old draw framebuffer
    fbo_ext->glBindFramebuffer(osg::FrameBufferObject::DRAW_FRAMEBUFFER, oldDrawFBO);

    // get depth settings
    if (_settings.valid())
    {
        const osg::Matrix & projection = state.getProjectionMatrix();
        const float 
            znear = projection(3, 2) / (projection(2, 2) - 1.0),
            zfar = projection(3, 2) / (1.0 + projection(2, 2));
        //double left, right, bottom, top, znear, zfar;
        //state.getProjectionMatrix().getFrustum(left, right, bottom, top, znear, zfar);
        
        const osg::Vec2f settings = osg::Vec2f((zfar - znear) / (zfar * znear), -1.0 / znear);
        _settings->set(settings);
    }

#if defined(_SCREEN_TEXTURE_DEBUG_)

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_TEXTURE_RECTANGLE);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);

        glTexCoord2f(viewport->width(), 0.0f);
        glVertex2f(1.0f, 0.0f);

        glTexCoord2f(viewport->width(), viewport->height());
        glVertex2f(1.0f, 1.0f);

        glTexCoord2f(0.0f, viewport->height());
        glVertex2f(0.0f, 1.0f);
    glEnd();
    glDisable(GL_TEXTURE_RECTANGLE);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();

    state.setActiveTextureUnit(texunit);

    state.checkGLErrors("ScreenTexture");

#endif // _SCREEN_TEXTURE_DEBUG_
}


}
