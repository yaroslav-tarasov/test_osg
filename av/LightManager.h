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
    struct Light
    {
        osg::Transform * transform;

        cg::range_2f     spotFalloff;
        cg::range_2f     distanceFalloff;
        cg::colorf       color;

        cg::point_3f     position;
        cg::vector_3f    direction;

        bool             active;

        inline Light() : transform(nullptr), active(false) { }
    };

public:
    static void Create();
    static void Release();

    static LightManager * GetInstance();



public:

    void      update  ( osg::NodeVisitor * nv );
    uint32_t  addLight( osg::MatrixTransform* mt);
    uint32_t  addLight( const Light& light);


    FIXME(Check before return);
    
    Light&   getLight(uint32_t id) {return m_LightsMap[id];};

private:
	uint32_t genUID();

private:

    static osg::ref_ptr<LightManager> g_Instance;

    typedef std::map<unsigned,Light>  LightsMap;
    LightsMap                         m_LightsMap;

};


} // namespace avScene
