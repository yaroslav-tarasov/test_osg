#pragma once


typedef osg::Texture2D               ITexture;
typedef osg::ref_ptr<osg::Texture2D> ITexturePtr;

struct LightMapRenderer : osg::Group
{
    // get matrix
    virtual cg::matrix_4f const & GetViewTextureMatrix() const = 0; 
    // get night mode
    virtual bool                GetNightMode()         const = 0;

    // add spot light
    struct SpotData
    {
        cg::point_3f         view_pos;
        cg::point_3f         view_dir;
        cg::colorf           spot_color;
        cg::range_2f         dist_falloff;
        cg::range_2f         angle_falloff;
    };

    virtual void AddSpotLight( SpotData const & spot ) = 0;
};

typedef osg::ref_ptr<LightMapRenderer> ILightMapRendererPtr;


// create method
ILightMapRendererPtr createLightMapRenderer(osg::Group * sceneRoot);