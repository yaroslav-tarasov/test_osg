#pragma once

class Prerender : public osg::CameraNode
{
public:

    Prerender();

    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    Prerender(const Prerender& prerender,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

//  META_Node(osgx, Prerender);

    void init();

    osg::Texture2D        * getTexture() const { return _texture.get(); }

protected:

    void traverse(osg::NodeVisitor& nv);

    osg::ref_ptr<osg::Texture2D>         _texture;
};

