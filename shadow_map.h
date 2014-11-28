#pragma once



class ShadowMap : public osg::Group // CameraNode
{
public:
    typedef std::function<osg::Light*()> get_light_f;

public:
    explicit ShadowMap(int wh) {init(wh,wh);}

    ShadowMap(int width, int height);

    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    ShadowMap(const ShadowMap& sm,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    void init(int width, int height);

    osg::Texture2D        * getTexture() const { return _texture.get(); }

    void setLightGetter(get_light_f lg)     {_get_light = lg;};
    void setScene      (osg::Node* scene );

protected:
    virtual bool addChild( Node *child ){return osg::Group::addChild(child);};
#if 1
    void traverse(osg::NodeVisitor& nv);
#endif

    void cull( osgUtil::CullVisitor & pNV  );
    
    osg::Vec3 computeOrthogonalVector(const osg::Vec3& direction) const;

    osg::Texture2D        * createTexture(int width, int height);
    osg::TextureRectangle * createDepth  (int width, int height);

private:
    osg::ref_ptr<osg::Texture2D>         _texture;
    get_light_f                          _get_light;
    osg::BoundingBox                     _bb;
    osg::BoundingSphere                  _bs;
    osg::ref_ptr<osg::Camera>            _camera;
    osg::ref_ptr<osg::Uniform>           _shadowMat;
};

namespace creators
{
    osg::ref_ptr<ShadowMap> GetShadowMap();
}