#ifndef SIMOSG_PRERENDER_H
#define SIMOSG_PRERENDER_H 

class Prerender : public osg::CameraNode
{
public:

    Prerender() {}

    Prerender(int width, int height);

    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    Prerender(const Prerender& prerender,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

//  META_Node(osgx, Prerender);

    void init(int width, int height);

    osg::Texture2D        * getTexture() const { return _texture.get(); }

protected:

    void traverse(osg::NodeVisitor& nv);

    osg::Texture2D        * createTexture(int width, int height);
    osg::TextureRectangle * createDepth  (int width, int height);

    osg::ref_ptr<osg::Texture2D>         _texture;
};


#endif