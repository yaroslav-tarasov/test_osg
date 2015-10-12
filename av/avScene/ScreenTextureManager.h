#pragma once 

#include "ScreenTexture.h"

namespace avScene
{

class ScreenTextureManager : public osg::Node
{
public:

    ScreenTextureManager();
    ~ScreenTextureManager();

    // user must call this function on update traversal, and, if needed depth texture settings uniform, 
    // add this uniform to own state set
    ScreenTexture * request( unsigned order, ScreenTexture::Type type );


public:

    //META_Node(osgScene, ScreenTextureManager);
    virtual osg::Object * cloneType() const { return NULL; }
    virtual osg::Object * clone( const osg::CopyOp & copyop ) const { return NULL; }

    virtual bool isSameKindAs( const osg::Object * obj ) const { return dynamic_cast<const ScreenTextureManager *>(obj) != NULL; }
    virtual const char * className() const { return "ScreenTextureManager"; }
    virtual const char * libraryName() const { return "osgScene"; }
    virtual void accept( osg::NodeVisitor & nv );


private:

    typedef std::pair<unsigned, ScreenTexture::Type> ScreenTexturePair;
    typedef std::map<ScreenTexturePair, osg::ref_ptr<ScreenTexture> > ScreenTextureMap;
    ScreenTextureMap _depthTextureMap;
};

}
