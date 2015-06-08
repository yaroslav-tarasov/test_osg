#pragma once

namespace avScene
{

class LightManager 
    : public osg::Node
{
private:

    LightManager();
    ~LightManager();


public:

    static void Create();
    static void Release();

    static LightManager * GetInstance();

    struct Light
    {
        osg::MatrixTransform * transform;

        cg::range_2f spotFalloff;
        cg::range_2f distanceFalloff;
        cg::colorf color;

        cg::point_3f position;
        cg::vector_3f direction;

        bool active;

        inline Light() : transform(NULL), active(false) { }
    };

public:

    void update  ( osg::NodeVisitor * nv );
    void addLight(uint32_t id, osg::MatrixTransform* mt);
    void addLight(uint32_t id, const Light& light);

private:

    static osg::ref_ptr<LightManager> g_Instance;



    typedef std::map<unsigned,Light> LightsMap;
    LightsMap m_LightsMap;
};


} // namespace svScene
