#pragma once 

namespace avScene
{

class ScreenTexture : public osg::Drawable
{
public:

    enum Type
    {
        DEPTH_TEXTURE = 0,
        COLOR_TEXTURE = 1,
    };


public:

    ScreenTexture( unsigned order, Type type );
    ~ScreenTexture();

    osg::TextureRectangle * getTexture() const;
    osg::Uniform * getSettings() const;

    void validate();


protected:

    // osg::Object
    virtual osg::Object * cloneType() const { return NULL; };
    virtual osg::Object * clone( const osg::CopyOp & copyop ) const { return NULL; };

    // osg::Drawable
    virtual void drawImplementation( osg::RenderInfo & renderInfo ) const;


private:

    mutable bool _isNeededValidation;

    mutable osg::ref_ptr<osg::TextureRectangle> _texture;

    mutable osg::ref_ptr<osg::FrameBufferObject> _resolveFbo;
    unsigned _blitBitMask;

    osg::ref_ptr<osg::Uniform> _settings;
};


}

