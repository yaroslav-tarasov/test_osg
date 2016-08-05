#include "stdafx.h"

#include <osg/buffered_value>
#include <osg/ref_ptr>
#include <osg/GLExtensions>
#include <GL/glut.h>
#include <osg/Version>


namespace  {
    
    class SPTGLExtensions;


    class SPTGLExtensions : public osg::Referenced
    {
    public:
        SPTGLExtensions( unsigned int contextID );
        SPTGLExtensions( const SPTGLExtensions &rhs );
        void lowestCommonDenominator( const SPTGLExtensions &rhs );
        void setupGLExtensions( unsigned int contextID );

        void glTexturePageCommitmentEXT(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit) const;

        static SPTGLExtensions* getExtensions( unsigned int contextID,bool createIfNotInitalized );
    protected:

        typedef void ( GL_APIENTRY *TexturePageCommitmentEXT ) (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);

        TexturePageCommitmentEXT _glTexturePageCommitmentEXT;

    };

    SPTGLExtensions::SPTGLExtensions( unsigned int contextID )
    {
        setupGLExtensions( contextID );
    }

    SPTGLExtensions::SPTGLExtensions( const SPTGLExtensions &rhs )
        : Referenced()
    {
        _glTexturePageCommitmentEXT           = rhs._glTexturePageCommitmentEXT;
    }


    void SPTGLExtensions::lowestCommonDenominator( const SPTGLExtensions &rhs )
    {
        if ( !rhs._glTexturePageCommitmentEXT )
        {
            _glTexturePageCommitmentEXT = rhs._glTexturePageCommitmentEXT;
        }
    }

    void SPTGLExtensions::setupGLExtensions( unsigned int contextID )
    {
        _glTexturePageCommitmentEXT = 0;
        osg::setGLExtensionFuncPtr( _glTexturePageCommitmentEXT, "glTexturePageCommitmentEXT" );
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
            
            SPTGLExtensions *ext = SPTGLExtensions::getExtensions( renderInfo.getContextID(),true );
            glFrontFace( GL_CW );
            glutSolidTeapot( _size );
            glFrontFace( GL_CCW );
        }

    protected:
        float _size;
    };

}


int main_sparse_texture( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    osg::setNotifyLevel( osg::INFO ); 

	osg::ref_ptr<osg::Group> root= new osg::Group;
	osg::ref_ptr<osg::Geode> teapot = new osg::Geode;
	teapot->addDrawable( new TeapotDrawable(1.0f) );
	root->addChild(teapot);
    
#if 0
	glm::ivec3 PageSize;
	glGetInternalformativ(GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &PageSize.x);
	glGetInternalformativ(GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &PageSize.y);
	glGetInternalformativ(GL_TEXTURE_2D_ARRAY, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_Z_ARB, 1, &PageSize.z);   
#endif

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